dnl
dnl Check for BOINC components
dnl

dnl
dnl SZDG_BOINC_COMMON
dnl
dnl Check for components common for the client and server API
dnl
AC_DEFUN([SZDG_BOINC_COMMON], [
	AC_ARG_WITH([boinc], AS_HELP_STRING([--with-boinc@<:@=DIR@:>@],
		[Use BOINC (compiled or installed in DIR)]),, [with_boinc=auto])

	no_boinc=
	if test "$with_boinc" = no; then
		no_boinc=yes
	fi

	if test "$no_boinc" != yes; then
		case "$with_boinc" in
			yes|auto)
				BOINC_INCLUDES="/usr/include/BOINC"
				BOINC_CPPFLAGS="-I/usr/include/BOINC"
				BOINC_LDFLAGS=
				;;
			*)
				# Check if this is an installed or just a compiled-in-place
				# version
				if test -d "$with_boinc/sched"; then
					BOINC_CPPFLAGS="-I$with_boinc/api -I$with_boinc/lib -I$with_boinc/sched -I$with_boinc/tools -I$with_boinc/db"
					BOINC_LDFLAGS="-L$with_boinc/api -L$with_boinc/lib -L$with_boinc/sched"
				else
					BOINC_CPPFLAGS="-I$with_boinc/include/BOINC"
					BOINC_LDFLAGS="-L$with_boinc/lib"
				fi
				;;
		esac
	fi

	if test "$no_boinc" != yes; then
		BOINC_COMMON_LIBS="-lboinc -lcrypto"
	fi

	AC_SUBST([BOINC_CPPFLAGS])
	AC_SUBST([BOINC_LDFLAGS])
])

dnl
dnl SZDG_BOINC_SERVER
dnl
dnl Check for BOINC server-side API
dnl
AC_DEFUN([SZDG_BOINC_SERVER], [
	AC_REQUIRE([SZDG_BOINC_COMMON])

	dnl
	dnl First check for MySQL
	dnl
	AC_PATH_PROG([MYSQL_CONFIG], [mysql_config])
	if test "$MYSQL_CONFIG" = ""; then
		no_boinc=yes
	else
		MYSQL_CPPFLAGS="`$MYSQL_CONFIG --include`"
		MYSQL_LIBS=`$MYSQL_CONFIG --libs | sed -e 's,-L/usr/lib , ,'`
	fi
	AC_SUBST([MYSQL_CPPFLAGS])
	AC_SUBST([MYSQL_LIBS])

	dnl
	dnl Check for BOINC 4.x/5.x
	dnl
	AC_LANG_PUSH([C++])
	AH_TEMPLATE([BOINC_VERSION], [BOINC major version])
	if test "$no_boinc" != yes; then
		save_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOINC_CPPFLAGS"
		AC_CACHE_CHECK([for BOINC version], [boinc_cv_boinc_version], [
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sched_msgs.h>],
			[SCHED_MSG_LOG::Kind level = SCHED_MSG_LOG::CRITICAL;])],
				[boinc_cv_boinc_version=4],
				[boinc_cv_boinc_version=5])
			])
		CPPFLAGS="$save_CPPFLAGS"
		AC_DEFINE_UNQUOTED([BOINC_VERSION], [$boinc_cv_boinc_version])
	fi
	AC_LANG_POP([C++])

	if test "$no_boinc" != yes; then
		BOINC_SERVER_CPPFLAGS="$BOINC_CPPFLAGS $MYSQL_CPPFLAGS"
		BOINC_SERVER_LIBS="-lsched $BOINC_COMMON_LIBS -lstdc++ $MYSQL_LIBS"
	fi

	AC_SUBST([BOINC_SERVER_CPPFLAGS])
	AC_SUBST([BOINC_SERVER_LIBS])

	if test "$with_boinc" = yes && test "$no_boinc" = yes; then
		AC_MSG_ERROR([BOINC development environment was not found])
	fi
])

dnl
dnl SZDG_BOINC_CLIENT
dnl
dnl Check for BOINC client-side API
dnl
AC_DEFUN([SZDG_BOINC_CLIENT], [
	AC_REQUIRE([SZDG_BOINC_COMMON])

	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $BOINC_CPPFLAGS"
	save_LDFLAGS="$LDFLAGS $BOINC_LDFLAGS"
	LDFLAGS="$LDFLAGS $BOINC_LDFLAGS"
	AC_CHECK_HEADERS([boinc_api.h filesys.h diagnostics.h],, [no_boinc=yes])
	AC_LANG_PUSH([C++])
	AC_CHECK_LIB([boinc_api], [boinc_init], [true], [no_boinc=yes],
		[-lboinc -lpthread -lm])
	AC_LANG_POP([C++])
	LDFLAGS="$save_LDFLAGS"
	CPPFLAGS="$save_CPPFLAGS"

	if test "$with_boinc" = yes && test "$no_boinc" = yes; then
		AC_MSG_ERROR([BOINC development environment was not found])
	fi

	BOINC_CLIENT_LIBS="-lboinc_api $BOINC_COMMON_LIBS -lstdc++ -lpthread -lm"
	AC_SUBST([BOINC_CLIENT_LIBS])
])
