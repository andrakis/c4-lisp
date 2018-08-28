//
#include <stdlib.h>
#include <stdio.h>
#include "native.h"
#include "syscalls.h"
#include "internal.h"
#include "cell.h"

#ifndef NUMTYPE
#define NUMTYPE long
#endif

NUMTYPE syscall1(NUMTYPE signal) {
    return internal_syscall1(signal);
}

NUMTYPE syscall2(NUMTYPE signal, NUMTYPE arg1) {
    return internal_syscall2(signal, arg1);
}

NUMTYPE syscall3(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2) {
    return internal_syscall3(signal, arg1, arg2);
}

NUMTYPE syscall4(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3) {
    return internal_syscall4(signal, arg1, arg2, arg3);
}

NUMTYPE syscall_init(NUMTYPE section, NUMTYPE endmarker) {
	switch(section) {
		case 1: return endmarker == _SYS1_END;
		case 2: return endmarker == _SYS2_END;
		case 3: return endmarker == _SYS3_END;
		case 4: return endmarker == _SYS4_END;
		default: return 0;
	}
}
