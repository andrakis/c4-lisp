/* 
 * File:   stacktrace.h
 * Author: daedalus
 *
 * Created on 29 August 2018, 12:24 AM
 */

#ifndef STACKTRACE_H
#define STACKTRACE_H

#include "native.h"

#if TARGET_WIN
#elif TARGET_GCC
	#include <execinfo.h>
#endif /* TARGET_WIN | TARGET_GCC */

/* Maximum depth of stacktrace */
#ifndef STACKTRACE_MAXDEPTH
	#define STACKTRACE_MAXDEPTH 128
#endif

#ifdef __cplusplus
#include <exception>
#include <memory>
#include <sstream>
#include <bpstd/string_view.hpp>

class StacktraceException : public std::exception {
	std::string message;
public:
	StacktraceException(bpstd::string_view reason) {
		std::stringstream ss;
		ss << "Exception: " << reason << std::endl;
	#if TARGET_GCC
		int frames;
		void *callstack[STACKTRACE_MAXDEPTH];
		frames = backtrace(callstack, STACKTRACE_MAXDEPTH);
		ss << "Stack follows:" << std::endl;
		char **symbols = backtrace_symbols(callstack, frames);
		for(int i = 0; i < frames; ++i) {
			ss << symbols[i] << std::endl;
		}
	#elif TARGET_WIN
		ss << "Stack trace unavailable on this platform." << std::endl;
	#endif
		message = ss.str();
	}
	StacktraceException(const StacktraceException &other) {
		message = other.message;
	}
	~StacktraceException() { }
	const char *what() const noexcept {
		return message.c_str();
	}
};

extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STACKTRACE_H */