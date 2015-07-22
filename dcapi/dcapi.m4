dnl
dnl SZDG_DCAPI
dnl
dnl Check for the Distributed Computing Platform development files
dnl

AC_DEFUN([_SZDG_DCAPI_COMMON], [
	AC_ARG_WITH([dcapi_flavor], AS_HELP_STRING([--with-dcapi-flavor],
		[DCAPI flavor to use @<:@boinc@:>@]),,
		[with_dcapi_flavor=boinc])
	PKG_PROG_PKG_CONFIG([0.17])
])

dnl
dnl Check for DC-API client libraries.
dnl
dnl Usage: SZDG_DCAPI_CLIENT([ACTION-IF-FOIND], [ACTION-IF-NOT-FOUND])
dnl
AC_DEFUN([SZDG_DCAPI_CLIENT], [
	AC_REQUIRE([_SZDG_DCAPI_COMMON])

	DCAPI_CLIENT_LIBS=
	DCAPI_CLIENT_CFLAGS=

	AC_MSG_CHECKING([for DCAPI_CLIENT])
	PKG_CHECK_EXISTS([dcapi-${with_dcapi_flavor}-client],
		[dcapi_client=yes],
		[dcapi_client=no])
	AC_MSG_RESULT([$dcapi_client])
	if test "$dcapi_client" != no; then
		DCAPI_CLIENT_LIBS=`pkg-config --libs --static dcapi-${with_dcapi_flavor}-client`
		DCAPI_CLIENT_CPPFLAGS=`pkg-config --cflags dcapi-${with_dcapi_flavor}-client`

		DCAPI_CLIENT_LIBS="-Wl,-Bstatic $DCAPI_CLIENT_LIBS -Wl,-Bdynamic"

		save_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $DCAPI_CLIENT_CFLAGS"
		save_LIBS="$LIBS"
		LIBS="$LIBS $DCAPI_CLIENT_LIBS"
		AC_LANG_PUSH([C])
		AC_CHECK_HEADERS([dc_client.h], [], [dcapi_client=no])
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <dc_client.h>]],
			[[(void)DC_initClient()]])],
			[true],
			[dcapi_client=no])
		AC_LANG_POP([C])
		CPPFLAGS="$save_CPPFLAGS"
		LIBS="$save_LIBS"
	fi

	AC_SUBST([DCAPI_CLIENT_LIBS])
	AC_SUBST([DCAPI_CLIENT_CFLAGS])

	dnl Backward compatibility: produce an error if the above tests fail
	dnl and both $1 and $2 are empty
	m4_ifval(m4_normalize[$1], [], [m4_ifvaln(m4_normalize[$2], [],
	if test "$dcapi_client" != yes; then
		AC_MSG_ERROR([DC-API client libraries were not found])
	fi)])dnl

	if test "$dcapi_client" = yes; then
		m4_ifval(m4_normalize[$1], [$1], [:])
	m4_ifvaln([$2],[else
		$2])dnl
	fi
])

dnl
dnl Check for DC-API master libraries.
dnl
dnl Usage: SZDG_DCAPI_MASTER([ACTION-IF-FOIND], [ACTION-IF-NOT-FOUND])
dnl
AC_DEFUN([SZDG_DCAPI_MASTER], [
	AC_REQUIRE([_SZDG_DCAPI_COMMON])

	PKG_CHECK_MODULES([DCAPI_MASTER],
		[dcapi-${with_dcapi_flavor}-master],
		[dcapi_master=yes],
		[dcapi_master=no])

	if test "$dcapi_master" != no; then
		save_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $DCAPI_MASTER_CFLAGS"
		save_LIBS="$LIBS"
		LIBS="$LIBS $DCAPI_MASTER_LIBS"
		AC_LANG_PUSH([C])
		AC_CHECK_HEADERS([dc.h], [], [dcapi_master=no])
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <dc.h>]],
			[[(void)DC_initMaster((char *)0)]])],
			[true],
			[dcapi_master=no])
		AC_LANG_POP([C])
		CPPFLAGS="$save_CPPFLAGS"
		LIBS="$save_LIBS"
	fi

	dnl Backward compatibility: produce an error if the above tests fail
	dnl and both $1 and $2 are empty
	m4_ifval(m4_normalize[$1], [], [m4_ifvaln(m4_normalize[$2], [],
	if test "$dcapi_master" != yes; then
		AC_MSG_ERROR([DC-API master libraries were not found])
	fi)])dnl

	if test "$dcapi_master" = yes; then
		m4_ifval(m4_normalize[$1], [$1], [:])
	m4_ifvaln([$2],[else
		$2])dnl
	fi
])
