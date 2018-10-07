//
#include <stdlib.h>
#include <stdio.h>
#include <map>

#include "core/native.h"
#include "core/extras.h"
#include "core/syscalls.h"
#include "core/solar.hpp"

#define VERBOSE 0
#if VERBOSE
#define VERB(Code) Code
#else
#define VERB(Code)
#endif

#ifndef NUMTYPE
#define NUMTYPE long
#endif

using namespace C4;

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
NUMTYPE null_main(NUMTYPE argc, char **argv) {
	throw Solar::null_function("syscall_main");
}

#if VERBOSE
struct verbose_constructors {
	verbose_constructors(const char *name) : _name(name) {
		dprintf(2, "vv: new %s()\n", _name);
	}
	virtual ~verbose_constructors() {
		dprintf(2, "vv: ~%s()\n", _name);
	}
private:
	const char *_name;
};

#define VERBOSE_CONSTRUCTORS(Name) verbose_constructors(Name),
#define VERBOSE_CONSTRUCTORS_INHERIT : verbose_constructors
#else
#define VERBOSE_CONSTRUCTORS(Name)
#define VERBOSE_CONSTRUCTORS_INHERIT
#endif

struct platform_runtime VERBOSE_CONSTRUCTORS_INHERIT {
	Solar::Library rtl;
	Solar::Func<syscall_init_t> syscall_init_f;
	Solar::Func<syscall1_t> syscall1_f;
	Solar::Func<syscall2_t> syscall2_f;
	Solar::Func<syscall3_t> syscall3_f;
	Solar::Func<syscall4_t> syscall4_f;
	Solar::Func<syscall_main_t> syscall_main_f;

	platform_runtime()
		: VERBOSE_CONSTRUCTORS("platform_runtime")
		  rtl(),
		  syscall_init_f(null_init),
		  syscall1_f(null_sys1),
		  syscall2_f(null_sys2),
		  syscall3_f(null_sys3),
		  syscall4_f(null_sys4),
		  syscall_main_f(null_main)
	{ }

	platform_runtime(const char *runtime)
		: VERBOSE_CONSTRUCTORS("platform_runtime")
		  rtl(runtime),
		  syscall_init_f(rtl, "syscall_init"),
		  syscall1_f(rtl, "syscall1"),
		  syscall2_f(rtl, "syscall2"),
		  syscall3_f(rtl, "syscall3"),
		  syscall4_f(rtl, "syscall4"),
		  syscall_main_f(rtl, "syscall_main", true)
	{
	}
};

struct platform_runtime  null_runtime;
struct platform_runtime *syscalls_runtime;

class runtime_map {
	typedef std::map<std::string,platform_runtime*> runtimes_map;

	runtimes_map runtimes;
	
public:
	runtime_map() {
		VERB(dprintf(2, "runtime_map()\n"));
	}
	~runtime_map() {
		VERB(dprintf(2, "~runtime_map()\n"));
		for(auto it = runtimes.begin(); it != runtimes.end(); ++it) {
			if(it->second == &null_runtime) continue;
			VERB(dprintf(2, "  ~runtime_map.delete(%s)\n", it->first.c_str()));
			delete it->second;
		}
	}
	bool empty() const { return runtimes.cbegin() != runtimes.cend(); }
	bool has_key(std::string name) const {
		runtimes_map::const_iterator it;

		it = runtimes.find(name);
		return it != runtimes.cend();
	}
	auto emplace(std::string name, platform_runtime *runtime) {
		return runtimes.emplace(name, runtime);
	}
	auto operator[](const std::string &name) {
		return runtimes[name];
	}
} runtimes;

void process_changed(NUMTYPE *process) {
	void *platform = 0;
	
	if(process != 0)
		platform = get_process_platform(process);
	if(platform == 0)
		syscalls_runtime = &null_runtime;
	else
		syscalls_runtime = reinterpret_cast<platform_runtime*>(platform);
}

NUMTYPE platform_init(const char *runtime) noexcept {
	NUMTYPE *p;

	if(runtimes.empty()) {
		// First init. Fixes issue with compilation on OR1000
		VERB(dprintf(2, "platform_init() first run\n"));
		runtimes.emplace("null", &null_runtime);
		syscalls_runtime = &null_runtime;
	}

	VERB(dprintf(2, "platform_init(%s)\n", runtime));
	try {
		if(runtime) {
			if(runtimes.has_key(runtime)) {
				syscalls_runtime = runtimes[runtime];
			} else {
				VERB(dprintf(2, "  new platform_runtime(%s)\n", runtime));
				syscalls_runtime = new platform_runtime(runtime);
				VERB(dprintf(2, "  emplace runtime\n"));
				runtimes.emplace(std::string(runtime), syscalls_runtime);
			}
		} else {
			VERB(dprintf(2, "  setting syscalls_runtime to null runtime\n"));
			syscalls_runtime = &null_runtime;
		}
		
		if((p = get_vm_active_process())) {
			VERB(dprintf(2, "  setting process platform\n"));
			set_process_platform(syscalls_runtime, p);
		}
		VERB(dprintf(2, "platform_init() complete\n"));
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

void *platform_get() NOEXCEPT {
	return reinterpret_cast<void*>(syscalls_runtime);
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

NUMTYPE syscall_main(NUMTYPE argc, char **argv) {
	if(syscalls_runtime->syscall_main_f.valid()) {
		return syscalls_runtime->syscall_main_f(argc, argv);
	} else {
		dprintf(2, "syscall_main: missing from library\n");
		return -1;
	}
}