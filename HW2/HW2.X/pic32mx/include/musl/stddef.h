#ifndef _STDDEF_H
#define _STDDEF_H

#ifdef __cplusplus
/* ISO C++ says that the macro NULL must expand to an integer constant
   expression, hence '((void *) 0)' is not allowed in C++.  */
#if defined(__GNUG__)
/* GNU C++ has a __null macro that behaves like an integer ('int' or
   'long') but has the same size as a pointer.  Use that, to avoid
   warnings.  */
#define NULL __null
#else
#define NULL 0L
#endif /* __GNUG__ */
#else
#define NULL ((void *)0)
#endif /* __cplusplus */

#define __NEED_ptrdiff_t
#define __NEED_size_t
#define __NEED_wchar_t
#if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
#define __NEED_max_align_t
#endif

#include <bits/alltypes.h>

#if __GNUC__ > 3
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((size_t)( (char *)&(((type *)0)->member) - (char *)0 ))
#endif

#endif
