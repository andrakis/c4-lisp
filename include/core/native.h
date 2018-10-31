/**
 * Detection of compilation settings, defines if appropriate:
 *   ENV32BIT  : 32bit executable
 *   ENV64BIT  : 64bit executable
 *   NUMTYPE   : appropriate number type (int or long) depending on above
 *   TARGET_WIN: compiling for windows
 *   TARGET_GCC: compiling with gcc
 *   TARGET_GLIBC: compiling for glibc (not all Linux platforms use it)
 *   NOEXCEPT  : on C: (empty), on C++: noexcept
 * These allow for platform handling behaviour based on compiler.
 *
 * The following more specific defines are presently not used for other
 * purposes.
 *   HOST_LINUX
 *   HOST_IPHONE
 *   HOST_OSX
 *   HOST_LINUX
 *   HOST_UNIX
 *   HOST_POSIX
 *   HOST_ANDROID
 * */
#ifndef __NATIVE_H
#define __NATIVE_H

// Determine configuration from CONF_ macros
#if !defined(DEBUG) && !defined(NDEBUG) && !defined(RELEASE)
#if CONF_Debug
#define DEBUG 1
#elif CONF_Release
#define RELEASE 1
#else
#error Could not ascertain build mode (CONF=Release|Debug)
#endif
#endif

#ifdef __cplusplus
	#define NOEXCEPT noexcept
#else
	#define NOEXCEPT
#endif

/* Determine whether we're in 32 or 64bit mode */
#if !defined(ENV64BIT) && !defined(ENV32BIT)
	/* Check windows */
	#if _WIN32 || _WIN64
		#if _WIN64
			#define ENV64BIT 1
		#else
			#define ENV32BIT 1
		#endif
	#endif

	/* Check GCC */
	#if __GNUC__
		#if __x86_64__ || __ppc64__
			#define ENV64BIT 1
		#else
			#define ENV32BIT 1
		#endif
	#endif
#endif /* !ENV64BIT && !ENV32BIT */

/* Number type definition or overrides */
#ifdef ENV64BIT
	#define NUMTYPE long
	/* HACK: int is not big enough for 64bit environment.
	 *			 however, it is tricky to rewrite the interpreter to use pointers.
	 */
	#ifdef C5_NATIVE
		#define int long
	#endif
#else
	#define NUMTYPE int
#endif

/* Compiler / OS detection */
#if defined(_WIN64) || defined(_WIN32)
	#define TARGET_WIN 1
#elif __APPLE__
	#define TARGET_GCC 1
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
		#define TARGET_IPHONE 1
    #else
        #define TARGET_OSX 1
    #endif
#elif __ANDROID__
	#define TARGET_GCC 1
	#define TARGET_ANDROID 1
#elif __GLIBC__
	#define TARGET_GCC 1
	#define TARGET_GLIBC 1
	#define TARGET_LINUX 1
#elif __linux
    /* linux */
	#define TARGET_GCC 1
	#define TARGET_LINUX 1
#elif __unix /* all unices not caught above */
    /* Unix */
	#define TARGET_UNIX 1
#elif __posix
    /* POSIX */
	#define TARGET_GCC 1
	#define TARGET_POSIX 1
#endif

#if TARGET_GCC
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

/* Final checks */
#if !defined(ENV32BIT) && !defined(ENV64BIT)
#error "Unable to detect architecture"
#endif
#if !defined(TARGET_WIN) && !defined(TARGET_GCC)
#error "Unable to detect compiler (valid: windows, gcc)"
#endif

#ifndef CND_PLATFORM
	#if TARGET_LINUX
		#define CND_PLATFORM "GNU-Linux"
	#else
		#error "Unable to determine CND_PLATFORM"
	#endif
#endif // CND_PLATFORM

#ifndef CND_CONF
	#if DEBUG && !NDEBUG
		#define CND_CONF "Debug"
	#else
		#define CND_CONF "Release"
	#endif
#endif

#if TARGET_GCC
#define RTL_SO "so"
#include "core/gcc-settings.h"
#else
#error "Unable to determine shared object extension for this platform"
#endif

#endif
