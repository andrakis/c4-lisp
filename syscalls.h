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

#ifdef __cplusplus
}
#endif

#ifndef SYSCALLS
#define SYSCALLS
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
	SYS2_CELL_VALUE,// (int Cell) -> int. Value (const char*)
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

#endif

#endif
