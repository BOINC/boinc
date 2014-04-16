dnl -*- autoconf -*-

AC_INIT([boinczip], [1.0.0])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([configure.ac])
AC_PROG_CXX

AM_CONDITIONAL(OS_DARWIN, [echo $host_os | grep '^darwin' > /dev/null])
AM_CONDITIONAL(OS_LINUX, [echo $host_os | grep '^linux' > /dev/null])
AM_CONDITIONAL(OS_WIN32_MINGW, [echo $host_os | grep '^mingw' > /dev/null])
AM_CONDITIONAL(OS_OS2, [echo $host_os | grep '^os2' > /dev/null])
AM_CONDITIONAL(OS_ARM_LINUX, [echo $host_alias | grep '^arm-linux' > /dev/null])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
