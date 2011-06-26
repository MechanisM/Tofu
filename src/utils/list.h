/*
 * Generic doubly linked list implementation
 *
 * Copyright (c) 2011, Alessandro Ghedini <al3xbio@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>

typedef struct NODE {
	void *value;

	struct NODE *prev;
	struct NODE *next;
} list_node_t;

/**
 * list_init - initialize a list
 *
 * This function initializes an empty list.
 *
 * Example:
 *
 *    list_node_t *l = list_init();
 *
 */

static inline list_node_t *list_init();

/**
 * list_head - return the head of a list
 * @param l The list
 *
 * This function returns the head (the first element ) of the given list.
 *
 * Example:
 *
 *    list_node_t *head = list_head(l);
 *
 */

static inline list_node_t *list_head(list_node_t *l);

/**
 * list_tail - return the tail of a list
 * @param l The list
 *
 * This function returns the tail (the last element ) of the given list.
 *
 * Example:
 *
 *    list_node_t *tail = list_tail(l);
 *
 */

static inline list_node_t *list_tail(list_node_t *l);

/**
 * list_is_empty - check if the list is empty
 * @param l The list
 *
 * This function checks wether the given list is empty or not.
 *
 * Example:
 *
 *    if (list_is_empty(l))
 *      ...
 *
 */

static inline bool list_is_empty(list_node_t *l);

/**
 * list_insert_head - insert an element on top
 * @param l The list
 * @param item The item to insert
 *
 * This function inserts the given element on top of the list.
 *
 * Example:
 *
 *    char *item = "Hello, World!";
 *    list_insert_head(l, (void *) item);
 *
 */

static inline void list_insert_head(list_node_t *l, void *item);

/**
 * list_insert_tail - insert an element on bottom
 * @param l The list
 * @param item The item to insert
 *
 * This function inserts the given element on the bottom of the list.
 *
 * Example:
 *
 *    char *item = "Hello, World!";
 *    list_insert_tail(l, (void *) item);
 *
 */

static inline void list_insert_tail(list_node_t *l, void *item);

/**
 * list_remove - remove an element from the list
 * @param l The list
 *
 * This function removes the current element from the list.
 *
 * Example:
 *
 *    list_node_t *head = list_head(l);
 *    list_remove(head);
 *
 */

static inline void list_remove(list_node_t *l);

/**
 * list_destroy - destroy a list
 * @param l The list
 *
 * This function completely destroys the given list.
 *
 * Example:
 *
 *    list_destroy(l);
 *
 */

static inline void list_destroy(list_node_t *l);

/**
 * list_foreach - iterate the whole list
 * @param item The temporary item used as iterator
 * @param l The list to iterate
 *
 * This function iterates the given list from the head to the tail.
 *
 * Example:
 *
 *    list_node_t *iter;
 *
 *    list_foreach(iter, l) {
 *      ...
 *    }
 *
 */

#define list_foreach(ITEM, L) \
	for (ITEM = list_head(L); ITEM != L; ITEM = ITEM -> next)

/**
 * list_reverse_foreach - reverse iterate the whole list
 * @param item The temporary item used as iterator
 * @param l The list to iterate
 *
 * This macro iterates the given list from the tail to the head.
 *
 * Example:
 *
 *    list_node_t *iter;
 *
 *    list_reverse_foreach(iter, l) {
 *      ...
 *    }
 *
 */

#define list_reverse_foreach(ITEM, L) \
	for (ITEM = list_tail(L); ITEM != L; ITEM = ITEM -> prev)

/**
 * list_append - append one list to onether
 * @param l1 The first list
 * @param l2 The second list
 *
 * This macro appends the list 'l2' to the tail of 'l1'. 'l1' is modified.
 *
 * Example:
 *
 *    list_append(l1, l2);
 *
 */

static inline void list_append(list_node_t *l1, list_node_t *l2);

static inline list_node_t *list_init() {
	list_node_t *t = (list_node_t *) malloc(sizeof(list_node_t));

	if (t == NULL)
		return NULL;

	t -> prev = t;
	t -> next = t;

	return t;
}

static inline list_node_t *list_head(list_node_t *l) {
	return l -> next;
}

static inline list_node_t *list_tail(list_node_t *l) {
	return l -> prev;
}

static inline bool list_is_empty(list_node_t *l) {
	if (l == NULL || l == l -> next)
		return true;

	return false;
}

static inline void list_insert_head(list_node_t *l, void *item) {
	list_node_t *t = list_init();

	t -> value = item;

	t -> prev = l;
	t -> next = l -> next;
	t -> next -> prev = t;

	l -> next = t;
}

static inline void list_insert_tail(list_node_t *l, void *item) {
	list_node_t *t = list_init();

	t -> value = item;

	t -> prev = l -> prev;
	t -> next = l;
	t -> prev -> next = t;

	l -> prev = t;
}

static inline void list_remove(list_node_t *l) {
	l -> prev -> next = l -> next;
	l -> next -> prev = l -> prev;

	if (!list_is_empty(l))
		free(l);
}

static inline void list_destroy(list_node_t *l) {
	list_node_t *iter;

	list_reverse_foreach(iter, l) {
		list_remove(iter);
	}

	free(l);
}

static inline void list_append(list_node_t *l1, list_node_t *l2) {
	list_node_t *tail1 = list_tail(l1);
	list_node_t *tail2 = list_tail(l2);

	tail1 -> next = list_head(l2);
	tail2 -> next = l1;
}

#endif
