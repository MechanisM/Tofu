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

#include <math.h>

#include <zmq.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

#include <assert.h>

#include "ctx.h"
#include "rep.h"
#include "req.h"
#include "handler.h"

#include "../backend.h"

#include "../bstring/bstrlib.h"

#include "../utils/list.h"
#include "../utils/http.h"
#include "../utils/common.h"

void tofu_backend_zmq_loop(tofu_ctx_t *ctx);

tofu_backend_t tofu_backend_zmq = {
	.id   = BACKEND_ZMQ,
	.name = "zmq",

	.loop = tofu_backend_zmq_loop,
};

static tofu_req_t *mongrel2tofu(char *tnetstr);
static void tofu_backend_zmq_send(tofu_ctx_t *ctx, tofu_rep_t *rep, void *send);

void tofu_backend_zmq_loop(tofu_ctx_t *ctx) {
	zmq_pollitem_t items[1];
	void *zmq_ctx = zmq_init(1);

	char *recv_spec  = ctx -> backend_opts[0];
	char *recv_ident = ctx -> backend_opts[1];

	char *send_spec  = ctx -> backend_opts[2];
	char *send_ident = ctx -> backend_opts[3];

	void *recv = zmq_socket(zmq_ctx, ZMQ_PULL);
	void *send = zmq_socket(zmq_ctx, ZMQ_PUB);

	zmq_connect(recv, recv_spec);
	zmq_setsockopt(recv, ZMQ_IDENTITY, recv_ident, strlen(recv_ident));

	zmq_connect(send, send_spec);
	zmq_setsockopt(send, ZMQ_IDENTITY, send_ident, strlen(send_ident));

	items[0].socket = recv;
	items[0].events = ZMQ_POLLIN;

	while (zmq_poll(items, 1, 100000) >= 0) {
		if (items[0].revents == ZMQ_POLLIN) {
			zmq_msg_t msg;
			tofu_req_t *req;
			tofu_rep_t *rep;
			char *tnetstr;

			zmq_msg_init(&msg);
			zmq_recv(recv, &msg, ZMQ_RCVMORE);
			tnetstr = zmq_msg_data(&msg);

			req = mongrel2tofu(tnetstr);
			rep = tofu_dispatch(ctx, req);
			tofu_backend_zmq_send(ctx, rep, send);

			tofu_rep_free(rep);
			tofu_req_free(req);

			/*free(tnetstr);*/
			zmq_msg_close(&msg);
		}
	}
}

static void tofu_backend_zmq_send(tofu_ctx_t *ctx, tofu_rep_t *rep, void *send) {
	zmq_msg_t msg;
	list_node_t *iter;
	int connid_len = 1;
	bstring resp;
	bstring body    = cstr2bstr("");
	bstring headers = cstr2bstr("");

	char *recv_ident = ctx -> backend_opts[1];

	list_foreach(iter, rep -> chunks) {
		char *chunk = iter -> value;
		bstring bchunk = cstr2bstr(chunk);

		bconcat(body, bchunk);
		bdestroy(bchunk);
	}

	list_foreach(iter, rep -> headers) {
		pair_t *header = iter -> value;
		bstring head = bformat("%s: %s\n", header -> name, header -> value);
		bconcat(headers, head);
		bdestroy(head);
	}

	if (rep -> connid != 0)
		connid_len = log10(rep -> connid) + 1;

	resp = bformat(
		"%s %d:%d, HTTP/1.1 %d %d\r\n%s%s: %d\n\n%s",
		recv_ident, (int) connid_len, rep -> connid, rep -> status, httpmsg(rep -> status),
		bdata(headers), "Content-Length", blength(body), bdata(body)
	);

	zmq_msg_init_data(&msg, bdata(resp), blength(resp), NULL, NULL);
	zmq_send(send, &msg, 0);
	zmq_msg_close(&msg);

	bdestroy(resp);
	bdestroy(body);
	bdestroy(headers);
}

static tofu_req_t *mongrel2tofu(char *tnetstr) {
	int etok, stok = 0, connid, headers_len, body_len;
	bstring str;
	bstring uuid, id, path;
	bstring headerl, headers;
	bstring bodyl, body;

	tofu_req_t *req;
	const char *method, *uri;

	struct tagbstring space = bsStatic(" ");
	struct tagbstring colon = bsStatic(":");
	struct tagbstring comma = bsStatic(",");

	json_t *obj;
	json_t *root;
	json_error_t error;

	str = cstr2bstr(tnetstr);

	/* exract UUID */
	etok = binchr(str, stok, &space);
	uuid = bmidstr(str, stok, etok - stok);

	/* exract Connection ID */
	stok = etok + 1;
	etok = binchr(str, stok, &space);
	id   = bmidstr(str, stok, etok - stok);
	sscanf(bdata(id), "%d", &connid);

	/* exract PATH */
	stok = etok + 1;
	etok = binchr(str, stok, &space);
	path = bmidstr(str, stok, etok - stok);

	/* exract headers */
	stok = etok + 1;
	etok = binchr(str, stok, &colon);
	headerl = bmidstr(str, stok, etok - stok);
	sscanf(bdata(headerl), "%d", &headers_len);

	stok = etok + 1;
	headers = bmidstr(str, stok, headers_len);

	/* exract body */
	stok += headers_len + 1;
	etok = binchr(str, stok, &colon);
	bodyl = bmidstr(str, stok, etok - stok);
	sscanf(bdata(bodyl), "%d", &body_len);

	stok = etok + 1;
	body = bmidstr(str, stok, body_len);

	root = json_loads(bdata(headers), 0, &error);

	obj = json_object_get(root, "METHOD");
	method = json_string_value(obj);

	obj = json_object_get(root, "URI");
	uri = json_string_value(obj);

	req = tofu_req_init(connid, method, uri);

	json_decref(root);
	json_decref(obj);

	bdestroy(str);
	bdestroy(uuid);
	bdestroy(id);
	bdestroy(path);
	bdestroy(headerl);
	bdestroy(headers);
	bdestroy(bodyl);
	bdestroy(body);

	return req;
}
