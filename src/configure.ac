#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([1.10 -Wall no-define foreign]) 

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([curl], curl_easy_getinfo)
AC_CHECK_LIB([m], pow)
AC_CHECK_LIB([pam], pam_get_user)
AC_CHECK_LIB([jsonparser], json_parse)

# Checks for header files.
AC_CHECK_HEADERS([limits.h stddef.h stdint.h stdlib.h string.h syslog.h])
AC_CHECK_HEADERS([security/pam_ext.h security/pam_appl.h security/pam_modules.h])
AC_CHECK_HEADERS([json-parser/json.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strchr strdup strerror strtol strnlen])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
