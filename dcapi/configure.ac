AC_INIT([Distributed Computing API], [0.2], [podhorszki@sztaki.hu], [dcapi])
AC_CONFIG_MACRO_DIR([cf])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_SRCDIR([include/dc.h])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

LIBTOOL="$LIBTOOL --silent"

if test "$ac_cv_c_compiler_gnu" = yes; then
	CFLAGS="-Wall -Wmissing-prototypes -Wstrict-prototypes -Wsign-compare ${CFLAGS}"
	CXXFLAGS="-Wall -Wsign-compare ${CXXFLAGS}"
fi

SZDG_BOINC_SERVER
if test "$no_boinc" != yes; then
	AC_CHECK_HEADERS([uuid/uuid.h],, [AC_MSG_ERROR([libuuid headers are missing])])
	AC_CHECK_LIB([uuid], [uuid_generate], [true], [AC_MSG_ERROR([libuuid is missing])])
fi

AC_ARG_WITH([clgr], AS_HELP_STRING([--with-clgr@<:@=DIR@:>@],
	[Use ClusterGrid (installed in DIR)]),, [with_clgr=yes])

AC_ARG_ENABLE([local], AS_HELP_STRING([--enable-local],
	[Build local DC-API]),, [enable_local=yes])

AM_CONDITIONAL([WITH_BOINC], [test "$no_boinc" != yes])
AM_CONDITIONAL([WITH_CLGR], [test "$with_clgr" != no])
AM_CONDITIONAL([WITH_LOCAL], [test "$enable_local" == yes])

AC_CONFIG_FILES([Makefile \
	include/Makefile \
	common/Makefile \
	boinc/Makefile \
	local/Makefile \
	clgr/Makefile])
AC_OUTPUT
