dnl Usage:
dnl    DC_TEST_SHOBJFLAGS(shobjflags, shobjldflags, action-if-not-found)
dnl
AC_DEFUN(DC_TEST_SHOBJFLAGS, [
  AC_SUBST(SHOBJFLAGS)
  AC_SUBST(SHOBJLDFLAGS)

  OLD_LDFLAGS="$LDFLAGS"
  SHOBJFLAGS=""

  LDFLAGS="$OLD_LDFLAGS $1 $2"

  AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHOBJFLAGS="$1"; SHOBJLDFLAGS="$2" ], [
    LDFLAGS="$OLD_LDFLAGS"
    $3
  ])

  LDFLAGS="$OLD_LDFLAGS"
])

AC_DEFUN(DC_GET_SHOBJFLAGS, [
  AC_SUBST(SHOBJFLAGS)
  AC_SUBST(SHOBJLDFLAGS)

  AC_MSG_CHECKING(how to create shared objects)

  if test -z "$SHOBJFLAGS" -a -z "$SHOBJLDFLAGS"; then
    DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-shared -rdynamic], [
      DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-shared], [
	DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-shared -rdynamic -mimpure-text], [
	  DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-shared -mimpure-text], [
	    DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-shared -rdynamic -Wl,-G,-z,textoff], [
	      DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-shared -Wl,-G,-z,textoff], [
		DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-shared -dynamiclib -flat_namespace -undefined suppress -bind_at_load], [
		  DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-dynamiclib -flat_namespace -undefined suppress -bind_at_load], [
		    DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-Wl,-dynamiclib -Wl,-flat_namespace -Wl,-undefined,suppress -Wl,-bind_at_load], [
		      DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-dynamiclib -flat_namespace -undefined suppress], [
		        DC_TEST_SHOBJFLAGS([-fPIC -DPIC], [-dynamiclib], [
		          AC_MSG_RESULT(cant)
		          AC_MSG_ERROR([We are unable to make shared objects.])
                        ])
		      ])
		    ])
		  ])
		])
	      ])
	    ])
	  ])
	])
      ])
    ])
  fi

  AC_MSG_RESULT($SHOBJLDFLAGS $SHOBJFLAGS)

  DC_SYNC_SHLIBOBJS
])

AC_DEFUN(DC_SYNC_SHLIBOBJS, [
  AC_SUBST(SHLIBOBJS)
  SHLIBOBJS=""
  for obj in $LIB@&t@OBJS; do
    SHLIBOBJS="$SHLIBOBJS `echo $obj | sed 's/\.o$/_shr.o/g'`"
  done
])

AC_DEFUN(DC_SYNC_RPATH, [
  OLD_LDFLAGS="$LDFLAGS"

  for tryrpath in "-Wl,-rpath" "-Wl,--rpath" "-Wl,-R"; do
    LDFLAGS="$OLD_LDFLAGS $tryrpath -Wl,/tmp"
    AC_LINK_IFELSE(AC_LANG_PROGRAM([], [ return(0); ]), [
      rpathldflags="$tryrpath"
      break
    ])
  done
  unset tryrpath

  LDFLAGS="$OLD_LDFLAGS"
  unset OLD_LDFLAGS

  ADDLDFLAGS=""
  for opt in $LDFLAGS; do
    if echo "$opt" | grep '^-L' >/dev/null; then
      rpathdir=`echo "$opt" | sed 's@^-L *@@'`
      ADDLDFLAGS="$ADDLDFLAGS $rpathldflags -Wl,$rpathdir"
    fi
  done
  unset opt rpathldflags

  LDFLAGS="$LDFLAGS $ADDLDFLAGS"

  unset ADDLDFLAGS
])

AC_DEFUN(DC_CHK_OS_INFO, [
	AC_CANONICAL_HOST
	AC_SUBST(SHOBJEXT)
	AC_SUBST(SHOBJFLAGS)
	AC_SUBST(SHOBJLDFLAGS)
	AC_SUBST(CFLAGS)
	AC_SUBST(CPPFLAGS)
	AC_SUBST(AREXT)

	AC_MSG_CHECKING(host operating system)
	AC_MSG_RESULT($host_os)

	SHOBJEXT="so"
	AREXT="a"

	case $host_os in
		darwin*)
			SHOBJEXT="dylib"
			;;
		hpux*)
			SHOBJEXT="sl"
			;;
		mingw32msvc*)
			SHOBJEXT="dll"
			SHOBJFLAGS="-DPIC"
			CFLAGS="$CFLAGS -mno-cygwin -mms-bitfields"
			CPPFLAGS="$CPPFLAGS -mno-cygwin -mms-bitfields"
			SHOBJLDFLAGS='-shared -Wl,--dll -Wl,--enable-auto-image-base -Wl,--output-def,$[@].def,--out-implib,$[@].a'
			;;
		cygwin*)
			SHOBJEXT="dll"
			SHOBJFLAGS="-fPIC -DPIC"
			CFLAGS="$CFLAGS -mms-bitfields"
			CPPFLAGS="$CPPFLAGS -mms-bitfields"
			SHOBJLDFLAGS='-shared -Wl,--enable-auto-image-base -Wl,--output-def,$[@].def,--out-implib,$[@].a'
			;;
	esac
])
AC_DEFUN(DC_SETVERSIONSCRIPT, [
	VERSIONSCRIPT="$1"
	SYMFILE="$2"

	delete_symfile='0'
	if test ! -f "${SYMFILE}"; then
		delete_symfile='1'

		echo '' > "${SYMFILE}"
	fi

	SAVE_LDFLAGS="${LDFLAGS}"

	AC_MSG_CHECKING([for how to set version script])

	for tryaddldflags in "-Wl,--version-script -Wl,${VERSIONSCRIPT}" "-Wl,-exported_symbols_list -Wl,${SYMFILE}"; do
		LDFLAGS="${SAVE_LDFLAGS} ${tryaddldflags}"
		AC_TRY_LINK([], [], [
			addldflags="${tryaddldflags}"

			break
		])
	done

	if test "${delete_symfile}" = "1"; then
		rm -f "${SYMFILE}"
	fi

	if test -n "${addldflags}"; then
		LDFLAGS="${SAVE_LDFLAGS} ${addldflags}"
		AC_MSG_RESULT($addldflags)
	else
		LDFLAGS="${SAVE_LDFLAGS}"
		AC_MSG_RESULT([don't know])
	fi
])

AC_DEFUN(DC_FIND_STRIP_AND_REMOVESYMS, [
	SYMFILE="$1"

	dnl Determine how to strip executables
	AC_CHECK_TOOLS(OBJCOPY, objcopy gobjcopy, [false])
	AC_CHECK_TOOLS(STRIP, strip gstrip, [false])

	if test "x${STRIP}" = "xfalse"; then
		STRIP="${OBJCOPY}"
	fi

	WEAKENSYMS='true'
	REMOVESYMS='true'
	SYMPREFIX=''

	case $host_os in
		darwin*)
			REMOVESYMS="${STRIP} -i -u -s ${SYMFILE}"
			SYMPREFIX="_"
			;;
		*)
			if test "x${OBJCOPY}" != "xfalse"; then
				WEAKENSYMS="${OBJCOPY} --keep-global-symbols=${SYMFILE}"
				REMOVESYMS="${OBJCOPY} --discard-all"
			elif test "x${STRIP}" != "xfalse"; then
				REMOVESYMS="${STRIP} -x"
			fi
			;;
	esac

	AC_SUBST(WEAKENSYMS)
	AC_SUBST(REMOVESYMS)
	AC_SUBST(SYMPREFIX)
])
