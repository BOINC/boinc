
dnl $Id$

AC_DEFUN([AC_CHECK_MYSQL],[
AC_ARG_VAR([MYSQL_CONFIG], [mysql_config program])

if test -z "$MYSQL_CONFIG"; then
  AC_PATH_PROG(MYSQL_CONFIG,mysql_config,,[$PATH:/usr/local/mysql/bin])
fi

# THIS SMALL CHANGE TO THE STANDARD .M4 FILE SIMPLY SETS A VARIABLE IF
# MYSQL IS NOT TEHRE.
if test -z "$MYSQL_CONFIG"; then
    no_mysql=yes
else
    AC_MSG_CHECKING(mysql libraries)
    MYSQL_LIBS=`${MYSQL_CONFIG} --libs`

    # This is so silly, but Apple actually has a bug in their
    # mysql_config script (same bug in their curl-config, too).
    # Fixed in 10.4, but we need to correct this for 10.3...
    case "${host}" in
    powerpc-apple-darwin7*)
        MYSQL_LIBS=`echo $MYSQL_LIBS | sed -e 's|-arch i386||g'`
        ;;
    esac
    AC_MSG_RESULT($MYSQL_LIBS)
    AC_MSG_CHECKING(mysql includes)
    MYSQL_CFLAGS=`${MYSQL_CONFIG} --cflags`
    # Same bug...
    case "${host}" in
    powerpc-apple-darwin7*)
        MYSQL_CFLAGS=`echo $MYSQL_CFLAGS | sed -e 's|-arch i386||g'`
        ;;
    esac
    AC_MSG_RESULT($MYSQL_CFLAGS)
    no_mysql=no
fi
AC_SUBST(MYSQL_LIBS)
AC_SUBST(MYSQL_CFLAGS)

])
