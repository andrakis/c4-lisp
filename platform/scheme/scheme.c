#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include "core/c5_native.h"

#define SYSCALLS
#include "core/syscalls.h"

#include "core/extras.h"

int g_debug, g_tests;
char *g_code, *g_argv0;

// enum cell_type {
enum { Symbol, Number, List, Proc, Lambda };
// };

enum { STDIN, STDOUT, STDERR };

// Special values
enum { ENV_NOPARENT = 0 };

void showhelp() {
	dprintf(STDERR, "lisp on c4\n");
	dprintf(STDERR, "Invocation: %s [-d] [file | -t | -e code]\n", g_argv0);
	dprintf(STDERR, "	-d      Enable debug mode.\n");
	dprintf(STDERR, "	file    Read and execute given file.\n");
	dprintf(STDERR, "	-t      Run tests.\n");
	dprintf(STDERR, "	-e code Code to run\n");
	dprintf(STDERR, "\nIf no code or file is given, the following code is run:\n%s\n", g_code);
}

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
	SYS3_CELL_NEW,    // (int Tag, char *Value) -> int.
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

// predefined symbols as chars
char *s_quote,
     *s_if,
     *s_set,
     *s_define,
     *s_lambda,
     *s_begin,
     *s_false,
     *s_true;

// References to atoms
void *sym_nil, *sym_true, *sym_false;

// Library interface
void *getsym_nil() { return (void*)syscall1(SYS1_ATOM_NIL); }
void *getsym_true() { return (void*)syscall1(SYS1_ATOM_TRUE); }
void *getsym_false() { return (void*)syscall1(SYS1_ATOM_FALSE); }

void *cell_new() {
	return (void*)syscall1(SYS1_CELL_NEW);
}

void *cell_new2(int type, char *value) {
	return (void*)syscall3(SYS3_CELL_NEW, type, (int) value);
}

void *cell_new_str(char *str) {
	return (void*)syscall2(SYS2_CELL_NEW_STR, (int)str);
}

void *cell_new_strn(char *str, int n) {
	return (void*)syscall3(SYS3_CELL_NEW_STRN, (int)str, n);
}

int cell_empty(void *cell) {
	return syscall2(SYS2_CELL_EMPTY, (int)cell);
}

void cell_free(void *cell) {
	syscall2(SYS2_CELL_FREE, (int)cell);
}

void *cell_front(void *cell) {
	return (void*)syscall2(SYS2_CELL_FRONT, (int)cell);
}

void *cell_index(int index, void *cell) {
	return (void*)syscall3(SYS3_CELL_INDEX, index, (int)cell);
}

int cell_size(void *cell) {
	return syscall2(SYS2_CELL_SIZE, (int)cell);
}

char *cell_cstr(void *cell) {
	return (char*)syscall2(SYS2_CELL_CSTR, (int)cell);
}

void cstr_free(char *str) {
	syscall2(SYS2_CSTR_FREE, (int)str);
}

char *list_cstr(void *cells) {
	return (char*)syscall2(SYS2_LIST_CSTR, (int)cells);
}

void *cell_index_default(int index, void *def, void *cell) {
	if(index >= cell_size(cell))
		return def;
	return cell_index(index, cell);
}

int cell_strcmp(char *str, void *cell) {
	return syscall3(SYS3_CELL_STRCMP, (int)(void*)str, (int)cell);
}

void *cell_tail(void *cell, void *dest) {
	return (void*)syscall3(SYS3_CELL_TAIL, (int)cell, (int)dest);
}

void *cell_env_get(void *cell) {
	return (void*)syscall2(SYS2_CELL_ENV_GET, (int)cell);
}

void *cell_set_lambda(void *env, void *cell) {
	syscall3(SYS3_CELL_SETTYPE, Lambda, (int)cell);
	syscall3(SYS3_CELL_SETENV, (int)env, (int)cell);
	return cell;
}

int cell_type(void *cell) {
	return syscall2(SYS2_CELL_TYPE, (int)cell);
}

char *cell_value(void *cell) {
	return (char*)syscall2(SYS2_CELL_VALUE, (int)cell);
}

int cell_reset(void *src, void *dst) {
	return syscall3(SYS3_CELL_RESET, (int)src, (int)dst);
}

char *env_cstr(void *env) {
	return (char*)syscall2(SYS2_ENV_CSTR, (int)env);
}

int env_has(char *s, void *env) {
	return syscall3(SYS3_ENV_HAS, (int)(void*)s, (int)env);
}

// env_lookup(cell*, environment*) -> cell*();
void *env_lookup(void *cell, void *env) {
	return (void*)syscall3(SYS3_ENV_LOOKUP, (int)cell, (int)env);
}

// env_new(environment*Parent) -> environment*()
void *env_new(void *parent) {
	return (void*)syscall2(SYS2_ENV, (int)parent);
}

// env_new_args(cells *, cells *, environment*) -> environment*();
void *env_new_args(void *names, void *values, void *parent) {
	return (void*)syscall4(SYS4_ENV_NEWARGS, (int)names, (int)values, (int)parent);
}

// env_set(char*, char*, environment*e) -> e;
void *env_set(void *name, void *value, void *env) {
	return (void*)syscall4(SYS4_ENV_SET, (int)name, (int)value, (int)env);
}

// env_free(environment*) -> void;
void env_free(void *env) {
	syscall2(SYS2_FREE_ENV, (int)env);
}

void add_globals(void *env) {
	syscall2(SYS2_ADD_GLOBS, (int)env);
}

void list_free(void *list) {
	syscall2(SYS2_LIST_FREE, (int)list);
}

void *list_new() {
	return (void*)syscall2(SYS2_LIST_NEW, 0);
}

void *list_push_back(void *cell, void *list) {
	return (void*)syscall3(SYS3_LIST_PUSHB, (int)cell, (int)list);
}

void *proc_invoke(void *exps, void *proc, void *dest) {
	return (void*)syscall4(SYS4_CALL_PROC, (int)exps, (int)proc, (int)dest);
}

int lisp_parse(char *code, void *dest) {
	return syscall3(SYS3_PARSE, (int)code, (int)dest);
}

// Utility functions
void print_cell2(void *cell, int fd) {
	char *str;

	if(!cell) {
		dprintf(fd, "[0x0]");
	} else {
		str = cell_cstr(cell);
		dprintf(fd, "%s", str);
		cstr_free(str);
	}
}

void print_cell(void *cell) {
	print_cell2(cell, STDERR);
}

void print_cells2(void *cells, int fd) {
	char *str;
	
	str = list_cstr(cells);
	dprintf(fd, "%s", str);
	cstr_free(str);
}

void print_cells(void *cells) {
	print_cells2(cells, STDERR);
}

void print_env2(void *env, int fd) {
	char *str;

	str = env_cstr(env);
	dprintf(fd, "%s", str);
	cstr_free(str);
}

void print_env(void *env) {
	print_env2(env, STDERR);
}

// Parsing
void *l4_tokenize(char *s) {
	void *tokens, *tmp;
	char *t;
	
	tokens = list_new();
	while(*s) {
		// Skip whitespace
		while(*s == ' ') ++s;
		// Start or end of list
		if(*s == '(' || *s == ')') {
			tmp = cell_new_str(*s++ == '(' ? "(" : ")");
			list_push_back(tmp, tokens);
			cell_free(tmp);
		} else {
			t = s;
			while(*t && *t != ' ' && *t != '(' && *t != ')') ++t;
			tmp = cell_new_strn(s, t - s);
			list_push_back(tmp, tokens);
			s = t;
			cell_free(tmp);
		}
	}

	return tokens;
}

int isdig(char d) {
	return d >= '0' && d <= '9';
}

void *l4_atom(char *token) {
	if(isdig(*token) || (*token == '-' && isdig(token[1])))
		return cell_new2(Number, token);
	return cell_new2(Symbol, token);
}

/*void *l4_read_from(char *tokens) {
	char *token;

	if(tokens == 0 || *tokens == 0) {
		dprintf(STDERR, "read_from: unexpected end of tokens\n");
		exit(2);
	}
}*/

// Eval implementation
void *eval(void *x, void *env) {
	int type, size, i, flag_handled;
	void *result, *first, *test, *conseq, *alt;
	void *env2;
	void *proc, *exps;
	void *t1, *t2;
	
	while(1) {
		flag_handled = 0;
		if(g_debug) {
			dprintf(STDERR, "eval(");
			print_cell(x);
			dprintf(STDERR, ", 0x%x)\n", env);
		}

		result = 0;
		type = cell_type(x);
		if(type == Symbol) {
			if(g_debug) {
				dprintf(STDERR, "  env.lookup(");
				print_cell(x);
				dprintf(STDERR, ") => ");
			}
			result = env_lookup(x, env);
			if(g_debug) {
				print_cell(result);
				dprintf(STDERR, "\n");
			}
			if(!result) {
				dprintf(STDERR, "Unknown symbol: ");
				print_cell(x);
				dprintf(STDERR, "\nSymbol table:\n");
				print_env(env);
				dprintf(STDERR, "\n");
				exit(1);
			}
			return result;
		}
		if(type == Number) {
			/*if(g_debug) {
				print_cell(x);
				dprintf(STDERR, "\n");
			}*/
			return x;
		}
		if(cell_empty(x)) {
			return sym_nil;
		}

		first = cell_front(x);
		type = cell_type(first);
		if(g_debug) {
			dprintf(STDERR, "  fn_call := ");
			print_cell(x);
			dprintf(STDERR, " \n");
		}
		if(type == Symbol) {
			if(cell_strcmp("quote", first) == 0) {	// (quote exp)
				return cell_index(1, x);
			}
			if(cell_strcmp("if", first) == 0) {		// (if test conseq [alt])
				test = cell_index(1, x);
				conseq = cell_index(2, x);
				alt = cell_index_default(3, sym_nil, x);
				/*if(g_debug) {
					dprintf(STDERR, "  !if [%x\t\t%x\t\t%x]\n", test, conseq, alt);
					dprintf(STDERR, "      [");
					print_cell(test); dprintf(STDERR, "\t\t");
					print_cell(conseq); dprintf(STDERR, "\t\t");
					print_cell(alt); dprintf(STDERR, "]\n");
				}*/
				result = eval(test, env);
				if(g_debug) {
					dprintf(STDERR, "  ===> "); print_cell(result); dprintf(STDERR, "\n");
				}
				result = (cell_strcmp(s_false, result) == 0) ? alt : conseq;
				// recurse
				//cell_free(x);
				x = result;
				flag_handled = 1;
				
				/*result = eval(result, env);
				if(g_debug) {
					dprintf(STDERR, "    => "); print_cell(result); dprintf(STDERR, "\n");
				}
				return result;*/
			}
			if(cell_strcmp("set!", first) == 0) {	// (set! var exp)
				//cell_free(x);
				result = eval(cell_index(2, x), env);
				env_set(cell_index(1, x), result, env);
				return result;
			}
			if(cell_strcmp("define", first) == 0) {	// (define var exp)
				//cell_free(x);
				result = eval(cell_index(2, x), env);
				env_set(cell_index(1, x), result, env);
				return result;
			}
			if(cell_strcmp("lambda", first) == 0) {	// (lambda (var*) exp)
				x = cell_set_lambda(env, x);
				//if(g_debug) dprintf(STDERR, "set lambda on item, env=%x\n", env);
				return x;
			}
			if(cell_strcmp("begin", first) == 0) {	// (begin exp*)
				// TODO: use iterator
				size = cell_size(x);
				i = 1;
				while(i < size - 1)
					eval(cell_index(i++, x), env);
				//return eval(cell_index(i, x), env);
				// recurse
				//cell_free(x);
				x = cell_index(i, x);
				flag_handled = 1;
			}
		}

		// (proc exp*)
		if(!flag_handled) {
			exps = list_new();
			proc = eval(first, env);
			size = cell_size(x);
			i = 1;
			
			// Evaluate arguments
			while(i < size) {
				t1 = cell_index(i++, x);
				/*if(g_debug) {
					dprintf(STDERR, "    arg at %ld: ", i - 1);
					print_cell(t1);
					dprintf(STDERR, "\n");
				}*/
				t2 = eval(t1, env);
				//cell_free(t1);
				/*if(g_debug) {
					dprintf(STDERR, " => ");
					print_cell(t2);
					dprintf(STDERR, "\n");
				}*/
				list_push_back(t2, exps);
				//cell_free(t2);
			}
			type = cell_type(proc);
			if(type == Lambda) {
				/*if(g_debug) dprintf(STDERR, "  proc type Lambda: ");
				if(g_debug) print_cell(proc);
				if(g_debug) {
					dprintf(STDERR, "\nproc args: "); print_cell(cell_index(1, proc));
					dprintf(STDERR, "\nproc values: "); print_cells(exps); dprintf(STDERR, "\n");
					dprintf(STDERR, "  proc env: %x\n", cell_env_get(proc));
				}*/
				// Create new environment parented to current
				env2 = env_new_args(
					cell_index(1, proc) /* arg names list */,
					exps                /* values */,
					cell_env_get(proc)  /* parent */);
				// exps no longer needed
				//if(g_debug) dprintf(STDERR, "  free exps\n");
				list_free(exps);
				// recurse
				//cell_free(x);
				x = cell_index(2, proc);
				//env_free(env2);
				env = env2;
			} else if(type == Proc) {
				if(g_debug) dprintf(STDERR, "  proc type Proc() => ");
				result = cell_new();
				proc_invoke(exps, proc, result);
				list_free(exps);
				if(g_debug) {
					print_cell(result);
					dprintf(STDERR, "\n");
				}
				//cell_free(x);
				return result;
			} else {
				dprintf(STDERR, "Invalid call in eval\n");
				exit(1);
			}
		}
	}
}

#define main lispmain

int min (int a, int b) { return (a <= b) ? a : b; }

int my_strlen(char *s) {
	int l;

	l = 0;
	while(*s++) ++l;
	return l;
}

int my_strcmp(char *a, char *b) {
	int l;

	l = my_strlen(a);
	if(l != my_strlen(b)) return 1;

	return memcmp((void*)a, (void*)b, l);
}

int run_single_test(char *code, char *expected, void *env) {
	void *tokens, *result;
	char *str;
	int no_match;

	tokens = cell_new();
	if(lisp_parse(code, tokens)) {
		dprintf(STDERR, "Failed to parse code: %s\n", code);
		return 1;
	}

	result = eval(tokens, env);
	str = cell_cstr(result);
	no_match = my_strcmp(str, expected);

	dprintf(STDERR, "Test %smatch, expected '%s' got '%s'",
		(no_match ? "mis" : ""), expected, str);
	if(no_match)
		dprintf(STDERR, ", code: %s", code);
	dprintf(STDERR, "\n");
	cstr_free(str);
	cell_free(tokens);
	return no_match;
}

void *TEST_global_env;
int   TEST_success, TEST_fail;
int TEST (char *code, char *expected) {
	int result;

	result = run_single_test(code, expected, TEST_global_env);
	if(!result) // success
		TEST_success = TEST_success + 1;
	else
		TEST_fail = TEST_fail + 1;
	return result;
}

int run_tests() {
	// the 29 unit g_tests for lis.py
	TEST_success = TEST_fail = 0;
	TEST_global_env = env_new(ENV_NOPARENT);
	add_globals(TEST_global_env);
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
	dprintf(STDERR, "Tests %ssuccessful, successes=%ld failures=%ld\n",
		(TEST_fail > 0 ? "un" : ""), TEST_success, TEST_fail);

	env_free(TEST_global_env);
	return TEST_fail > 0;
}

int g_source_mallocd;
char *read_file(char *name) {
	int fd, size, size_read;
	char *result;

	if( (fd = open(name, 0)) < 0) {
		dprintf(STDERR, "could not open file: %s\n", name);
		exit(3);
	}

	size = fdsize(fd) + 1;
	if( !(result = malloc(size)) ) {
		dprintf(STDERR, "failed to allocate %ld bytes for reading file.\n", size);
		exit(4);
	}

	if( (size_read = read(fd, result, size)) <= 0 ) {
		dprintf(STDERR, "read() returned %ld\n", size_read);
		exit(5);
	}
	g_source_mallocd = 1;
	result[size_read] = 0;
	return result;
}

void parse_args(int argc, char **argv) {
	char ch;

	while(argc > 0) {
		if(**argv == '-') {
			ch = (*argv)[1];
			if (ch == 'd')
				g_debug = 1;
			else if(ch == 'h') {
				showhelp();
				exit(1);
			} else if(ch == 'e') {
				if(argc == 0) {
					dprintf(STDERR, "-e requires an argument, try -h\n", *argv);
					exit(1);
				}
				--argc; ++argv;
				// Code to run
				g_code = *argv;
			} else if(ch == 't') {
				g_tests = 1;
			} else {
				dprintf(STDERR, "Unknown option %s, try -h\n", *argv);
				exit(1);
			}
		} else {
			g_code = read_file(*argv);
		}
		--argc; ++argv;
	}
}

int main(int argc, char **argv)
{
	void *global, *tokens, *result;
	char *tmp;

	// setup globals
	g_debug = 0;
	g_source_mallocd = 0;
	g_argv0 = argv[0];
	//g_code = "(print (quote Hello))";
	//g_code = "(print (quote Two plus 2 is) (+ 2 2))";
	g_code =
		"(begin "
		"    (define y 5) "
		"    (print y "
		"        (quote (doubled is)) "
		"        ((lambda (x) (+ x x)) y))"
		")";
	--argc; ++argv;
	parse_args(argc, argv);

#if __C4__
	// Requires the scheme library
	tmp = "scheme";
	if(g_debug)
		dprintf(2, "Load runtime library %s: %s\n", "scheme", runtime_path(tmp));
	platform_init(runtime_path(tmp));
#endif

	// Ensure syscalls are up to date
	if(!syscall_init(1, _SYS1_END) ||
	   !syscall_init(2, _SYS2_END) ||
	   !syscall_init(3, _SYS3_END) ||
	   !syscall_init(4, _SYS4_END)) {
		dprintf(STDERR, "Syscall init failed, recompile / check headers\n");
		return -1;
	}

	// Setup predefined symbols used in eval
	s_quote = "quote";
	s_if = "if";
	s_set = "set!";
	s_define = "define";
	s_lambda = "lambda";
	s_begin = "begin";
	s_false = "#f";
	s_true = "#t";
	
	sym_nil = getsym_nil();
	sym_true = getsym_true();
	sym_false = getsym_false();

	if(g_tests)
		return run_tests();

	global = env_new(ENV_NOPARENT);
	add_globals(global);
	if(g_debug) {
		dprintf(STDERR, " env.global: 0x%x\n", global);

		dprintf(STDERR, "Has +: %d\n", env_has("+", global));
		dprintf(STDERR, "Has foo: %d\n", env_has("foo", global));
	}
	
	tokens = cell_new();
	if(lisp_parse(g_code, tokens)) {
		dprintf(STDERR, "Failed to parse code: %s\n", g_code);
		return 1;
	}
	if(g_debug) {
		tmp = cell_cstr(tokens);
		dprintf(STDERR, " tokens: 0x%x (%s)\n", tokens, tmp);
		cstr_free(tmp);
	}

	result = eval(tokens, global);
	if(!result) {
		dprintf(STDERR, "Failed to get result with code %s\n", g_code);
	} else {
		print_cell2(result, STDOUT);
		dprintf(STDERR, "\n");
	}
	

	if(g_debug) dprintf(STDERR, "Cleaning up\n");
	cell_free(tokens);
	env_free(global);
	if(g_source_mallocd) free(g_code);
	return 0;
}
