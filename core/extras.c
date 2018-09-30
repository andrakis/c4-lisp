// Extra functions for interpreter

#include <stdio.h>
#include <sys/stat.h>
#include "core/native.h"

NUMTYPE fdsize (int fd) {
	struct stat st;
	fstat(fd, &st);
	return st.st_size;
}

#define RTL "rtl"
#define MAX_PATH 1024

//#define runtime_path(LIB) RTL "/" CND_PLATFORM "/lib" LIB "." CND_CONF "." RTL_SO
char *runtime_path(char *lib) {
	static char path_buf[MAX_PATH];
	char *p = path_buf;

	// TODO: insecure
	sprintf(p, "%s/%s/lib%s.%s.%s", RTL, CND_PLATFORM, lib, CND_CONF, RTL_SO);
	return p;
}