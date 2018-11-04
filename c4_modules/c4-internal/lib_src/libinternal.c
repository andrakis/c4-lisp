#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CONF_Debug 1
#include "core/c4_module.h"
#include "core/syscalls.h"

#ifdef __cplusplus
extern "C" {
#endif
// Import C4 symbols
extern char *opcodes;
extern char *p, *data;
extern NUMTYPE *e, *sym, debug, verbose;
// TODO: use with care
extern NUMTYPE *vm_processes, *vm_active_process, vm_proc_max, vm_proc_count, vm_cycle_count;
// Syscall1 functions
void     setup_opcodes();
void     setup_symbols();
NUMTYPE  process_new();
NUMTYPE *get_vm_active_process();
// Syscall2 functions
void    *get_process_platform(NUMTYPE *process);
NUMTYPE  parse(char *source);
void change_process(NUMTYPE *process);
// Syscall3 functions
void     set_process_platform(void *platform, NUMTYPE *process);
NUMTYPE *lookup_symbol(char *name, NUMTYPE *symbols);
NUMTYPE  run_cycle(NUMTYPE *process, NUMTYPE cycles);
// Syscall4 functions
NUMTYPE *create_process(char *source, int argc, char **argv);
// Syscall5 functions
void     process_setmeta(NUMTYPE *process, NUMTYPE *sym, NUMTYPE *e, char *data);
void     process_setregisters(NUMTYPE *process, NUMTYPE *bp, NUMTYPE *sp, NUMTYPE *pc);
// Syscall6 functions
void     process_init(NUMTYPE *process, NUMTYPE *sym, NUMTYPE *code, char *data, NUMTYPE *sp);
#ifdef __cplusplus
}
#endif


unsigned initialized = 0;

// These enums can be copy/pasted into C4 source.
enum /* Syscalls1 */ {
	SYS1_GET_OPCODES_S, // get opcodes as char*
	SYS1_GET_SYMBOLS_S, // get symbols as char*
	SYS1_GET_SYMBOLCNT, // get symbol count
	SYS1_C4_PROC_NEW,   // process_new()
	SYS1_C4_VM_PROC,    // get_vm_active_process()
	_SYS1_END
};

enum /* Syscalls2 */ {
	SYS2_C4_PROC_PLATFGET, // int *get_process_platform(int *process)
	SYS2_C4_PARSE,         // int  parse(char *code)
	SYS2_C4_PROC_CHANGE,   // void change_process(int *process)
	_SYS2_END
};

enum /* Syscalls3 */ {
	SYS3_C4_PROC_PLATFSET, // void set_process_platform(void *platform, int *process)
	SYS3_C4_LOOKUP_SYMBOL, // int *lookup_symbol(char *name, int *symbols)
	SYS3_C4_RUN_CYCLE,     // int  run_cycle(int *process, int cycles)
	_SYS3_END
};

enum /* Syscalls4 */ {
	// PROC_CREATE: create a process by parsing given code, pushing given arguments,
	//              and setting entry point to symbol "main".
	SYS4_C4_PROC_CREATE,   // int *create_process(char *source, int argc, char **argv)
	_SYS4_END
};

enum /* Syscalls5 */ {
	SYS5_C4_PROC_SETMETA,  // void process_setmeta     (int *process, int *sym, int *e, char *data)
	SYS5_C4_PROC_SETREGS,  // void process_setregisters(int *process, int *bp, int *sp, int *pc)
	_SYS5_END
};

enum /* Syscalls6 */ {
	SYS6_C4_PROC_INIT,     // void process_init(int *process, int *sym, int *code, char *data, int *sp)
	_SYS6_END
};

enum { Stdin, Stdout, Stderr };

#define OPCODES_MAX  1024
#define SYMBOLS_MAX  (OPCODES_MAX * 5)
#define MIN(A,B)     ((A) < B ? A : B)
#define MIN_STR(S,M) (MIN(strlen(S), M))

struct {
	char opcodes[OPCODES_MAX];
	char symbols[SYMBOLS_MAX];
	unsigned symbol_count;
} runtime_info;

unsigned count_symbols (const char *syms) {
	unsigned count = 0;

	while(*syms)
		if(*syms++ == ' ') ++count;
	++count; // for item at end

	return count;
}

void initialize () {
	char *__restrict tmp;
	setup_opcodes();
	setup_symbols(); // puts results in p
	tmp = (char *__restrict)&runtime_info.opcodes;
	strncpy(tmp, opcodes, MIN_STR(opcodes, OPCODES_MAX));
	tmp = (char *__restrict)&runtime_info.symbols;
	strncpy(tmp, p, MIN_STR(p, SYMBOLS_MAX));
	runtime_info.symbol_count = count_symbols(tmp);
}

NUMTYPE syscall_init(NUMTYPE section, NUMTYPE endmarker) {
	if(!initialized) {
		initialize();
		initialized = 1;
	}
	switch(section) {
		case 1: return endmarker == _SYS1_END;
		case 2: return endmarker == _SYS2_END;
		case 3: return endmarker == _SYS3_END;
		case 4: return endmarker == _SYS4_END;
		case 5: return endmarker == _SYS5_END;
		case 6: return endmarker == _SYS6_END;
		default: return 0;
	}
}

NUMTYPE syscall1(NUMTYPE signal) {
	switch(signal) {
		case SYS1_GET_OPCODES_S: return (NUMTYPE)&runtime_info.opcodes;
		case SYS1_GET_SYMBOLS_S: return (NUMTYPE)&runtime_info.symbols;
		case SYS1_GET_SYMBOLCNT: return (NUMTYPE) runtime_info.symbol_count;
		case SYS1_C4_PROC_NEW  : return (NUMTYPE)process_new();
		case SYS1_C4_VM_PROC   : return (NUMTYPE)get_vm_active_process();
		default:
			dprintf(Stderr, "Invalid syscall1: %ld\n", (unsigned long)signal);
			exit(1);
	}
}

NUMTYPE syscall2(NUMTYPE signal, NUMTYPE arg1) {
	switch(signal) {
		case SYS2_C4_PROC_PLATFGET:
			return (NUMTYPE)get_process_platform((NUMTYPE*)arg1);
		case SYS2_C4_PARSE:
			return parse((char*)arg1);
		case SYS2_C4_PROC_CHANGE:
			change_process((NUMTYPE*)arg1);
			return 0;
		default:
			dprintf(Stderr, "Invalid syscall1: %ld\n", (unsigned long)signal);
			exit(1);
	}
}

NUMTYPE syscall3(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2) {
	switch(signal) {
		case SYS3_C4_PROC_PLATFSET:
			set_process_platform((void*)arg1, (NUMTYPE*)arg2);
			return 0;
		case SYS3_C4_LOOKUP_SYMBOL:
			return (NUMTYPE)lookup_symbol((char *)arg1, (NUMTYPE*)arg2);
		case SYS3_C4_RUN_CYCLE:
			return run_cycle((NUMTYPE*)arg1, arg2);
		default:
			dprintf(Stderr, "Invalid syscall1: %ld\n", (unsigned long)signal);
			exit(1);
	}
}

NUMTYPE syscall4(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3) {
	switch(signal) {
		case SYS4_C4_PROC_CREATE:
			return (NUMTYPE)create_process((char*)arg1, arg2, (char**)arg3);
		default:
			dprintf(Stderr, "Invalid syscall1: %ld\n", (unsigned long)signal);
			exit(1);
	}
}

NUMTYPE syscall5(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3, NUMTYPE arg4) {
	switch(signal) {
		case SYS5_C4_PROC_SETMETA:
			process_setmeta((NUMTYPE*)arg1, (NUMTYPE*)arg2, (NUMTYPE*)arg3, (char*)arg4);
			return 0;
		case SYS5_C4_PROC_SETREGS:
			process_setregisters((NUMTYPE*)arg1, (NUMTYPE*)arg2, (NUMTYPE*)arg3, (NUMTYPE*)arg4);
			return 0;
		default:
			dprintf(Stderr, "Invalid syscall1: %ld\n", (unsigned long)signal);
			exit(1);
	}
}

NUMTYPE syscall6(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3, NUMTYPE arg4, NUMTYPE arg5) {
	switch(signal) {
		case SYS6_C4_PROC_INIT:
			process_init((NUMTYPE*)arg1, (NUMTYPE*)arg2, (NUMTYPE*)arg3, (char*)arg4, (NUMTYPE*)arg5);
			return 0;
		default:
			dprintf(Stderr, "Invalid syscall1: %ld\n", (unsigned long)signal);
			exit(1);
	}
}
