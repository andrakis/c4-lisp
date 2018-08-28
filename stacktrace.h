/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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
#include <bpstd/string_view.hpp>

class StacktraceException : public std::exception {
	#if TARGET_GCC
		int frames;
		void *callstack[STACKTRACE_MAXDEPTH];
	#endif /* TARGET_GCC */
	std::string message;
public:
	StacktraceException(bpstd::string_view);
	StacktraceException(const StacktraceException &);
	virtual ~StacktraceException();
	void printStackTrace();
};

extern "C" {
#endif /* __cplusplus */

void c_st_throw(char*);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STACKTRACE_H */