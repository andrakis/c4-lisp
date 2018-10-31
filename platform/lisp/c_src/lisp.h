#ifndef LISP_H
#define LISP_H

// Ignore formatting warnings
#pragma GCC diagnostic ignored "-Wformat"

extern int debug;

typedef enum CellTypes {
	Number,
	String,
	List,
	Lambda,
	Proc
} CellType;

struct LinkedList_t;
struct Environment_t;
typedef long long CellValue;
typedef struct LinkedList_t LinkedList;
typedef struct Environment_t Env;
typedef struct Cell_t Cell;

struct Cell_t {
	CellType type;
	CellValue value;
	LinkedList *list;
	Env *env;
};

// Cell: Internal API
Cell *cell__alloc () { return (Cell*)malloc(sizeof(Cell)); }
void  cell__construct4(CellType type, CellValue value, LinkedList *list, Cell *cell);
void  cell__construct(Cell *cell);
void  cell__destruct(Cell *cell);
// Cell: Public API
Cell *cell_new (CellType type, CellValue value, LinkedList *list);
void  cell_free (Cell *cell);
void  cell_copy (Cell *source, Cell *dest);
Cell *cell_dupe (Cell *source);

struct LinkedList_t {
	Cell head;
	LinkedList *tail;
};
// List: internal API
LinkedList *list__alloc () { return (LinkedList*)malloc(sizeof(LinkedList)); }
void        list__construct3 (Cell *head, LinkedList *tail, LinkedList *list);
void        list__destruct (LinkedList *list);
// List: Public API
LinkedList *list_new (Cell *head, LinkedList *tail);
void        list_free (LinkedList *list);
void        list_copy (LinkedList *source, LinkedList *dest);
LinkedList *list_dupe (LinkedList *source);

struct Environment_t {
	Env *outer; // reference to parent environment
	LinkedList members;
	int ref_count;
};
void env__free (Env *env);
void env__drop_reference (Env *env);

enum FileDescriptors { Stdin = 0, Stdout = 1, Stderr = 2 };
#endif
