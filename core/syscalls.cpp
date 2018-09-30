//
#include <stdlib.h>
#include <stdio.h>

#include "core/native.h"
#include "core/extras.h"
#include "core/syscalls.h"
#include "core/solar.hpp"

#ifndef NUMTYPE
#define NUMTYPE long
#endif

using namespace C4;

typedef NUMTYPE (syscall_init_t)(NUMTYPE, NUMTYPE);
typedef NUMTYPE (syscall1_t)(NUMTYPE);
typedef NUMTYPE (syscall2_t)(NUMTYPE,NUMTYPE);
typedef NUMTYPE (syscall3_t)(NUMTYPE,NUMTYPE,NUMTYPE);
typedef NUMTYPE (syscall4_t)(NUMTYPE,NUMTYPE,NUMTYPE,NUMTYPE);

// Null platform implementation
NUMTYPE null_init(NUMTYPE a, NUMTYPE b) {
	throw Solar::null_function("syscall_init");
}
NUMTYPE null_sys1(NUMTYPE sig) {
	throw Solar::null_function("syscall1");
}
NUMTYPE null_sys2(NUMTYPE sig, NUMTYPE arg1) {
	throw Solar::null_function("syscall2");
}
NUMTYPE null_sys3(NUMTYPE sig, NUMTYPE arg1, NUMTYPE arg2) {
	throw Solar::null_function("syscall3");
}
NUMTYPE null_sys4(NUMTYPE sig, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3) {
	throw Solar::null_function("syscall4");
}

struct platform_runtime {
	Solar::Library rtl;
	Solar::Func<syscall_init_t> syscall_init_f;
	Solar::Func<syscall1_t> syscall1_f;
	Solar::Func<syscall2_t> syscall2_f;
	Solar::Func<syscall3_t> syscall3_f;
	Solar::Func<syscall4_t> syscall4_f;

	platform_runtime()
		: rtl(),
		  syscall_init_f(null_init),
		  syscall1_f(null_sys1),
		  syscall2_f(null_sys2),
		  syscall3_f(null_sys3),
		  syscall4_f(null_sys4)
	{ }

	platform_runtime(const char *runtime)
		: rtl(runtime),
		  syscall_init_f(rtl, "syscall_init"),
		  syscall1_f(rtl, "syscall1"),
		  syscall2_f(rtl, "syscall2"),
		  syscall3_f(rtl, "syscall3"),
		  syscall4_f(rtl, "syscall4")
	{
	}
};

struct platform_runtime  null_runtime;
struct platform_runtime *syscalls_runtime = &null_runtime;

void process_changed(NUMTYPE *process) {
	void *platform = get_process_platform(process);
	if(platform == 0)
		syscalls_runtime = &null_runtime;
	else
		syscalls_runtime = (platform_runtime*)platform;
}

NUMTYPE platform_init(const char *runtime) noexcept {
	NUMTYPE *p;

	//dprintf(2, "platform_init(%s)\n", runtime);
	try {
		if(runtime) {
			syscalls_runtime = new platform_runtime(runtime);
		} else {
			// free current runtime
			if(syscalls_runtime != &null_runtime)
				delete syscalls_runtime;
			syscalls_runtime = &null_runtime;
		}
		p = get_vm_active_process();
		if(p)
			set_process_platform(syscalls_runtime, p);
		return 0;
	} catch (Solar::library_notfound lnf) {
		dprintf(2, "Cannot load platform: %s\n", lnf.what());
		return 1;
	} catch (Solar::symbol_notfound snf) {
		dprintf(2, "Failed to load symbol: %s\n", snf.what());
		return 2;
	} catch (std::exception e) {
		dprintf(2, "Platform init failed: %s\n", e.what());
		return 3;
	}
}

NUMTYPE syscall1(NUMTYPE signal) {
    return syscalls_runtime->syscall1_f(signal);
}

NUMTYPE syscall2(NUMTYPE signal, NUMTYPE arg1) {
    return syscalls_runtime->syscall2_f(signal, arg1);
}

NUMTYPE syscall3(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2) {
    return syscalls_runtime->syscall3_f(signal, arg1, arg2);
}

NUMTYPE syscall4(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3) {
    return syscalls_runtime->syscall4_f(signal, arg1, arg2, arg3);
}

NUMTYPE syscall_init(NUMTYPE section, NUMTYPE endmarker) {
	return syscalls_runtime->syscall_init_f(section, endmarker);
}
