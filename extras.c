// Extra functions for interpreter

#include <sys/stat.h>
#include "native.h"

NUMTYPE fdsize (int fd) {
	struct stat st;
	fstat(fd, &st);
	return st.st_size;
}