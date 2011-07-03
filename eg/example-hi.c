#include <tofu.h>

tofu_rep_t *hello(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "Hello World!");

	return rep;
}

void main() {
	char *opts[] = { "0.0.0.0", "8080" };
	tofu_ctx_t *ctx = tofu_ctx_init(TOFU_EVHTTP, opts);

	tofu_handle_with(ctx, GET, "/hi", hello);
	tofu_loop(ctx);
}
