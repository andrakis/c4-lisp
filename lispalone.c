// Lisp standalone implementation
// No external dependencies

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

/** Globals **/
short g_debug = 1;

/** Enumerations **/
enum FileDescriptors { Stdin, Stdout, Stderr };

enum CellTypes {
	Nil,             // nil type
	Number,          // value => CellValue
	Symbol,          // value => char*
	String,          // value => char*
	List,            // value ignored; see 'list' member
	Lambda,          // value ignored; see 'list' member, 'env' member
	Proc             // value => (Cell)(List args)
};

enum LSFlagsValues {
	None    = 0x00,  //
	Dynamic = 0x01,  // if set, was allocated using dynamic memory (malloc, etc)
};

/** Forward declarations and typedefs **/
struct Cell_st;
struct List_st;
struct Env_st;

typedef struct Cell_st LSCell;
typedef struct List_st LSList;
typedef struct Map_st  LSMap;
typedef struct Env_st  LSEnv;

typedef unsigned LSCellType;
typedef long long LSCellValue;
typedef unsigned LSRefCounter;
typedef unsigned LSFlags;
typedef unsigned long long LSIndex;

LSCell sym_nil,
       sym_true,
       sym_false;

/** Structures **/
struct Cell_st {
	LSCellType type;
	LSCellValue value;
	LSList *list;
	LSEnv *env;
	LSFlags flags;
};
void cell_construct(LSCell *cell, LSCellType type, LSCellValue value, LSList *list, LSEnv *env, LSFlags flags);
void cell_destruct (LSCell *cell);
LSCell *cell_new(LSCellType type, LSCellValue value, LSList *list, LSEnv *env);
LSCell *cell_dupe(LSCell *cell);  // Duplicate and return pointer
void    cell_copy(LSCell *source, LSCell *dest); // Copy from one to another
LSCell  cell_index_or(LSCell cell, LSIndex index, LSCell defaultval);
LSIndex cell_strcmp(LSCell cell, const char *compare);
LSCell  cell_head(LSCell cell);
LSList *cell_tail(LSCell cell);

struct List_st {
	LSCell head;
	LSList *tail;
	LSFlags flags;
};
void list_construct(LSList *list, LSCell head, LSList *tail, LSFlags flags);
void list_destruct (LSList *list);
LSList *list_dupe(LSList *list);
LSList *list_new(LSCell head, LSList *tail);
LSCell  list_index(LSList *list, LSIndex index);
LSCell  list_index_or(LSList *list, LSIndex index, LSCell defaultval);
LSCell  list_head(LSList *list);
LSList *list_tail(LSList *list);
void list_free(LSList *list) {
	if(list->tail)
		list_free(list->tail);
	if(list->flags & Dynamic)
		free(list);
}
int  list_is_empty(LSList *list) { return list->head.type == Nil; }
void list_push_back(LSList *list, LSCell value);

struct Map_st {
	LSList *keys;
	LSList *values;
	LSFlags flags;
};
void map_construct(LSMap *map, LSFlags flags);
void map_destruct(LSMap *map);
LSCell map_find(LSMap *map, char *name);
void map_set(LSMap *map, char *name, LSCell value);
void map_free(LSMap *map) {
	if(map->keys && map->keys->flags & Dynamic)
		list_free(map->keys);
	if(map->values && map->values->flags & Dynamic)
		list_free(map->values);
	if(map->flags & Dynamic)
		free(map);
}

struct Env_st {
	LSEnv *outer;
	LSMap members;
	LSRefCounter refcount;
	LSFlags flags;
};
void   env_construct(LSEnv *env, LSEnv *outer, LSFlags flags);
void   env_destruct (LSEnv *env);
LSEnv *env_new(LSEnv *outer, LSFlags flags);
LSEnv *env_new3(LSEnv *outer, LSList *names, LSList *values);
LSCell env_find(LSEnv *env, LSCell key);
void   env_set (LSEnv *env, LSCell key, LSCell value);
void   env_free (LSEnv *env);
void   env_dereference (LSEnv *env);

/** Eval functions */
LSCell eval(LSCell x, LSEnv *env);

/** Cell implementation **/
void cell_construct(LSCell *cell, LSCellType type, LSCellValue value, LSList *list, LSEnv *env, LSFlags flags) {
	cell->type = type;
	cell->value = value;
	cell->list = list;
	cell->env = env;
	cell->flags = flags;
}
void cell_destruct(LSCell *cell) {
	if(cell->env != 0) {
		env_dereference(cell->env);
	}
}
LSCell *cell_new(LSCellType type, LSCellValue value, LSList *list, LSEnv *env) {
	LSCell *cell;
	
	if(!(cell = (LSCell*)malloc(sizeof(LSCell)))) return 0;
	cell_construct(cell, type, value, list, env, Dynamic);
	return cell;
}
LSCell *cell_dupe(LSCell *cell) {
	return cell_new(cell->type, cell->value, cell->list, cell->env);
}
LSCell cell_index(LSCell cell, LSIndex index) {
	return list_index(cell.list, index);
}
LSCell cell_index_or(LSCell cell, LSIndex index, LSCell defaultval) {
	return list_index_or(cell.list, index, defaultval);
}
void cell_copy(LSCell *source, LSCell *dest) {
	memcpy(dest, source, sizeof(LSCell));
}

LSIndex cell_strcmp(LSCell cell, const char *compare) {
	if(cell.type == Symbol || cell.type == String)
		return strcmp((char *)cell.value, compare);
	return 1;
}
LSCell  cell_head(LSCell cell) {
	assert(cell.type == List || cell.type == Lambda);

	return list_head(cell.list);
}
LSList *cell_tail(LSCell cell) {
	assert(cell.type == List || cell.type == Lambda);

	return list_tail(cell.list);
}

/** List implementation **/
void list_construct(LSList *list, LSCell head, LSList *tail, LSFlags flags) {
	memcpy(&list->head, &head, sizeof(LSCell));
	if((list->tail = tail)) {
		list->tail = list_dupe(tail);
	}
	list->flags = flags;
}
LSList *list_new(LSCell head, LSList *tail) {
	LSList *list;

	if(!(list = (LSList*)malloc(sizeof(LSList)))) return 0;
	list_construct(list, head, tail, Dynamic);
	return list;
}
LSList *list_empty() {
	LSList *list;

	if(!(list = (LSList*)malloc(sizeof(LSList)))) return 0;
	list->head.type = Nil;
	list->tail = 0;
	return list;
}
LSList *list_dupe(LSList *list) {
	return list_new(list->head, list->tail);
}
LSCell list_index(LSList *list, LSIndex index) {
	return list_index_or(list, index, sym_nil);
}
LSCell list_index_or(LSList *list, LSIndex index, LSCell defaultval) {
	if(index == 0)
		return list->head;
	else if(list->tail == 0)
		return defaultval;
	return list_index_or(list->tail, index - 1, defaultval);
}
LSCell  list_head(LSList *list) {
	return list->head;
}
LSList *list_tail(LSList *list) {
	return list->tail;
}
void list_push_back(LSList *list, LSCell value) {
	LSList *it;

	// find tail
	it = list;
	while(it->tail) it = it->tail;
	
	// allocate new
	it->tail = list_new(value, 0);
}

/** Environment implementation **/
void env_construct(LSEnv *env, LSEnv *outer, LSFlags flags) {
	env->outer = outer;
	map_construct(&env->members, flags);
	env->refcount = 0;
	env->flags = flags;
}
LSEnv *env_new(LSEnv *outer, LSFlags flags) {
	LSEnv *env;

	if(!(env = (LSEnv*)malloc(sizeof(LSEnv)))) return 0;
	env_construct(env, outer, flags);
	return env;
}
LSEnv *env_new3(LSEnv *outer, LSList *names, LSList *values) {
	LSEnv *env;
	LSList *it1, *it2;

	if(!(env = (LSEnv*)malloc(sizeof(LSEnv)))) return 0;

	it1 = names;
	it2 = values;

	while(it1 && it2 &&
	     it1->head.type != Nil && it2->head.type != Nil) {
		env_set(env, it1->head, it2->head);
		it1 = it1->tail;
		it2 = it2->tail;
	}

	return env;
}
void env_set (LSEnv *env, LSCell key, LSCell value) {
	LSCell nkey, nvalue;

	map_set(&env->members, (char*)key.value, value);
}
LSCell env_find(LSEnv *env, LSCell key) {
	return map_find(&env->members, (char*)key.value);
}
void env_dereference (LSEnv *env) {
	assert(env->refcount > 0);
	if(--env->refcount == 0)
		if(env->flags & Dynamic)
			env_free(env);
}
void env_free (LSEnv *env) {
	assert(env->flags & Dynamic);
	map_free(&env->members);
	free(env);
}

/** Map implementation **/
void map_construct(LSMap *map, LSFlags flags) {
	map->keys= list_new(sym_nil, 0);
	map->values = list_new(sym_nil, 0);
	map->flags = flags;
}
LSCell map_find(LSMap *map, char *name) {
	LSCell result;
	LSList *it1, *it2;

	it1 = map->keys;
	it2 = map->values;
	assert(it1 != 0);
	assert(it2 != 0);

	while(it1 && it2) {
		if(!cell_strcmp(it1->head, name))
			return it2->head;
		it1 = it1->tail;
		it2 = it2->tail;
	}

	result.type = Nil;
	return result;
}
void map_set(LSMap *map, char *name, LSCell value) {
}

/** Eval: exec an if statement **/
LSCell eval_if(LSCell x, LSEnv *env) {
	LSCell test, conseq, alt;

	test = cell_index(x, 1);
	conseq = cell_index(x, 2);
	alt = cell_index_or(x, 3, sym_nil);

	if(cell_strcmp(eval(test, env), (char*)sym_false.value))   // not match false
		return eval(conseq, env);
	else
		return eval(alt, env);
}

/** Eval: exec a begin statement **/
LSCell eval_begin(LSCell x, LSEnv *env) {
	LSList *curr;

	assert(env != 0);

	assert(x.type == List);
	assert(x.list != 0);

	curr = x.list;
	while(curr->tail) {
		eval(curr->head, env);
		curr = curr->tail;
	}

	return eval(curr->head, env);
}

LSCell proc_invoke(LSCell proc, LSList *args) {
	return sym_nil;
}

LSCell eval_proc(LSCell proc, LSList *args, LSEnv *env) {
	LSList *exps, *exp_it, *parms;
	LSCell  result, body;
	LSEnv  *new_env;

	exps = list_empty();
	exp_it = args;
	while(exp_it && !list_is_empty(exp_it)) {
		list_push_back(exps, eval(exp_it->head, env));
		exp_it = exp_it->tail;
	}

	if(proc.type == Lambda) {
		body = cell_index(proc, 2);       // body
		parms = cell_index(proc, 1).list; // parms
		new_env = env_new3(proc.env, parms, exps);
		result = eval(body, new_env);
	} else if(proc.type == Proc) {
		result = proc_invoke(proc, exps);
	} else {
		dprintf(Stderr, "not a function\n");
		exit(5);
	}

	list_free(exps);
	return result;
}

/** Eval loop implementation **/
LSCell eval(LSCell x, LSEnv *env) {
	LSCell item0;

	if(x.type == Symbol)
		return env_find(env, x);
	if(x.type == Number || x.type == String)
		return x;
	if(x.list == 0)
		return sym_nil;
	
	item0 = cell_index(x, 0);
	if(item0.type == Symbol) {
		if(!cell_strcmp(item0, "quote"))        // (quote exp)
			return cell_index(x, 1);
		if(!cell_strcmp(item0, "if"))           // (if test conseq [alt])
			return eval_if(x, env);
		if(!cell_strcmp(item0, "set!") ||       // (define var exp)
		   !cell_strcmp(item0, "define")) {
			env_set(env, cell_index(x, 1), cell_index(x, 2));
			return x;
		}
		if(!cell_strcmp(item0, "lambda")) {     // (lambda (var*) exp)
			x.env = env;
			return x;
		}
		if(!cell_strcmp(item0, "begin")) {      // (begin exp*)
			return eval_begin(x, env);
		}
	}

	// (proc exp*)
	return eval_proc(eval(item0, env), cell_tail(x), env);
}

void setup() {
	cell_construct(&sym_nil, Symbol, (LSCellValue)"nil", 0, 0, 0);
	cell_construct(&sym_true, Symbol, (LSCellValue)"#t", 0, 0, 0);
	cell_construct(&sym_false, Symbol, (LSCellValue)"#f", 0, 0, 0);
}

void test() {
}

int main (int argc, char **argv) {
	setup();
	test();


	if(g_debug) {
		dprintf(Stderr, "Sizes:\n");
		dprintf(Stderr, "\tCell: %ld (%ldb)\n", sizeof(LSCell), sizeof(LSCell) / 8);
		dprintf(Stderr, "\tList: %ld (%ldb)\n", sizeof(LSList), sizeof(LSList) / 8);
		dprintf(Stderr, "\tMap : %ld (%ldb)\n", sizeof(LSMap), sizeof(LSMap) / 8);
		dprintf(Stderr, "\tEnv : %ld (%ldb)\n", sizeof(LSEnv), sizeof(LSEnv) / 8);
	}
	return 0;
}
