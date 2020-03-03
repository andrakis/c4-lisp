/**
 * lisp/list.c
 *   A linked list implementation
 */

#include "../../../include/core/c5_native.h"

enum {
	LIST_MAGIC,
	LIST_HEAD,
	LIST_TAIL,
	LIST__SZ
};

enum { MAGIC_LIST = 0xbeef };

int *list_new (void *head, int *tail) {
	int *l;
	if((l = malloc(LIST__SZ * sizeof(int)))) {
		l[LIST_MAGIC] = MAGIC_LIST;
		l[LIST_HEAD] = head;
		l[LIST_TAIL] = tail;
	}
	return l;
}

void void_free (int *list) {
	if(list[LIST_MAGIC] == MAGIC_LIST) {
		free(list);
	}
}

int *list_push_front (int *list, void *item) {
	return list_new(item, list);
}

int list_tests () {
	printf("Doing list tests...\n");
	// Not really doing anything yet
	printf("Done!\n");
	return 0;
}

// -- C4 SNIP
// The contents here should not be printed out by the bin/makec4.c tool
int main (int argc, char **argv) {
	// Run tests
	list_tests();
}
