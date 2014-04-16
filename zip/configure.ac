dnl -*- autoconf -*-

dnl $Id$

dnl not sure exactly what the minimum version is (but 2.13 wont work)
AC_PREREQ(2.58)
AC_INIT(BOINCZIP, 1.0.0)
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_TARGET

dnl generate .tar.gz, .tar.bz2, .zip
dnl AM_INIT_AUTOMAKE(dist-bzip2 dist-zip)
AM_INIT_AUTOMAKE(dist-zip)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR(boinc_zip.cpp)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

dnl Heres another way to set up host-specific stuff
AM_CONDITIONAL(OS_DARWIN, [echo $host_os | grep '^darwin' > /dev/null])
if echo $host_os | grep '^darwin' >/dev/null ; then
  if test `uname -r | sed 's/\.//g'` -lt 800 ; then
    AC_DEFINE_UNQUOTED(DARWIN_10_3, [1],[Define to 1 if compiling under OS X 10.3 or earlier])
  fi
fi

AM_CONDITIONAL(OS_LINUX, [echo $host_os | grep '^linux' > /dev/null])
AM_CONDITIONAL(OS_WIN32_MINGW, [echo $host_os | grep '^mingw' > /dev/null])
AM_CONDITIONAL(OS_OS2, [echo $host_os | grep '^os2' > /dev/null])
AM_CONDITIONAL(OS_ARM_LINUX, [echo $host_alias | grep '^arm-linux' > /dev/null])

AC_CONFIG_FILES([
                 Makefile
		])

AC_CONFIG_HEADER([config.h])

AC_OUTPUT
