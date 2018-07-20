
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
	SYS1_ATOM_TRUE, // void -> int. get pointer to true atom
	SYS1_ATOM_NIL  // void -> int. get pointer to nil atom
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
	SYS2_ADD_GLOBS  // (int Env) -> void. Add global symbols to given env
};
// };

// enum Syscalls3 {
// syscalls that take 2 arguments (the signal, 2 args)
enum {
	SYS3_CELL_NEW,    // (int Tag, char *Value) -> int.
	SYS3_CELL_STRCMP, // (char *s, int Cell) -> 0 | 1. Returns 0 on match, like strmp
	SYS3_CELL_ENV_SET,// (int Env, int Cell) -> void.
	SYS3_LIST_INDEX,  // (int Index, int List) -> int.Cell.
	SYS3_LIST_PUSHB,  // (int Cell, int List) -> List.
	SYS3_ENV_GET,     // (char *Name, int Env) -> int.Cell.
	SYS3_ENV_HAS,     // (char *Name, int Env) -> 0 | 1.
	SYS3_CALL_PROC    // (int Cell, int Cells::Args) -> int.Cell.
};
// };

// enum Syscalls4 {
// syscalls that take 3 arguments (the signal, 3 args)
enum {
	SYS4_ENV_SET      // (char *Name, int Cell, int Env) -> Env.
};
// };

// predefined symbols as chars
char *sym_quote,
     *sym_if,
     *sym_set,
     *sym_define,
     *sym_lambda,
     *sym_begin;

int cell_strcmp(char *str, int cell) {
	return syscall3(SYS3_CELL_STRCMP, (int)(int*)str, cell);
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

int env_has(char *s, int env) {
	return syscall3(SYS3_ENV_HAS, (int)(int*)s, env);
}

int main(int argc, char **argv)
{
	int global, tokens;
	char *code;

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
	return 0;
}
