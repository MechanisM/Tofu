#define GET	0
#define POST	1
#define PUT	2
#define DELETE	3

typedef struct {
	int method;
	char *uri;

	void *params;
} tofu_req_t;

tofu_req_t *tofu_req_init();
void        tofu_req_free(tofu_req_t *req);

char       *tofu_param(tofu_req_t *req, const char *name);
