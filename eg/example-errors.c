#include <tofu.h>

tofu_rep_t *handler_404(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>404</title></head>\n");
	tofu_write(rep, "<body>Not found</body>\n");

	return rep;
}

tofu_rep_t *handler_500(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>500</title></head>\n");
	tofu_write(rep, "<body>Error!!!</body>\n");

	return rep;
}

int main() {
	char *opts[] = { "0.0.0.0", "2000" };
	tofu_ctx_t *ctx = tofu_ctx_init(TOFU_BACKEND_EVHTTP, opts);

	tofu_rescue_with(ctx, 404, handler_404);
	tofu_rescue_with(ctx, 500, handler_500);

	tofu_loop(ctx);
	tofu_ctx_free(ctx);

	return 0;
}
