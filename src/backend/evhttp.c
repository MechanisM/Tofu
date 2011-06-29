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
#include <string.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>

#include "ctx.h"
#include "rep.h"
#include "req.h"
#include "handler.h"

#include "../backend.h"

#include "../utils/list.h"
#include "../utils/http.h"
#include "../utils/common.h"

void tofu_backend_evhttp_loop(tofu_ctx_t *ctx);
static void tofu_backend_evhttp_cb(struct evhttp_request *req, void *arg);

tofu_backend_t tofu_backend_evhttp = {
	.id   = BACKEND_EVHTTP,
	.name = "evhttp",

	.loop = tofu_backend_evhttp_loop,
};

void tofu_backend_evhttp_loop(tofu_ctx_t *ctx) {
	char *listen = ctx -> backend_opts[0];
	int port = atoi(ctx -> backend_opts[1]);
	struct evhttp *http;
	struct event_base *base;
	struct evhttp_bound_socket *handle;

	base = event_base_new();

	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		exit(-1);
	}

	http = evhttp_new(base);

	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		exit(-1);
	}

	evhttp_set_gencb(http, tofu_backend_evhttp_cb, ctx);

	handle = evhttp_bind_socket_with_handle(http, listen, port);

	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n", port);
		exit(-1);
	}

	event_base_dispatch(base);
}

static void tofu_backend_evhttp_cb(struct evhttp_request *evreq, void *arg) {
	tofu_req_t *req;
	tofu_rep_t *rep;
	list_node_t *iter;
	tofu_ctx_t *ctx = (tofu_ctx_t *) arg;

	struct evbuffer *evb = NULL;
	struct evhttp_uri *decoded = NULL;
	char *uri = evhttp_request_get_uri(evreq);
	char *method = NULL, *path, *decoded_path;

	switch (evhttp_request_get_command(evreq)) {
		case EVHTTP_REQ_GET:
			method = "GET";
			break;
		case EVHTTP_REQ_POST:
			method = "POST";
			break;
		case EVHTTP_REQ_PUT:
			method = "PUT";
			break;
		case EVHTTP_REQ_DELETE:
			method = "DELETE";
			break;
		default: method = NULL;
	}

	decoded = evhttp_uri_parse(uri);

	path = evhttp_uri_get_path(decoded);
	decoded_path = evhttp_uridecode(path, 0, NULL);

	req = tofu_req_init(0, method, decoded_path);
	rep = tofu_dispatch(ctx, req);

	evb = evbuffer_new();

	list_foreach(iter, rep -> headers) {
		pair_t *header = iter -> value;
		evhttp_add_header(
			evhttp_request_get_output_headers(evreq),
			header -> name, header -> value
		);
	}

	list_foreach(iter, rep -> chunks) {
		const char *chunk = iter -> value;
		evbuffer_add(evb, chunk, strlen(chunk));
	}

	evhttp_send_reply(evreq, rep -> status, httpmsg(rep -> status), evb);

	evhttp_uri_free(decoded);
	evbuffer_free(evb);
	free(decoded_path);

	tofu_rep_free(rep);
	tofu_req_free(req);
}
