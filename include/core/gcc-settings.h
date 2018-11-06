#ifndef __GCC_SETTINGS_H
#define __GCC_SETTINGS_H

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

// Ignore formatting warnings
#pragma GCC diagnostic ignored "-Wformat"

// Macros for warning attributes
#define WARN_UNUSED __attribute__ ((warn_unused_result))
#endif
