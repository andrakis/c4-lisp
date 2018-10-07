#ifndef __SYSCALLS
#define __SYSCALLS

#include "core/native.h"

#ifdef __cplusplus
extern "C" {
#endif
	void process_changed(NUMTYPE *process);

	NUMTYPE platform_init(const char *runtime) NOEXCEPT;
	void   *platform_get() NOEXCEPT;

	NUMTYPE syscall_init(NUMTYPE section, NUMTYPE endmarker);
	NUMTYPE syscall1(NUMTYPE);
	NUMTYPE syscall2(NUMTYPE, NUMTYPE);
	NUMTYPE syscall3(NUMTYPE, NUMTYPE, NUMTYPE);
	NUMTYPE syscall4(NUMTYPE, NUMTYPE, NUMTYPE, NUMTYPE);
	NUMTYPE syscall_main(NUMTYPE argc, char **argv);

	typedef NUMTYPE (syscall_init_t)(NUMTYPE, NUMTYPE);
	typedef NUMTYPE (syscall1_t)(NUMTYPE);
	typedef NUMTYPE (syscall2_t)(NUMTYPE,NUMTYPE);
	typedef NUMTYPE (syscall3_t)(NUMTYPE,NUMTYPE,NUMTYPE);
	typedef NUMTYPE (syscall4_t)(NUMTYPE,NUMTYPE,NUMTYPE,NUMTYPE);
	typedef NUMTYPE (syscall_main_t)(NUMTYPE argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
