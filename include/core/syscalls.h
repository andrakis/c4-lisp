#ifndef __SYSCALLS
#define __SYSCALLS

#include "core/native.h"

#ifdef __cplusplus
extern "C" {
#endif
	void process_changed(NUMTYPE *process);

	NUMTYPE platform_init(const char *runtime) NOEXCEPT;

	NUMTYPE syscall1(NUMTYPE);
	NUMTYPE syscall2(NUMTYPE, NUMTYPE);
	NUMTYPE syscall3(NUMTYPE, NUMTYPE, NUMTYPE);
	NUMTYPE syscall4(NUMTYPE, NUMTYPE, NUMTYPE, NUMTYPE);
	NUMTYPE syscall_init(NUMTYPE section, NUMTYPE endmarker);

#ifdef __cplusplus
}
#endif

#endif
