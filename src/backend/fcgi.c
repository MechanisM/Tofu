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
#include <fcgi_stdio.h>

#include "ctx.h"
#include "rep.h"
#include "req.h"
#include "handler.h"

#include "../backend.h"

#include "../utils/list.h"
#include "../utils/http.h"
#include "../utils/common.h"

void tofu_backend_fcgi_loop(tofu_ctx_t *ctx);
static void tofu_backend_fcgi_send(tofu_rep_t *rep);

tofu_backend_t tofu_backend_fcgi = {
	.id   = BACKEND_FCGI,
	.name = "fcgi",

	.loop = tofu_backend_fcgi_loop,
};

void tofu_backend_fcgi_loop(tofu_ctx_t *ctx) {
	while (FCGI_Accept() >= 0) {
		char *body;
		int body_len;
		tofu_req_t *req;

		body_len = getenv("CONTENT_LENGTH");
		body = calloc(body_len + 1, 1);
		fread(body, 1, body_len, stdin);

		req = tofu_req_init(
			0,
			getenv("REQUEST_METHOD"),
			getenv("REQUEST_URI"),
			body, body_len
		);

		tofu_rep_t *rep = tofu_dispatch(ctx, req);

		tofu_backend_fcgi_send(rep);

		tofu_rep_free(rep);
		tofu_req_free(req);

		OS_LibShutdown();
		/*FCGI_Finish();*/
	}
}

void tofu_backend_fcgi_send(tofu_rep_t *rep) {
	list_node_t *iter;

	printf("Status: %d %s\n", rep -> status, httpmsg(rep -> status));

	list_foreach(iter, rep -> headers) {
		pair_t *header = iter -> value;
		printf("%s: %s\n", header -> name, header -> value);
	}

	printf("\n");

	list_foreach(iter, rep -> chunks) {
		const char *chunk = iter -> value;
		printf(chunk);
	}
}
