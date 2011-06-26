typedef struct {
	int connid;
	int status;

	void *headers;
	void *chunks;
} tofu_rep_t;

tofu_rep_t *tofu_rep_init();
void        tofu_rep_free(tofu_rep_t *rep);

void        tofu_write(tofu_rep_t *rep, const char *s);
void        tofu_head(tofu_rep_t *rep, const char *field, const char *value);
