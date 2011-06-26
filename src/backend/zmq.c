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
#include "../bstring/bstraux.h"

#include "../utils/list.h"
#include "../utils/common.h"

void tofu_backend_zmq_loop(tofu_ctx_t *ctx);
void tofu_backend_zmq_send(tofu_rep_t *rep);

tofu_backend_t tofu_backend_zmq = {
	.id   = BACKEND_ZMQ,
	.name = "zmq",

	.loop = tofu_backend_zmq_loop,
	.send = tofu_backend_zmq_send,
};

static tofu_req_t *mongrel2tofu(char *tnetstr);
static void zmq_bstr_free(void *data, void *bstr);

static char *recv_spec  = "tcp://127.0.0.1:9999";
static char *recv_ident = "54c6755b-9628-40a4-9a2d-cc82a816345e";

static char *send_spec  = "tcp://127.0.0.1:9998";
static char *send_ident = "b0541e27-9e77-48c1-80ef-24819ae3a97b";

void tofu_backend_zmq_loop(tofu_ctx_t *ctx) {
	zmq_pollitem_t items[1];
	void *zmq_ctx = zmq_init(1);

	void *recv = zmq_socket(zmq_ctx, ZMQ_PULL);

	char *recv_spec  = "tcp://127.0.0.1:9999";
	char *recv_ident = "54c6755b-9628-40a4-9a2d-cc82a816345e";

	zmq_connect(recv, recv_spec);
	zmq_setsockopt(recv, ZMQ_IDENTITY, recv_ident, strlen(recv_ident));

	items[0].socket = recv;
	items[0].events = ZMQ_POLLIN;

	while (1) {
		int rc = zmq_poll(items, 1, 100000);

		if (items[0].revents == ZMQ_POLLIN) {
			zmq_msg_t msg;
			tofu_req_t *req;
			tofu_rep_t *rep;
			char *tnetstr;

			zmq_msg_init(&msg);
			zmq_recv(recv, &msg, ZMQ_RCVMORE);
			tnetstr = zmq_msg_data(&msg);
			zmq_msg_close(&msg);

			req = mongrel2tofu(tnetstr);
			rep = tofu_dispatch(ctx, req);
			tofu_backend_zmq_send(rep);

			tofu_rep_free(rep);
			tofu_req_free(req);

			/*tofu_req_t *req = tofu_req_init(
				getenv("REQUEST_METHOD"),
				getenv("REQUEST_URI")
			);

			tofu_rep_t *rep = tofu_dispatch(ctx, req);
			tofu_backend_zmq_send(rep);

			tofu_rep_free(rep);
			tofu_req_free(req);
			free(tnetstr);*/
		}
	}
}

void tofu_backend_zmq_send(tofu_rep_t *rep) {
	bstring resp    = cstr2bstr("");
	bstring body    = cstr2bstr("");
	bstring headers = cstr2bstr("");
	list_node_t *iter;

	zmq_msg_t msg;
	void *zmq_ctx = zmq_init(1);
	void *send = zmq_socket(zmq_ctx, ZMQ_PUB);

	char *send_spec  = "tcp://127.0.0.1:9998";
	char *send_ident = "b0541e27-9e77-48c1-80ef-24819ae3a97b";

	zmq_connect(send, send_spec);
	zmq_setsockopt(send, ZMQ_IDENTITY, send_ident, strlen(send_ident));

	if (rep == NULL) {
		printf("Not found!!\n");
		return;
	}

	list_foreach(iter, rep -> chunks) {
		char *chunk = iter -> value;
		bstring bchunk = cstr2bstr(chunk);

		bconcat(body, bchunk);
		bstrFree(bchunk);
	}

	list_foreach(iter, rep -> headers) {
		tofu_pair_t *header = iter -> value;
		bstring head = bformat("%s: %s\n", header -> name, header -> value);
		bconcat(headers, head);
		bstrFree(head);
	}

	resp = bformat("%s 2:%d, HTTP/1.1 200 OK\r\n%s%s: %d\n\n%s", recv_ident, rep -> connid, headers -> data, "Content-Length", blength(body), body -> data);

	zmq_msg_init_data(&msg, bdata(resp), blength(resp), zmq_bstr_free, resp);
	zmq_send(send, &msg, 0);
	zmq_msg_close(&msg);
}

static tofu_req_t *mongrel2tofu(char *tnetstr) {
	int etok, stok = 0, connid, headers_len, body_len;
	bstring str;
	bstring uuid, id, path;
	bstring headerl, headers;
	bstring bodyl, body;

	tofu_req_t *req;
	char *method, *uri;

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
	free(headers);

	obj = json_object_get(root, "METHOD");
	method = json_string_value(obj);

	obj = json_object_get(root, "URI");
	uri = json_string_value(obj);

	assert(method != NULL);

	req = tofu_req_init(connid, method, uri);

	bstrFree(str);
	bstrFree(uuid);
	bstrFree(id);
	bstrFree(path);
	bstrFree(headerl);
	bstrFree(headers);
	bstrFree(bodyl);
	bstrFree(body);

	return req;
}

static void zmq_bstr_free(void *data, void *bstrv) {
	bstring bstr = bstrv;
	bstrFree(bstr);
}
