
AC_DEFUN([BOINC_GETSOCKOPT_TYPE],[
AC_LANG_PUSH(C)
AC_CHECK_HEADERS([windows.h])
if test "${ac_cv_header_windows_h}" = "yes" ; then
  ac_includes_default="${ac_includes_default}
#include <windows.h>
"
fi
AC_CHECK_HEADERS([sys/socket.h])
if test "${ac_cv_header_sys_socket_h}" = "yes" ; then
  ac_includes_default="${ac_includes_default}
#include <sys/socket.h>
"
fi
AC_CHECK_TYPES([socklen_t])
B_SV_CFLAGS="${CFLAGS}"
B_SV_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS=""
if test "${ac_cv_c_compiler_gnu}" = "yes" ; then
  CFLAGS="${CFLAGS} -Werror -pedantic"
fi
AC_CACHE_CHECK([type of getsockopt() parameter five],
[boinc_cv_getsockopt_type],
[
  if test "${ac_cv_type_socklen_t}" = "yes" ; then
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([[
#define CONFIG_TEST
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
      ]],
      [
        socklen_t l;
        return getsockopt(0,0,0,(void *)0,&l);
      ])],
      [
        boinc_cv_getsockopt_type="socklen_t"
      ]
    )
  fi
  if test -z "${boinc_cv_getsockopt_type}"; then
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([[
#define CONFIG_TEST
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
      ]],
      [
        size_t l;
        return getsockopt(0,0,0,(void *)0,&l);
      ])],
      [
        boinc_cv_getsockopt_type="size_t"
      ],
      [
        boinc_cv_getsockopt_type="int"
      ]
    )
  fi
])
CFLAGS="${B_SV_CFLAGS}"
CPPFLAGS="${B_SV_CPPFLAGS}"
AC_DEFINE_UNQUOTED([BOINC_SOCKLEN_T],[${boinc_cv_getsockopt_type}],[Define to the type pointed to by the 5th parameter of getsockopt])
AC_LANG_POP(C)
AH_BOTTOM([
#ifndef HAVE_SOCKLEN_T
typedef BOINC_SOCKLEN_T socklen_t;
#endif
])
])

