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

char *p, *lp, // current position in source code
     *data,   // data/bss pointer
     *opcodes;// opcodes string

int *e, *le,  // current position in emitted code
    *id,      // currently parsed identifier
    *sym,     // symbol table (simple list of identifiers)
    tk,       // current token
    ival,     // current token value
    ty,       // current expression type
    loc,      // local variable offset
    line,     // current line number
    debug,    // print executed instructions
    verbose;  // print more detailed info

// tokens and classes (operators last and in precedence order)
enum {
	Num = 128, Fun, Sys, Glo, Loc, Id,
	Char, Else, Enum, If, Int, Return, Sizeof, While,
	Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// opcodes
enum {
	LEA ,IMM ,JMP ,JSR ,BZ  ,BNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PSH ,
	OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
	OPEN,READ,CLOS,PRTF,DPRT,MALC,FREE,MSET,MCMP,EXIT,FDSZ,
	// Platform related functions
	PINI,RPTH,
	// VM related functions
	PCHG /* Process changed */,
	// Our syscalls
	SYS1,SYS2,SYS3,SYS4,SYSI,
	// Pointer to last element
	_SYMS
};

// Sets up the static data containing opcodes (for source output)
void setup_opcodes() {
	// Must be in same order as instructions enum
	opcodes =
		// VM primitives
		"LEA ,IMM ,JMP ,JSR ,BZ  ,BNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,"
		"SC  ,PSH ,OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,"
		"SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
		// C functions
		"OPEN,READ,CLOS,PRTF,DPRT,MALC,FREE,MSET,MCMP,EXIT,FDSZ,"
		"PINI,RPTH,"
		"PCHG,"
		// Syscalls
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
		// Dynamci runtime platform functions
		"platform_init runtime_path "
		"process_changed "
		// Syscalls
		"syscall1 syscall2 syscall3 syscall4 syscall_init "
		// void data type
		"void "
		// main
		"main";
}

// types
enum { CHAR, INT, PTR };

// identifier offsets (since we can't create an ident struct)
enum { Tk, Hash, Name, NameLen, Class, Type, Val, HClass, HType, HVal, Idsz };

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

char *srcprefix;

void print_data_string (char *str, int len) {
	char *start, *end;
	char ch;

	printf("%c", '"');

	start = str;
	end = str;
	while(len-- >= 0) {
		ch = *str++;
		++end;
		// Special character handling
		if(ch == '\n' || ch == 0 || ch == '\\' || ch < ' ') {
			if(ch == '\n') ch = 'n';
			else if(ch == '\r') ch = 'r';
			else if(ch == 0) ch = '0';
			printf("%.*s", (end - 1) - start, start);
			if(ch < ' ')
				printf("<%ld>", (int)ch);
			else
				printf("\\%c", ch);
			start = end;
		}
	}

	if(start != end) {
		printf("%.*s", end - start, start);
	}

	printf("%c", '"');
}

void next()
{
	char *pp;
	int t;

	while (tk = *p) {
		++p;
		if (tk == '\n') {
			printf("%s %d: %.*s", srcprefix, line, p - lp, lp);
			lp = p;
			while (le < e) {
				printf("%8.4s", &opcodes[*++le * 5]);
				if (*le <= ADJ) printf(" %ld\n", *++le); else printf("\n");
			}
			++line;
		}
		else if (tk == '#') {
			pp = p;
			while (*p != 0 && *p != '\n') ++p;
			printf("%s #%.*s\n", srcprefix, p - pp, pp);
		}
		else if ((tk >= 'a' && tk <= 'z') || (tk >= 'A' && tk <= 'Z') || tk == '_') {
			// Symbol lookup
			pp = p - 1;
			while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_')
				tk = tk * 147 + *p++;
			tk = (tk << 6) + (p - pp);
			id = sym;
			while (id[Tk]) {
				if (tk == id[Hash] && !memcmp((char *)id[Name], pp, p - pp)) {
					// Symbol found (simple match on hash, full memcmp if hash match.
					// Update token reference.
					tk = id[Tk]; return;
				}
				id = id + Idsz;
			}
			// Token not found, create it.
			id[Name] = (int)pp;
			id[NameLen] = p - pp;
			id[Hash] = tk;
			tk = id[Tk] = Id;
			return;
		}
		else if (tk >= '0' && tk <= '9') { // Number parsing
			// Decimal number
			if (ival = tk - '0') { while (*p >= '0' && *p <= '9') ival = ival * 10 + *p++ - '0'; }
			// Hexadecimal number
			else if (*p == 'x' || *p == 'X') {
				while ((tk = *++p) && ((tk >= '0' && tk <= '9') || (tk >= 'a' && tk <= 'f') || (tk >= 'A' && tk <= 'F')))
					ival = ival * 16 + (tk & 15) + (tk >= 'A' ? 9 : 0);
			}
			// Octal
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
		else if (tk == '\'' || tk == '"') { // Quotation parsing
			pp = data;
			while (*p != 0 && *p != tk) {
				// Escape codes in string. TODO: add more support.
				if ((ival = *p++) == '\\') {
					if ((ival = *p++) == 'n') ival = '\n';
				}
				// String only: update data pointer
				if (tk == '"') *data++ = ival;
			}
			++p;
			// String or number?
			if (tk == '"') ival = (int)pp; else tk = Num;
			// If data was allocated, output it
			if(data - pp > 0) {
				printf("    data len %ld addr %ld ", data - pp, ival);
				print_data_string(pp, data - pp);
				printf("\n");
			}
			return;
		}
		else if (tk == '=') { if (*p == '=') { ++p; tk = Eq; } else tk = Assign; return; }
		else if (tk == '+') { if (*p == '+') { ++p; tk = Inc; } else tk = Add; return; }
		else if (tk == '-') {
			if (*p == '-') {
				// Decrement
				++p; tk = Dec;
			} else if(*p >= '0' && *p <= '9') {
				// Handle number and make it negative
				next();
				if(tk != Num) // Must be a number
					tk = 0;
				else
					ival = ival * -1;
			} else
				tk = Sub;
			return; 
		}
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

	if (!tk) { printf("%d: unexpected eof in expression\n", line); exit(-1); }
	else if (tk == Num) { *++e = IMM; *++e = ival; next(); ty = INT; }
	else if (tk == '"') {
		*++e = IMM; *++e = ival; next();
		while (tk == '"') next();
		data = (char *)((int)data + sizeof(int*) & -sizeof(int*)); ty = PTR;
	}
	else if (tk == Sizeof) {
		next(); if (tk == '(') next(); else { printf("%d: open paren expected in sizeof\n", line); exit(-1); }
		ty = INT; if (tk == Int) next(); else if (tk == Char) { next(); ty = CHAR; }
		while (tk == Mul) { next(); ty = ty + PTR; }
		if (tk == ')') next(); else { printf("%d: close paren expected in sizeof\n", line); exit(-1); }
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
			else { printf("%d: bad function call\n", line); exit(-1); }
			if (t) { *++e = ADJ; *++e = t; }
			ty = d[Type];
		}
		else if (d[Class] == Num) { *++e = IMM; *++e = d[Val]; ty = INT; }
		else {
			if (d[Class] == Loc) { *++e = LEA; *++e = loc - d[Val]; }
			else if (d[Class] == Glo) { *++e = IMM; *++e = d[Val]; }
			else { printf("%d: undefined variable\n", line); exit(-1); }
			*++e = ((ty = d[Type]) == CHAR) ? LC : LI;
		}
	}
	else if (tk == '(') {
		next();
		if (tk == Int || tk == Char) {
			t = (tk == Int) ? INT : CHAR; next();
			while (tk == Mul) { next(); t = t + PTR; }
			if (tk == ')') next(); else { printf("%d: bad cast\n", line); exit(-1); }
			expr(Inc);
			ty = t;
		}
		else {
			expr(Assign);
			if (tk == ')') next(); else { printf("%d: close paren expected\n", line); exit(-1); }
		}
	}
	else if (tk == Mul) {
		next(); expr(Inc);
		if (ty > INT) ty = ty - PTR; else { printf("%d: bad dereference\n", line); exit(-1); }
		*++e = (ty == CHAR) ? LC : LI;
	}
	else if (tk == And) {
		next(); expr(Inc);
		if (*e == LC || *e == LI) --e; else { printf("%d: bad address-of\n", line); exit(-1); }
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
		else { printf("%d: bad lvalue in pre-increment\n", line); exit(-1); }
		*++e = PSH;
		*++e = IMM; *++e = (ty > PTR) ? sizeof(int*) : sizeof(char);
		*++e = (t == Inc) ? ADD : SUB;
		*++e = (ty == CHAR) ? SC : SI;
	}
	else { printf("%d: bad expression\n", line); exit(-1); }

	while (tk >= lev) { // "precedence climbing" or "Top Down Operator Precedence" method
		t = ty;
		if (tk == Assign) {
			next();
			if (*e == LC || *e == LI) *e = PSH; else { printf("%d: bad lvalue in assignment\n", line); exit(-1); }
			expr(Assign); *++e = ((ty = t) == CHAR) ? SC : SI;
		}
		else if (tk == Cond) {
			next();
			*++e = BZ; d = ++e;
			expr(Assign);
			if (tk == ':') next(); else { printf("%d: conditional missing colon\n", line); exit(-1); }
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
			else { printf("%d: bad lvalue in post-increment\n", line); exit(-1); }
			*++e = PSH; *++e = IMM; *++e = (ty > PTR) ? sizeof(int*) : sizeof(char);
			*++e = (tk == Inc) ? ADD : SUB;
			*++e = (ty == CHAR) ? SC : SI;
			*++e = PSH; *++e = IMM; *++e = (ty > PTR) ? sizeof(int*) : sizeof(char);
			*++e = (tk == Inc) ? SUB : ADD;
			next();
		}
		else if (tk == Brak) {
			next(); *++e = PSH; expr(Assign);
			if (tk == ']') next(); else { printf("%d: close bracket expected\n", line); exit(-1); }
			if (t > PTR) { *++e = PSH; *++e = IMM; *++e = sizeof(int*); *++e = MUL;	}
			else if (t < PTR) { printf("%d: pointer type expected\n", line); exit(-1); }
			*++e = ADD;
			*++e = ((ty = t - PTR) == CHAR) ? LC : LI;
		}
		else { printf("%d: compiler error tk=%d\n", line, tk); exit(-1); }
	}
}

void stmt()
{
	int *a, *b;

	if (tk == If) {
		next();
		if (tk == '(') next(); else { printf("%d: open paren expected\n", line); exit(-1); }
		expr(Assign);
		if (tk == ')') next(); else { printf("%d: close paren expected\n", line); exit(-1); }
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
		if (tk == '(') next(); else { printf("%d: open paren expected\n", line); exit(-1); }
		expr(Assign);
		if (tk == ')') next(); else { printf("%d: close paren expected\n", line); exit(-1); }
		*++e = BZ; b = ++e;
		stmt();
		*++e = JMP; *++e = (int)a;
		*b = (int)(e + 1);
	}
	else if (tk == Return) {
		next();
		if (tk != ';') expr(Assign);
		*++e = LEV;
		if (tk == ';') next(); else { printf("%d: semicolon expected\n", line); exit(-1); }
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
		if (tk == ';') next(); else { printf("%d: semicolon expected\n", line); exit(-1); }
	}
}

int assemble(char *source) {
	int *bp, *sp, *pc;
	int *t, i, bt, ty;
	int *process, *idmain, poolsz;
	poolsz = 256*1024; // arbitrary size

	if (!(process = (int*)malloc(__B * sizeof(int*)))) { printf("Could not malloc(%d) process space\n", __B); return 0; }

	// Reset globals
	if (!(sym = malloc(poolsz))) { printf("could not malloc(%d) symbol area\n", poolsz); return 0; }
	if (!(le = e = malloc(poolsz))) { printf("could not malloc(%d) text area\n", poolsz); return 0; }
	if (!(data = malloc(poolsz))) { printf("could not malloc(%d) data area\n", poolsz); return 0; }
	if (!(sp = malloc(poolsz))) { printf("could not malloc(%d) stack area\n", poolsz); return 0; }

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
					if (tk != Id) { printf("%d: bad enum identifier %d\n", line, tk); return -1; }
					next();
					if (tk == Assign) {
						next();
						if (tk != Num) { printf("%d: bad enum initializer\n", line); return -1; }
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
			if (tk != Id) { printf("%d: bad global declaration\n", line); return -1; }
			if (id[Class]) { printf("%d: duplicate global definition for %s\n", line, id[Class]); return -1; }
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
					if (tk != Id) { printf("%d: bad parameter declaration\n", line); return -1; }
					if (id[Class] == Loc) { printf("%d: duplicate parameter definition\n", line); return -1; }
					id[HClass] = id[Class]; id[Class] = Loc;
					id[HType]  = id[Type];  id[Type] = ty;
					id[HVal]   = id[Val];   id[Val] = i++;
					next();
					if (tk == ',') next();
				}
				next();
				if (tk != '{') { printf("%d: bad function definition\n", line); return -1; }
				loc = ++i;
				next();
				while (tk == Int || tk == Char) {
					bt = (tk == Int) ? INT : CHAR;
					next();
					while (tk != ';') {
						ty = bt;
						while (tk == Mul) { next(); ty = ty + PTR; }
						if (tk != Id) { printf("%d: bad local declaration\n", line); return -1; }
						if (id[Class] == Loc) { printf("%d: duplicate local definition\n", line); return -1; }
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

	if (!(pc = (int *)idmain[Val])) { printf("main() not defined\n"); return 0; }
	
	
	id = (int*)sym;
	i = 0;
	while(id[Tk]) {
		if(1 || id[Class] && id[Class] != Sys) {
			printf("symbol %.*s eq 0x%x: ", id[NameLen], (char*)id[Name], id[Val]);
			print_data_string((char*)id, Idsz);
			printf("\n");
		}
		id = id + Idsz;
	}
	//free((void*)process[B_p_sym]);

	printf("Data segment %ld: ", data - process[B_p_data]);
	print_data_string((char*)process[B_p_data], data - process[B_p_data]);
	printf("\n");

	if(verbose) {
		printf("Process image information:\n");
		printf("  Emitted code size: %d\n", (int)(e - process[B_p_e]));
		printf("  Emitted data size: %d\n", (int)(data - process[B_p_data]));
		printf("  main() entry point: %d\n", (int)pc);
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

	return 0;
}

int main(int argc, char **argv)
{
	int fd;
	char *pp, *tmp;
	int i, ii, srcsize, cycle_count, exitcode;

	// Set globals
	setup_opcodes();
	sym = le = e = 0;
	data = 0;
	B_MAGIC = 0xBEEF;
	verbose = 0;
	srcprefix = ";; ";

	--argc; ++argv;
	i = 0; // when to exit parameter parsing
	while(argc > 0 && **argv == '-' && i == 0) {
		if ((*argv)[1] == 'd') { debug = 1; }
		// -v      enable verbose mode
		else if((*argv)[1] == 'v') { verbose = 1; }
		// --      end parameter passing
		else if((*argv)[1] == '-') { i = 1; }
		else {
			printf("Invalid argument: %s\n", *argv);
			return -1;
		}
		--argc; ++argv;
	}
	if (argc < 1) { printf("usage: chasm file.c\n"); return -1; }

	while(argc > 0) {
		if (verbose) printf("->Process: %s\n", tmp);
		if ((fd = open(*argv, 0)) < 0) {
			printf("could not open(%s), ensure options are before filename\n", *argv); return -1;
		}
		srcsize = fdsize(fd) + 1;

		if (!(p = pp = malloc(srcsize))) { printf("could not malloc(%d) source area\n", srcsize); return -1; }
		if ((ii = read(fd, p, srcsize)) <= 0) { printf("read() returned %d\n", i); return -1; }
		p[ii] = 0;
		close(fd);
		i = assemble(p);
		if(i < 0)
			return i;
		--argc; ++argv;
	}

	return 0;
}

