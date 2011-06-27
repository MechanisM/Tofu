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
#include "bstring/bstraux.h"

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

static bstring process_route(bstring route, list_node_t *params);
static bool compare_url(bstring uri, bstring regex, list_node_t *params);

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
		list_foreach(iter, ctx -> rescuers) {
			tofu_rescuer_t *rescue = iter -> value;

			if (req -> error == rescue -> error)
				rep = rescue -> callback(req);
		}

		rep -> status = req -> error;
	} else {
		list_foreach(iter, ctx -> handlers) {
			request_uri = cstr2bstr(req -> uri);
			tofu_handler_t *handle = iter -> value;
			bstring regex = process_route(handle -> route, req -> params);

			if ((req -> method == handle -> method) &&
				compare_url(request_uri, regex, req -> params)) {

				rep = handle -> callback(req);
				rep -> status = 200;
			}

			bstrFree(request_uri);
			bstrFree(regex);
		}

		if (rep == NULL) {
			int error = 404;
			list_foreach(iter, ctx -> rescuers) {
				tofu_rescuer_t *rescue = iter -> value;

				if (error == rescue -> error) {
					rep = rescue -> callback(req);
					rep -> status = error;
				}
			}
		}
	}

	rep -> connid = req -> connid;

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
			(const char *) string -> data, blength(string),
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
			(const char *) string -> data,
			ovector, rc, 1, (const char **) &c
		);

		breplace(string, ovector[2] - 1, strlen(c) + 1, replace, ' ');

		param = malloc(sizeof(tofu_pair_t));

		param -> name = strdup(c);
		list_insert_tail(params, param);

		pcre_free_substring(c);
	}

	binsertch(string, 0, 1, '^');

	bstrFree(replace);
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
		(const char *) regex -> data, 0,
		&error, &error_offset, NULL
	);

	rc = pcre_exec(
		re, NULL, (const char *) uri -> data,
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
			(const char *) uri -> data,
			ovector, rc, i, &c
		);

		param -> value = strdup(c);
	}

	pcre_free_substring(c);
	pcre_free(re);

	return true;
}

