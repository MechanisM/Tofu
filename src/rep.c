#include <stdlib.h>
#include <string.h>

#include "rep.h"

#include "utils/list.h"
#include "utils/common.h"

tofu_rep_t *tofu_rep_init() {
	tofu_rep_t *rep = malloc(sizeof(tofu_rep_t));

	rep -> headers = list_init();
	rep -> chunks  = list_init();

	return rep;
}

void tofu_rep_free(tofu_rep_t *rep) {
	list_node_t *iter;

	if (rep == NULL)
		return;

	list_destroy(rep -> chunks);

	list_reverse_foreach(iter, rep -> headers) {
		iter -> prev -> next = iter -> next;
		iter -> next -> prev = iter -> prev;

		if (!list_is_empty(iter)) {
			tofu_pair_t *header = iter -> value;

			free(header -> name);
			free(header -> value);

			free(header);
			free(iter);
		}
	}

	free(rep -> headers);
	free(rep);
}

void tofu_write(tofu_rep_t *rep, const char *chunk) {
	list_insert_tail(rep -> chunks, (void *) chunk);
}

void tofu_head(tofu_rep_t *rep, const char *field, const char *value) {
	tofu_pair_t *header = malloc(sizeof(tofu_pair_t));

	header -> name  = (void *) strdup(field);
	header -> value = (void *) strdup(value);

	list_insert_tail(rep -> headers, (void *) header);
}
