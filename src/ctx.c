#include <stdlib.h>

#include "ctx.h"
#include "rep.h"
#include "req.h"

#include "bstring/bstrlib.h"
#include "bstring/bstraux.h"

#include "utils/list.h"

typedef struct {
	int method;
	bstring route;
	tofu_rep_t *(*callback)(tofu_req_t *req);
} tofu_handler_t;

tofu_ctx_t *tofu_ctx_init(int backend) {
	tofu_ctx_t *ctx = malloc(sizeof(tofu_ctx_t));

	ctx -> handlers = list_init();
	ctx -> backend  = backend;

	return ctx;
}

void tofu_ctx_free(tofu_ctx_t *ctx) {
	list_node_t *iter;

	if (ctx == NULL)
		return;

	list_reverse_foreach(iter, ctx -> handlers) {
		iter -> prev -> next = iter -> next;
		iter -> next -> prev = iter -> prev;

		if (!list_is_empty(iter)) {
			tofu_handler_t *handle = iter -> value;
			bstring route = handle -> route;

			bstrFree(route);

			free(handle);
			free(iter);
		}
	}

	free(ctx -> handlers);
	free(ctx);
}
