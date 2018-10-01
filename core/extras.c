// Extra functions for interpreter

#include <stdio.h>
#include <sys/stat.h>
#include "core/native.h"

NUMTYPE fdsize (int fd) {
	struct stat st;
	fstat(fd, &st);
	return st.st_size;
}

#define RTL "lib"
#define MAX_PATH 1024

char path_buf[MAX_PATH];

// RTL "/lib" LIB "." CND_CONF "-" CND_PLATFORM "." RTL_SO
char *runtime_path(char *lib) {
	char *p = path_buf;

	snprintf(p, MAX_PATH, "%s/lib%s.%s-%s.%s", RTL, lib, CND_CONF, CND_PLATFORM, RTL_SO);
	p[MAX_PATH] = 0;
	return p;
}