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
		GENERIC_EXCEPTION(null_library)
		GENERIC_EXCEPTION(null_function)
	
		class Library;
		typedef void *LibraryHandle;
		typedef void *LibrarySymbol;

		template<typename FuncType>
		class Func {
		protected:
			typedef FuncType base_type;
			typedef FuncType *ptr_type;

			std::string func_name;
			const ptr_type func;
			Func(void *lib, const char *name)
				: func_name(name), 
				  func(get_dlsym(lib, name)) {
			}

			ptr_type get_dlsym(void *handle, const char *name) const {
#if TARGET_GCC
				void *sym;
				char *error;

				if(handle == nullptr)
					throw null_library();

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
			Func() : func_name("<null_f>"), func(0) { }
			Func(ptr_type fn) : func_name("<unnamed>"), func(fn) { }
			Func(ptr_type fn, const char *name)
				: func_name(name), func(fn) { }
			virtual ~Func() { }

			template<typename ... Args>
			auto operator()(Args ... args) {
				return func(args...);
			}
		};

		template<typename FuncType>
		class NullFunc : public Func<FuncType> {
		protected:
			using Func<FuncType>::func_name;

		public:
			NullFunc() { }
			NullFunc(const char *name) : Func<FuncType>(name) { }
			~NullFunc() { }

			template<typename ... Args>
			auto operator()(Args ... args) {
				throw null_function(func_name);
			}
		};

		class Library {
			void *handle;
			template<typename FuncType> friend class Func;

		public:
			static const int FLAGS_DEFAULT = RTLD_LAZY;

			Library() : handle(nullptr) { }
			Library(const char *lib, int flags = FLAGS_DEFAULT) {
				dlerror(); // clear existing
				handle = dlopen(lib, flags);
				if(!handle) throw library_notfound(dlerror());
			}
			~Library() {
				if(handle != nullptr)
					dlclose(handle);
			}

			template<typename FuncType>
			Func<FuncType> lookup(const char *name) {
				return Func<FuncType>(*this, name);
			}
		};

		template<typename FuncType>
		Func<FuncType>::Func(const Library &lib, const char *name) 
			: func_name(name),
			  func(get_dlsym(lib.handle, name))
		{
		}
	}
}
extern "C" {

#endif




#ifdef __cplusplus
}
#endif

#endif /* SOLAR_HPP */

