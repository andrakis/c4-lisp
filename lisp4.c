#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include "c5_native.h"

#define SYSCALLS
#include "syscalls.h"

// enum cell_type {
enum { Symbol, Number, List, Proc, Lambda };
// };

// Special values
enum { ENV_NOPARENT = 0 };

// enum Syscalls1 {
// syscalls that take no argument (the signal only)
enum {
	SYS1_ATOM_FALSE, // void -> int. get pointer to false atom
	SYS1_ATOM_TRUE,  // void -> int. get pointer to true atom
	SYS1_ATOM_NIL,   // void -> int. get pointer to nil atom
	SYS1_CELL_NEW,   // void -> int. Create new empty cell
	_SYS1_END        // Must be last element
};
// };

// enum Syscalls2 {
// syscalls that take 1 argument (the signal, 1 arg)
enum {
	SYS2_ISDIG,  // (char Digit) -> 1 | 0. check if is digit
	SYS2_CELL_COPY, // (int Cell) -> int. copy a cell from a pointer
	SYS2_CELL_EMPTY,// (int Cell) -> 1 | 0. check if empty
	SYS2_CELL_ENV_GET, // (int Cell) -> int. get pointer to cell's env
	SYS2_CELL_FREE, // (int Cell) -> void. Free a cell object
	SYS2_CELL_FRONT, // (int Cell) -> int. Cell
	SYS2_CELL_LIST, // (int Cell) -> int.
	SYS2_CELL_SIZE, // (int Cell) -> int.
	SYS2_CELL_CSTR, // (int Cell) -> char*.
	SYS2_CELL_TAIL, // (int Cell) -> int. List
	SYS2_CELL_TYPE, // (int Cell) -> int. Tag
	SYS2_CELL_VALUE,// (int Cell) -> int. Value as string
	SYS2_LIST,      // (int Content) -> int.
	SYS2_LIST_EMPTY,// (int List) -> 1 | 0.
	SYS2_LIST_FREE, // (int List) -> void.
	SYS2_LIST_SIZE, // (int List) -> int.
	SYS2_ENV,       // (int Outer) -> int.
	SYS2_FREE_ENV,  // (int Env) -> void. Free an environment object
	SYS2_ADD_GLOBS,  // (int Env) -> void. Add global symbols to given env
	_SYS2_END        // Must be last element
};
// };

// enum Syscalls3 {
// syscalls that take 2 arguments (the signal, 2 args)
enum {
	SYS3_CELL_ENV_SET,// (int Env, int Cell) -> void.
	SYS3_CELL_INDEX,  // (int Index, int Cell) -> int. Cell
	SYS3_CELL_NEW,    // (int Tag, char *Value) -> int.
	SYS3_CELL_RESET,  // (int Dest, int Source) -> void. Reset cell value to Source values
	SYS3_CELL_SETENV, // (int Env, int Cell) -> int. Cell
	SYS3_CELL_SETTYPE,// (int Type, int Cell) -> int. Cell
	SYS3_CELL_STRCMP, // (char *s, int Cell) -> 0 | 1. Returns 0 on match, like strmp
	SYS3_LIST_INDEX,  // (int Index, int List) -> int.Cell.
	SYS3_LIST_PUSHB,  // (int Cell, int List) -> List.
	SYS3_ENV_GET,     // (char *Name, int Env) -> int.Cell.
	SYS3_ENV_HAS,     // (char *Name, int Env) -> 0 | 1.
	SYS3_ENV_LOOKUP,  // (int Cell, int Env) -> int. Cell
	SYS3_CALL_PROC,   // (int Cells::List, int Cell) -> int. Cell.
	SYS3_PARSE,       // (char *Code, int Cell) -> int. 0 success, 1 failure
	SYS3_LISP_MAIN,   // (int Argc, char **Argv) -> int. Return code
	_SYS3_END         // Must be last element
};
// };

// enum Syscalls4 {
// syscalls that take 3 arguments (the signal, 3 args)
enum {
	SYS4_ENV_SET,     // (char *Name, int Cell, int Env) -> Env.
	SYS4_ENV_NEWARGS, // (int Names::list, int Values::list, int Parent::env) -> int. Env
	_SYS4_END        // Must be last element
};
// };

// predefined symbols as chars
char *s_quote,
     *s_if,
     *s_set,
     *s_define,
     *s_lambda,
     *s_begin,
     *s_false,
     *s_true;

void *sym_nil;

void *cell_new() {
	return (void*)syscall1(SYS1_CELL_NEW);
}

// Wrappers around various syscalls
int cell_empty(void *cell) {
	return syscall2(SYS2_CELL_EMPTY, (int)cell);
}

void cell_free(void *cell) {
	syscall2(SYS2_CELL_FREE, (int)cell);
}

void *cell_front(void *cell) {
	return (void*)syscall2(SYS2_CELL_FRONT, (int)cell);
}

void *cell_index(int index, void *cell) {
	return (void*)syscall3(SYS3_CELL_INDEX, index, (int)cell);
}

int cell_size(void *cell) {
	return syscall2(SYS2_CELL_SIZE, (int)cell);
}

char *cell_cstr(void *cell) {
	return (char*)syscall2(SYS2_CELL_CSTR, (int)cell);
}

void *cell_index_default(int index, void *def, void *cell) {
	if(index >= cell_size(cell))
		return def;
	return cell_index(index, cell);
}

int cell_strcmp(char *str, void *cell) {
	return syscall3(SYS3_CELL_STRCMP, (int)(void*)str, (int)cell);
}

void *cell_tail(void *cell) {
	return (void*)syscall2(SYS2_CELL_TAIL, (int)cell);
}

void *cell_set_lambda(void *env, void *cell) {
	cell = (void*)syscall3(SYS3_CELL_SETTYPE, Lambda, (int)cell);
	return (void*)syscall3(SYS3_CELL_SETENV, (int)env, (int)cell);
}

int cell_type(void *cell) {
	return syscall2(SYS2_CELL_TYPE, (int)cell);
}

char *cell_value(void *cell) {
	return (char*)syscall2(SYS2_CELL_VALUE, (int)cell);
}

int env_has(char *s, void *env) {
	return syscall3(SYS3_ENV_HAS, (int)(void*)s, (int)env);
}

// env_lookup(char*, environment*) -> cell*();
void *env_lookup(void *name, void *env) {
	return (void*)syscall3(SYS3_ENV_LOOKUP, (int)name, (int)env);
}

// env_new(environment*Parent) -> environment*()
void *env_new(void *parent) {
	return (void*)syscall2(SYS2_ENV, (int)parent);
}

// env_new_args(cells *, cells *, environment*) -> environment*();
void *env_new_args(void *names, void *values, void *parent) {
	return (void*)syscall4(SYS4_ENV_NEWARGS, (int)names, (int)values, (int)parent);
}

// env_set(char*, char*, environment*e) -> e;
void *env_set(void *name, void *value, void *env) {
	return (void*)syscall4(SYS4_ENV_SET, (int)name, (int)value, (int)env);
}

// env_free(environment*) -> void;
void env_free(void *env) {
	syscall2(SYS2_FREE_ENV, (int)env);
}

void add_globals(void *env) {
	syscall2(SYS2_ADD_GLOBS, (int)env);
}

void list_free(void *list) {
	syscall2(SYS2_LIST_FREE, (int)list);
}

void *list_new() {
	return (void*)syscall2(SYS2_LIST, 0);
}

void *list_push_back(void *cell, void *list) {
	return (void*)syscall3(SYS3_LIST_PUSHB, (int)cell, (int)list);
}

void *proc_invoke(void *exps, void *proc) {
	return (void*)syscall3(SYS3_CALL_PROC, (int)exps, (int)proc);
}

int parse(char *code, void *dest) {
	return syscall3(SYS3_PARSE, (int)code, (int)dest);
}

void *eval(void *x, void *env) {
	int type, size, i;
	void *result, *first, *test, *conseq, *alt;
	void *proc, *exps;

	result = 0;
	type = cell_type(x);
	if(type == Symbol) return env_lookup(cell_value(x), env);
	if(type == Number) return x;
	if(cell_empty(x)) return sym_nil;

	first = cell_front(x);
	type = cell_type(cell_front(x));
	if(type == Symbol) {
		if(cell_strcmp("quote", first) == 0)			// (quote exp)
			return cell_tail(x);
		if(cell_strcmp("if", first) == 0) {		// (if test conseq [alt])
			test = cell_index(1, x);
			conseq = cell_index(2, x);
			alt = cell_index_default(3, sym_nil, x);
			result = eval(test, env);
			result = (cell_strcmp(s_false, result) == 0) ? alt : conseq;
			// TODO: free x
			return result;
		}
		if(cell_strcmp("set!", first) == 0) {	// (set! var exp)
			result = eval(cell_index(2, x), env);
			env_set(cell_index(1, x), result, env);
			return result;
		}
		if(cell_strcmp("define", first) == 0) {	// (define var exp)
			result = eval(cell_index(2, x), env);
			env = env_set(cell_index(1, x), result, env);
			return result;
		}
		if(cell_strcmp("lambda", first) == 0)	// (lambda (var*) exp)
			return cell_set_lambda(env, x);
		if(cell_strcmp("begin", first) == 0) {	// (begin exp*)
			// TODO: use iterator
			size = cell_size(x);
			i = 1;
			while(i < size - 1)
				eval(cell_index(i++, x), env);
			return eval(cell_index(i, x), env);
		}
	}

	// (proc exp*)
	exps = list_new();
	proc = eval(first, env);
	size = cell_size(x);
	i = 1;
	while(i < size)
		exps = list_push_back(eval(cell_index(i++, x), env), exps);
	type = cell_type(proc);
	if(type == Lambda) {
		// Create new environment parented to current
		env = env_new_args(
			cell_index(1, proc) /* arg names list */,
			exps                /* values */,
			env                 /* parent */);
		// exps no longer needed
		list_free(exps);
		result = eval(cell_index(2, proc), env);
		// TODO: free x
		return result;
	} else if(type == Proc) {
		result = proc_invoke(exps, proc);
		list_free(exps);
		// TODO: free x
		return result;
	}

	printf("Invalid call in eval\n");
	exit(1);
}

#define main lispmain

int debug;

int main(int argc, char **argv)
{
	void *global, *tokens;
	char *code, *tmp;

	// Ensure syscalls are up to date
	if(!syscall_init(1, _SYS1_END) ||
	   !syscall_init(2, _SYS2_END) ||
	   !syscall_init(3, _SYS3_END) ||
	   !syscall_init(4, _SYS4_END)) {
		printf("Syscall init failed, recompile / check headers\n");
		return -1;
	}

	debug = 1;

	// Setup predefined symbols used in eval
	s_quote = "quote";
	s_if = "if";
	s_set = "set!";
	s_define = "define";
	s_lambda = "lambda";
	s_begin = "begin";

	code = "(print \"Hello!\")";
	global = env_new(ENV_NOPARENT);
	add_globals(global);
	if(debug) printf(" env.global: %x\n", global);

	printf("Has +: %d\n", env_has("+", global));
	printf("Has foo: %d\n", env_has("foo", global));

	tokens = cell_new();
	if(parse(code, tokens)) {
		printf("Failed to parse code: %s\n", code);
		return 1;
	}
	if(debug) {
		tmp = cell_cstr(tokens);
		printf(" tokens: %x (%s)\n", tokens, tmp);
		free(tmp);
	}

	//eval(tokens, global);

	printf("Cleaning up\n");
	cell_free(tokens);
	env_free(global);
	return 0;
}
