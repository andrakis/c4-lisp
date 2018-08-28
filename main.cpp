/* 
 * File:   main.cpp.cpp
 * Author: daedalus
 *
 * Created on 20 July 2018, 9:28 PM
 */

#include "stacktrace.h"

#ifdef __cplusplus
extern "C" {
#endif

int c5_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

/*
 * C++ entry point.
 */
int main(int argc, char** argv) {
	try {
		return c5_main(argc, argv);
	} catch (StacktraceException se) {
		se.printStackTrace();
		return -1;
	}
	return 0;
}

