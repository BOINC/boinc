
AC_DEFUN([BOINC_CHECK_CFLAG],[
  AC_LANG_PUSH(C)
  sv_flags="${CFLAGS}"
  AC_MSG_CHECKING(if C compiler supports $1)
  CFLAGS="${CFLAGS} $1"
  AC_LINK_IFELSE([
    AC_LANG_PROGRAM([[
      #define CONFIG_TEST
      int foo() {return 1;}
    ]],
    [ return foo(); ])],
    [
      AC_MSG_RESULT(yes)
      $2
    ],
    [
      AC_MSG_RESULT(no)
      CFLAGS="${sv_flags}"
      $3
    ]
  )
  AC_LANG_POP(C)
])

AC_DEFUN([BOINC_CHECK_CXXFLAG],[
  AC_LANG_PUSH(C++)
  sv_flags="${CXXFLAGS}"
  AC_MSG_CHECKING(if C++ compiler supports $1)
  CXXFLAGS="${CXXFLAGS} $1"
  AC_LINK_IFELSE([
    AC_LANG_PROGRAM([[
      #define CONFIG_TEST
      int foo() {return 1;}
    ]],
    [ return foo(); ])],
    [
      AC_MSG_RESULT(yes)
      $2
    ],
    [
      AC_MSG_RESULT(no)
      CXXFLAGS="${sv_flags}"
      $3
    ]
  )
  AC_LANG_POP(C++)
])

AC_DEFUN([BOINC_CHECK_LDFLAG],[
  AC_LANG_PUSH(C)
  sv_flags="${LDFLAGS}"
  AC_MSG_CHECKING(if $LD supports $1)
  LDFLAGS="${LDFLAGS} $1"
  AC_LINK_IFELSE([
    AC_LANG_PROGRAM([[
      #define CONFIG_TEST
      int foo() {return 1;}
    ]],
    [ return foo(); ])],
    [
      AC_MSG_RESULT(yes)
      $2
    ],
    [
      AC_MSG_RESULT(no)
      LDFLAGS="${sv_flags}"
      $3
    ]
  )
  AC_LANG_POP(C)
])
