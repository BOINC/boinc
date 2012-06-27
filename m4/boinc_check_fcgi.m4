AC_DEFUN([BOINC_CHECK_FCGI],[
if ( test "x${enable_server}" = "xyes" ) || ( test "x${enable_libraries}" = "xyes" ) ; then
  if test "x${enable_fcgi}" = "xyes" ; then
    AC_MSG_CHECKING([if CFLAG '-include fcgi_stdio.h' works])
    AC_LANG_PUSH(C)
    save_cflags="${CFLAGS}"
    CFLAGS="-I${prefix} -include fcgi_stdio.h -D_USING_FCGI_ ${CFLAGS}"
    AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM(
        [[
#define CONFIG_TEST
        ]],
        [
fprintf(stderr,"Hello\n");
return 0;
        ]
      )],
      [
        AC_MSG_RESULT(yes)
      ],
      [
        AC_MSG_RESULT(no)
        AC_MSG_WARN([ fcgi-stdio.h not found.
------------------------------------------------------------
Disabling FCGI.  Will not build components that require FCGI
------------------------------------------------------------
        ])
        enable_fcgi="no"
      ]
    )
    CFLAGS="${save_cflags}"
    AC_LANG_POP(C)
  fi
  if test "x${enable_fcgi}" = "xyes" ; then
    AC_CHECK_LIB([fcgi],[FCGI_printf],[enable_fcgi="yes"],[
      enable_fcgi="no"
      AC_MSG_WARN([libfcgi not found.
------------------------------------------------------------
Disabling FCGI.  Will not build components that require FCGI
------------------------------------------------------------
      ])
    ])
  fi
fi
])

