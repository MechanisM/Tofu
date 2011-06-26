typedef struct {
	void *handlers;
	int   backend;
} tofu_ctx_t;

tofu_ctx_t *tofu_ctx_init();
void        tofu_ctx_free(tofu_ctx_t *ctx);
