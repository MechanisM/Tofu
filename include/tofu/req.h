#define GET	0
#define POST	1
#define PUT	2
#define DELETE	3

typedef struct {
	int connid;
	int method;

	char *uri;
	char *headers;
	char *body;

	void *params;
} tofu_req_t;

tofu_req_t *tofu_req_init(int connid, char *method, char *uri);
void        tofu_req_free(tofu_req_t *req);

char       *tofu_param(tofu_req_t *req, const char *name);
