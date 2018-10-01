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

// RTL "/lib" LIB "." CND_CONF "-" CND_PLATFORM "." RTL_SO
char *runtime_path(char *lib) {
	static char path_buf[MAX_PATH];
	char *p = path_buf;

	// TODO: insecure
	sprintf(p, "%s/lib%s.%s-%s.%s", RTL, lib, CND_CONF, CND_PLATFORM, RTL_SO);
	return p;
}