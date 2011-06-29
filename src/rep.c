/*
 * (Yet another) C web framework
 *
 * Copyright (c) 2011, Alessandro Ghedini
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of the Mongrel2 Project, Zed A. Shaw, nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>

#include "rep.h"

#include "bstring/bstrlib.h"

#include "utils/list.h"
#include "utils/common.h"

tofu_rep_t *tofu_rep_init() {
	tofu_rep_t *rep = malloc(sizeof(tofu_rep_t));

	rep -> status  = 0;
	rep -> headers = list_init();
	rep -> chunks  = list_init();

	return rep;
}

void tofu_rep_free(tofu_rep_t *rep) {
	list_node_t *iter, *safe;

	if (rep == NULL)
		return;

	list_destroy(rep -> chunks);

	list_reverse_foreach_safe(iter, safe, rep -> headers) {
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

void tofu_writef(tofu_rep_t *rep, const char *fmt, ...) {
	int ret;
	bstring b = bfromcstr("");

	bvformata(ret, b , fmt, fmt);

	list_insert_tail(rep -> chunks, (void *) strdup(bdata(b)));

	bdestroy(b);
}

void tofu_head(tofu_rep_t *rep, const char *field, const char *value) {
	tofu_pair_t *header = malloc(sizeof(tofu_pair_t));

	header -> name  = (void *) strdup(field);
	header -> value = (void *) strdup(value);

	list_insert_tail(rep -> headers, (void *) header);
}

void tofu_status(tofu_rep_t *rep, int status) {
	rep -> status = status;
}
