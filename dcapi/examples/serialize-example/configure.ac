AC_INIT([Example Application for master checkpoint], [6.00], [desktopgrid@lpds.sztaki.hu], [serialize-example])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([client.c])

dnl The platform code uses the target triplet
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC

dnl We need sed for substituting the XML templates
AC_PATH_PROG([SED], [sed])

dnl Enable extra warnings if we are using GCC
if test "$ac_cv_c_compiler_gnu" = yes; then
	CFLAGS="-Wall -Wmissing-prototypes -Wstrict-prototypes -Wsign-compare ${CFLAGS}"
fi

AC_ARG_ENABLE([client], AS_HELP_STRING([--disable-client], [do not build the client application]),, [enable_client=yes])
AC_ARG_ENABLE([master], AS_HELP_STRING([--disable-master], [do not build the master application]),, [enable_master=yes])

AM_CONDITIONAL([BUILD_CLIENT], [test "$enable_client" = yes])
AM_CONDITIONAL([BUILD_MASTER], [test "$enable_master" = yes])

if test "$enable_client" = yes; then
	SZDG_DCAPI_CLIENT([], [AC_MSG_ERROR([DC-API client libraries were not found])])
fi

if test "$enable_master" = yes; then
	SZDG_DCAPI_MASTER([], [AC_MSG_ERROR([DC-API master libraries were not found])])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
