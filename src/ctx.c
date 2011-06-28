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

#include <stdlib.h>

#include "ctx.h"
#include "rep.h"
#include "req.h"

#include "bstring/bstrlib.h"

#include "utils/list.h"

typedef struct {
	int method;
	bstring route;
	tofu_rep_t *(*callback)(tofu_req_t *req);
} tofu_handler_t;

typedef struct {
	int error;
	tofu_rep_t *(*callback)(tofu_req_t *req);
} tofu_rescuer_t;

tofu_ctx_t *tofu_ctx_init(int backend, char *opts[]) {
	tofu_ctx_t *ctx = malloc(sizeof(tofu_ctx_t));

	ctx -> handlers     = list_init();
	ctx -> rescuers     = list_init();
	ctx -> backend      = backend;
	ctx -> backend_opts = opts;

	return ctx;
}

void tofu_ctx_free(tofu_ctx_t *ctx) {
	list_node_t *iter, *safe;

	if (ctx == NULL)
		return;

	list_reverse_foreach_safe(iter, safe, ctx -> handlers) {
		iter -> prev -> next = iter -> next;
		iter -> next -> prev = iter -> prev;

		if (!list_is_empty(iter)) {
			tofu_handler_t *handle = iter -> value;
			bstring route = handle -> route;

			bdestroy(route);

			free(handle);
			free(iter);
		}
	}

	free(ctx -> handlers);

	list_reverse_foreach_safe(iter, safe, ctx -> rescuers) {
		iter -> prev -> next = iter -> next;
		iter -> next -> prev = iter -> prev;

		if (!list_is_empty(iter)) {
			tofu_rescuer_t *rescue = iter -> value;

			free(rescue);
			free(iter);
		}
	}

	free(ctx -> rescuers);

	free(ctx);
}
