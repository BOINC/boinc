AC_DEFUN([AX_WINSOCK],[
  AC_ARG_WITH([winsock],
  	AC_HELP_STRING([--with-winsock],
	 	[use windows sockets even if sys/socket.h exists]))
  if test "${with_winsock}" = "yes" ; then
    AC_DEFINE(USE_WINSOCK,[1],[Define to 1 to use windows sockets])
    ac_cv_header_sys_socket_h="no"
  else
    ac_cv_header_winsock2_h="no"
    ac_cv_header_winsock_h="no"
  fi
  AC_CHECK_HEADERS([winsock2.h winsock.h windows.h sys/socket.h])
])
