#ifndef	_ERRNO_H
#define _ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>

#include <bits/errno.h>

extern __attribute__((section(".bss.errno"))) int errno;

#ifdef __cplusplus
}
#endif

#endif

