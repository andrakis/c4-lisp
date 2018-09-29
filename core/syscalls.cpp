//
#include <stdlib.h>
#include <stdio.h>

#include "core/native.h"
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

struct platform_runtime {
	Solar::Library *rtl;
	Solar::Func<syscall_init_t> syscall_init_f;
	Solar::Func<syscall1_t> syscall1_f;
	Solar::Func<syscall2_t> syscall2_f;
	Solar::Func<syscall3_t> syscall3_f;
	Solar::Func<syscall4_t> syscall4_f;

	platform_runtime(const char *runtime)
		: rtl(new Solar::Library(runtime)),
		  syscall_init_f(*rtl, "syscall_init"),
		  syscall1_f(*rtl, "syscall1"),
		  syscall2_f(*rtl, "syscall2"),
		  syscall3_f(*rtl, "syscall3"),
		  syscall4_f(*rtl, "syscall4")
	{
	}
};

struct platform_runtime *syscalls_runtime;

NUMTYPE platform_init(const char *runtime) noexcept {
	try {
		syscalls_runtime = new platform_runtime(runtime);
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
