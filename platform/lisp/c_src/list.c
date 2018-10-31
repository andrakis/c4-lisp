#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lisp.h"

void        list__construct3 (Cell *head, LinkedList *tail, LinkedList *list) {
	assert(list != 0);

	cell_copy(head, &list->head);
	if((list->tail = tail) != 0) {
		list->tail = list_dupe(tail);
	}
}

LinkedList *list_new (Cell *head, LinkedList *tail) {
	LinkedList *list = list__alloc();
	if(!list) return 0;

	list__construct3(head, tail, list);

	return list;
}

LinkedList *list_dupe (LinkedList *source) {
	LinkedList *list = list__alloc();
	if(!list) return list;

	list_copy(source, list);
	if(source->tail != 0)
		list->tail = list_dupe(source->tail);
	
	return list;
}
void        list_copy (LinkedList *source, LinkedList *dest) {
	memcpy(dest, source, sizeof(LinkedList));
}
