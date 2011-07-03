#include <tofu.h>

tofu_rep_t *eg_handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	int body_len;
	char *body = tofu_body(req, &body_len);

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>Ciao</title></head>\n");

	if (body_len > 0)
		tofu_writef(rep, "<body>mao %s</body>\n", body);
	else
		tofu_write(rep, "<body>no body</body>\n");

	return rep;
}

int main() {
	char *opts[] = { "0.0.0.0", "2000" };
	tofu_ctx_t *ctx = tofu_ctx_init(TOFU_EVHTTP, opts);

	tofu_handle_with(ctx, POST, "/post/:data", eg_handler);

	tofu_loop(ctx);
	tofu_ctx_free(ctx);

	return 0;
}
