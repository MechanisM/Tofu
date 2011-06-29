/*
 * (Yet another) C web framework
 *
 * Copyright (c) 2011, Alessandro Ghedini
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of the Mongrel2 Project, Zed A. Shaw, nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <pcre.h>

#include "ctx.h"
#include "req.h"
#include "rep.h"
#include "handler.h"

#include "bstring/bstrlib.h"

#include "utils/list.h"
#include "utils/common.h"

typedef struct {
	int method;
	bstring route;
	tofu_rep_t *(*callback)(tofu_req_t *req);
} tofu_handler_t;

typedef struct {
	int error;
	tofu_rep_t *(*callback)(tofu_req_t *req);
} tofu_rescuer_t;

#define OVECCOUNT 30

static tofu_rep_t *rescu_from_error(tofu_ctx_t *ctx, tofu_req_t *req, int error);
static bstring process_route(bstring route, list_node_t *params);
static bool compare_url(bstring uri, bstring regex, list_node_t *params);

static tofu_rep_t *default_error_handler(tofu_req_t *req, int error);

void tofu_handle_with(tofu_ctx_t *ctx, int method, const char *route, tofu_rep_t *(*callback)(tofu_req_t *req)) {
	tofu_handler_t *handle = malloc(sizeof(tofu_handler_t));

	handle -> method   = method;
	handle -> route    = cstr2bstr(route);
	handle -> callback = callback;

	list_insert_tail(ctx -> handlers, (void *) handle);
}

void tofu_rescue_with(tofu_ctx_t *ctx, int error, tofu_rep_t *(*callback)(tofu_req_t *req)) {
	tofu_rescuer_t *rescue = malloc(sizeof(tofu_rescuer_t));

	rescue -> error    = error;
	rescue -> callback = callback;

	list_insert_tail(ctx -> rescuers, (void *) rescue);
}

tofu_rep_t *tofu_dispatch(tofu_ctx_t *ctx, tofu_req_t *req) {
	list_node_t *iter;
	bstring request_uri;
	tofu_rep_t *rep = NULL;

	if (req -> error != 0) {
		rep = rescu_from_error(ctx, req, req -> error);
		goto ret;
	}

	list_foreach(iter, ctx -> handlers) {
		request_uri = cstr2bstr(req -> uri);
		tofu_handler_t *handle = iter -> value;
		bstring regex = process_route(handle -> route, req -> params);

		if ((req -> method == handle -> method) &&
			compare_url(request_uri, regex, req -> params)) {

			rep = handle -> callback(req);
			if (rep -> status == 0)
				rep -> status = 200;
		}

		bdestroy(request_uri);
		bdestroy(regex);
	}

	if (rep == NULL)
		rep = rescu_from_error(ctx, req, 404);

ret:
	rep -> connid = req -> connid;
	return rep;
}

static tofu_rep_t *rescu_from_error(tofu_ctx_t *ctx, tofu_req_t *req, int error) {
	list_node_t *iter;
	tofu_rep_t *rep = NULL;

	list_foreach(iter, ctx -> rescuers) {
		tofu_rescuer_t *rescue = iter -> value;

		if (error == rescue -> error)
			rep = rescue -> callback(req);
	}

	if (rep == NULL)
		rep = default_error_handler(req, error);

	if (rep -> status == 0)
		rep -> status = error;

	return rep;
}

static tofu_rep_t *default_error_handler(tofu_req_t *req, int error) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");

	switch (error) {
		case 404:
			tofu_write(rep, "<!DOCTYPE html><head><title>Not Found!</title></head><body><h1>404 - Resource not found!</h1></body></html>\n");
			break;
		case 500:
			tofu_write(rep, "<!DOCTYPE html><head><title>Error!</title></head><body><h1>500 - Internal server error!</h1></body></html>\n");
			break;
	}

	return rep;
}

static bstring process_route(bstring route, list_node_t *params) {
	pcre *re;
	const char *error;
	int error_offset, rc;

	tofu_pair_t *param;
	int ovector[OVECCOUNT];
	bstring string = bstrcpy(route);
	bstring replace = cstr2bstr("(\\w*)");
	memset(ovector, 0, OVECCOUNT * sizeof(int));

	re = pcre_compile("/:(\\w*)", 0, &error, &error_offset, NULL);

	for (;;) {
		char *c;
		int options = 0;
		int start_offset = ovector[1];

		rc = pcre_exec(
			re, NULL,
			(const char *) bdata(string), blength(string),
			start_offset, options,
			ovector, OVECCOUNT
		);

		if (rc == PCRE_ERROR_NOMATCH) {
			if (options == 0)
				break;

			ovector[1] = start_offset + 1;
			continue;
		}

		pcre_get_substring(
			(const char *) bdata(string),
			ovector, rc, 1, (const char **) &c
		);

		breplace(string, ovector[2] - 1, strlen(c) + 1, replace, ' ');

		param = malloc(sizeof(tofu_pair_t));

		param -> name = strdup(c);
		list_insert_tail(params, param);

		pcre_free_substring(c);
	}

	binsertch(string, 0, 1, '^');
	binsertch(string, blength(string)+1, 1, '$');

	bdestroy(replace);
	pcre_free(re);

	return string;
}

static bool compare_url(bstring uri, bstring regex, list_node_t *params) {
	pcre *re;
	list_node_t *iter;
	const char *error, *c;
	int error_offset, rc, i = 0;

	tofu_pair_t *param;
	int ovector[OVECCOUNT];
	memset(ovector, 0, OVECCOUNT * sizeof(int));

	re = pcre_compile(
		(const char *) bdata(regex), 0,
		&error, &error_offset, NULL
	);

	rc = pcre_exec(
		re, NULL, (const char *) bdata(uri),
		blength(uri), 0, 0, ovector, OVECCOUNT
	);

	if (rc == PCRE_ERROR_NOMATCH) {
		pcre_free(re);
		return false;
	}

	list_foreach(iter, params) {
		i++;
		param = iter -> value;

		pcre_get_substring(
			(const char *) bdata(uri),
			ovector, rc, i, &c
		);

		param -> value = strdup(c);
	}

	pcre_free_substring(c);
	pcre_free(re);

	return true;
}
