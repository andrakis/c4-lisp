#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include "core/c5_native.h"

#define SYSCALLS
#include "core/syscalls.h"

int debug;

// enum cell_type {
enum { Symbol, Number, List, Proc, Lambda };
// };

// Special values
enum { ENV_NOPARENT = 0 };

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
	SYS2_FREE_ENV,  // (int Env) -> void. Free an environment object
	SYS2_ADD_GLOBS,  // (int Env) -> void. Add global symbols to given env
	_SYS2_END        // Must be last element
};
// };

// enum Syscalls3 {
// syscalls that take 2 arguments (the signal, 2 args)
enum {
	SYS3_CELL_ENV_SET,// (int Env, int Cell) -> void.
	SYS3_CELL_INDEX,  // (int Index, int Cell) -> int. Cell
	SYS3_CELL_NEW,    // (int Tag, char *Value) -> int.
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

void *sym_nil, *sym_true, *sym_false;

void *getsym_nil() { return (void*)syscall1(SYS1_ATOM_NIL); }
void *getsym_true() { return (void*)syscall1(SYS1_ATOM_TRUE); }
void *getsym_false() { return (void*)syscall1(SYS1_ATOM_FALSE); }

void *cell_new() {
	return (void*)syscall1(SYS1_CELL_NEW);
}

// Wrappers around various syscalls
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

void *cell_set_lambda(void *env, void *cell) {
	cell = (void*)syscall3(SYS3_CELL_SETTYPE, Lambda, (int)cell);
	return (void*)syscall3(SYS3_CELL_SETENV, (int)env, (int)cell);
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

int parse(char *code, void *dest) {
	return syscall3(SYS3_PARSE, (int)code, (int)dest);
}

void print_cell(void *cell) {
	char *str;

	if(!cell) {
		printf("[0x0]");
	} else {
		str = cell_cstr(cell);
		printf("%s", str);
		free(str);
	}
}

void print_cells(void *cells) {
	char *str;
	
	str = list_cstr(cells);
	printf("%s", str);
	free(str);
}

void *eval(void *x, void *env) {
	int type, size, i;
	void *result, *first, *test, *conseq, *alt;
	void *proc, *exps;
	void *t1, *t2;
	
	if(debug) printf("eval(%x, %x)\n", x, env);

	result = 0;
	type = cell_type(x);
	if(type == Symbol) {
		if(debug) {
			printf("  env.lookup(");
			print_cell(x);
			printf(") => ");
		}
		result = env_lookup(x, env);
		if(debug) {
			print_cell(result);
			printf("\n");
		}
		if(!result) {
			printf("Unknown symbol: ");
			print_cell(x);
			printf("\n");
			exit(1);
		}
		return result;
	}
	if(type == Number) {
		if(debug) {
			print_cell(x);
			printf("\n");
		}
		return x;
	}
	if(cell_empty(x)) {
		return sym_nil;
	}

	first = cell_front(x);
	type = cell_type(first);
	if(debug) {
		printf("  fn_call := ");
		print_cell(first);
		printf(" (...)\n");
	}
	if(type == Symbol) {
		if(cell_strcmp("quote", first) == 0) {	// (quote exp)
			cell_tail(x, x);
			return x;
		}
		if(cell_strcmp("if", first) == 0) {		// (if test conseq [alt])
			test = cell_index(1, x);
			conseq = cell_index(2, x);
			alt = cell_index_default(3, sym_nil, x);
			if(debug) {
				printf("  !if [%x\t\t%x\t\t%x]\n", test, conseq, alt);
				printf("      [");
				print_cell(test); printf("\t\t");
				print_cell(conseq); printf("\t\t");
				print_cell(alt); printf("]\n");
			}
			result = eval(test, env);
			if(debug) {
				printf("  !ifresult: %x\n", result);
				printf("           : ");
				print_cell(result); printf("\n");
				printf("  !  cell_strcmp(%x, %x)\n", s_false, result);
			}
			result = (cell_strcmp(s_false, result) == 0) ? alt : conseq;
			return result;
		}
		if(cell_strcmp("set!", first) == 0) {	// (set! var exp)
			result = eval(cell_index(2, x), env);
			env_set(cell_index(1, x), result, env);
			return result;
		}
		if(cell_strcmp("define", first) == 0) {	// (define var exp)
			result = eval(cell_index(2, x), env);
			env = env_set(cell_index(1, x), result, env);
			return result;
		}
		if(cell_strcmp("lambda", first) == 0)	// (lambda (var*) exp)
			return cell_set_lambda(env, x);
		if(cell_strcmp("begin", first) == 0) {	// (begin exp*)
			// TODO: use iterator
			size = cell_size(x);
			i = 1;
			while(i < size - 1)
				eval(cell_index(i++, x), env);
			return eval(cell_index(i, x), env);
		}
	}

	// (proc exp*)
	exps = list_new();
	proc = eval(first, env);
	size = cell_size(x);
	i = 1;
	
	if(debug) {
		printf("  proc, args created at %x, proc cell is: %x\n", exps, proc);
		printf("  number args: %ld\n", size);
	}
	
	// Evaluate arguments
	while(i < size) {
		t1 = cell_index(i++, x);
		if(debug) {
			printf("    arg at %ld: ", i - 1);
			print_cell(t1);
			printf("\n");
		}
		t2 = eval(t1, env);
		if(debug) {
			printf(" => ");
			print_cell(t2);
			printf("\n");
		}
		list_push_back(t2, exps);
	}
	type = cell_type(proc);
	if(type == Lambda) {
		if(debug) printf("  proc type Lambda\n");
		// Create new environment parented to current
		env = env_new_args(
			cell_index(1, proc) /* arg names list */,
			exps                /* values */,
			env                 /* parent */);
		// exps no longer needed
		list_free(exps);
		result = eval(cell_index(2, proc), env);
		return result;
	} else if(type == Proc) {
		if(debug) printf("  proc type Proc\n");
		proc_invoke(exps, proc, x);
		list_free(exps);
		return x;
	}

	printf("Invalid call in eval\n");
	exit(1);
}

#define main lispmain

char *code;
void parse_args(int argc, char **argv) {
	while(argc > 0) {
		if(**argv == '-') {
			if ((*argv)[1] == 'd')
				debug = 1;
			else if((*argv)[1] == 'h') {
				printf("lisp on c4\n");
				printf("Invocation: %s [-d] [code]\n", argv[-1]);
				printf("	-d      Enable debug mode\n");
				printf("	code    Code to run\n");
				exit(1);
			} else {
				printf("Unknown option %s, try -h\n", *argv);
				exit(1);
			}
		} else {
			// Code to run
			code = *argv;
			// clear argc
			argc = 1;
		}
		--argc; ++argv;
	}
}

int main(int argc, char **argv)
{
	void *global, *tokens, *result;
	char *tmp;

	// Ensure syscalls are up to date
	if(!syscall_init(1, _SYS1_END) ||
	   !syscall_init(2, _SYS2_END) ||
	   !syscall_init(3, _SYS3_END) ||
	   !syscall_init(4, _SYS4_END)) {
		printf("Syscall init failed, recompile / check headers\n");
		return -1;
	}

	// setup globals
	debug = 0;
	//code = "(print (quote Hello))";
	code = "(print (quote Two plus 2 is) (+ 1 1))";
	--argc; ++argv;
	parse_args(argc, argv);

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

	global = env_new(ENV_NOPARENT);
	add_globals(global);
	if(debug) {
		printf(" env.global: %x\n", global);

		printf("Has +: %d\n", env_has("+", global));
		printf("Has foo: %d\n", env_has("foo", global));
	}
	
	tokens = cell_new();
	if(parse(code, tokens)) {
		printf("Failed to parse code: %s\n", code);
		return 1;
	}
	if(debug) {
		tmp = cell_cstr(tokens);
		printf(" tokens: %x (%s)\n", tokens, tmp);
		free(tmp);
	}

	result = eval(tokens, global);
	if(!result) {
		printf("Failed to get result with code %s\n", code);
	} else {
		print_cell(result);
		printf("\n");
	}
	

	if(debug) printf("Cleaning up\n");
	cell_free(tokens);
	env_free(global);
	return 0;
}
