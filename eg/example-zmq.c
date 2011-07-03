#include <tofu.h>

tofu_rep_t *lol_handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>Ciao</title></head>\n");
	tofu_writef(rep, "<body>ciao %s</body>\n", tofu_param(req, "ciao"));

	return rep;
}

tofu_rep_t *mao_handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");
	tofu_write(rep, "<!DOCTYPE html>\n<head><title>Ciao</title></head>\n");
	tofu_writef(rep, "<body>mao %s</body>\n", tofu_param(req, "ciao"));

	return rep;
}

int main() {
	char *opts[] = {
		/* recv spec and ident */
		"tcp://127.0.0.1:9999", "54c6755b-9628-40a4-9a2d-cc82a816345e",
		/* send spec and ident */
		"tcp://127.0.0.1:9998", "b0541e27-9e77-48c1-80ef-24819ae3a97b"
	};

	tofu_ctx_t *ctx = tofu_ctx_init(TOFU_ZMQ, opts);

	tofu_handle_with(ctx, GET, "/lol/:ciao", lol_handler);
	tofu_handle_with(ctx, GET, "/mao/:ciao", mao_handler);

	tofu_loop(ctx);
	tofu_ctx_free(ctx);

	return 0;
}
