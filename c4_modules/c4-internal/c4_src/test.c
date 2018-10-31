#include <stdio.h>

#include "core/c5_native.h"
#include "core/syscalls.h"

enum /* Syscalls1 */ {
	SYS1_GET_OPCODES_S, // get opcodes as char*
	SYS1_GET_SYMBOLS_S, // get symbols as char*
	SYS1_GET_SYMBOLCNT, // get symbol count
	_SYS1_END
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