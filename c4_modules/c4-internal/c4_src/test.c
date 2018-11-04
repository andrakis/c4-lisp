#include <stdio.h>

#include "core/c5_native.h"
#include "core/syscalls.h"

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

int main (int argc, char **argv) {
	char *ops, *syms;
	int count;

	if(platform_init(runtime_path("c4-internal"))) {
		dprintf(Stderr, "Failed to init library `internal`\n");
		return 1;
	}

	if(!syscall_init(1, _SYS1_END)) {
		dprintf(Stderr, "Syscall init failed\n");
		return 2;
	}

	ops  = (char*)syscall1(SYS1_GET_OPCODES_S);
	syms = (char*)syscall1(SYS1_GET_SYMBOLS_S);
	count= (int)syscall1(SYS1_GET_SYMBOLCNT);

	printf("*** Opcodes: ***\n%s\n", ops);
	printf("*** Symbols: ***\n%s\n", syms);
	printf("Count: %ld\n", count);

	return 0;
}
