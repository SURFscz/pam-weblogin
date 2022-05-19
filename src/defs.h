#ifndef DEFS_H
#define DEFS_H

/* To enable the Dynamic Allocation Functions, features from ISO/IEC TR 24731-2:2010 */
#define __STDC_WANT_LIB_EXT2__ 1

/* compatibility with X/Open 7 (POSIX 2008) */
#define _XOPEN_SOURCE 700

/* compatibility with POSIX 2008 */
#define _POSIX_C_SOURCE 200809L

/* include most default macros */
#define _GNU_SOURCE

/* for advanced features on MacOS (such as vsyslog()) */
#define _DARWIN_C_SOURCE

#define UNUSED __attribute__ ((unused))

#endif
