#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./lisp.h"

void cell__construct(Cell *cell) {
	if(debug)
		dprintf(Stderr, "  cell(0x%x)\n", (void*)cell);
	cell->env = 0;
}

void cell__construct4(CellType type, CellValue value, LinkedList *list, Cell *cell) {
	cell__construct(cell);
	cell->type = type;
	cell->value = value;
	cell->list = list;
}

void cell__destruct(Cell *cell) {
	if(debug)
		dprintf(Stderr, "  ~cell(0x%x)\n", cell);
	// if lambda, derefence environment
	if(cell->type == Lambda) {
		assert(cell->env != 0);
		env__drop_reference(cell->env);
		// must ensure we don't destruct multiple times, this triggers assert next time around
		cell->env = 0;
	}
	// if list, free children
	if(cell->type == List || cell->type == Lambda) {
		assert(cell->list != 0);
		list_free(cell->list);
	}
}

Cell *cell_new (CellType type, CellValue value, LinkedList *list) {
	Cell *cell = cell__alloc();
	if(!cell) return 0;

	cell__construct4(type, value, list, cell);

	return cell;
}

void cell_copy (Cell *source, Cell *dest) {
	memcpy(dest, source, sizeof(Cell));
}

void cell_free (Cell *cell) {
	cell__destruct(cell);
	free(cell);
}
