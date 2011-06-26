#include <tofu.h>

tofu_rep_t *lol_handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");

	tofu_write(rep, "<!DOCTYPE html>\n");
	tofu_write(rep, "<head>\n");
	tofu_write(rep, "<title>Ciao</title>\n");
	tofu_write(rep, "</head>\n");
	tofu_write(rep, "<body>\n");
	tofu_write(rep, "ciao \n");
	tofu_write(rep, tofu_param(req, "ciao"));
	tofu_write(rep, "</body>\n");

	return rep;
}

tofu_rep_t *mao_handler(tofu_req_t *req) {
	tofu_rep_t *rep = tofu_rep_init();

	tofu_head(rep, "Content-Type", "text/html");

	tofu_write(rep, "<!DOCTYPE html>\n");
	tofu_write(rep, "<head>\n");
	tofu_write(rep, "<title>Ciao</title>\n");
	tofu_write(rep, "</head>\n");
	tofu_write(rep, "<body>\n");
	tofu_write(rep, "mao\n");
	tofu_write(rep, tofu_param(req, "ciao"));
	tofu_write(rep, "</body>\n");

	return rep;
}

int main() {
	tofu_ctx_t *ctx = tofu_ctx_init(1);

	tofu_handler_add(ctx, GET, "/lol/:ciao", lol_handler);
	tofu_handler_add(ctx, GET, "/mao/:ciao", mao_handler);

	tofu_loop(ctx);

	tofu_ctx_free(ctx);

	return 0;
}
