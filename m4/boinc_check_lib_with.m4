AC_DEFUN([BOINC_CHECK_LIB_WITH],[
dnl arguments  $1 library name (-l$1)
dnl            $2 function name
dnl            $3 variable to append LDFLAGS to
  AC_LANG_PUSH(C)
  _sv_libs="$LIBS"
  AC_ARG_WITH([lib$1],
    AC_HELP_STRING([--with-lib$1=DIR],[look for the $1 library in DIR]),
   [_lib_with=$withval],[_lib_with=no])
  _NEW_LDFLAGS=
  if test "x${_lib_with}" != "xno" ; then
    if test -d $_lib_with/. ; then
      _NEW_LDFLAGS="${_NEW_LDFLAGS} -L${_lib_with}"
      LIBS="${LIBS} ${_NEW_LDFLAGS}"
    fi
    AC_CHECK_LIB([$1],[$2],[_lib_found=yes])
    if test "x${_lib_found}" = "xyes" ; then
      if test -f ${_lib_with} ; then
        _NEW_LDFLAGS="${_NEW_LDFLAGS} ${_lib_with}"
      elif test -h ${_lib_with} -a ! -d ${_lib_with}/. ; then
        _NEW_LDFLAGS="${_NEW_LDFLAGS} ${_lib_with}"
      else
        _NEW_LDFLAGS="${_NEW_LDFLAGS} -l$1"
      fi
      $3="${$3} ${_NEW_LDFLAGS}"
    fi
  fi
  LIBS="${_sv_libs}"
  AC_LANG_POP(C)
])

