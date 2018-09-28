/**
 * Part of the c4-lisp (aka c5) project.
 * 
 * Original c4.c comments:
	// c4.c - C in four functions

	// char, int, and pointer types
	// if, while, return, and expression statements
	// just enough features to allow self-compilation and a bit more

	// Written by Robert Swierczek
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include "core/c5_native.h"
#include "core/syscalls.h"
#include "core/extras.h"

// Shared data (used in compilation)
char *p, *lp, // current position in source code
     *data,   // data/bss pointer
     *opcodes;// opcodes string
// Additional shared data and program flags
int *e, *le,  // current position in emitted code
    *id,      // currently parsed identifier
    *sym,     // symbol table (simple list of identifiers)
    tk,       // current token
    ival,     // current token value
    ty,       // current expression type
    loc,      // local variable offset
    line,     // current line number
    src,      // print source and assembly flag
    debug,    // print executed instructions
    verbose;  // print more detailed info
// VM symbols
int *vm_processes, vm_proc_max, vm_proc_count, vm_cycle_count;

// tokens and classes (operators last and in precedence order)
enum {
	Num = 128, Fun, Sys, Glo, Loc, Id,
	Char, Else, Enum, If, Int, Return, Sizeof, While,
	Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// opcodes
enum {
	// VM primitives
	LEA ,IMM ,JMP ,JSR ,BZ  ,BNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PSH ,
	OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
	// C functions
	OPEN,READ,CLOS,PRTF,DPRT,MALC,FREE,MSET,MCMP,EXIT,FDSZ,
	// Extended system calls (handled externally)
	SYS1,SYS2,SYS3,SYS4,SYSI,
	// Pointer to last element
	_SYMS
};

// Sets up the static data containing opcodes (for source output)
void setup_opcodes() {
	// Must be in same order as instructions enum
	opcodes =
		"LEA ,IMM ,JMP ,JSR ,BZ  ,BNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,"
		"SC  ,PSH ,OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,"
		"SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
		"OPEN,READ,CLOS,PRTF,DPRT,MALC,FREE,MSET,MCMP,EXIT,FDSZ,"
		"SYS1,SYS2,SYS3,SYS4,SYSI";
}

// Sets up the static data containing keywords and imported functions.
void setup_symbols() {
	// Must match sequence of instructions enum
	p = 
		// Keywords
		"char else enum if int return sizeof while "
		// C functions
		"open read close printf dprintf malloc free memset memcmp exit fdsize "
		// Syscalls
		"syscall1 syscall2 syscall3 syscall4 syscall_init "
		// void data type
		"void "
		// main
		"main";
}

// types
enum { CHAR, INT, PTR };

// file streams
enum { STDIN, STDOUT, STDERR };

// identifier offsets (since we can't create an ident struct)
enum { Tk, Hash, Name, Class, Type, Val, HClass, HType, HVal, Idsz };

int B_MAGIC;

// Process members
enum {
	B_magic,
	B_sym, B_e, B_data, B_exitcode,
	B_p_sym, B_p_e, B_p_data, B_p_sp,   // process initial values
	B_pc, B_bp, B_sp, B_a, B_cycle,
	B_i,  B_t, B_halted,
	__B
};


void next()
{
	char *pp;
	int t;

	while (tk = *p) {
		++p;
		if (tk == '\n') {
			if (src) {
				dprintf(STDERR, "%d: %.*s", line, p - lp, lp);
				lp = p;
				while (le < e) {
					dprintf(STDERR, "%8.4s", &opcodes[*++le * 5]);
					if (*le <= ADJ) dprintf(STDERR, " %d\n", *++le); else dprintf(STDERR, "\n");
				}
			}
			++line;
		}
		else if (tk == '#') {
			while (*p != 0 && *p != '\n') ++p;
		}
		else if ((tk >= 'a' && tk <= 'z') || (tk >= 'A' && tk <= 'Z') || tk == '_') {
			pp = p - 1;
			while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_')
				tk = tk * 147 + *p++;
			tk = (tk << 6) + (p - pp);
			id = sym;
			while (id[Tk]) {
				if (tk == id[Hash] && !memcmp((char *)id[Name], pp, p - pp)) { tk = id[Tk]; return; }
				id = id + Idsz;
			}
			id[Name] = (int)pp;
			id[Hash] = tk;
			tk = id[Tk] = Id;
			return;
		}
		else if (tk >= '0' && tk <= '9') {
			if (ival = tk - '0') { while (*p >= '0' && *p <= '9') ival = ival * 10 + *p++ - '0'; }
			else if (*p == 'x' || *p == 'X') {
				while ((tk = *++p) && ((tk >= '0' && tk <= '9') || (tk >= 'a' && tk <= 'f') || (tk >= 'A' && tk <= 'F')))
					ival = ival * 16 + (tk & 15) + (tk >= 'A' ? 9 : 0);
			}
			else { while (*p >= '0' && *p <= '7') ival = ival * 8 + *p++ - '0'; }
			tk = Num;
			return;
		}
		else if (tk == '/') {            // C++ style comment: //
			if (*p == '/') {
				++p;
				while (*p != 0 && *p != '\n') ++p;
			}
			else if (*p == '*') {        // C style comment: /* */
				++p;
				t = 0;
				while (*p != 0 && t == 0) {
						pp = p + 1;
						if(*p == '\n')
							line++;
						else if(*p == '*' && *pp == '/')
							 t = 1;
						++p;
				}
				++p;
			}
			else {
				tk = Div;
				return;
			}
		}
		else if (tk == '\'' || tk == '"') {
			pp = data;
			while (*p != 0 && *p != tk) {
				if ((ival = *p++) == '\\') {
					if ((ival = *p++) == 'n') ival = '\n';
				}
				if (tk == '"') *data++ = ival;
			}
			++p;
			if (tk == '"') ival = (int)pp; else tk = Num;
			return;
		}
		else if (tk == '=') { if (*p == '=') { ++p; tk = Eq; } else tk = Assign; return; }
		else if (tk == '+') { if (*p == '+') { ++p; tk = Inc; } else tk = Add; return; }
		else if (tk == '-') { if (*p == '-') { ++p; tk = Dec; } else tk = Sub; return; }
		else if (tk == '!') { if (*p == '=') { ++p; tk = Ne; } return; }
		else if (tk == '<') { if (*p == '=') { ++p; tk = Le; } else if (*p == '<') { ++p; tk = Shl; } else tk = Lt; return; }
		else if (tk == '>') { if (*p == '=') { ++p; tk = Ge; } else if (*p == '>') { ++p; tk = Shr; } else tk = Gt; return; }
		else if (tk == '|') { if (*p == '|') { ++p; tk = Lor; } else tk = Or; return; }
		else if (tk == '&') { if (*p == '&') { ++p; tk = Lan; } else tk = And; return; }
		else if (tk == '^') { tk = Xor; return; }
		else if (tk == '%') { tk = Mod; return; }
		else if (tk == '*') { tk = Mul; return; }
		else if (tk == '[') { tk = Brak; return; }
		else if (tk == '?') { tk = Cond; return; }
		else if (tk == '~' || tk == ';' || tk == '{' || tk == '}' || tk == '(' || tk == ')' || tk == ']' || tk == ',' || tk == ':') return;
	}
}

void expr(int lev)
{
	int t, *d;

	if (!tk) { dprintf(STDERR, "%d: unexpected eof in expression\n", line); exit(-1); }
	else if (tk == Num) { *++e = IMM; *++e = ival; next(); ty = INT; }
	else if (tk == '"') {
		*++e = IMM; *++e = ival; next();
		while (tk == '"') next();
		data = (char *)((int)data + sizeof(int*) & -sizeof(int*)); ty = PTR;
	}
	else if (tk == Sizeof) {
		next(); if (tk == '(') next(); else { dprintf(STDERR, "%d: open paren expected in sizeof\n", line); exit(-1); }
		ty = INT; if (tk == Int) next(); else if (tk == Char) { next(); ty = CHAR; }
		while (tk == Mul) { next(); ty = ty + PTR; }
		if (tk == ')') next(); else { dprintf(STDERR, "%d: close paren expected in sizeof\n", line); exit(-1); }
		*++e = IMM; *++e = (ty == CHAR) ? sizeof(char) : sizeof(int*);
		ty = INT;
	}
	else if (tk == Id) {
		d = id; next();
		if (tk == '(') {
			next();
			t = 0;
			while (tk != ')') { expr(Assign); *++e = PSH; ++t; if (tk == ',') next(); }
			next();
			if (d[Class] == Sys) *++e = d[Val];
			else if (d[Class] == Fun) { *++e = JSR; *++e = d[Val]; }
			else { dprintf(STDERR, "%d: bad function call\n", line); exit(-1); }
			if (t) { *++e = ADJ; *++e = t; }
			ty = d[Type];
		}
		else if (d[Class] == Num) { *++e = IMM; *++e = d[Val]; ty = INT; }
		else {
			if (d[Class] == Loc) { *++e = LEA; *++e = loc - d[Val]; }
			else if (d[Class] == Glo) { *++e = IMM; *++e = d[Val]; }
			else { dprintf(STDERR, "%d: undefined variable\n", line); exit(-1); }
			*++e = ((ty = d[Type]) == CHAR) ? LC : LI;
		}
	}
	else if (tk == '(') {
		next();
		if (tk == Int || tk == Char) {
			t = (tk == Int) ? INT : CHAR; next();
			while (tk == Mul) { next(); t = t + PTR; }
			if (tk == ')') next(); else { dprintf(STDERR, "%d: bad cast\n", line); exit(-1); }
			expr(Inc);
			ty = t;
		}
		else {
			expr(Assign);
			if (tk == ')') next(); else { dprintf(STDERR, "%d: close paren expected\n", line); exit(-1); }
		}
	}
	else if (tk == Mul) {
		next(); expr(Inc);
		if (ty > INT) ty = ty - PTR; else { dprintf(STDERR, "%d: bad dereference\n", line); exit(-1); }
		*++e = (ty == CHAR) ? LC : LI;
	}
	else if (tk == And) {
		next(); expr(Inc);
		if (*e == LC || *e == LI) --e; else { dprintf(STDERR, "%d: bad address-of\n", line); exit(-1); }
		ty = ty + PTR;
	}
	else if (tk == '!') { next(); expr(Inc); *++e = PSH; *++e = IMM; *++e = 0; *++e = EQ; ty = INT; }
	else if (tk == '~') { next(); expr(Inc); *++e = PSH; *++e = IMM; *++e = -1; *++e = XOR; ty = INT; }
	else if (tk == Add) { next(); expr(Inc); ty = INT; }
	else if (tk == Sub) {
		next(); *++e = IMM;
		if (tk == Num) { *++e = -ival; next(); } else { *++e = -1; *++e = PSH; expr(Inc); *++e = MUL; }
		ty = INT;
	}
	else if (tk == Inc || tk == Dec) {
		t = tk; next(); expr(Inc);
		if (*e == LC) { *e = PSH; *++e = LC; }
		else if (*e == LI) { *e = PSH; *++e = LI; }
		else { dprintf(STDERR, "%d: bad lvalue in pre-increment\n", line); exit(-1); }
		*++e = PSH;
		*++e = IMM; *++e = (ty > PTR) ? sizeof(int*) : sizeof(char);
		*++e = (t == Inc) ? ADD : SUB;
		*++e = (ty == CHAR) ? SC : SI;
	}
	else { dprintf(STDERR, "%d: bad expression\n", line); exit(-1); }

	while (tk >= lev) { // "precedence climbing" or "Top Down Operator Precedence" method
		t = ty;
		if (tk == Assign) {
			next();
			if (*e == LC || *e == LI) *e = PSH; else { dprintf(STDERR, "%d: bad lvalue in assignment\n", line); exit(-1); }
			expr(Assign); *++e = ((ty = t) == CHAR) ? SC : SI;
		}
		else if (tk == Cond) {
			next();
			*++e = BZ; d = ++e;
			expr(Assign);
			if (tk == ':') next(); else { dprintf(STDERR, "%d: conditional missing colon\n", line); exit(-1); }
			*d = (int)(e + 3); *++e = JMP; d = ++e;
			expr(Cond);
			*d = (int)(e + 1);
		}
		else if (tk == Lor) { next(); *++e = BNZ; d = ++e; expr(Lan); *d = (int)(e + 1); ty = INT; }
		else if (tk == Lan) { next(); *++e = BZ;  d = ++e; expr(Or);  *d = (int)(e + 1); ty = INT; }
		else if (tk == Or)  { next(); *++e = PSH; expr(Xor); *++e = OR;  ty = INT; }
		else if (tk == Xor) { next(); *++e = PSH; expr(And); *++e = XOR; ty = INT; }
		else if (tk == And) { next(); *++e = PSH; expr(Eq);  *++e = AND; ty = INT; }
		else if (tk == Eq)  { next(); *++e = PSH; expr(Lt);  *++e = EQ;  ty = INT; }
		else if (tk == Ne)  { next(); *++e = PSH; expr(Lt);  *++e = NE;  ty = INT; }
		else if (tk == Lt)  { next(); *++e = PSH; expr(Shl); *++e = LT;  ty = INT; }
		else if (tk == Gt)  { next(); *++e = PSH; expr(Shl); *++e = GT;  ty = INT; }
		else if (tk == Le)  { next(); *++e = PSH; expr(Shl); *++e = LE;  ty = INT; }
		else if (tk == Ge)  { next(); *++e = PSH; expr(Shl); *++e = GE;  ty = INT; }
		else if (tk == Shl) { next(); *++e = PSH; expr(Add); *++e = SHL; ty = INT; }
		else if (tk == Shr) { next(); *++e = PSH; expr(Add); *++e = SHR; ty = INT; }
		else if (tk == Add) {
			next(); *++e = PSH; expr(Mul);
			if ((ty = t) > PTR) { *++e = PSH; *++e = IMM; *++e = sizeof(int*); *++e = MUL; }
			*++e = ADD;
		}
		else if (tk == Sub) {
			next(); *++e = PSH; expr(Mul);
			if (t > PTR && t == ty) { *++e = SUB; *++e = PSH; *++e = IMM; *++e = sizeof(int*); *++e = DIV; ty = INT; }
			else if ((ty = t) > PTR) { *++e = PSH; *++e = IMM; *++e = sizeof(int*); *++e = MUL; *++e = SUB; }
			else *++e = SUB;
		}
		else if (tk == Mul) { next(); *++e = PSH; expr(Inc); *++e = MUL; ty = INT; }
		else if (tk == Div) { next(); *++e = PSH; expr(Inc); *++e = DIV; ty = INT; }
		else if (tk == Mod) { next(); *++e = PSH; expr(Inc); *++e = MOD; ty = INT; }
		else if (tk == Inc || tk == Dec) {
			if (*e == LC) { *e = PSH; *++e = LC; }
			else if (*e == LI) { *e = PSH; *++e = LI; }
			else { dprintf(STDERR, "%d: bad lvalue in post-increment\n", line); exit(-1); }
			*++e = PSH; *++e = IMM; *++e = (ty > PTR) ? sizeof(int*) : sizeof(char);
			*++e = (tk == Inc) ? ADD : SUB;
			*++e = (ty == CHAR) ? SC : SI;
			*++e = PSH; *++e = IMM; *++e = (ty > PTR) ? sizeof(int*) : sizeof(char);
			*++e = (tk == Inc) ? SUB : ADD;
			next();
		}
		else if (tk == Brak) {
			next(); *++e = PSH; expr(Assign);
			if (tk == ']') next(); else { dprintf(STDERR, "%d: close bracket expected\n", line); exit(-1); }
			if (t > PTR) { *++e = PSH; *++e = IMM; *++e = sizeof(int*); *++e = MUL;	}
			else if (t < PTR) { dprintf(STDERR, "%d: pointer type expected\n", line); exit(-1); }
			*++e = ADD;
			*++e = ((ty = t - PTR) == CHAR) ? LC : LI;
		}
		else { dprintf(STDERR, "%d: compiler error tk=%d\n", line, tk); exit(-1); }
	}
}

void stmt()
{
	int *a, *b;

	if (tk == If) {
		next();
		if (tk == '(') next(); else { dprintf(STDERR, "%d: open paren expected\n", line); exit(-1); }
		expr(Assign);
		if (tk == ')') next(); else { dprintf(STDERR, "%d: close paren expected\n", line); exit(-1); }
		*++e = BZ; b = ++e;
		stmt();
		if (tk == Else) {
			*b = (int)(e + 3); *++e = JMP; b = ++e;
			next();
			stmt();
		}
		*b = (int)(e + 1);
	}
	else if (tk == While) {
		next();
		a = e + 1;
		if (tk == '(') next(); else { dprintf(STDERR, "%d: open paren expected\n", line); exit(-1); }
		expr(Assign);
		if (tk == ')') next(); else { dprintf(STDERR, "%d: close paren expected\n", line); exit(-1); }
		*++e = BZ; b = ++e;
		stmt();
		*++e = JMP; *++e = (int)a;
		*b = (int)(e + 1);
	}
	else if (tk == Return) {
		next();
		if (tk != ';') expr(Assign);
		*++e = LEV;
		if (tk == ';') next(); else { dprintf(STDERR, "%d: semicolon expected\n", line); exit(-1); }
	}
	else if (tk == '{') {
		next();
		while (tk != '}') stmt();
		next();
	}
	else if (tk == ';') {
		next();
	}
	else {
		expr(Assign);
		if (tk == ';') next(); else { dprintf(STDERR, "%d: semicolon expected\n", line); exit(-1); }
	}
}

int run_cycle(int *process, int cycles) {
	int *pc, *sp, *bp, a; // vm registers
	int cycle, rem_cycle; // cycle counts
	int i, *t; // temps

	if(process[B_magic] != B_MAGIC) {
		dprintf(STDERR, "Invalid process magic: %d\n", process[B_magic]);
		process[B_halted] = 1;
		process[B_exitcode] = -1;
	}

	if(process[B_halted]) return 1;

	// Load process state
	pc    = (int*)process[B_pc];
	sp    = (int*)process[B_sp];
	bp    = (int*)process[B_bp];
	a     = process[B_a];
	cycle = process[B_cycle];
    rem_cycle = cycles;

	while(--rem_cycle > 0) {
		i = *pc++; ++cycle;
		if (debug) {
		  dprintf(STDERR, "%d> %.4s", cycle, &opcodes[i * 5]);
		  if (i <= ADJ) dprintf(STDERR, " %d\n", *pc); else dprintf(STDERR, "\n");
		}
		// Basic VM operations
		if      (i == LEA) a = (int)(bp + *pc++);                             // load local address
		else if (i == IMM) a = *pc++;                                         // load global address or immediate
		else if (i == JMP) pc = (int *)*pc;                                   // jump
		else if (i == JSR) { *--sp = (int)(pc + 1); pc = (int *)*pc; }        // jump to subroutine
		else if (i == BZ)  pc = a ? pc + 1 : (int *)*pc;                      // branch if zero
		else if (i == BNZ) pc = a ? (int *)*pc : pc + 1;                      // branch if not zero
		else if (i == ENT) { *--sp = (int)bp; bp = sp; sp = sp - *pc++; }     // enter subroutine
		else if (i == ADJ) sp = sp + *pc++;                                   // stack adjust
		else if (i == LEV) { sp = bp; bp = (int *)*sp++; pc = (int *)*sp++; } // leave subroutine
		else if (i == LI)  a = *(int *)a;                                     // load int
		else if (i == LC)  a = *(char *)a;                                    // load char
		else if (i == SI)  *(int *)*sp++ = a;                                 // store int
		else if (i == SC)  a = *(char *)*sp++ = a;                            // store char
		else if (i == PSH) *--sp = a;                                         // push
        // Basic arithmatic operations
		else if (i == OR)  a = *sp++ |  a;
		else if (i == XOR) a = *sp++ ^  a;
		else if (i == AND) a = *sp++ &  a;
		else if (i == EQ)  a = *sp++ == a;
		else if (i == NE)  a = *sp++ != a;
		else if (i == LT)  a = *sp++ <  a;
		else if (i == GT)  a = *sp++ >  a;
		else if (i == LE)  a = *sp++ <= a;
		else if (i == GE)  a = *sp++ >= a;
		else if (i == SHL) a = *sp++ << a;
		else if (i == SHR) a = *sp++ >> a;
		else if (i == ADD) a = *sp++ +  a;
		else if (i == SUB) a = *sp++ -  a;
		else if (i == MUL) a = *sp++ *  a;
		else if (i == DIV) a = *sp++ /  a;
		else if (i == MOD) a = *sp++ %  a;
        // C functions as VM operations
		else if (i == OPEN) a = open((char *)sp[1], *sp);
		else if (i == READ) a = read(sp[2], (char *)sp[1], *sp);
		else if (i == CLOS) a = close(*sp);
		else if (i == PRTF) { t = sp + pc[1]; a = printf((char *)t[-1], t[-2], t[-3], t[-4], t[-5], t[-6]); }
		else if (i == DPRT) { t = sp + pc[1]; a = dprintf(t[-1], (char *)t[-2], t[-3], t[-4], t[-5], t[-6], t[-7]); }
		else if (i == MALC) a = (int)malloc(*sp);
		else if (i == FREE) free((void *)*sp);
		else if (i == MSET) a = (int)memset((char *)sp[2], sp[1], *sp);
		else if (i == MCMP) a = memcmp((char *)sp[2], (char *)sp[1], *sp);
		else if (i == EXIT) {
			if(verbose) dprintf(STDERR, "exit(%d) cycle = %d\n", *sp, cycle);
			process[B_halted] = 1;
			process[B_exitcode] = *sp;
			rem_cycle = 0; 
		}
		else if (i == SYS1) { a = (int)syscall1(*sp); }
		else if (i == SYS2) { a = (int)syscall2(sp[1], *sp); }
		else if (i == SYS3) { a = (int)syscall3(sp[2], sp[1], *sp); }
		else if (i == SYS4) { a = (int)syscall4(sp[3], sp[2], sp[1], *sp); }
		else if (i == SYSI) { a = (int)syscall_init(sp[1], *sp); }
		else if (i == FDSZ) { a = fdsize(*sp); }
		else {
			dprintf(STDERR, "unknown instruction = %d! cycle = %d\n", i, cycle);
			process[B_halted] = 1;
			process[B_exitcode] = -1;
			rem_cycle = 0; 
		}
	}

	// Save process state
	process[B_pc] = (int)pc;
	process[B_sp] = (int)sp;
	process[B_bp] = (int)bp;
	process[B_a] = a;
	process[B_cycle] = cycle;

	return process[B_halted];
}

int *create_process(char *source, int argc, char **argv) {
	int *bp, *sp, *pc;
	int *t, i, bt, ty;
	int *process, *idmain, poolsz;
	poolsz = 256*1024; // arbitrary size

	if (!(process = (int*)malloc(__B * sizeof(int*)))) { dprintf(STDERR, "Could not malloc(%d) process space\n", __B); return 0; }

	// Reset globals
	if (!(sym = malloc(poolsz))) { dprintf(STDERR, "could not malloc(%d) symbol area\n", poolsz); return 0; }
	if (!(le = e = malloc(poolsz))) { dprintf(STDERR, "could not malloc(%d) text area\n", poolsz); return 0; }
	if (!(data = malloc(poolsz))) { dprintf(STDERR, "could not malloc(%d) data area\n", poolsz); return 0; }
	if (!(sp = malloc(poolsz))) { dprintf(STDERR, "could not malloc(%d) stack area\n", poolsz); return 0; }

	process[B_p_sym] = (int)sym;
	process[B_p_e] = (int)e;
	process[B_p_data] = (int)data;
	process[B_p_sp] = (int)sp;

	memset(sym,  0, poolsz);
	memset(e,    0, poolsz);
	memset(data, 0, poolsz);

	setup_symbols();

	i = Char; while (i <= While) { next(); id[Tk] = i++; } // add keywords to symbol table
	i = OPEN; while (i < _SYMS) { next(); id[Class] = Sys; id[Type] = INT; id[Val] = i++; } // add library to symbol table
	next(); id[Tk] = Char; // handle void type
	next(); idmain = id; // keep track of main

	// update global used in parsing process
	p = source;

	// parse declarations
	line = 1;
	next();
	while (tk) {
		bt = INT; // basetype
		if (tk == Int) next();
		else if (tk == Char) { next(); bt = CHAR; }
		else if (tk == Enum) {
			next();
			if (tk != '{') next();
			if (tk == '{') {
				next();
				i = 0;
				while (tk != '}') {
					if (tk != Id) { dprintf(STDERR, "%d: bad enum identifier %d\n", line, tk); return 0; }
					next();
					if (tk == Assign) {
						next();
						if (tk != Num) { dprintf(STDERR, "%d: bad enum initializer\n", line); return 0; }
						i = ival;
						next();
					}
					id[Class] = Num; id[Type] = INT; id[Val] = i++;
					if (tk == ',') next();
				}
				next();
			}
		}
		while (tk != ';' && tk != '}') {
			ty = bt;
			while (tk == Mul) { next(); ty = ty + PTR; }
			if (tk != Id) { dprintf(STDERR, "%d: bad global declaration\n", line); return 0; }
			if (id[Class]) { dprintf(STDERR, "%d: duplicate global definition for %s\n", line, id[Class]); return 0; }
			next();
			id[Type] = ty;
			if (tk == '(') { // function
				id[Class] = Fun;
				id[Val] = (int)(e + 1);
				next(); i = 0;
				while (tk != ')') {
					ty = INT;
					if (tk == Int) next();
					else if (tk == Char) { next(); ty = CHAR; }
					while (tk == Mul) { next(); ty = ty + PTR; }
					if (tk != Id) { dprintf(STDERR, "%d: bad parameter declaration\n", line); return 0; }
					if (id[Class] == Loc) { dprintf(STDERR, "%d: duplicate parameter definition\n", line); return 0; }
					id[HClass] = id[Class]; id[Class] = Loc;
					id[HType]  = id[Type];  id[Type] = ty;
					id[HVal]   = id[Val];   id[Val] = i++;
					next();
					if (tk == ',') next();
				}
				next();
				if (tk != '{') { dprintf(STDERR, "%d: bad function definition\n", line); return 0; }
				loc = ++i;
				next();
				while (tk == Int || tk == Char) {
					bt = (tk == Int) ? INT : CHAR;
					next();
					while (tk != ';') {
						ty = bt;
						while (tk == Mul) { next(); ty = ty + PTR; }
						if (tk != Id) { dprintf(STDERR, "%d: bad local declaration\n", line); return 0; }
						if (id[Class] == Loc) { dprintf(STDERR, "%d: duplicate local definition\n", line); return 0; }
						id[HClass] = id[Class]; id[Class] = Loc;
						id[HType]  = id[Type];  id[Type] = ty;
						id[HVal]   = id[Val];   id[Val] = ++i;
						next();
						if (tk == ',') next();
					}
					next();
				}
				*++e = ENT; *++e = i - loc;
				while (tk != '}') stmt();
				*++e = LEV;
				id = sym; // unwind symbol table locals
				while (id[Tk]) {
					if (id[Class] == Loc) {
						id[Class] = id[HClass];
						id[Type] = id[HType];
						id[Val] = id[HVal];
					}
					id = id + Idsz;
				}
			}
			else {
				id[Class] = Glo;
				id[Val] = (int)data;
				data = data + sizeof(int*);
			}
			if (tk == ',') next();
		}
		next();
	}

	if (!(pc = (int *)idmain[Val])) { dprintf(STDERR, "main() not defined\n"); return 0; }

	// setup stack
	bp = sp = (int *)((int)sp + poolsz);
    // write stack values and small function to exit if main returns
    //
    //  [BOTTOM OF STACK]
    //  $label0           // addr of return location, left on the
    //                       stack so that the LEV instruction sets
    //                       pc to the location of the PSH instruction.
    //  // arguments to main
    //  argv              //  char **argv
    //  argc              //  int    argc
    //  label0:           // return location, pc is set to this
    //  PSH               // push value returned by main
    //  EXIT              // and exit with it
    //  [TOP OF STACK]
	*--sp = EXIT; // call exit if main returns
	*--sp = PSH; t = sp;
	*--sp = argc;
	*--sp = (int)argv;
	*--sp = (int)t;

	free((void*)process[B_p_sym]);

	if(verbose) {
		dprintf(STDERR, "Process image information:\n");
		dprintf(STDERR, "  Emitted code size: %d\n", (int)(e - process[B_p_e]));
		dprintf(STDERR, "  Emitted data size: %d\n", (int)(data - process[B_p_data]));
		dprintf(STDERR, "  main() entry point: %d\n", (int)pc);
	}

	process[B_magic] = B_MAGIC;
	process[B_exitcode] = 0;
	process[B_sym] = (int)sym;
	process[B_e] = (int)e;
	process[B_data] = (int)data;
	process[B_bp] = (int)bp;
	process[B_sp] = (int)sp;
	process[B_pc] = (int)pc;
	process[B_a] = 0;
	process[B_cycle] = 0;
	process[B_halted] = 0;

	return process;
}

void free_process(int *process) {
	if(process[B_magic] != B_MAGIC) {
		printf("free_process(): invalid magic\n");
		return;
	}

	process[B_magic] = 0;
	free((void*)process[B_p_e]);
	free((void*)process[B_p_sp]);
	free((void*)process[B_p_data]);
	free((void*)process);
}

#if 0
// Dummy value for when c5.c is run indirectly
enum { SYS3_LISP_MAIN };
#endif

int c5_lispmain(int argc, char **argv) {
#if 0
	dprintf(STDERR, "Indirect call to c5_lispmain not supported\n");
	return 1;
#endif
	return syscall3(SYS3_LISP_MAIN, argc, (int)argv);
}

// Override name when compiling natively
#define main c5_main
int main(int argc, char **argv)
{
	int fd;
	char *pp, *tmp;
	int *process;
	int i, ii, srcsize, exitcode;

	vm_cycle_count = 1000;

	// Set globals
	setup_opcodes();
	sym = le = e = 0;
	data = 0;
	B_MAGIC = 0xBEEF;
	verbose = 0;

	// Allocate vm_processes
	vm_proc_max = 32;
	vm_proc_count = 0;
	if (!(vm_processes = (int*)malloc(vm_proc_max * sizeof(int*)))) { dprintf(STDERR, "Failed to allocate vm_processes area\n"); return -1; }
	memset(vm_processes, 0, vm_proc_max * sizeof(int*));

	--argc; ++argv;
	i = 0; // when to exit parameter parsing
	while(argc > 0 && **argv == '-' && i == 0) {
		// -s      show source and exit
		if ((*argv)[1] == 's') { src = 1; }
		// -d      show source during execution
		else if ((*argv)[1] == 'd') { debug = 1; }
		// -c 123  set cycle count to 123
		else if ((*argv)[1] == 'c') {
			--argc; ++argv;
			// inline atoi
			vm_cycle_count = 0;
			pp = *argv;
			while(*pp != 0) {
				vm_cycle_count = vm_cycle_count * 10 + (*pp++ - '0');
			}
			dprintf(STDERR, "Cycle count set to %i\n", vm_cycle_count);
		}
		// -r xyz  start additional process
		else if((*argv)[1] == 'r') {
			--argc; ++argv;
			vm_processes[vm_proc_count++] = (int)*argv;
			--argc; ++argv;
		}
		// -L      enter lispmain
		else if((*argv)[1] == 'L') {
#if 0
			// start lisp4.c instead
			vm_processes[vm_proc_count++] = (int)"lisp4.c";
			if(0) // Dummy out the next call
#else
				free(vm_processes);
#if 0
			if(0) // Dummy out the next call
#endif
			return c5_lispmain(argc, argv);
#endif
		}
		// -v      enable verbose mode
		else if((*argv)[1] == 'v') { verbose = 1; }
		// --      end parameter passing
		else if((*argv)[1] == '-') { i = 1; }
		else {
			dprintf(STDERR, "Invalid argument: %s\n", *argv);
			free(vm_processes);
			return -1;
		}
		--argc; ++argv;
	}
	if (vm_proc_count < 1 && argc < 1) { free(vm_processes); dprintf(STDERR, "usage: c5 [-L] [-s] [-d] [-v] [-c nnn] [-r file] file args...\n"); return -1; }
	if (argc > 0) vm_processes[vm_proc_count++] = (int)*argv;

	// Start all vm_processes
	i = 0;
	while(i < vm_proc_count) {
		tmp = (char*)vm_processes[i];
		if (verbose) dprintf(STDERR, "->Start process: %s\n", tmp);
		if ((fd = open(tmp, 0)) < 0) {
			dprintf(STDERR, "could not open(%s), ensure options are before filename\n", tmp); return -1;
		}
		srcsize = fdsize(fd) + 1;

		if (!(p = pp = malloc(srcsize))) { dprintf(STDERR, "could not malloc(%d) source area\n", srcsize); return -1; }
		if ((ii = read(fd, p, srcsize)) <= 0) { dprintf(STDERR, "read() returned %d\n", i); return -1; }
		p[ii] = 0;
		close(fd);
		process = create_process(p, argc, argv);
		free((void*)pp);
		if(process == 0) { dprintf(STDERR, "Invalid process\n"); return -1; }
		vm_processes[i++] = (int)process;
	}

	// run...
	i = 0;
	exitcode = 0;
	while(vm_proc_count > 0) {
		process = (int*)vm_processes[i];
		if(process) {
			if(run_cycle(process, vm_cycle_count) == 1) {
				// Finished
				if(debug) dprintf(STDERR, "Proc %d finished: %d\n", i, process[B_exitcode]);
				exitcode = process[B_exitcode];
				free_process(process);
				vm_processes[i] = 0;
				--vm_proc_count;
			}
		}
		++i;
		if(i == vm_proc_max) i = 0;
	}

	free((void*)vm_processes);

	return exitcode;
}
