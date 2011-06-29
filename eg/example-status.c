#include <tofu.h>

tofu_rep_t *handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_status(rep, 418);
	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>I'm a teapot</title></head><body><h1>418 - Sorry, I'm just a teapot!</h1></body>\n");

	return rep;
}

int main() {
	char *opts[] = { "0.0.0.0", "2000" };
	tofu_ctx_t *ctx = tofu_ctx_init(TOFU_BACKEND_EVHTTP, opts);

	tofu_handle_with(ctx, GET, "/coffee", handler);

	tofu_loop(ctx);
	tofu_ctx_free(ctx);

	return 0;
}
