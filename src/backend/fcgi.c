#include <stdlib.h>
#include <fcgi_stdio.h>

#include "ctx.h"
#include "rep.h"
#include "req.h"
#include "handler.h"

#include "../backend.h"

#include "../utils/list.h"
#include "../utils/common.h"


void tofu_backend_fcgi_loop(tofu_ctx_t *ctx);
void tofu_backend_fcgi_send(tofu_rep_t *rep);

tofu_backend_t tofu_backend_fcgi = {
	.id   = BACKEND_FCGI,
	.name = "fcgi",

	.loop = tofu_backend_fcgi_loop,
	.send = tofu_backend_fcgi_send,
};

void tofu_backend_fcgi_loop(tofu_ctx_t *ctx) {
	while (FCGI_Accept() >= 0) {
		tofu_req_t *req = tofu_req_init(
			getenv("REQUEST_METHOD"),
			getenv("REQUEST_URI")
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

	if (rep == NULL)
		return;

	list_foreach(iter, rep -> headers) {
		tofu_pair_t *header = iter -> value;
		printf("%s: %s\n", header -> name, header -> value);
	}

	printf("\n");

	list_foreach(iter, rep -> chunks) {
		const char *chunk = iter -> value;
		printf(chunk);
	}
}
