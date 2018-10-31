#ifndef __LISP_H
#define __LISP_H

#include <string.h>

#define min(A,B) ((A) < (B) ? (B) : (A))

#define BUF_MAX  1024
static char buf[BUF_MAX];
static char *copytostatic(char *s) {
	strncpy(&buf, s, min(strlen(s), BUF_MAX));
	return &buf;
}
#endif
