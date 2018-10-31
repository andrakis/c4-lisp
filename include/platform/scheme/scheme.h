#ifndef __PLATFORM_SCHEME_H
#define __PLATFORM_SCHEME_H

#include "core/native.h"
#include "platform/scheme/cell.h"

#ifdef __cplusplus
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Forward declarations
struct cell;
struct environment; // forward declaration; cell and environment reference each other
typedef std::shared_ptr<environment> env_p;

typedef std::vector<cell> cells;
typedef cells::const_iterator cellit;

std::string to_string(const cell &exp);
std::string to_string(const cells &exp);
std::string to_string(const environment &exp);

// a variant that can hold any kind of lisp value
////////////////////// cell
struct cell {
	typedef cell(*proc_type)(const std::vector<cell> &);
	typedef std::vector<cell>::const_iterator iter;
	typedef std::map<std::string, cell> map;
	cell_type type;
	std::string val;
	cells list;
	proc_type proc;
	env_p env;
	cell(cell_type type = Symbol) : type(type), val(""), list(), env(0) {}
	cell(cell_type type, const std::string & val) : type(type), val(val), list(), env(0) {}
	cell(const cells &lst) : type(List), val(""), list(lst), env(0) {}
	cell(proc_type proc) : type(Proc), proc(proc), env(0) {}
	void reset(const cell &c) {
		type = c.type;
		val = c.val;
		list = c.list;
		env = c.env;
	}
	bool is_list() const { return type == List || type == Lambda; }
	bool is_empty() const { return is_list() ? list.empty() : true; }
	size_t list_size() const { return is_list() ? list.size() : 0; }
	std::string str() const { return to_string(*this); }
	bool operator==(const cell &other) const {
		//if(other == this) return true;
		if(other.type != type) return false;
		switch(type) {
			case Number: case Symbol:
				return val == other.val;
			case List: case Lambda:
				return list == other.list && (type == Lambda ? (env == other.env) : true);
			case Proc:
				return proc == other.proc;
			default:
				return false;
		}
	}
	bool operator!=(const cell &other) const {
		return !(*this == other);
	}
};

inline bool endswith(const std::string &search, const std::string &str) {
	if(str.length() < search.length()) return false;
	auto it1 = str.cend() - search.length();
	auto it2 = search.cbegin();
	for ( ; it1 != str.cend() && it2 != search.cend(); ++it1, ++it2) {
		if(*it1 != *it2) return false;
	}
	return true;
}

////////////////////// environment

// a dictionary that (a) associates symbols with cells, and
// (b) can chain to an "outer" dictionary
struct environment {
	environment(env_p outer = 0) : outer_(outer) {}

	environment(const cell & _parms, const cells & args, env_p outer)
		: outer_(outer)
	{
		const cells &parms = _parms.list;
		cellit a = args.begin();
		for (cellit p = parms.begin(); p != parms.end(); ++p)
			env_[p->val] = *a++;
	}

	// map a variable name onto a cell
	typedef std::map<std::string, cell> map;

	// return a reference to the innermost environment where 'var' appears
	map & find(const std::string & var)
	{
		if (env_.find(var) != env_.end())
			return env_; // the symbol exists in this environment
		if (outer_)
			return outer_->find(var); // attempt to find the symbol in some "outer" env
		std::cout << "unbound symbol '" << var << "'\n";
		exit(1);
	}

	bool has(const std::string &var) const {
		if (env_.find(var) != env_.end())
			return true;
		if (outer_)
			return outer_->has(var);
		return false;
	}

	// return a reference to the cell associated with the given symbol 'var'
	cell & operator[] (const std::string & var)
	{
		return env_[var];
	}

	std::string str() const {
		std::string s("{");
		for(auto it = env_.cbegin(); it != env_.cend(); ++it)
			s += it->first + ": " + to_string(it->second) + ", ";
		s += "PARENT: ";
		if(outer_) {
			s += outer_->str();
		} else {
			s += "none";
		}
		s += "}";
		return s;
	}

private:
	map env_; // inner symbol->cell mapping
	env_p outer_; // next adjacent outer env, or 0 if there are no further environments
};

extern "C" {
	
#endif

const char *c_str(NUMTYPE cell);
NUMTYPE c_isdig(char c);
NUMTYPE c_cell_new(NUMTYPE tag, const char *value);
NUMTYPE c_cell_copy(NUMTYPE _cell);
NUMTYPE c_cell_env_get(NUMTYPE _cell);
NUMTYPE c_cell_env_set(NUMTYPE _env, NUMTYPE _cell);
NUMTYPE c_cell_list(NUMTYPE _cell);
NUMTYPE c_cell_strcmp(const char *s, NUMTYPE _cell);
NUMTYPE c_cell_type(NUMTYPE _cell);
NUMTYPE c_cell_value(NUMTYPE _cell); // const char *
NUMTYPE c_list(NUMTYPE _content);
NUMTYPE c_list_empty(NUMTYPE _list);
NUMTYPE c_list_size(NUMTYPE _list);
NUMTYPE c_list_index(NUMTYPE index, NUMTYPE _list) NOEXCEPT;
NUMTYPE c_list_push_back(NUMTYPE _cell, NUMTYPE _list);
NUMTYPE c_atom_false();
NUMTYPE c_atom_true();
NUMTYPE c_atom_nil();

NUMTYPE c_free_cell(NUMTYPE _cell);
NUMTYPE c_free_env(NUMTYPE _env);

NUMTYPE c_environment(NUMTYPE _outer);
NUMTYPE c_env_get(const char *_name, NUMTYPE _env);
NUMTYPE c_env_has(const char *_name, NUMTYPE _env);
NUMTYPE c_env_set(const char *_name, NUMTYPE _value, NUMTYPE _env);

NUMTYPE c_add_globals(NUMTYPE _env);
NUMTYPE c_call_proc(NUMTYPE _cell, NUMTYPE _args);

NUMTYPE c_parse(const char *code, NUMTYPE _dest) NOEXCEPT;
NUMTYPE lispmain(NUMTYPE, char **);

NUMTYPE internal_syscall1(NUMTYPE);
NUMTYPE internal_syscall2(NUMTYPE, NUMTYPE);
NUMTYPE internal_syscall3(NUMTYPE, NUMTYPE, NUMTYPE);
NUMTYPE internal_syscall4(NUMTYPE, NUMTYPE, NUMTYPE, NUMTYPE);

// scheme syscalls

#ifndef SYSCALLS
#define SYSCALLS
// enum Syscalls1 {
// syscalls that take no argument (the signal only)
enum {
	SYS1_ATOM_FALSE, // void -> int. get pointer to false atom
	SYS1_ATOM_TRUE,  // void -> int. get pointer to true atom
	SYS1_ATOM_NIL,   // void -> int. get pointer to nil atom
	SYS1_CELL_NEW,   // void -> int. Create new empty cell
	_SYS1_END        // Must be last element
};
// };

// enum Syscalls2 {
// syscalls that take 1 argument (the signal, 1 arg)
enum {
	SYS2_ISDIG,  // (char Digit) -> 1 | 0. check if is digit
	SYS2_CELL_NEW_STR, // (char *str) -> int. Cell
	SYS2_CELL_COPY, // (int Cell) -> int. copy a cell from a pointer
	SYS2_CELL_EMPTY,// (int Cell) -> 1 | 0. check if empty
	SYS2_CELL_ENV_GET, // (int Cell) -> int. get pointer to cell's env
	SYS2_CELL_FREE, // (int Cell) -> void. Free a cell object
	SYS2_CELL_FRONT, // (int Cell) -> int. Cell
	SYS2_CELL_LIST, // (int Cell) -> int.
	SYS2_CELL_SIZE, // (int Cell) -> int.
	SYS2_CELL_CSTR, // (int Cell) -> char*.
	SYS2_CELL_TYPE, // (int Cell) -> int. Tag
	SYS2_CELL_VALUE,// (int Cell) -> int. Value as string
	SYS2_LIST,      // (int Content) -> int.
	SYS2_LIST_CSTR, // (int List) -> char*.
	SYS2_LIST_EMPTY,// (int List) -> 1 | 0.
	SYS2_LIST_FREE, // (int List) -> void.
	SYS2_LIST_NEW,  // (int List|0) -> int. List
	SYS2_LIST_SIZE, // (int List) -> int.
	SYS2_ENV,       // (int Outer) -> int.
	SYS2_ENV_CSTR,  // (int Env) -> char.
	SYS2_FREE_ENV,  // (int Env) -> void. Free an environment object
	SYS2_ADD_GLOBS,  // (int Env) -> void. Add global symbols to given env
	SYS2_CSTR_FREE, // (char *str) -> void. Free a C string
	_SYS2_END        // Must be last element
};
// };

// enum Syscalls3 {
// syscalls that take 2 arguments (the signal, 2 args)
enum {
	SYS3_CELL_ENV_SET,// (int Env, int Cell) -> void.
	SYS3_CELL_INDEX,  // (int Index, int Cell) -> int. Cell
	SYS3_CELL_NEW,    // (int Tag, char *Value) -> int. Cell
	SYS3_CELL_NEW_STRN,//(char *str, int n) -> int. Cell
	SYS3_CELL_RESET,  // (int Source, int Dest) -> void. Reset cell value to Source values
	SYS3_CELL_SETENV, // (int Env, int Cell) -> int. Cell
	SYS3_CELL_SETTYPE,// (int Type, int Cell) -> int. Cell
	SYS3_CELL_STRCMP, // (char *s, int Cell) -> 0 | 1. Returns 0 on match, like strmp
	SYS3_CELL_TAIL,   // (int Cell, int Dest) -> void.
	SYS3_LIST_INDEX,  // (int Index, int List) -> int.Cell.
	SYS3_LIST_PUSHB,  // (int Cell, int List) -> List.
	SYS3_ENV_GET,     // (char *Name, int Env) -> int.Cell.
	SYS3_ENV_HAS,     // (char *Name, int Env) -> 0 | 1.
	SYS3_ENV_LOOKUP,  // (int Cell, int Env) -> int. Cell
	SYS3_PARSE,       // (char *Code, int Cell) -> int. 0 success, 1 failure
	SYS3_LISP_MAIN,   // (int Argc, char **Argv) -> int. Return code
	_SYS3_END         // Must be last element
};
// };

// enum Syscalls4 {
// syscalls that take 3 arguments (the signal, 3 args)
enum {
	SYS4_CALL_PROC,   // (int Cells::List, int Cell, int Dest) -> void.
	SYS4_ENV_SET,     // (char *Name, int Cell, int Env) -> Env.
	SYS4_ENV_NEWARGS, // (int Names::list, int Values::list, int Parent::env) -> int. Env
	_SYS4_END        // Must be last element
};
// };

#endif

#ifdef __cplusplus
}
#endif
#endif
