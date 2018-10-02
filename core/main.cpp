/* 
 * File:   main.cpp.cpp
 * Author: daedalus
 *
 * Created on 20 July 2018, 9:28 PM
 */

#include <typeinfo>
#include <exception>
#include <stdexcept>
#include <iostream>
#include "core/stacktrace.h"
#include "core/except.h"

extern "C" {
	int c5_main(int argc, char **argv);
}

#define USE_TRY 1
#define CATCH_C4_EXCEPTIONS 1

#define __DO(Code) do { Code; } while(0)

#if USE_TRY
#define TRY(Code)  try { __DO(Code); }
#else
#define TRY(Code)  __DO(Code);
#undef CATCH_C4_EXCEPTIONS
#endif

#if CATCH_C4_EXCEPTIONS
#define CATCH_EXCEPT(Name, Code) catch (Name) { __DO(Code); }
#define CATCH_EXCEPT_ELSE(Code) catch (...) { __DO(Code); }
#else
#define CATCH_EXCEPT(Name, Code)
#define CATCH_EXCEPT_ELSE(Code)
#endif

/*
 * C++ entry point.
 */
int main(int argc, char** argv) {
	TRY({
		return c5_main(argc, argv);
	})
	CATCH_EXCEPT(const StacktraceException &se, {
		std::cerr << "Stacktrace Exception: " << se.what() << std::endl;
		return -1;
	})
	CATCH_EXCEPT(const C4::generic_exception &ge, {
		std::cerr << "C4 Exception " << ge.what() << std::endl;
		return -1;
	})
	CATCH_EXCEPT_ELSE({
		std::exception_ptr p = std::current_exception();
		std::cerr << "other C++ exception thrown: " <<
				(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
		return -1;
	})
	return 0;
}

