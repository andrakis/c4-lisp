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
		std::string _message;

	public:
		generic_exception(const std::string name)
			: _message(name) { }

		generic_exception(const std::string name, const std::string message)
			: _message(name + ": " + message) { }
		const char *what() const noexcept { return _message.c_str(); }
	};

#define QUOTE(Val) #Val

#define GENERIC_EXCEPTION(Name) \
	class Name : public generic_exception { \
	public: \
	Name() : generic_exception(QUOTE(Name), QUOTE(Name)) { } \
	Name(std::string msg) : generic_exception(QUOTE(Name), msg) { } \
	};
}

extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* EXCEPT_H */

