
dnl $Id$

AC_DEFUN([AC_CHECK_MYSQL],[
AC_ARG_VAR([MYSQL_CONFIG], [mysql_config program])

if test -z "$MYSQL_CONFIG"; then
  AC_PATH_PROG(MYSQL_CONFIG,mysql_config,,[$PATH:/usr/local/mysql/bin])
fi

if test -z "$MYSQL_CONFIG"; then
    AC_MSG_ERROR([mysql_config executable not found.  To build only the client, specify --disable-server])
else
    AC_MSG_CHECKING(mysql libraries)
    MYSQL_LIBS=`${MYSQL_CONFIG} --libs`
    AC_MSG_RESULT($MYSQL_LIBS)
    AC_MSG_CHECKING(mysql includes)
    MYSQL_CFLAGS=`${MYSQL_CONFIG} --cflags`
    AC_MSG_RESULT($MYSQL_CFLAGS)
fi
AC_SUBST(MYSQL_LIBS)
AC_SUBST(MYSQL_CFLAGS)

])
