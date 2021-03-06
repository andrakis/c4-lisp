#include <ctype.h>
#include <iostream>
#include <list>
#include <sstream>
#include <string.h>

#include <stdexcept> // for string_view on GCC < 5
#include "non-contrib/bpstd/string_view.hpp"

#include "core/native.h"
#include "core/stacktrace.h"
#include "core/syscalls.h"
#include "platform/scheme/scheme.h"

using namespace bpstd; // KDevelop refuses to belief this namespace exists

char *c_persistent_string(const std::string &s) {
	char *result;
	result = (char*)malloc(s.length() + 1);
	if(!result)
		return 0;
	strncpy(result, s.c_str(), s.length() + 1);
	return result;
}

NUMTYPE unhandled_event(string_view reason);
inline NUMTYPE unhandled_event(const std::stringstream &ss) {
	return unhandled_event(ss.str());
}

// return given mumber as a string
std::string str(NUMTYPE n) { std::ostringstream os; os << n; return os.str(); }
const char *c_str(NUMTYPE n) { return str(n).c_str(); }

// return true iff given character is '0'..'9'
bool isdig(char c) { return isdigit(static_cast<unsigned char>(c)) != 0; }

#ifdef DEBUG

// TODO? Might be useful for checking types given from C
const long _C_OBJ_MAGIC = 0xDEADBEEF;
static int _c_obj_uid_counter = 0;
struct _c_obj_container_base {
	template<typename T> T getAs() {
		return static_cast<T>(obj);
	}
	int uid;
	long magic;
protected:
	_c_obj_container_base(void *_obj) :
		uid(++_c_obj_uid_counter),
		magic(uid ^ _C_OBJ_MAGIC),
		obj(_obj) {
	}
	void *obj;
};

template<typename Inner>
struct _c_obj_container : public _c_obj_container_base {
	_c_obj_container(Inner value) :
		_c_obj_container_base((void*)value) {
	}
	Inner get() {
		return getAs<Inner>();
	}
};

template<typename T> T object_from_c(auto v) {
	void *vv = (void*)v;
	return static_cast<T>(vv);
}
NUMTYPE object_to_c(void *v) {
	return (NUMTYPE)v;
}
NUMTYPE object_to_c(const void *v) {
	return (NUMTYPE)v;
}
NUMTYPE number_from_c(auto v) {
	return (NUMTYPE)v;
}
NUMTYPE number_to_c(NUMTYPE v) {
	return v;
}
#define OBJ_FROM_C(type, value) object_from_c<type>(value)
#define OBJ_TO_C(value)         object_to_c(value)
#define NUMBER_FROM_C(value)    number_from_c(value)
#define NUMBER_TO_C(value)      number_to_c(value)
#else
#define OBJ_FROM_C(type, value) (static_cast<type>((void*)value))
#define OBJ_TO_C(value)         ((NUMTYPE)(value))
#define NUMBER_FROM_C(value)    ((NUMTYPE)(value))
#define NUMBER_TO_C(value)      ((NUMTYPE)(value))
#endif

#define CELL(value)             (OBJ_FROM_C(struct cell*, value))
#define CELLS(value)            (OBJ_FROM_C(cells*, value))
#define ENV(value)              (OBJ_FROM_C(struct environment*, value))
#define ENV_P(value)            (OBJ_FROM_C(env_p*, value))

NUMTYPE c_cell_new(NUMTYPE tag, const char *value) {
	cell *c = new cell((cell_type)tag, std::string(value));
	return OBJ_TO_C(c);
}

NUMTYPE c_cell_new_strn(const char *value, NUMTYPE n) {
	cell *c = new cell(Symbol, std::string(value, n));
	return OBJ_TO_C(c);
}

// Reset (copy) values from another cell
NUMTYPE c_cell_reset(NUMTYPE source, NUMTYPE dest) {
	cell *s = CELL(source);
	cell *d = CELL(dest);
	d->reset(*s);
	return 0;
}
NUMTYPE c_cell_copy(NUMTYPE _cell) {
	cell *c = CELL(_cell);
	cell *n = new cell(*c);
	return OBJ_TO_C(n);
}
NUMTYPE c_cell_empty(NUMTYPE _cell) {
    cell *c = CELL(_cell);
    return c->is_empty() ? 1 : 0;
}
NUMTYPE c_cell_env_get(NUMTYPE _cell) {
	cell *c = CELL(_cell);
	env_p *e = &c->env;
	return OBJ_TO_C(e);
}
NUMTYPE c_cell_env_set(NUMTYPE _env, NUMTYPE _cell) {
	env_p *env = ENV_P(_env);
	cell *c = CELL(_cell);
	c->env.swap(*env);
	return OBJ_TO_C(env);
}

NUMTYPE c_free_cell(NUMTYPE _cell) {
	cell *c = CELL(_cell);
	delete(c);
	return 0;
}

NUMTYPE c_call_proc(NUMTYPE _args, NUMTYPE _cell, NUMTYPE _dest) {
	cells *args = CELLS(_args);
	cell  *c = CELL(_cell);
	cell  *dest = CELL(_dest);
	
	// c->proc() returns a temporary
	const cell &res = c->proc(*args);
	dest->reset(res);
	return 0;
}

// Create a new cell of type list. Returns a cell*, not cells*
// c_list(cell*) -> cell*;
NUMTYPE c_list(NUMTYPE _content) {
	cell *content = CELL(_content);
	cell *c = new cell(List);
	if(content == 0) {
		c->list = cells();
	} else if(content->type == List) {
		c->list = cells(content->list);
	} else {
		c->list = cells();
		c->list.push_back(cell(*content));
	}
	return OBJ_TO_C(c);
}

const cell false_sym(Symbol, "#f");
const cell true_sym(Symbol, "#t"); // anything that isn't false_sym is true
const cell nil(Symbol, "nil");
const cell empty_sym(List);

NUMTYPE c_environment(NUMTYPE _outer) {
	env_p *outer = ENV_P(_outer);
	environment *env = (outer != 0 ? new environment(*outer) : new environment());
	env_p *p = new env_p(env);
	return OBJ_TO_C(p);
}

NUMTYPE c_free_env(NUMTYPE _env) {
	env_p *env = ENV_P(_env);
	delete(env);
    return 0;
}

NUMTYPE c_env_has(const char *_name, NUMTYPE _env) {
	std::string name(_name);
	env_p *env = ENV_P(_env);
	bool has = env->get()->has(name);

	return has ? 1 : 0;
}

NUMTYPE c_env_get(const char *_name, NUMTYPE _env) {
	std::string name(_name);
	env_p *env = ENV_P(_env);
	if(env->get()->has(name)) {
		cell *found = &env->get()->find(name)[name];
		return OBJ_TO_C(found);
	}

	return 0;
}

// c_env_lookup(cell*,env_p*) -> cell*;
NUMTYPE c_env_lookup(NUMTYPE _cell, NUMTYPE _env) {
	cell *c = CELL(_cell);
	env_p *env = ENV_P(_env);
	const std::string &str = c->val;
	// UNSAFE
	if(env->get()->has(str)) {
		c = &env->get()->find(str)[str];
		return OBJ_TO_C(c);
	}
	
	return 0;
}

NUMTYPE c_env_cstr(NUMTYPE _env) {
	env_p *env = ENV_P(_env);
	std::stringstream ss;
	char *result;

	ss << to_string(*(env->get()));
	result = c_persistent_string(ss.str());
	return OBJ_TO_C(result);
}

// return (*env)[x.list[1].val] = eval(x.list[2], env);
NUMTYPE c_env_set(NUMTYPE _name, NUMTYPE _value, NUMTYPE _env) {
	cell *name = CELL(_name);
	cell *value = CELL(_value);
	env_p *ep = ENV_P(_env);
	environment *env = ep->get();

	(*env)[name->val] = *value;
	return OBJ_TO_C(&(*env)[name->val]);
}

NUMTYPE c_env_newargs(NUMTYPE _names, NUMTYPE _values, NUMTYPE _parent) {
	const cell *names = CELL(_names);
	const cells *values = CELLS(_values);
	env_p *parent = ENV_P(_parent);

	environment *env = new environment(*names, *values, *parent);
	env_p *p = new env_p(env);
	return OBJ_TO_C(p);
}

////////////////////// built-in primitive procedures

cell proc_add(const cells & c)
{
	long n(atol(c[0].val.c_str()));
	for (cellit i = c.begin() + 1; i != c.end(); ++i) n += atol(i->val.c_str());
	return cell(Number, str(n));
}

cell proc_sub(const cells & c)
{
	long n(atol(c[0].val.c_str()));
	for (cellit i = c.begin() + 1; i != c.end(); ++i) n -= atol(i->val.c_str());
	return cell(Number, str(n));
}

cell proc_mul(const cells & c)
{
	long n(1);
	for (cellit i = c.begin(); i != c.end(); ++i) n *= atol(i->val.c_str());
	return cell(Number, str(n));
}

cell proc_div(const cells & c)
{
	long n(atol(c[0].val.c_str()));
	for (cellit i = c.begin() + 1; i != c.end(); ++i) n /= atol(i->val.c_str());
	return cell(Number, str(n));
}

cell proc_greater(const cells & c)
{
	long n(atol(c[0].val.c_str()));
	for (cellit i = c.begin() + 1; i != c.end(); ++i)
		if (n <= atol(i->val.c_str()))
			return false_sym;
	return true_sym;
}

cell proc_less(const cells & c)
{
	long n(atol(c[0].val.c_str()));
	for (cellit i = c.begin() + 1; i != c.end(); ++i)
		if (n >= atol(i->val.c_str()))
			return false_sym;
	return true_sym;
}

cell proc_less_equal(const cells & c)
{
	long n(atol(c[0].val.c_str()));
	for (cellit i = c.begin() + 1; i != c.end(); ++i)
		if (n > atol(i->val.c_str()))
			return false_sym;
	return true_sym;
}

cell proc_length(const cells & c) { return cell(Number, str(c[0].list.size())); }
cell proc_nullp(const cells & c) { return c[0].list.empty() ? true_sym : false_sym; }
cell proc_head(const cells & c) { return c[0].list[0]; }

cell proc_tail(const cells & c)
{
	if (c[0].list.size() < 2)
		return nil;
	cell result(c[0]);
	result.list.erase(result.list.begin());
	return result;
}

cell proc_append(const cells & c)
{
	cell result(List);
	result.list = c[0].list;
	for (cellit i = c[1].list.begin(); i != c[1].list.end(); ++i) result.list.push_back(*i);
	return result;
}

cell proc_cons(const cells & c)
{
	cell result(List);
	result.list.push_back(c[0]);
	for (cellit i = c[1].list.begin(); i != c[1].list.end(); ++i) result.list.push_back(*i);
	return result;
}

cell proc_list(const cells & c)
{
	cell result(List); result.list = c;
	return result;
}

cell proc_print(const cells &args) {
	for(auto i = args.cbegin(); i < args.cend(); ++i) {
		const cell &c = *i;
		std::cout << to_string(c) << " ";
	}
	std::cout << std::endl;
	return nil;
}

cell proc_equal (const cells &c) {
	auto it = c.cbegin();
	if(it == c.cend()) return false_sym;

	const cell &item = *it; ++it;
	while(it != c.cend()) {
		const cell &curr = *it;
		if(item != curr)
			return false_sym;
		++it;
	}
	return true_sym;
}

cell proc_nequal (const cells &c) {
	return (proc_equal(c) == false_sym) ? true_sym : false_sym;
}

// define the bare minimum set of primintives necessary to pass the unit tests
void add_globals(environment & env)
{
	env["nil"] = nil;   env["#f"] = false_sym;  env["#t"] = true_sym;
	env["append"] = cell(&proc_append);   env["head"] = cell(&proc_head);
	env["tail"] = cell(&proc_tail);      env["cons"] = cell(&proc_cons);
	env["length"] = cell(&proc_length);   env["list"] = cell(&proc_list);
	env["null?"] = cell(&proc_nullp);    env["+"] = cell(&proc_add);
	env["-"] = cell(&proc_sub);      env["*"] = cell(&proc_mul);
	env["/"] = cell(&proc_div);      env[">"] = cell(&proc_greater);
	env["<"] = cell(&proc_less);     env["<="] = cell(&proc_less_equal);
	env["="] = cell(&proc_equal);    env["!="] = cell(&proc_nequal);
	env["print"] = cell(&proc_print);
}

NUMTYPE c_add_globals(NUMTYPE _env) {
	env_p *env = ENV_P(_env);
	add_globals(*env->get());
	return OBJ_TO_C(env);
}

NUMTYPE c_cell_type(NUMTYPE _cell) {
	cell *c = CELL(_cell);
	return NUMBER_TO_C(c->type);
}

NUMTYPE c_cell_value(NUMTYPE _cell) {
	cell *c = CELL(_cell);
	return OBJ_TO_C((void*)c->val.c_str());
}

NUMTYPE c_cell_list(NUMTYPE _cell) {
	cell *c = CELL(_cell);
	if(c->type != List)
		return 0;
	return (NUMTYPE)&c->list;
}

// c_cell_str(cell&) -> char*;
// Caller is responsible for freeing the resulting string via SYS2_CSTR_FREE
NUMTYPE c_cell_cstr(const cell &c) {
	std::stringstream ss;
	char *result;
	
	ss << to_string(c);
	result = c_persistent_string(ss.str());
	return OBJ_TO_C(result);
}

NUMTYPE c_list_cstr(const cells &list) {
	std::stringstream ss;
	char *result;
	
	ss << to_string(list);
	result = c_persistent_string(ss.str());
	return OBJ_TO_C(result);
}

NUMTYPE c_cell_tail(const cell &list, cell &dest) {
	if(list.is_empty()) {
		dest.reset(empty_sym);
		return 0;
	}
	
	const cells tl(list.list.cbegin() + 1, list.list.cend());
	const cell tlc(tl);
	dest.reset(tlc);
	
	return 0;
}

NUMTYPE c_cell_strcmp(const char *s, NUMTYPE _cell) {
	cell *c = CELL(_cell);
	return (c->val == s) ? 0 : 1;
}


// Create a new cells object. Not to be confused with c_list
NUMTYPE c_list_new(NUMTYPE _content) {
	cells *result;
	if(_content) {
		cells *content = CELLS(_content);
		result = new cells(*content);
	} else {
		result = new cells();
	}
	return OBJ_TO_C(result);
}

NUMTYPE c_list_empty(NUMTYPE _list) {
	cells *list = CELLS(_list);
	return list->empty() ? 1 : 0;
}

NUMTYPE c_list_size(NUMTYPE _list) {
	cells *list = CELLS(_list);
	return (NUMTYPE)list->size();
}

NUMTYPE c_list_index(NUMTYPE index, NUMTYPE _list) NOEXCEPT {
	cells *list = CELLS(_list);

	try {
		cell *found = &(*list)[index];
		return OBJ_TO_C(found);
	} catch (...) {
		return 0;
	}
}

NUMTYPE c_list_push_back(NUMTYPE _cell, NUMTYPE _list) {
	cell *c = CELL(_cell);
	cells *list = CELLS(_list);
	list->push_back(*c);
	return 0;
}

////////////////////// eval - reference implementation

cell eval(cell x, environment * env)
{
#if false
	if (x.type == Symbol)
		return env->find(x.val)[x.val];
	if (x.type == Number)
		return x;
	if (x.list.empty())
		return nil;
	if (x.list[0].type == Symbol) {
		if (x.list[0].val == "quote")       // (quote exp)
			return x.list[1];
		if (x.list[0].val == "if")          // (if test conseq [alt])
			return eval(eval(x.list[1], env).val == "#f" ? (x.list.size() < 4 ? nil : x.list[3]) : x.list[2], env);
		if (x.list[0].val == "set!")        // (set! var exp)
			return env->find(x.list[1].val)[x.list[1].val] = eval(x.list[2], env);
		if (x.list[0].val == "define")      // (define var exp)
			return (*env)[x.list[1].val] = eval(x.list[2], env);
		if (x.list[0].val == "lambda") {    // (lambda (var*) exp)
			x.type = Lambda;
			// keep a reference to the environment that exists now (when the
			// lambda is being defined) because that's the outer environment
			// we'll need to use when the lambda is executed
			x.env = env;
			return x;
		}
		if (x.list[0].val == "begin") {     // (begin exp*)
			for (size_t i = 1; i < x.list.size() - 1; ++i)
				eval(x.list[i], env);
			return eval(x.list[x.list.size() - 1], env);
		}
	}
	// (proc exp*)
	cell proc(eval(x.list[0], env));
	cells exps;
	for (cell::iter exp = x.list.begin() + 1; exp != x.list.end(); ++exp)
		exps.push_back(eval(*exp, env));
	if (proc.type == Lambda) {
		// Create an environment for the execution of this lambda function
		// where the outer environment is the one that existed* at the time
		// the lambda was defined and the new inner associations are the
		// parameter names with the given arguments.
		// *Although the environmet existed at the time the lambda was defined
		// it wasn't necessarily complete - it may have subsequently had
		// more symbols defined in that environment.
		return eval(/*body*/proc.list[2], new environment(/*parms*/proc.list[1].list, /*args*/exps, proc.env));
	}
	else if (proc.type == Proc)
		return proc.proc(exps);

#else
	std::cout << "not a function\n";
	exit(1);
#endif
}


////////////////////// parse, read and user interaction

// convert given string to list of tokens
std::list<std::string> tokenize(const std::string & str)
{
	std::list<std::string> tokens;
	const char * s = str.c_str();
	while (*s) {
		while (isspace(*s))
			++s;
		if (*s == '(' || *s == ')')
			tokens.push_back(*s++ == '(' ? "(" : ")");
		else {
			const char * t = s;
			while (*t && !isspace(*t) && *t != '(' && *t != ')')
				++t;
			tokens.push_back(std::string(s, t));
			s = t;
		}
	}
	return tokens;
}

// numbers become Numbers; every other token is a Symbol
cell atom(const std::string & token)
{
	if (isdig(token[0]) || (token[0] == '-' && isdig(token[1])))
		return cell(Number, token);
	return cell(Symbol, token);
}

// return the Lisp expression in the given tokens
cell read_from(std::list<std::string> & tokens)
{
	if(tokens.empty()) throw std::exception();

	const std::string token(tokens.front());
	tokens.pop_front();
	if (token == "(") {
		cell c(List);
		while (tokens.front() != ")") {
			c.list.push_back(read_from(tokens));
			if(tokens.empty()) throw std::exception();
		}
		if(tokens.empty()) throw std::exception();
		tokens.pop_front();
		return c;
	}
	else
		return atom(token);
}

// return the Lisp expression represented by the given string
cell read(const std::string & s)
{
	std::list<std::string> tokens(tokenize(s));
	return read_from(tokens);
}
NUMTYPE c_parse(const char *str, NUMTYPE _dest) NOEXCEPT {
	cell *dest = CELL(_dest);
	try {
		dest->reset(read(std::string((char*)str)));
		return 0;
	} catch (...) {
		return 1;
	}
}

std::string to_string(const cell &exp);
std::string to_string(const cells &exp);
std::string to_string(const environment &exp);

// convert given cell to a Lisp-readable string
std::string to_string(const cell & exp)
{
	if (exp.type == List)
		return to_string(exp.list);
	else if (exp.type == Lambda)
		return "<Lambda>";
	else if (exp.type == Proc)
		return "<Proc>";
	return exp.val;
}

std::string to_string(const cells &exp) {
	std::string s("(");
	for(auto i = exp.cbegin(); i < exp.cend(); ++i)
		s += to_string(*i) + ' ';
	if (!s.empty() && s[s.size() - 1] == ' ')
		s.erase(s.size() - 1);
	return s + ')';
}

std::string to_string(const environment &env) {
	return env.str();
}

NUMTYPE c_cell_free(NUMTYPE _cell) {
	cell *c = CELL(_cell);
	delete c;
	return 0;
}

NUMTYPE unhandled_event(string_view reason) {
	throw StacktraceException(reason);
}

// Syscall handlers
NUMTYPE internal_syscall1(NUMTYPE signal) {
	switch(signal) {
		case SYS1_ATOM_FALSE: return OBJ_TO_C(&false_sym);
		case SYS1_ATOM_TRUE: return OBJ_TO_C(&true_sym);
		case SYS1_ATOM_NIL: return OBJ_TO_C(&nil);
		case SYS1_CELL_NEW: return c_cell_new(Number, "0");
	}

	std::stringstream ss;
	ss << "Invalid syscall #1: " << signal;
	return unhandled_event(ss);
}
NUMTYPE internal_syscall2(NUMTYPE signal, NUMTYPE arg1) {
	cell *c;
	cells *l;
	environment *e;
	env_p *ep;

	switch(signal) {
        case SYS2_ISDIG:
            return isdig((char)arg1) ? 1 : 0;
		case SYS2_CELL_NEW_STR:
			return OBJ_TO_C(new cell(Symbol, (char*)arg1));
		case SYS2_CELL_COPY:
        case SYS2_CELL_EMPTY:
		case SYS2_CELL_FRONT:
		case SYS2_CELL_ENV_GET:
		case SYS2_CELL_LIST:
		case SYS2_CELL_SIZE:
		case SYS2_CELL_CSTR:
		case SYS2_CELL_TYPE:
		case SYS2_CELL_VALUE:
		case SYS2_CELL_FREE:
            c = CELL(arg1);
            switch(signal) {
                case SYS2_CELL_COPY:
                    return OBJ_TO_C(new cell(*c));
                case SYS2_CELL_EMPTY:
                    return c->is_empty() ? 1 : 0;
                case SYS2_CELL_ENV_GET:
                    return OBJ_TO_C(&c->env);
                case SYS2_CELL_LIST: 
                    return OBJ_TO_C(&c->list);
                case SYS2_CELL_TYPE: 
                    return NUMBER_TO_C(c->type);
                case SYS2_CELL_VALUE: 
                    return OBJ_TO_C(&c->val);
                case SYS2_CELL_FREE:
					delete c;
					return 0;
				case SYS2_CELL_FRONT:
					if(c->is_empty())
						return OBJ_TO_C(&nil);
					return OBJ_TO_C(&c->list.front());
				case SYS2_CELL_SIZE:
					if(c->is_list())
						return NUMBER_TO_C(c->list.size());
					else
						return c->val.length();
				case SYS2_CELL_CSTR:
					return c_cell_cstr(*c);
            }
            break;
		case SYS2_LIST:
		case SYS2_LIST_NEW:
		case SYS2_LIST_EMPTY:
		case SYS2_LIST_SIZE:
		case SYS2_LIST_CSTR:
		case SYS2_LIST_FREE:
            l = CELLS(arg1);
            switch(signal) {
                case SYS2_LIST:
					return c_list(arg1);
				case SYS2_LIST_NEW:
					return c_list_new(arg1);
                case SYS2_LIST_EMPTY:
					return l->empty() ? 1 : 0;
                case SYS2_LIST_SIZE:
					return NUMBER_TO_C(l->size());
				case SYS2_LIST_CSTR:
					return c_list_cstr(*l);
				case SYS2_LIST_FREE: {
					delete l;
					return 0;
				}
            }
            break;
		case SYS2_ENV:
		case SYS2_FREE_ENV:
			ep = ENV_P(arg1);
            switch(signal) {
                case SYS2_ENV:
					if(ep != 0)
						e = new environment(*ep);
					else
						e = new environment();
					ep = new env_p(e);
					return OBJ_TO_C(ep);
                case SYS2_FREE_ENV:
					delete ep;
					return 0;
            }
            break;
		case SYS2_ADD_GLOBS:
			ep = ENV_P(arg1);
			e = ep->get();
			add_globals(*e);
			return OBJ_TO_C(ep);
		case SYS2_ENV_CSTR:
			return c_env_cstr(arg1);
		case SYS2_CSTR_FREE:
			free(reinterpret_cast<char*>(arg1));
			return 0;
	}
	
	std::stringstream ss;
	ss << "Invalid syscall #2: " << signal << ", args: " << arg1;
	return unhandled_event(ss);
}
NUMTYPE internal_syscall3(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2) {
	switch(signal) {
		case SYS3_CELL_NEW: return c_cell_new(arg1, (const char*)arg2);
		case SYS3_CELL_NEW_STRN: return c_cell_new_strn((const char*)arg1, arg2);
		case SYS3_CELL_STRCMP: return c_cell_strcmp((const char*)arg1, arg2);
		case SYS3_CELL_ENV_SET: return c_cell_env_set(arg1, arg2);
		case SYS3_CELL_INDEX: {
			cell *c = CELL(arg2);
			size_t index = static_cast<size_t>(arg1);
			if(index <= c->list_size())
				return OBJ_TO_C(&c->list[index]);
			return 0;
		}
		case SYS3_CELL_TAIL: return c_cell_tail(*CELL(arg1), *CELL(arg2));
		case SYS3_CELL_RESET: return c_cell_reset(arg1, arg2);
		case SYS3_LIST_INDEX: return c_list_index(arg1, arg2);
		case SYS3_LIST_PUSHB: return c_list_push_back(arg1, arg2);
		case SYS3_ENV_GET: return c_env_get((const char *)arg1, arg2);
		case SYS3_ENV_LOOKUP: return c_env_lookup(arg1, arg2);
		case SYS3_ENV_HAS: return c_env_has((const char *)arg1, arg2);
		case SYS3_PARSE: return c_parse((const char*)arg1, arg2);
		case SYS3_LISP_MAIN: return lispmain(arg1, (char**)arg2);
		case SYS3_CELL_SETTYPE: {
			cell *c = CELL(arg2);
			c->type = static_cast<cell_type>(arg1);
			return OBJ_TO_C(c);
		}
		case SYS3_CELL_SETENV: {
			env_p *env = ENV_P(arg1);
			cell *c = CELL(arg2);
			c->env = *env;
			return OBJ_TO_C(c);
		}
	}
	std::stringstream ss;
	ss << "Invalid syscall #3: " << signal << ", args: " << arg1 << ", " << arg2;
	return unhandled_event(ss);
}
NUMTYPE internal_syscall4(NUMTYPE signal, NUMTYPE arg1, NUMTYPE arg2, NUMTYPE arg3) {
	switch(signal) {
		case SYS4_CALL_PROC: return c_call_proc(arg1, arg2, arg3);
		case SYS4_ENV_SET: return c_env_set(arg1, arg2, arg3);
		case SYS4_ENV_NEWARGS: return c_env_newargs(arg1, arg2, arg3);
	}
	std::stringstream ss;
	ss << "Invalid syscall #4: " << signal << ", args: " << arg1 << ", " << arg2 << ", " << arg3;
	return unhandled_event(ss);
}

// the default read-eval-print-loop
void repl(const std::string & prompt, environment * env)
{
	for (;;) {
		std::cout << prompt;
		std::string line; std::getline(std::cin, line);
		std::cout << to_string(eval(read(line), env)) << '\n';
	}
}

int scheme_main()
{
	environment global_env; add_globals(global_env);
	repl("90> ", &global_env);
	return 0;
}

void scheme_test() {
	std::string line;
	environment _env, *env = &_env; add_globals(_env);
	eval(read("(define multiply-by (lambda (n) (lambda (y) (* y n))))"), env);
	eval(read("(define doubler (multiply-by 2))"), env);
	cell result = eval(read("(doubler 4)"), env);
	std::cout << to_string(result) << std::endl;
}

#ifdef _STANDALONE
////////////////////// unit tests
unsigned g_test_count;      // count of number of unit tests executed
unsigned g_fault_count;     // count of number of unit tests that fail
template <typename T1, typename T2>
void test_equal_(const T1 & value, const T2 & expected_value, const char * file, int line)
{
	++g_test_count;
	std::cerr
		//<< file
		<< '(' << line << ") : "
		<< " expected " << expected_value
		<< ", got " << value;
	if (value != expected_value) {
		++g_fault_count;
		std::cerr << " - FAIL\n";
	} else {
		std::cerr << " - success\n";
	}
}
// write a message to std::cout if value != expected_value
#define TEST_EQUAL(value, expected_value) test_equal_(value, expected_value, __FILE__, __LINE__)
// evaluate the given Lisp expression and compare the result against the given expected_result
#define TEST(expr, expected_result) TEST_EQUAL(to_string(eval(read(expr), &global_env)), expected_result)

unsigned scheme_complete_test() {
	environment global_env; add_globals(global_env);
	// the 29 unit tests for lis.py
	TEST("(quote (testing 1 (2.0) -3.14e159))", "(testing 1 (2.0) -3.14e159)");
	TEST("(+ 2 2)", "4");
	TEST("(+ (* 2 100) (* 1 10))", "210");
	TEST("(if (> 6 5) (+ 1 1) (+ 2 2))", "2");
	TEST("(if (< 6 5) (+ 1 1) (+ 2 2))", "4");
	TEST("(define x 3)", "3");
	TEST("x", "3");
	TEST("(+ x x)", "6");
	TEST("(begin (define x 1) (set! x (+ x 1)) (+ x 1))", "3");
	TEST("((lambda (x) (+ x x)) 5)", "10");
	TEST("(define twice (lambda (x) (* 2 x)))", "<Lambda>");
	TEST("(twice 5)", "10");
	TEST("(define compose (lambda (f g) (lambda (x) (f (g x)))))", "<Lambda>");
	TEST("((compose list twice) 5)", "(10)");
	TEST("(define repeat (lambda (f) (compose f f)))", "<Lambda>");
	TEST("((repeat twice) 5)", "20");
	TEST("((repeat (repeat twice)) 5)", "80");
	TEST("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))", "<Lambda>");
	TEST("(fact 3)", "6");
	//TEST("(fact 50)", "30414093201713378043612608166064768844377641568960512000000000000");
	TEST("(fact 12)", "479001600"); // no bignums; this is as far as we go with 32 bits
	TEST("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))", "<Lambda>");
	TEST("(list (abs -3) (abs 0) (abs 3))", "(3 0 3)");
	TEST("(define combine (lambda (f)"
		"(lambda (x y)"
		"(if (null? x) (quote ())"
		"(f (list (head x) (head y))"
		"((combine f) (tail x) (tail y)))))))", "<Lambda>");
	TEST("(define zip (combine cons))", "<Lambda>");
	TEST("(zip (list 1 2 3 4) (list 5 6 7 8))", "((1 5) (2 6) (3 7) (4 8))");
	TEST("(define riff-shuffle (lambda (deck) (begin"
		"(define take (lambda (n seq) (if (<= n 0) (quote ()) (cons (head seq) (take (- n 1) (tail seq))))))"
		"(define drop (lambda (n seq) (if (<= n 0) seq (drop (- n 1) (tail seq)))))"
		"(define mid (lambda (seq) (/ (length seq) 2)))"
		"((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))", "<Lambda>");
	TEST("(riff-shuffle (list 1 2 3 4 5 6 7 8))", "(1 5 2 6 3 7 4 8)");
	TEST("((repeat riff-shuffle) (list 1 2 3 4 5 6 7 8))", "(1 3 5 7 2 4 6 8)");
	TEST("(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))", "(1 2 3 4 5 6 7 8)");
	std::cout
		<< "total tests " << g_test_count
		<< ", total failures " << g_fault_count
		<< "\n";
	return g_fault_count ? EXIT_FAILURE : EXIT_SUCCESS;
}

int main (void) {
	scheme_complete_test();
}
#endif

