dnl Process this file with autoconf to produce a configure script.

AC_INIT([zexy], [2.2.5], [zmoelnig@iem.at], [zexy], [http://get.puredata.info/zexy])
AM_INIT_AUTOMAKE([1.10 foreign])

AC_PROG_LIBTOOL()
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile abs/Makefile reference/Makefile])
AC_CONFIG_FILES([tests/Makefile build/Makefile])

AC_CONFIG_SRCDIR([src/zexy.c])

IEM_OPERATING_SYSTEM
ARCH=$(uname -m)
KERN=$(uname -s)


dnl Checks for programs.
AC_PROG_CC

AC_SUBST(STK)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(DEFS)
AC_SUBST(LFLAGS)
AC_SUBST(EXTENSION)
AC_SUBST(LD)
AC_SUBST(STRIP)
AC_SUBST(STRIPFLAGS)
AC_SUBST(LIBRARY_VERSION)
AC_SUBST(BUILDLIBRARY)
AC_SUBST(REFERENCEPATH)
AC_SUBST(PDLIBDIR)
AC_SUBST(INCLUDES)

AC_ARG_WITH(pd,       [  --with-pd=</path/to/pd> where to look for pd-headers and and -libs])
AC_ARG_WITH(extension,[  --with-extension=<ext>  enforce a certain extension for the dynamic library (e.g. dll)])
AC_ARG_ENABLE(library,[  --disable-library       split the library into single externals])
AM_CONDITIONAL([LIBRARY], [test "x${enable_library}" != "xno"])


if test "x${libdir}" = "x\${exec_prefix}/lib"; then
 libdir='${exec_prefix}/lib/pd/extra'
fi


if test "x$with_pd" != "x"; then
 if test -d "${with_pd}"; then
   PDPATH=${with_pd}
 fi
 if test -d "${PDPATH}/src"; then
  AC_MSG_RESULT([adding ${PDPATH}/src to INCLUDES])
  INCLUDES="-I${PDPATH}/src ${INCLUDES}"

  AC_MSG_RESULT([adding ${PDPATH}/src to LDFLAGS])
  LDFLAGS="-L${PDPATH}/src ${LDFLAGS}"
 else
  if test -d "${PDPATH}"; then
   AC_MSG_RESULT([adding ${PDPATH} to INCLUDES])
   INCLUDES="-I${PDPATH} ${INCLUDES}"
  fi
 fi
 if test -d "${PDPATH}/bin"; then
  AC_MSG_RESULT([adding ${PDPATH}/bin to LDFLAGS])
  LDFLAGS="-L${PDPATH}/bin ${LDFLAGS}"
 else
  if test -d "${PDPATH}"; then
   AC_MSG_RESULT([adding ${PDPATH} to LDFLAGS])
   LDFLAGS="-L${PDPATH} ${LDFLAGS}"
  fi
 fi
fi

dnl Checks for libraries.
dnl Replace `main' with a function in -lc:
AC_CHECK_LIB(c, main)
AC_CHECK_LIB(crtdll, fclose)

AC_CHECK_LIB(coldname, lseek)
AC_CHECK_LIB(kernel32, main)
AC_CHECK_LIB(user32, main)
AC_CHECK_LIB(gdi32, main)

dnl Replace `main' with a function in -lm:
AC_CHECK_LIB(m, main)
dnl Replace `main' with a function in -lpthread:
dnl AC_CHECK_LIB(pthread, main)
dnl Replace `main' with a function in -lstk:
dnl AC_CHECK_LIB(stk, main, STK=yes)

AC_CHECK_LIB(pd, nullfn)
AC_CHECK_LIB(regex, regcomp)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(stdlib.h stdio.h string.h math.h time.h sys/time.h regex.h alloca.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS(select socket strerror)

IEM_CHECK_FAT(ARCH_FLAG)
AC_SUBST(ARCH_FLAG)

IEM_CHECK_CFLAGS([-mms-bitfields])
IEM_CHECK_SIMD
IEM_CHECK_LPT

AC_MSG_CHECKING([extension])
if test "x$with_extension" != "x"
then
 EXTENSION=$with_extension
else
 case "$KERN" in
 Darwin)
   EXTENSION=pd_drawin
   ;;
 Linux)
   EXTENSION=pd_linux
   ;;
 IRIX64)
   EXTENSION=pd_irix6
   ;;
 IRIX32)
   EXTENSION=pd_irix5
   ;;
 IRIX64)
   EXTENSION=pd_irix6
   ;;
 *CYGWIN*)
   EXTENSION=dll
   ;;
 *MINGW*)
   EXTENSION=dll
   ;;
 *)
   EXTENSION=pd_linux
   ;;
 esac
fi
AC_MSG_RESULT([$EXTENSION])


AC_CHECK_HEADERS([pd/m_pd.h m_pd.h], , 
	AC_ERROR([m_pd.h is desperately needed!
        install pd and/or use
        "--with-pd=</path/to/pd/>"]))

AC_OUTPUT()
