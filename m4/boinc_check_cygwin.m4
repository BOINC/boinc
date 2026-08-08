AC_DEFUN([BOINC_CHECK_CYGWIN],[
  AC_MSG_CHECKING([whether we are compiling for cygwin])
  case "$target" in
    *cygwin*)
        with_cygwin=yes
	;;
    *)  with_cygwin=no
        ;;
  esac
  AC_MSG_RESULT($with_cygwin)
  if test "${with_cygwin}" = "yes" ; then
    save_cflags="${CFLAGS}"
    CFLAGS="-I/usr/include/w32api -D_WIN32 -DWIN32 ${CFLAGS}"
  fi
  AX_WINSOCK
  if test "${with_cygwin}" = "yes" ; then
    CFLAGS="${save_cflags}"
    AC_MSG_CHECKING([whether to use X11 or WIN32 GUI])
    if test "${with_x+set}" = set; then
       with_x="${with_x}"
    else
       with_x="no"
    fi
    if test "${with_x}" = "no" ; then
      AC_MSG_RESULT(WIN32)
      ac_cv_header_windows_h=yes
      AC_DEFINE(XMD_H,[1],[Define to prevent redefinition of INT32 in jconfig.h])
      AC_DEFINE(CYGWIN_USE_WIN32,[1],[Define to 1 if CYGWIN should use the WIN32 GUI])
      LIBS="-L/usr/lib/w32api ${LIBS}"
      CFLAGS="-I/usr/include/w32api -D_WIN32 -DWIN32 ${CFLAGS}"
      CXXFLAGS="-I/usr/include/w32api  -D_WIN32 -DWIN32 ${CXXFLAGS}"
      CPPFLAGS="-I/usr/include/w32api  -D_WIN32 -DWIN32 ${CPPFLAGS}"
    else
      AC_MSG_RESULT(X11)
    fi
  fi
])
