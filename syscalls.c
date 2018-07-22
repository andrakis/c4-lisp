//
#include <stdlib.h>
#include <stdio.h>
#include "native.h"
#include "syscalls.h"
#include "internal.h"

#ifndef NUMTYPE
#define NUMTYPE long
#endif

NUMTYPE syscall1(NUMTYPE signal) {
	switch(signal) {
		case SYS1_ATOM_FALSE: return c_atom_false();
		case SYS1_ATOM_TRUE: return c_atom_true();
		case SYS1_ATOM_NIL: return c_atom_nil();
		default:
			printf("Invalid SYSCALL1: %d\n", signal);
			exit(-1);
	}
}

NUMTYPE syscall2(NUMTYPE signal, NUMTYPE arg1) {
	switch(signal) {
		case SYS2_ISDIG: return c_isdig((char)arg1);
		case SYS2_CELL_COPY: return c_cell_copy(arg1);
		case SYS2_CELL_ENV_GET: return c_cell_env_get(arg1);
		case SYS2_CELL_LIST: return c_cell_list(arg1);
		case SYS2_CELL_TYPE: return c_cell_type(arg1);
		case SYS2_CELL_VALUE: return c_cell_value(arg1);
		case SYS2_LIST: return c_list(arg1);
		case SYS2_LIST_EMPTY: return c_list_empty(arg1);
		case SYS2_LIST_SIZE: return c_list_size(arg1);
		case SYS2_ENV: return c_environment(arg1);
		case SYS2_FREE_CELL: return c_free_cell(arg1);
		case SYS2_FREE_ENV: return c_free_env(arg1);
		case SYS2_ADD_GLOBS:
			c_add_globals(arg1);
			return 0; // void
		default:
			printf("Invalid SYSCALL2: %d\n", signal);
			exit(-1);
	}
}

NUMTYPE syscall3(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2) {
	switch(signal) {
		case SYS3_CELL_NEW: return c_cell_new(arg1, (const char*)arg2);
		case SYS3_CELL_STRCMP: return c_cell_strcmp((const char*)arg1, arg2);
		case SYS3_CELL_ENV_SET: return c_cell_env_set(arg1, arg2);
		case SYS3_LIST_INDEX: return c_list_index(arg1, arg2);
		case SYS3_LIST_PUSHB: return c_list_push_back(arg1, arg2);
		case SYS3_ENV_GET: return c_env_get((const char *)arg1, arg2);
		case SYS3_ENV_HAS: return c_env_has((const char *)arg1, arg2);
		case SYS3_CALL_PROC: return c_call_proc(arg1, arg2);
		default:
			printf("Invalid SYSCALL3: %d\n", signal);
			exit(-1);
	}
}

NUMTYPE syscall4(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3) {
	switch(signal) {
		case SYS4_ENV_SET: return c_env_set((const char*)arg1, arg2, arg3);
		default:
			printf("Invalid SYSCALL4: %d\n", signal);
			exit(-1);
	}
}
