#include <tofu.h>

tofu_rep_t *first_handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>First</title></head>\n");
	tofu_writef(rep, "<body>First handler: %s</body>\n", tofu_param(req, "param"));

	return rep;
}

tofu_rep_t *second_handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>Second</title></head>\n");
	tofu_writef(rep, "<body>Second handler: %s</body>\n", tofu_param(req, "param"));

	return rep;
}

int main() {
	char *opts[] = { "0.0.0.0", "2000" };
	tofu_ctx_t *ctx = tofu_ctx_init(TOFU_EVHTTP, opts);

	tofu_handle_with(ctx, GET, "/first/:param", first_handler);
	tofu_handle_with(ctx, GET, "/second/:param", second_handler);

	tofu_loop(ctx);
	tofu_ctx_free(ctx);

	return 0;
}
