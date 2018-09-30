/* 
 * File:   except.h
 * Author: daedalus
 *
 * Created on 29 September 2018, 11:38 PM
 */

#ifndef EXCEPT_H
#define EXCEPT_H

#ifdef __cplusplus

#include <stdio.h>
#include <exception>
#include <string>

namespace C4 {
	class generic_exception : public std::exception {
		std::string message;

	public:
		generic_exception(const char *msg) : message(msg) {
		}
		generic_exception(const std::string msg) : message(msg) {
		}
		const char *what() const noexcept { return message.c_str(); }
	};

#define QUOTE(Val) #Val

#define GENERIC_EXCEPTION(Name) \
	class Name : public generic_exception { \
	public: \
	Name() : generic_exception(QUOTE(Name)) { } \
	Name(const char *msg) : generic_exception(msg) { } \
	Name(std::string msg) : generic_exception(msg) { } \
	};
}

extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* EXCEPT_H */

