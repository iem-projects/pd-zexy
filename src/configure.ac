dnl Process this file with autoconf to produce a configure script.
AC_INIT(zexy.c)

LIBNAME=zexy


dnl Checks for programs.
AC_PROG_CC

AC_SUBST(STK)
AC_SUBST(CFLAGS)
AC_SUBST(DFLAGS)
AC_SUBST(LFLAGS)
AC_SUBST(EXT)
AC_SUBST(LD)
AC_SUBST(STRIP)
AC_SUBST(STRIPFLAGS)
AC_SUBST(ZEXY_VERSION)
AC_SUBST(REFERENCEPATH)
AC_SUBST(PDLIBDIR)
AC_SUBST(INCLUDES)
AC_SUBST(SOURCES)
AC_SUBST(LIBNAME)



AC_ARG_WITH(pdversion, [  --with-pdversion=<ver>  enforce a certain pd-version (e.g. 0.37)])
AC_ARG_WITH(version,   [  --with-version=<ver>    enforce a certain zexy-version (e.g. 2.0)])
AC_ARG_WITH(extension, [  --with-extension=<ext>  enforce a certain extension for the dynamic library (e.g. dll)])
AC_ARG_ENABLE(lpt,    [  --enable-lpt            enable parallelport-support])
AC_ARG_ENABLE(icc,    [  --enable-icc            enable ICC-support])
if test `uname -s` = Darwin; 
then
AC_ARG_ENABLE(bundle, [  --enable-bundleloader   use bundle_loader (default: autodetect)])
fi

dnl Checks for libraries.
dnl Replace `main' with a function in -lc:
AC_CHECK_LIB(c, main)
AC_CHECK_LIB(crtdll, fclose)

AC_CHECK_LIB(coldname, lseek)
AC_CHECK_LIB(kernel32, main)

dnl Replace `main' with a function in -lm:
AC_CHECK_LIB(m, main)
dnl Replace `main' with a function in -lpthread:
dnl AC_CHECK_LIB(pthread, main)
dnl Replace `main' with a function in -lstk:
dnl AC_CHECK_LIB(stk, main, STK=yes)


dnl for now i can test ICC only on linux
dnl LATER we might want to use it for other platforms (namely:windoze) too

if test `uname -s` = Linux;
then
 AC_ARG_ENABLE(icc,    [  --enable-icc            enable ICC-support])
 if test "$enable_icc" = "yes"; then
  CC=icc
  LD=icc
 fi
fi

if test $includedir
then
 for id in $includedir
 do
  if test -d $id; then INCLUDES="-I$id $INCLUDES"; fi
 done
fi
if test $libdir
then
 for id in $libdir
 do
  if test -d $id; then LIBS="-L$id $LIBS"; fi
 done
fi

if test $includedir
then
 for id in $includedir
 do
   if test -d $id
   then 
    INCLUDES="-I$id $INCLUDES"
   fi
 done
fi

AC_CHECK_LIB(pd, nullfn)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(stdlib.h stdio.h string.h math.h time.h sys/time.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS(select socket strerror)

dnl check for "-mms-bitfields" cflag
dnl why is there no generic compiler-check for a given flag ?
dnl it would make things so easy: AC_CHECK_FLAG([-mms-bitfields],,)
AC_MSG_CHECKING("ms-bitfields")
cat > conftest.c << EOF
int main(){
  return 0;
}
EOF
if ${CC} ${INCLUDES} ${DFLAGS} -o conftest.o conftest.c ${CFLAGS} -mms-bitfields > /dev/null 2>&1
then
  echo "yes"
  CFLAGS="${CFLAGS} -mms-bitfields"
else
  echo "no"
fi



dnl if we don't have $LD set, we set it to $(CC)
dnl LD=${LD:=$CC}
if test "x$LD" = "x"
then
 if test "x$host" != "x"
 then
  LD=${host}-ld
  if $(which ${LD} > /dev/null)
  then
    :
  else
    LD=""
  fi
 fi
fi
LD=${LD:=$CC}



dnl if we don't have $STRIP set, we set it to ${host}-strip or strip
if test "x$STRIP" = "x"
then
 if test "x$host" != "x"
 then
  STRIP=${host}-strip
  if $(which ${host}-strip > /dev/null)
  then
    :
  else
    STRIP="echo fake strip"
  fi
 else
  STRIP=strip
 fi
fi
dnl STRIP=${STRIP:=strip}


DFLAGS="-DZEXY_LIBRARY"


dnl Checks for pd-version, to set the correct help-path
AC_MSG_CHECKING("pd\>=0.37")

if test "$with_pdversion" != ""
then
  echo -n "($with_pdversion)... "
  PD_VERSION="$with_pdversion"
else
 if test "x$cross_compiling" = "xno"
 then

cat > conftest.c << EOF
#include <stdio.h>
#include "m_pd.h"
int main(){
  printf("%d.%d\n", PD_MAJOR_VERSION, PD_MINOR_VERSION);
  return 0;
}
EOF
  if $CC $INCLUDES -o conftest.o conftest.c > /dev/null 2>&1
  then
   PD_VERSION=`./conftest.o`
  else
   PD_VERSION=""
  fi
 else
  dnl we are cross-compiling...
  echo -n "(X)..."
  PD_VERSION="0.38"
 fi
fi

let PD_MAJORVERSION=`echo $PD_VERSION | cut -d"." -f1`+0
let PD_MINORVERSION=`echo $PD_VERSION | cut -d"." -f2`+0

if test "$PD_MAJORVERSION" -gt 0 || test "$PD_MINORVERSION" -ge 37
then
  REFERENCEPATH=extra/help-
  echo "yes"
else
  REFERENCEPATH=doc/5.reference/
  echo "no"
fi

dnl check for zexy-version (but why...)
AC_MSG_CHECKING("zexy-version")

if test "$with_version" != ""
then
  echo -n "($with_version)...forced "
  ZEXY_VERSION="$with_version"
else
 if test "x$cross_compiling" = "xno"
 then
cat > conftest.c << EOF
#include <stdio.h>
#include "zexy.h"
int main(){
  printf("%s\n", VERSION);
  return 0;
}
EOF

  if $CC $INCLUDES $DFLAGS -o conftest.o conftest.c > /dev/null 2>&1
  then
   ZEXY_VERSION=`./conftest.o`
   echo "$ZEXY_VERSION"
  else
   ZEXY_VERSION="X"
   echo "(unknown)"
  fi
 else
   ZEXY_VERSION="X"
   echo "(X)"
 fi
fi


dnl
dnl  OK, checks which machines are here now
dnl this needs some rethinking when cross-compiling (?)
dnl

if test `uname -s` = Linux; 
then
  LFLAGS="-export_dynamic -shared"
  EXT=pd_linux	
  STRIPFLAGS="--strip-unneeded"
  if test "$enable_icc" = "yes"; then
    CC=icc
  fi

  if test $CC = "icc"
  then
    LD=$CC
    LFLAGS="-ip -ipo_obj "$LFLAGS
    CFLAGS="-ip -ipo_obj "$CFLAGS
  else
    :
  fi
fi

dnl This should use '-bundle_loader /path/to/pd/bin/pd' instead of'-undefined suppress'
dnl then strip might do something
if test `uname -s` = Darwin; 
then
  LD=cc
  EXT=pd_darwin
  STRIP="echo faking strip"
  STRIPFLAGS=

  if test "$enable_bundleloader" = ""; then
    if test "$PD_MAJORVERSION" -gt 0 || test "$PD_MINORVERSION" -ge 39; then
      enable_bundleloader="yes"
    else
      enable_bundleloader="no"
    fi
  fi
  if test "$enable_bundleloader" = "yes"; then
dnl i hope "prefix" points to where we want it...
dnl should it rather be @exec_prefix@ ???
    LFLAGS="-bundle -bundle_loader @prefix@/bin/pd"
  else
    LFLAGS="-bundle -undefined suppress -flat_namespace"
  fi
    

  if test "$enable_lpt" = "yes"; then
    AC_MSG_ERROR("lpt not supported on this platform");
  fi
  if test "$enable_lpt" = ""; then
    AC_MSG_WARN("lpt not supported on this platform - disabling");
    enable_lpt="no";
  fi
fi

if test `uname | sed -e 's/^MINGW.*/NT/'` = NT; 
then
  LD=gcc
  INCLUDES="-I@prefix@/src"
dnl mingw should (does?) define __WIN32__ which is now used
dnl  DFLAGS="-DMSW -DNT"
  LFLAGS="-shared @prefix@/bin/pd.dll"
  EXT=dll
else
  PDLIBDIR="/lib/pd"
dnl this is handled by AC_CHECK_LIB above
dnl  LIBS="-lc -lm"
fi
if test `uname -s` = IRIX64; 
then
  LFLAGS="-n32 -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
        -shared -rdata_shared"
  EXT=pd_irix6
  dnl DFLAGS="-DUNIX -DIRIX6"
  STRIPFLAGS="--strip-unneeded"
  if test "$enable_lpt" = "yes"; then
    AC_MSG_ERROR("lpt not supported on this platform");
  fi
  if test "$enable_lpt" = ""; then
    AC_MSG_WARN("lpt not supported on this platform - disabling");
    enable_lpt="no";
  fi
fi

if test `uname -s` = IRIX32; 
then
  LFLAGS="-o32 -DUNIX -DIRIX -O2 
         -shared -rdata_shared"
  EXT=pd_irix5
  dnl DFLAGS="-DUNIX -DIRIX5"
  STRIPFLAGS="--strip-unneeded"
  if test "$enable_lpt" = "yes"; then
    AC_MSG_ERROR("lpt not supported on this platform");
  fi
  if test "$enable_lpt" = ""; then
    AC_MSG_WARN("lpt not supported on this platform - disabling");
    enable_lpt="no";
  fi
fi


if test "x$with_extension" != "x"
then
 EXT=$with_extension
fi

AC_CHECK_HEADER(linux/ppdev.h, [ have_ppdev="yes" ], [ have_ppdev="no" ])

dnl check for LPT
AC_MSG_CHECKING("parallel-port")
if test "$enable_lpt" != "no"
then
  if test "$enable_lpt" = "yes"
  then
    DFLAGS="$DFLAGS -DZ_WANT_LPT"
    echo "yes (forced)"
  else

cat > conftest.c << EOF
# include <sys/io.h>
int main(){
 ioperm(0x3bc, 8, 1);
 outb(0, 0x3bc);
 ioperm(0x3bc, 8, 0);
 return 0;
}
EOF

   if $CC $INCLUDES -o conftest.o conftest.c > /dev/null 2>&1
   then
     DFLAGS="$DFLAGS -DZ_WANT_LPT"
     if test "x$have_ppdev" = "xyes"
     then
       DFLAGS="$DFLAGS -DHAVE_PPDEV"
       echo "yes (with device-support)"
     else
       echo "yes"
     fi
   else
     echo "no"
   fi
  fi
else
 echo "no (ignored)"
fi

AC_OUTPUT(Makefile)

rm -f conftest.*
./makesource.sh
