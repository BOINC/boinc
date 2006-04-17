AC_INIT([Distributed Computing API], [0.9], [desktopgrid@lpds.sztaki.hu], [dcapi])
AC_CONFIG_MACRO_DIR([cf])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_SRCDIR([include/dc.h])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_DISABLE_SHARED
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

AM_PATH_GLIB_2_0([2.6.0],,[AC_MSG_ERROR([glib2 is missing])])

GTK_DOC_CHECK([1.3])

BACKENDS=

dnl ===================================================================
dnl BOINC backend
dnl

AC_ARG_ENABLE([backend-boinc],
	AS_HELP_STRING([--enable-backend-boinc],
		[enable the BOINC backend @<:@default=yes@:>@]),,
	[enable_backend_boinc=yes])

if test "$enable_backend_boinc" = yes; then
	if test "$enable_client" = yes; then
		SZDG_BOINC_CLIENT
		AC_CHECK_HEADERS([uuid/uuid.h],, [AC_MSG_ERROR([libuuid headers are missing])])
		AC_CHECK_LIB([uuid], [uuid_generate], [true], [AC_MSG_ERROR([libuuid is missing])])
	fi
	if test "$enable_master" = yes; then
		SZDG_BOINC_SERVER
	fi
	if test "$no_boinc" = yes; then
		AC_MSG_ERROR([BOINC development environment was not found])
	fi
	BACKENDS="$BACKENDS boinc"
fi

AM_CONDITIONAL([WITH_BOINC], [test "$enable_backend_boinc" = yes])

dnl ===================================================================
dnl ClusterGrid backend
dnl

AC_ARG_ENABLE([backend-clgr],
	AS_HELP_STRING([--enable-backend-clgr],
		[enable the ClusterGrid backend @<:@default=no@:>@]),,
	[enable_backend_clgr=no])

if test "$enable_backend_clgr" = yes; then
	BACKENDS="$BACKENDS clgr"
fi

AM_CONDITIONAL([WITH_CLGR], [test "$enable_backend_clgr" = yes])

dnl ===================================================================
dnl Condor backend
dnl

AC_ARG_ENABLE([backend-condor],
	AS_HELP_STRING([--enable-backend-condor],
		[enable the Condor backend @<:@default=no@:>@]),,
	[enable_backend_condor=no])

if test "$enable_backend_condor" = yes; then
	BACKENDS="$BACKENDS condor"
fi

AM_CONDITIONAL([WITH_CONDOR], [test "$enable_backend_condor" = yes])

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
echo ""

AC_CONFIG_FILES([Makefile \
	include/Makefile \
	common/Makefile \
	boinc/Makefile \
	boinc/dcapi-boinc-client.pc \
	boinc/dcapi-boinc-master.pc \
	local/Makefile \
	local/dcapi-local-client.pc \
	local/dcapi-local-master.pc \
	clgr/Makefile \
	condor/Makefile \
	condor/dcapi-condor-client.pc \
	condor/dcapi-condor-master.pc \
	doc/Makefile \
	doc/client/Makefile \
	doc/common/Makefile \
	doc/master/Makefile])
AC_OUTPUT
