void tofu_handler_add(tofu_ctx_t *ctx, int method, const char *route, tofu_rep_t *(*callback)(tofu_req_t *req));

tofu_rep_t *tofu_dispatch(tofu_ctx_t *ctx, tofu_req_t *req);
