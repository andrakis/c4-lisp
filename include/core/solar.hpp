/* 
 * File:   solar.hpp
 * Author: daedalus
 *
 * Created on 29 September 2018, 11:12 PM
 *
 * SOLAR: Shared Object Loader At Runtime
 * 
 * Allows for loading of symbols from shared objects.
 * Provides an unified interface for portability.
 */

#ifndef SOLAR_HPP
#define SOLAR_HPP

#include "core/native.h"
#include "core/except.h"

#ifdef __cplusplus
#if TARGET_GCC
#include <dlfcn.h>
#endif

#include <utility>

namespace C4 {
	namespace Solar {
		GENERIC_EXCEPTION(library_notfound)
		GENERIC_EXCEPTION(symbol_notfound)
	
		class Library;
		typedef void *LibraryHandle;
		typedef void *LibrarySymbol;

		template<typename FuncType>
		class Func {
			typedef FuncType base_type;
			typedef FuncType *ptr_type;

			const ptr_type func;

			Func(void *lib, const char *name)
				: func(get_dlsym(lib, name)) {
			}

			ptr_type get_dlsym(void *handle, const char *name) const {
#if TARGET_GCC
				void *sym;
				char *error;

				dlerror(); // clear existing
				sym = dlsym(handle, name);
				error = dlerror();
				if(error) throw symbol_notfound(error);
#else
#error "C4::Solar: Platform not supported"
#endif
				return reinterpret_cast<ptr_type>(sym);
			}
		public:
			Func(const Library &, const char *name);

			template< typename ... Args>
			auto operator()(Args ... args) {
				return func(args...);
			}
		};

		class Library {
			void *handle;
			template<typename FuncType> friend class Func;

		public:
			static const int FLAGS_DEFAULT = RTLD_LAZY;

			Library(const char *lib, int flags = FLAGS_DEFAULT) {
				dlerror(); // clear existing
				handle = dlopen(lib, flags);
				if(!handle) throw library_notfound(dlerror());
			}
			~Library() {
				dlclose(handle);
			}

			template<typename FuncType>
			Func<FuncType> lookup(const char *name) {
				return Func<FuncType>(*this, name);
			}
		};

		template<typename FuncType>
		Func<FuncType>::Func(const Library &lib, const char *name) 
			: func(get_dlsym(lib.handle, name)) {
		}
	}
}
extern "C" {

#endif




#ifdef __cplusplus
}
#endif

#endif /* SOLAR_HPP */

