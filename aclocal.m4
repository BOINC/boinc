AC_DEFUN([AC_PROG_APACHE],[
AC_REQUIRE([AC_EXEEXT])dnl
AC_PATH_PROG(APACHE, apache$EXEEXT, nocommand)
if test "$APACHE" = nocommand; then
	AC_MSG_WARN([****************************************])
	AC_MSG_WARN([apache not found in $PATH])
	AC_MSG_WARN([The BOINC server may not function properly without apache installed.])
	AC_MSG_WARN([****************************************])
fi
])

AC_DEFUN([AC_PROG_MYSQL],[
AC_REQUIRE([AC_EXEEXT])dnl
AC_PATH_PROG(MYSQL, mysql$EXEEXT, nocommand)
if test "$MYSQL" = nocommand; then
	AC_MSG_WARN([****************************************])
	AC_MSG_WARN([mysql not found in $PATH])
	AC_MSG_WARN([The BOINC server may not function properly without mysql installed.])
	AC_MSG_WARN([****************************************])
fi
])

AC_DEFUN([AC_PROG_PHP],[
AC_REQUIRE([AC_EXEEXT])dnl
AC_PATH_PROG(PHP, php$EXEEXT, nocommand)
if test "$PHP" = nocommand; then
	AC_MSG_WARN([****************************************])
	AC_MSG_WARN([php not found in $PATH])
	AC_MSG_WARN([The test scripts and web utilities may not function properly without php installed.])
	AC_MSG_WARN([****************************************])
fi
])
