AC_INIT(pam_websso, 0.1)

dnl Locate standard tools
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_AIX
AC_LANG(C)

dnl Determine this platform
DC_CHK_OS_INFO

dnl Determine how to create shared objects on this platform
DC_GET_SHOBJFLAGS

dnl Determine how to strip executables and remove symbols
DC_FIND_STRIP_AND_REMOVESYMS(pam_websso.syms)

dnl Check for all required headers
AC_CHECK_HEADERS(security/pam_appl.h security/pam_modules.h,, [
        AC_WARN([Required header missing, compilation will likely fail.])
], [
#ifdef HAVE_SECURITY_PAM_APPL_H
#  include <security/pam_appl.h>
#endif
#ifdef HAVE_SECURITY_PAM_MODULES_H
#  include <security/pam_modules.h>
#endif
])

dnl Check for PAM libraries
AC_CHECK_LIB(pam, pam_get_user)

dnl Produce Makefile
AC_OUTPUT(Makefile pam_websso.syms)
