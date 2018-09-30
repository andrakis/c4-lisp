#ifndef _EXTRAS_H
#define _EXTRAS_H

#include "core/native.h"

#ifdef __cplusplus
extern "C" {
#endif

// Defined in extras.c
NUMTYPE fdsize (int fd);
char *runtime_path(char *lib);

// Defined in c4.c
NUMTYPE *get_vm_active_process();
void *get_process_platform(NUMTYPE *process);
void  set_process_platform(void *platform, NUMTYPE *process);

#ifdef __cplusplus
}
#endif
#endif
