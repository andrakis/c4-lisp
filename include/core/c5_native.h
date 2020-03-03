/* 
 * File:   c5_native.h
 * Author: daedalus
 *
 * Created on 29 August 2018, 1:49 AM
 *
 * Included by programs that want to run in both C4/C5 interpreter and
 * compile normally under GCC/etc.
 * Contains additional includes for C4/C5 instructions, such that one
 * does not need to remember to #include them constantly in .c4 programs.
 */

#ifndef C5_NATIVE_H
#define C5_NATIVE_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define C5_NATIVE
#include "native.h"

#endif /* C5_NATIVE_H */

