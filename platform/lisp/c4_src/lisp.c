/*
 * Lisp on C4
 *
 * Uses a custom stack, the "variable stack".
 */

/*
 * General notes on structures:
 *
 * Lists are represented as:
 *  cell[V_TYPE] = List
 *  cell[V_VAL]  = Head::cell(...)
 *  cell[V_ALT]  = Tail::cell(...)
 *
 * Lambdas are stored as:
 *  cell[V_TYPE] = Lambda
 *  cell[V_VAL]  = Details
 *  cell[V_ALT]  = environment*
 *  Details      = (lambda Params::list(cell())|cell() Body::list(cell()))
 * Eg:
	(lambda (x y) (+ x y)) =>
		cell(Lambda,
		V_ALT=env*,
		V_VAL =>
			cell(List,
			V_VAL =>
				cell(Symbol, V_VAL = "lambda"),
			V_ALT =>
				cell(List, V_VAL =>
					cell(Symbol, V_VAL => "x"),
					V_ALT =>
						cell(Symbol, "y")),
			V_ALT =>
				cell(List, V_VAL =>
					cell(Symbol,
					V_VAL => "+",
					V_ALT =>
						cell(Symbol, "x",
						V_ALT =>
							cell(Symbol, "y"))))
			)
		)
					
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "./lisp.h"

#ifndef __C4__
#define int long
#endif

// Ignore formatting warnings
#pragma GCC diagnostic ignored "-Wformat"

/** Standard enums */
enum /* CellType */ {
	Unknown = 0x00,
	Number  = 0x01,   // V_VAL => int
	String  = 0x02,   //       => char*
	Proc    = 0x04,   // V_VAL => int. (see Procs)
	List    = 0x10,   //       => int*
	Lambda  = 0x20,   // V_VAL => int*  V_ALT => int* Environment
	OnStack = 0x80    // 1 = was allocated on the stack, 0 = malloc
};
// Useful enums (avoid magic numbers elsewhere)
enum { CellTypeMask = 0x4F, NoAlt = 0 };

enum /* Streams */ {
	Stdin,
	Stdout,
	Stderr
};

// Available Procs
enum /* Procs */ { ProcAdd };

/** Structures implemented as enums **/

enum /* Cell */ {
	V_TYPE,    // @see CellType
	V_VAL,     // Value (or head of list)
	V_ALT,     // If List, pointer to next list element; if lambda, environment
	V__SZ      // Size of cell structure (must be last element)
};

enum /* Environment */ {
	E_OUTER,   // int* Outer environment pointer (Environent*), or 0 for empty
	E_MEMBERS, // int* List of members (each item being a list of (key value))
	E__SZ
};

/** Globals **/

char *argv0;   // Invocation

int *v_sp,      // variable stack pointer
    *v_bp,      // variable base pointer
    *v_sp_base,
     debug,    // debug the interpreter
     v_poolsz;  // variable pool size

void push (int value) {
	*++v_sp = (int)value;
}

int pop () {
	return *v_sp--;
}

int *v_alloc (int size) {
	// No memory checking is done
	int *sp;

	sp = v_sp;
	v_sp = v_sp + size;
	if(debug) dprintf(Stderr, "  v_alloc(%ld) = 0x%X, v_sp now +%ld\n", size, sp, v_sp - v_sp_base);
	return sp;
}

void v_free (int size) {
	// No checking is done
	v_sp = v_sp - size;
	if(debug) dprintf(Stderr, "  v_free(%ld) = 0x%X, v_sp now +%ld\n", size, v_sp, v_sp - v_sp_base);
}

char *str_buf;
int   str_buf_sz;

#ifdef __C4__
	// dummies so str_celltype still works
	int strlen(char *s) { return 0; }
	char *copytostatic(char *s) { return s; }
#endif

char *str_celltype (int type) {
	// TODO: better
	char *msg;

	if(type == Unknown)    msg = " Unknown  ";
	else if(type & Number) msg = " Number   ";
	else if(type & String) msg = " String   ";
	else if(type & Proc)   msg = " Proc     ";
	else                   msg = " Invalid  ";

#ifndef __C4__
	// Only run this section under gcc
	if(strlen(msg) != 0) {
		msg = copytostatic(msg);
	}
#endif

	msg[0] = ' '; msg[9] = ' ';
	if(type & List) {
		msg[0] = '['; msg[9] = ']';
	} else if(type & Lambda) {
		msg[0] = '{'; msg[9] = '}';
	}

	msg[8] = ' ';
	if(!(type & OnStack)) msg[8] = '*';
	return msg;
	// on gcc, this is required
	// also some fudging of keywords so C4 still accepts the code
	
}

// Constructor
void cell_init (int type, int val, int alt, int *cell) {
	cell[V_TYPE] = type;
	cell[V_VAL]  = val;
	cell[V_ALT]  = alt;
	if(debug) {
		dprintf(Stderr, "    cell(%s) at 0x%X", str_celltype(type), cell);
		if(type & String) dprintf(Stderr, ", = \"%s\"", (char*)cell[V_VAL]);
		else if(type & Number) dprintf(Stderr, ", = %ld", cell[V_VAL]);
		dprintf(Stderr, "\n");
	}
}
// Destructor
void cell_deinit (int *cell) {
	if(debug)
		dprintf(Stderr, "    ~cell at 0x%X\n", cell);
}

// Allocate a cell on the variable stack
int *v_cell () {
	int *cell;
	cell = v_alloc(V__SZ);
	if(debug) dprintf(Stderr, "    v_cell() = 0x%X\n", cell);
	return cell;
}
int *v_cell_new (int type, int value, int alt) {
	int *cell;
	if( !(cell = v_cell() ) ) return 0;
	cell_init(type | OnStack, value, alt, cell);
	return cell;
}
void v_cell_free () {
	int *cell;

	cell = v_sp - V__SZ;
	cell_deinit(cell);
	v_free(V__SZ);
}
// Allocate a cell using standard malloc
int *cell_new () {
	return (int*)malloc(V__SZ * sizeof(int));
}
int *cell_new3 (int type, int value, int alt) {
	int *cell;

	if( !(cell = cell_new()) ) return 0;
	cell_init(type, value, alt, cell);
	return cell;
}
void cell_free (int *cell) { cell_deinit(cell); free(cell); /* lol */ }
int cell_type (int *cell) { return cell[V_TYPE] & CellTypeMask; }
int cell_is_list     (int *cell) { return cell[V_TYPE] & List; }
int cell_is_lambda   (int *cell) { return cell[V_TYPE] & Lambda; }
int cell_is_onstack  (int *cell) { return cell[V_TYPE] & OnStack; }
int cell_is (int type,int *cell) { return cell[V_TYPE] & type; }

void environment_init (int *env) {
	if(debug)
		dprintf(Stderr, "    environment() at 0x%X\n", env);
	env[E_OUTER] = 0;
	env[E_MEMBERS] = 0;
}
void environment_deinit (int *env) {
	if(debug)
		dprintf(Stderr, "    ~environment at 0x%X\n", env);
	if(env[E_MEMBERS]) {
		cell_free((int*)env[E_MEMBERS]);
		env[E_MEMBERS] = 0;
	}
}
// Allocate an environment on the variable stack
int *v_env () {
	int *env;

	if( !(env = v_alloc(E__SZ)) ) return 0;
	environment_init(env);
	return env;
}
void v_env_free () {
	int *e;

	e = v_sp - E__SZ;
	environment_deinit(e);
	v_free(E__SZ);
}
// Allocate an environment using standard malloc
int *env () {
	int *e;
	if( !(e = (int*)malloc(E__SZ * sizeof(int*))) ) return 0;
	environment_init(e);
	return e;
}
void env_free (int *e) {
	environment_deinit(e);
	free(e);
}

void env__insert (char *key, int *val, int *env) {
	if(env[MEMBERS] == 0) {
		env[MEMBERS] = (int)cell_new3(List | String, (int)"end$", NoAlt);
	}
	// insert [key value]
	cell_push_front(cell_new3(List | String, (int)key,
		(int)cell_new_from(val)), (int*)env[MEMBERS]);
}

int v_sp_base_test () {
	int a, b, c;

	a = 1;
	b = 512;
	c = 1024;

	push(a); push(b); push(c);
	a = pop(); b = pop(); c = pop();

	if(a != 1024 || b != 512 || c != 1) {
		dprintf(Stderr, "variable stack test failed: %ld %ld %ld\n", a, b, c);
		return 1;
	}

	return 0;
}

// Copy from source -> dest
void cell_copy (int *source, int *dest) {
	cell_init(source[V_TYPE], source[V_VAL], source[V_ALT], dest);
}

// Push a copy of given cell
void v_cell_push (int *source) {
	int *dest;

	dest = v_cell();
	cell_copy(source, dest);
}

// Swap the given cell values
void cell_swap (int *a, int *b) {
	int word;

	// TODO: more efficient to v_alloc, memcopy, memcopy, v_free() ?
	word = b[V_TYPE]; b[V_TYPE] = a[V_TYPE]; a[V_TYPE] = word;
	word = b[V_VAL]; b[V_VAL] = a[V_VAL]; a[V_VAL] = word;
	word = b[V_ALT]; b[V_ALT] = a[V_ALT]; a[V_ALT] = word;
}

int cell_push_front (int *source, int *dest) {
	/*
	// Swap their values (head <-> tail). This keeps the pointer for
	// the original data valid whilst allowing lists to grow.
	cell_swap(source, dest);
	// Point dest's tail at original head
	dest[V_ALT] = (int)source;
	*/
	int *newcell;

	// allocate a new cell (not on the stack) and copy its details
	if( !(newcell = cell_new() )) return 1;
	cell_copy(source, newcell);
	// swap dest with the new cell (so its at front)
	cell_swap(newcell, dest);
	newcell[V_ALT] = (int)dest;

	return 0;
}

int cell_push_back (int *value, int *cell) {
	int *newcell;

	if( !(newcell = cell_new() ) ) return 1;
	cell_copy(value, newcell);
	// TODO: free/destruct V_ALT?
	cell[V_ALT] = (int)newcell;
}

// Push a copy of given environment.
// What this actually does instead is push an empty environment onto the stack
// and set its outer pointer to source.
void v_env_push (int *source) {
	int *dest;

	dest = v_env();
	dest[E_OUTER] = (int)source;
	dest[E_MEMBERS] = 0;
}

void print_cell(int fd, int *cell) {
	int t;

	//if(debug) dprintf(fd, "<cell.0x%X.type=%s>", cell, str_celltype(cell[V_TYPE]));

	t = cell[V_TYPE];
	if(t & List) dprintf(fd, "(");

	if(t & String)
		dprintf(fd, "\"%s\"", (char*)cell[V_VAL]);
	else if(t & Number)
		dprintf(fd, "%ld", cell[V_VAL]);
	else if(t & Lambda)
		dprintf(fd, "<Lambda>");
	else if(t & Proc)
		dprintf(fd, "<Proc>");

	if(t & List && cell[V_ALT]) {
		dprintf(fd, " | ");
		print_cell(fd, (int*)cell[V_ALT]);
	}

	if(t & List) dprintf(fd, ")");
}

void print_env(int fd, int *env) {
}

void proc_add () {
	int *cells, *result, *first, *current;
	int type;

	cells = v_sp - V__SZ;
	first = (int*)cells[V_VAL]; // at first element
	type = cell_type(first);
	current = (int*)first[V_ALT];
	result = v_cell_new(cell_type(first), first[V_VAL], NoAlt);
	while(current) {
		if(cell_type(current) != type) {
			dprintf(Stderr, "Cannot add different types (0x%X != 0x%X)", cell_type(current), type);
			exit(-1);
		} else {
			if(type == Number) result[V_VAL] = result[V_VAL] + current[V_VAL];
			else if(type == String) {
				dprintf(Stderr, "Cannot add strings at this time\n");
				exit(-1);
			} else {
				dprintf(Stderr, "Cannot add given type at this time: %s\n", str_celltype(type));
				exit(-1);
			}
			current = (int*)current[V_ALT];
		}
	}

	// leaves result on the stack
}

void proc_invalid () {
	int *cell;

	cell = v_sp - V__SZ;
	dprintf(Stderr, "Invalid proc invocation %ld\n", cell[V_VAL]);
	exit(-1);
}

// Expects on stack:
//  cell*
void proc_dispatch() {
	int *cell, proc;

	cell = v_sp - V__SZ;
	proc = cell[V_VAL];
	if(proc == ProcAdd) proc_add();
	else proc_invalid();
}

// expects two symbols on stack:
// environment, cell (to eval)
// pushes result onto stack as a cell
void eval () {
	int *environment, *x;
	int *bp;

	// A classic combo: PUSH BP; MOV BP, SP
	bp = v_bp; v_bp = v_sp;

	// remember our stack grows upwards, so we take a negative offset of bp
	environment = v_bp - E__SZ; // grab environment (positioned at v_sp)
	x = environment - V__SZ;    // grab x (positioned before environment)

	dprintf(Stderr, "eval() with symbol 0x%X: ", x);
	print_cell(Stderr, x);
	dprintf(Stderr, "\nwith environment 0x%X: ", environment);
	print_env(Stderr, environment);
	dprintf(Stderr, "\n");

	v_cell_push(x); // return x as is
	// restore bp
	v_bp = bp;
}

int lisp_test () {
	int *cell_code, *cell_a, *cell_b;
	int *x;
	int *global_env;
	int *result;

	global_env = v_env();

	cell_b = v_cell_new(List | Number, 3, NoAlt);
	cell_a = v_cell_new(List | Number, 2, (int)cell_b);
	cell_code = v_cell_new(List | String, (int)"+", (int)cell_a);

	x = cell_code;
	v_bp = v_sp;  // save bp for entry to eval
	v_cell_push(x);
	v_env_push(global_env);
	eval();
	result = v_sp - V__SZ;
	dprintf(Stdout, "Result: ");
	print_cell(Stdout, result);
	dprintf(Stdout, "\n");

	// Free in reverse order
	v_cell_free(); // the return value
	v_env_free();  // pushed environment
	v_cell_free(); // pushed x value

	v_cell_free(); // cell_code, x
	v_cell_free(); // cell_a
	v_cell_free(); // cell_b
	v_env_free();  // global_env

	return 0;
}

int main (int argc, char **argv) {
	int i;
	// The variable stack is used for cells and lists, and grows upwards.

	// Set globals
	argv0 = *argv;
	debug = 1;
	v_poolsz = sizeof(int) * 256 * 1024; // decent amount of space

	if( !(v_sp_base = (int*)malloc(v_poolsz)) ) {
		dprintf(Stderr, "%s: unable to allocate %ld bytes for variable stack.\n", argv0, v_poolsz);
		return 1;
	}

	v_sp = v_bp = v_sp_base;

	if( (i = v_sp_base_test()) ) return i;

	if( (i = lisp_test()) ) return i;

	free(v_sp_base);
	return 0;
}

