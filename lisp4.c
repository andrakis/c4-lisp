#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include "native.h"

#define SYSCALLS
#include "syscalls.h"

// enum cell_type {
enum { Symbol, Number, List, Proc, Lambda };
// };

// enum Syscalls1 {
// syscalls that take no argument (the signal only)
enum {
	SYS1_ATOM_FALSE, // void -> int. get pointer to false atom
	SYS1_ATOM_TRUE,  // void -> int. get pointer to true atom
	SYS1_ATOM_NIL,   // void -> int. get pointer to nil atom
	_SYS1_END        // Must be last element
};
// };

// enum Syscalls2 {
// syscalls that take 1 argument (the signal, 1 arg)
enum {
	SYS2_ISDIG,  // (char Digit) -> 1 | 0. check if is digit
	SYS2_CELL_COPY, // (int Cell) -> int. copy a cell from a pointer
	SYS2_CELL_ENV_GET, // (int Cell) -> int. get pointer to cell's env
	SYS2_CELL_LIST, // (int Cell) -> int.
	SYS2_CELL_TYPE, // (int Cell) -> int. Tag
	SYS2_CELL_VALUE,// (int Cell) -> int. Value as string
	SYS2_LIST,      // (int Content) -> int.
	SYS2_LIST_EMPTY,// (int List) -> 1 | 0.
	SYS2_LIST_SIZE, // (int List) -> int.
	SYS2_ENV,       // (int Outer) -> int.
	SYS2_FREE_CELL, // (int Cell) -> void. Free a cell object
	SYS2_FREE_ENV,  // (int Env) -> void. Free an environment object
	SYS2_ADD_GLOBS,  // (int Env) -> void. Add global symbols to given env
	_SYS2_END        // Must be last element
};
// };

// enum Syscalls3 {
// syscalls that take 2 arguments (the signal, 2 args)
enum {
	SYS3_CELL_NEW,    // (int Tag, char *Value) -> int.
	SYS3_CELL_RESET,  // (int Dest, int Source) -> void. Reset cell value to Source values
	SYS3_CELL_STRCMP, // (char *s, int Cell) -> 0 | 1. Returns 0 on match, like strmp
	SYS3_CELL_ENV_SET,// (int Env, int Cell) -> void.
	SYS3_LIST_INDEX,  // (int Index, int List) -> int.Cell.
	SYS3_LIST_PUSHB,  // (int Cell, int List) -> List.
	SYS3_ENV_GET,     // (char *Name, int Env) -> int.Cell.
	SYS3_ENV_HAS,     // (char *Name, int Env) -> 0 | 1.
	SYS3_CALL_PROC,   // (int Cell, int Cells::Args) -> int.Cell.
	_SYS3_END         // Must be last element
};
// };

// enum Syscalls4 {
// syscalls that take 3 arguments (the signal, 3 args)
enum {
	SYS4_ENV_SET,     // (char *Name, int Cell, int Env) -> Env.
	_SYS4_END        // Must be last element
};
// };

// predefined symbols as chars
char *sym_quote,
     *sym_if,
     *sym_set,
     *sym_define,
     *sym_lambda,
     *sym_begin;

// Wrappers around various syscalls
int cell_strcmp(char *str, void *cell) {
	return syscall3(SYS3_CELL_STRCMP, (void*)str, cell);
}

void *list_new() {
	return (void*)syscall2(SYS2_LIST, 0);
}

void list_free(void *list) {
	syscall2(SYS2_LIST_FREE, list);
}

void *list_push_back(void *element, int *list) {
	return (void*)syscall3(SYS3_LIST_PUSHB, element, list);
}

void *list_front(int *list) {
	return (int*)syscall2(SYS2_LIST_FRONT, list);
}

void *list_pop_front(void *list) {
	return (void*)syscall2(SYS2_LIST_POPF, list);
}

void *tokenise(char *s) { // -> list()
	void *tokens;
	char *t;

	tokens = list_new();

	while(*s) {
		while(*s == ' ') ++s;
		if(*s == '(' || *s == ')')
			list_push_back(*s++ == '(' ? "(" : ")", tokens);
		else {
			t = s;
			while(*t && *t != ' ' && *t != '(' && *t != ')')
				++t;
			list_push_back_range(s, t, tokens);
			s = t;
		}
	}

	return tokens;
}

void *atom(void *token /* cell */) { // cell *
	char *t;
	void *c;

	t = cell_str(token);
	if(isdig(t[0]) || (t[0] == '-' && isdig(t[1]))) {
		c = cell_new_number(t);
	} else {
		c = cell_new_str(t);
	}

	str_free(t);
	return c;
}

void *read_from(void *tokens /* list(str) */) { // -> cell()
	void *token, *c, *t; // cell *token, *c, *t;

	token = list_front(tokens);
	tokens = list_pop_front(tokens);
	if(cell_strcmp("(", token) == 0) {
		c = list_new();
		while(cell_strcmp(")", list_front(tokens)) != 0) {
			tokens = list_push_back(read_from(tokens), c);
		}
		t = list_front(tokens);
		tokens = list_pop_front(tokens);
		cell_free(token); // clear token grabbed
		cell_free(t); // clear token grabbed
		return c;
	} else {
		t = atom(token);
		cell_free(token); // clear token grabbed
		return t;
	}
}

void *read(char *s) { // -> cell*
	void *tokens, *cell;

	tokens = tokenise(s);
	cell = read_from(tokens);
	list_free(tokens);
	return cell;
}

int eval_instruction(int list, int x, int env) {
	int el0, el0_type, el0_value;

	// Get first element
	el0 = syscall3(SYS3_LIST_INDEX, 0, list);
	el0_type = syscall2(SYS2_CELL_TYPE, el0);
	if(el0_type == Symbol) {
		if(cell_strcmp("quote", el0)) {
			return syscall3(SYS3_LIST_INDEX, 1, list);
		} else {
			printf("Invalid instruction: %s\n", (char*)syscall2(SYS2_CELL_VALUE, el0));
		}
	}

	return x;
}

int eval(int x, int env) {
	int type, temp;
	char *s;

	type = syscall2(SYS2_CELL_TYPE, x);

	if(type == Symbol) {
		// Get value as string
		s = (char*)syscall2(SYS2_CELL_VALUE, x);
		// Lookup symbol in environment
		return syscall3(SYS3_ENV_GET, (int)(int*)s, env);
	} else if(type == Number) {
		return x;
	} else if(type == List) {
		// Get list
		temp = syscall2(SYS2_CELL_LIST, x);
		// Is empty?
		if(syscall2(SYS2_LIST_EMPTY, temp))
			return syscall1(SYS1_ATOM_NIL);
		// Act on instruction
		return eval_instruction(temp, x, env);
	} else {
		printf("Invalid construct in eval\n");
		exit(-1);
	}
}

int env_has(char *s, void *env) {
	return syscall3(SYS3_ENV_HAS, (void*)s, env);
}

int main(int argc, char **argv)
{
	void *global, *tokens;
	char *code;

	// Ensure syscalls are up to date
	if(!syscall_init(1, _SYS1_END) ||
	   !syscall_init(2, _SYS2_END) ||
	   !syscall_init(3, _SYS3_END) ||
	   !syscall_init(4, _SYS4_END)) {
		printf("Syscall init failed, recompile / check headers\n");
		return -1;
	}

	// Setup predefined symbols used in eval
	sym_quote = "quote";
	sym_if = "if";
	sym_set = "set!";
	sym_define = "define";
	sym_lambda = "lambda";
	sym_begin = "begin";

	code = "(print \"Hello!\")";
	global = syscall2(SYS2_ENV, 0); // no parent
	syscall2(SYS2_ADD_GLOBS, global); // add globals

	printf("Has +: %d\n", env_has("+", global));
	printf("Has foo: %d\n", env_has("foo", global));

	printf("Cleaning up\n");
	syscall2(SYS2_FREE_ENV, global);
	return 0;
}
