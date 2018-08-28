#include <exception>
#include <iostream>
#include <sstream>
#include "stacktrace.h"

// Normal constructor
StacktraceException::StacktraceException(bpstd::string_view m) {
	message = std::string(m.cbegin(), m.cend());
#if TARGET_GCC
	frames = backtrace(callstack, STACKTRACE_MAXDEPTH);
#elif TARGET_WIN
#endif
}

// Copy constructor
StacktraceException::StacktraceException(const StacktraceException &se) {
	message = se.message;
	frames = se.frames;
	for(int i = 0; i < STACKTRACE_MAXDEPTH; ++i) {
		callstack[i] = se.callstack[i];
	}
}

StacktraceException::~StacktraceException() {
#if TARGET_GCC
#elif TARGET_WIN
#endif
}

void StacktraceException::printStackTrace() {
	std::cout
		<< "Exception: " << message << std::endl
		<< "Stack follows:" << std::endl;
#if TARGET_GCC
	char **symbols = backtrace_symbols(callstack, frames);
	for(int i = 0; i < frames; ++i) {
		std::cout << symbols[i] << std::endl;
	}
#elif TARGET_WIN
#endif
}