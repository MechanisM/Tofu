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
 *     * Neither the name of the Tofu project, Alessandro Ghedini, nor the
 *       names of its contributors may be used to endorse or promote products
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

#include <stdlib.h>
#include <string.h>

#include "req.h"

#include "bstring/bstrlib.h"

#include "utils/list.h"
#include "utils/common.h"

tofu_req_t *tofu_req_init(int connid, char *method, char *uri, char *body, int body_len) {
	tofu_req_t *req = malloc(sizeof(tofu_req_t));

	req -> error  = 0;
	req -> connid = connid;
	req -> params = list_init();

	if ((method == NULL) || uri == NULL) {
		req -> error = 400;
		return req;
	}

	req -> uri    = strdup(uri);

	if (strcmp(method, "GET") == 0)
		req -> method = GET;
	else if (strcmp(method, "POST") == 0)
		req -> method = POST;
	else if (strcmp(method, "PUT") == 0)
		req -> method = PUT;
	else if (strcmp(method, "DELETE") == 0)
		req -> method = DELETE;
	else
		req -> error = 400;

	req -> body     = strdup(body);
	req -> body_len = body_len;

	return req;
}

void tofu_req_free(tofu_req_t *req) {
	list_node_t *iter, *safe;

	if (req == NULL)
		return;

	list_reverse_foreach_safe(iter, safe, req -> params) {
		iter -> prev -> next = iter -> next;
		iter -> next -> prev = iter -> prev;

		if (!list_is_empty(iter)) {
			pair_t *param = iter -> value;
			bstring name = param -> name;
			bstring value = param -> value;

			free(name);
			free(param);
			free(iter);
		}
	}

	free(req -> params);
	free(req -> uri);
	free(req);
}

char *tofu_param(tofu_req_t *req, const char *name) {
	list_node_t *iter;

	list_foreach(iter, req -> params) {
		pair_t *pair  = iter -> value;
		char *name_curr  = pair -> name;
		char *value_curr = pair -> value;

		if (strcmp(name, name_curr) == 0)
			return value_curr;
	}

	return "";
}

char *tofu_body(tofu_req_t *req, int *size) {
	*size = req -> body_len;
	return req -> body;
}
