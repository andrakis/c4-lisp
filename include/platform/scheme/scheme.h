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

#ifdef __cplusplus
}
#endif
#endif
