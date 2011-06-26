#define BACKEND_FCGI	0
#define BACKEND_ZMQ	1

typedef struct {
	int id;
	char *name;

	void (*loop)(tofu_ctx_t *ctx);
	void (*send)(tofu_rep_t *rep);
} tofu_backend_t;

tofu_backend_t backend;
