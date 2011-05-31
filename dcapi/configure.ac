AC_INIT([Distributed Computing API], [0.11], [desktopgrid@lpds.sztaki.hu], [dcapi])
AC_CONFIG_MACRO_DIR([cf])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_SRCDIR([include/dc.h])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

LIBTOOL="$LIBTOOL --silent"

dnl Enable useful warnings
if test "$ac_cv_c_compiler_gnu" = yes; then
	CFLAGS="-Wall -Wmissing-prototypes -Wstrict-prototypes -Wsign-compare ${CFLAGS}"
	CXXFLAGS="-Wall -Wsign-compare ${CXXFLAGS}"
fi

AC_ARG_ENABLE([client],
	AS_HELP_STRING([--disable-client], [do not build the client application]),,
	[enable_client=yes])
AC_ARG_ENABLE([master],
	AS_HELP_STRING([--disable-master], [do not build the master application]),,
	[enable_master=yes])

AM_CONDITIONAL([BUILD_CLIENT], [test "$enable_client" = yes])
AM_CONDITIONAL([BUILD_MASTER], [test "$enable_master" = yes])

dnl glib is used only on the master side
if test "$enable_master" = yes; then
	AM_PATH_GLIB_2_0([2.6.0],,[AC_MSG_ERROR([glib2 is missing])])
fi

GTK_DOC_CHECK([1.3])
MON_PKG_JAVA

BACKENDS=

dnl ===================================================================
dnl BOINC backend
dnl

AC_ARG_ENABLE([backend-boinc],
	AS_HELP_STRING([--enable-backend-boinc],
		[enable the BOINC backend @<:@default=yes@:>@]),,
	[enable_backend_boinc=auto])

if test "$enable_backend_boinc" != no; then
	SZDG_BOINC_COMMON
	if test "$enable_master" = yes; then
		SZDG_BOINC_SERVER
	fi
	if test "$enable_client" = yes; then
		SZDG_BOINC_CLIENT
		AC_CHECK_HEADERS([uuid/uuid.h],, [no_boinc=yes])
		AC_CHECK_LIB([uuid], [uuid_unparse_lower], [true], [no_boinc=yes])
	fi
	if test "$no_boinc" = yes; then
		if test "$enable_backend_boinc" = yes; then
			AC_MSG_ERROR([BOINC development environment was not found])
		fi
	else
		BACKENDS="$BACKENDS boinc"
	fi
fi

AM_CONDITIONAL([WITH_BOINC], [echo $BACKENDS | grep -q boinc])

dnl ===================================================================
dnl Condor backend
dnl

AC_ARG_ENABLE([backend-condor],
	AS_HELP_STRING([--enable-backend-condor],
		[enable the Condor backend @<:@default=no@:>@]),,
	[enable_backend_condor=auto])

AC_ARG_WITH([condor-headers],
	AS_HELP_STRING([--with-condor-headers=DIR],
		[Use condor headers in DIR]), [CONDOR_CPPFLAGS="-I$withval"])
AC_ARG_WITH([condor-libs],
	AS_HELP_STRING([--with-condor-libs=DIR],
		[Use condor libraries in DIR]), [CONDOR_LDFLAGS="-L$withval"])
AC_SUBST([CONDOR_CPPFLAGS])
AC_SUBST([CONDOR_LDFLAGS])

no_condor=
if test "$enable_backend_condor" != no; then
	AC_LANG_PUSH([C++])
	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $CONDOR_CPPFLAGS"
	save_LDFLAGS="$LDFLAGS"
	LDFLAGS="$LDFLAGS $CONDOR_LDFLAGS"
	AC_CHECK_HEADERS([read_user_log.h],, [no_condor=yes],
		[AC_INCLUDES_DEFAULT
#include <time.h>
])
	LDFLAGS="$save_LDFLAGS"
	CPPFLAGS="$save_CPPFLAGS"
	AC_LANG_POP([C++])

	if test "$no_condor" = yes; then
		if test "$enable_backend_condor" = yes; then
			AC_MSG_ERROR([Condor development environment is missing])
		fi
	else
		BACKENDS="$BACKENDS condor"
	fi
fi

AM_CONDITIONAL([WITH_CONDOR], [echo $BACKENDS | grep -q condor])

dnl ===================================================================
dnl Local backend
dnl

AC_ARG_ENABLE([backend-local],
	AS_HELP_STRING([--enable-backend-local],
		[enable the local backend @<:@default=yes@:>@]),,
	[enable_backend_local=yes])

if test "$enable_backend_local" = yes; then
	BACKENDS="$BACKENDS local"
fi

AM_CONDITIONAL([WITH_LOCAL], [test "$enable_backend_local" == yes])

dnl ===================================================================
dnl Generate output files
dnl

echo ""
echo "Enabled backends: $BACKENDS"
echo "Build master: $enable_master"
echo "Build client: $enable_client"
echo ""

AC_CONFIG_FILES([Makefile \
	include/Makefile \
	common/Makefile \
	boinc/Makefile \
	boinc/dcapi-boinc-client.pc \
	boinc/dcapi-boinc-master.pc \
	condor/Makefile \
	condor/dcapi-condor-client.pc \
	condor/dcapi-condor-master.pc \
	local/Makefile \
	local/dcapi-local-client.pc \
	local/dcapi-local-master.pc \
	java/Makefile \
	java/hu/sztaki/lpds/dc/client/Version.java \
	doc/Makefile])
AC_OUTPUT
