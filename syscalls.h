#ifndef __SYSCALLS
#define __SYSCALLS

#include "native.h"

#ifdef __cplusplus
extern "C" {
#endif
	NUMTYPE syscall1(NUMTYPE);
	NUMTYPE syscall2(NUMTYPE, NUMTYPE);
	NUMTYPE syscall3(NUMTYPE, NUMTYPE, NUMTYPE);
	NUMTYPE syscall4(NUMTYPE, NUMTYPE, NUMTYPE, NUMTYPE);
	NUMTYPE syscall_init(NUMTYPE section, NUMTYPE endmarker);

#ifdef __cplusplus
}
#endif

#ifndef SYSCALLS
#define SYSCALLS
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
	SYS2_CELL_FRONT, // (int Cell) -> int. Cell
	SYS2_CELL_LIST, // (int Cell) -> int.
	SYS2_CELL_SIZE, // (int Cell) -> int.
	SYS2_CELL_TAIL, // (int Cell) -> int. List
	SYS2_CELL_TYPE, // (int Cell) -> int. Tag
	SYS2_CELL_VALUE,// (int Cell) -> int. Value as string
	SYS2_LIST,      // (int Content) -> int.
	SYS2_LIST_EMPTY,// (int List) -> 1 | 0.
	SYS2_LIST_FREE, // (int List) -> void.
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

#endif

#endif
