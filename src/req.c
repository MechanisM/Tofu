#include <stdlib.h>
#include <string.h>

#include "req.h"

#include "bstring/bstrlib.h"
#include "bstring/bstraux.h"

#include "utils/list.h"
#include "utils/common.h"

tofu_req_t *tofu_req_init(int connid, char *method, char *uri) {
	tofu_req_t *req = malloc(sizeof(tofu_req_t));

	req -> connid = connid;

	if (strcmp(method, "GET") == 0)
		req -> method = GET;
	else if (strcmp(method, "POST") == 0)
		req -> method = POST;
	else if (strcmp(method, "PUT") == 0)
		req -> method = PUT;
	else if (strcmp(method, "DELETE") == 0)
		req -> method = DELETE;
	else ;
		/* error */

	req -> uri    = strdup(uri);
	req -> params = list_init();

	return req;
}

void tofu_req_free(tofu_req_t *req) {
	list_node_t *iter;

	if (req == NULL)
		return;

	list_reverse_foreach(iter, req -> params) {
		iter -> prev -> next = iter -> next;
		iter -> next -> prev = iter -> prev;

		if (!list_is_empty(iter)) {
			tofu_pair_t *param = iter -> value;
			bstring name = param -> name;
			bstring value = param -> value;

			free(name);
			/*free(value);*/

			free(param);
			free(iter);
		}
	}

	free(req -> params);
	free(req -> uri);
	free(req);
}

char *tofu_param(tofu_req_t *req, const char *name) {
	list_node_t *iter;

	list_foreach(iter, req -> params) {
		tofu_pair_t *pair  = iter -> value;
		char *name_curr  = pair -> name;
		char *value_curr = pair -> value;

		if (strcmp(name, name_curr) == 0)
			return value_curr;
	}

	return "";
}
