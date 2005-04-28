
AC_DEFUN([BOINC_GETSOCKOPT_TYPE],[
dnl TODO: use compiler to test these; there probably exists an autoconf macro already!
AC_MSG_CHECKING(type of getsockopt() parameter four)
case "$target" in
*-linux* | *solaris* | *openbsd* )
        AC_DEFINE(GETSOCKOPT_SOCKLEN_T, 1, [getsockopt uses socklen_t])
	AC_MSG_RESULT(socklen_t)
        ;;
*sysv5OpenUNIX8* | *hpux* | i*86*cygwin* )
        AC_DEFINE(GETSOCKOPT_SIZE_T, 1, [getsockopt uses size_t])
	AC_MSG_RESULT(size_t)
        ;;
esac
])

