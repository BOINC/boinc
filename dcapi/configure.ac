AC_INIT([Distributed Computing API], [0.1], [podhorszki@sztaki.hu], [dcapi])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_SRCDIR([include/dc.h])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

LIBTOOL="$LIBTOOL --silent"

AC_ARG_WITH([boinc], AS_HELP_STRING([--with-boinc@<:@=DIR@:>@],
	[Use BOINC (installed in DIR)]),, [with_boinc=auto])

no_boinc=
if test "$with_boinc" = no; then
	no_boinc=yes
fi
if test "$no_boinc" != yes; then
	case "$with_boinc" in
		yes|auto)
			BOINC_INCLUDES="/usr/include/BOINC"
			;;
		*)
			BOINC_INCLUDES="$with_boinc"
			;;
	esac

	AC_PATH_PROG([MYSQL_CONFIG], [mysql_config])
	if test "$MYSQL_CONFIG" = ""; then
		no_boinc=yes
	fi
fi
if test "$no_boinc" != yes; then
	BOINC_CPPFLAGS="-I$BOINC_INCLUDES -I$BOINC_INCLUDES/db -I$BOINC_INCLUDES/lib -I$BOINC_INCLUDES/sched -I$BOINC_INCLUDES/tools"

	if test -d "$BOINC_INCLUDES/RSAEuro/source"; then
		BOINC_CPPFLAGS="$BOINC_CPPFLAGS -I$BOINC_INCLUDES/RSAEuro/source"
	else
		if test -d "$BOINC_INCLUDES/RSAEuro"; then
			BOINC_CPPFLAGS="$BOINC_CPPFLAGS -I$BOINC_INCLUDES/RSAEuro"
		fi
	fi

	BOINC_LIBS="-lboinc -lboinc_api -lsched -lboinc_zip"
	AC_CHECK_LIB([rsaeuro], [RSAPrivateEncrypt],
		[BOINC_LIBS="$BOINC_LIBS -lrsaeuro"],
		[BOINC_LIBS="$BOINC_LIBS -lcrypto"])

	MYSQL_CPPFLAGS=`$MYSQL_CONFIG --cflags`
	MYSQL_LIBS=`$MYSQL_CONFIG --libs | sed -e 's,-L/usr/lib , ,'`

	AC_SUBST([BOINC_CPPFLAGS])
	AC_SUBST([BOINC_LIBS])
	AC_SUBST([MYSQL_CPPFLAGS])
	AC_SUBST([MYSQL_LIBS])
fi

AC_LANG([C++])

AH_TEMPLATE([BOINC_VERSION], [BOINC major version])
if test "$no_boinc" != yes; then
	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $BOINC_CPPFLAGS"
	AC_CACHE_CHECK([for BOINC version], [dc_cv_boinc_version], [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sched_msgs.h>],
		[SCHED_MSG_LOG::Kind level = SCHED_MSG_LOG::CRITICAL;])],
			[dc_cv_boinc_version=4],
			[dc_cv_boinc_version=5])
		])
	CPPFLAGS="$save_CPPFLAGS"
	AC_DEFINE([BOINC_VERSION], [$dc_cv_boinc_version])
fi

AC_ARG_WITH([clgr], AS_HELP_STRING([--with-clgr@<:@=DIR@:>@],
	[Use ClusterGrid (installed in DIR)]),, [with_clgr=yes])

AC_ARG_ENABLE([local], AS_HELP_STRING([--enable-local],
	[Build local DC-API]),, [enable_local=yes])

AM_CONDITIONAL([WITH_BOINC], [test "$with_boinc" != no])
AM_CONDITIONAL([WITH_CLGR], [test "$with_clgr" != no])
AM_CONDITIONAL([WITH_LOCAL], [test "$enable_local" == yes])

AC_CONFIG_FILES([Makefile \
	include/Makefile \
	common/Makefile \
	boinc/Makefile \
	local/Makefile \
	clgr/Makefile])
AC_OUTPUT
