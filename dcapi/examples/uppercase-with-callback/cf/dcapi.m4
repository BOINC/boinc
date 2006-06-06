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

AC_DEFUN([SZDG_DCAPI_CLIENT], [
	AC_REQUIRE([_SZDG_DCAPI_COMMON])

	PKG_CHECK_MODULES([DCAPI_CLIENT], [dcapi-${with_dcapi_flavor}-client],,
		[AC_MSG_ERROR([DC-API client implementation not found])])
	
	dnl Minimal check to see if the headers are usable
	AC_CHECK_HEADERS([dc_client.h])
])

AC_DEFUN([SZDG_DCAPI_MASTER], [
	AC_REQUIRE([_SZDG_DCAPI_COMMON])

	PKG_CHECK_MODULES([DCAPI_MASTER], [dcapi-${with_dcapi_flavor}-master],,
		[AC_MSG_ERROR([DC-API master implementation not found])])

	dnl Minimal check to see if the headers are usable
	AC_CHECK_HEADERS([dc.h])
])
