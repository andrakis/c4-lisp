// A c4-compatible simple linked list

#include <malloc.h>

// struct List {
enum {
	List_head,    // void *data -> any;
	List_tail,    // struct List *tail -> List*;;
	List_count,   // Number of elements in tail + 1 (ours)
	_List_end
};
// }

// non-c4 hack
#define int long

void *list_struct_head (int *list) { return (void*)list[List_head]; }
int  *list_struct_tail (int *list) { return (int*)list[List_tail]; }
int   list_struct_count(int *list) { return list[List_count]; }

void list_free (int *list) { free(list); }
int *list_struct_new (void *head, int *tail) {
	int *list;
	int  count;

	list = (int*)malloc(sizeof(int) * _List_end);
	if(!list) return 0;

	if(tail)
		count = list_struct_count(tail);
	else
		count = 0;
	if(head)
		count++;

	list[List_head] = (int)head;
	list[List_tail] = (int)tail;
	list[List_count] = count;

	return list;
}

enum { HEAD_EMPTY = 0, TAIL_EMPTY = 0 };

int *list_new () { return list_struct_new(HEAD_EMPTY, TAIL_EMPTY); }
int *list_new_value (void *value) { return list_struct_new(value, TAIL_EMPTY); }
int *list_new_value_tail (void *value, int *tail) { return list_struct_new(value, tail); }
