#ifndef _EXTRAS_H
#define _EXTRAS_H

#include <sys/stat.h>
#include "core/native.h"

#ifdef __cplusplus
extern "C" {
#endif

static NUMTYPE fdsize (int fd) {
	struct stat st;
	fstat(fd, &st);
	return st.st_size;
}

#define RTL "lib"
#define MAX_PATH 1024

// RTL "/lib" LIB "." CND_CONF "-" CND_PLATFORM "." RTL_SO
static char *runtime_path(char *lib) {
	static char path_buf[MAX_PATH];
	char *p = path_buf;

	snprintf(p, MAX_PATH, "%s/lib%s.%s-%s.%s", RTL, lib, CND_CONF, CND_PLATFORM, RTL_SO);
	p[MAX_PATH] = 0;
	return p;
}

// Defined in c4.c
NUMTYPE *get_vm_active_process();
void *get_process_platform(NUMTYPE *process);
void  set_process_platform(void *platform, NUMTYPE *process);

#ifdef __cplusplus
}
#endif
#endif
