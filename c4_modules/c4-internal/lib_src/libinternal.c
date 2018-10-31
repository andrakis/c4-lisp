#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/c4_module.h"
#include "core/syscalls.h"

#ifdef __cplusplus
extern "C" {
#endif
// Import C4 symbols
extern char *opcodes;
extern char *p;
void setup_opcodes();
void setup_symbols();
#ifdef __cplusplus
}
#endif


unsigned initialized = 0;

enum /* Syscalls1 */ {
	SYS1_GET_OPCODES_S, // get opcodes as char*
	SYS1_GET_SYMBOLS_S, // get symbols as char*
	SYS1_GET_SYMBOLCNT, // get symbol count
	_SYS1_END
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
	tmp = &runtime_info.opcodes;
	strncpy(tmp, opcodes, MIN_STR(opcodes, OPCODES_MAX));
	tmp = &runtime_info.symbols;
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
		default: return 0;
	}
}

NUMTYPE syscall1(NUMTYPE signal) {
	switch(signal) {
		case SYS1_GET_OPCODES_S: return (NUMTYPE)&runtime_info.opcodes;
		case SYS1_GET_SYMBOLS_S: return (NUMTYPE)&runtime_info.symbols;
		case SYS1_GET_SYMBOLCNT: return (NUMTYPE) runtime_info.symbol_count;
		default:
			dprintf(Stderr, "Invalid syscall1: %ld\n", (unsigned long)signal);
			exit(1);
	}
}
