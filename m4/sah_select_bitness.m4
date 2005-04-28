AC_DEFUN([SAH_DEFAULT_BITNESS],[
  if test -z "${COMPILER_MODEL_BITS}"
  then
    AC_MSG_CHECKING(default bitness of compiler)
    echo "int main() { return 0; }" >conftest.c
    ${CC} ${CFLAGS} -c conftest.c >&5
    COMPILER_MODEL_BITS=32
    if test -f conftest.${OBJEXT} ; then
      if test -n "`file conftest.${OBJEXT} | grep -i 64-bit`" 
      then
        COMPILER_MODEL_BITS=64
      else
        if test -n "`file conftest.${OBJEXT} | grep -i 16-bit`" 
	then
          COMPILER_MODEL_BITS=16
        fi
      fi
    fi
    /bin/rm conftest.c
    AC_MSG_RESULT($COMPILER_MODEL_BITS)
  fi
])
  

AC_DEFUN([SAH_SELECT_BITNESS],[
  AC_LANG_PUSH(C)
  SAH_DEFAULT_BITNESS
  AC_MSG_CHECKING(Selecting $1 bit model)
  echo "int main() { return 0; }" >conftest.$ac_ext 
  if test "$1" != "${COMPILER_MODEL_BITS}"
  then
    ${CC} ${CFLAGS} ${CPPFLAGS} -m$1 -c conftest.$ac_ext >&5
    if test -f conftest.${OBJEXT} ; then
      if test -n "`file conftest.${OBJEXT} | grep -i $1-bit`"
      then
        CFLAGS="${CFLAGS} -m$1"
	AC_MSG_RESULT(-m$1)
	COMPILER_MODEL_BITS=$1
      fi
      AC_MSG_RESULT(failed)
    fi
  else
    AC_MSG_RESULT(ok)
  fi
  AC_LANG_POP(C)
])

AC_DEFUN([SAH_OPTION_BITNESS],[
  AC_ARG_ENABLE(bitness,
    AC_HELP_STRING([--enable-bitness=(32,64)],
      [enable 32 or 64 bit object/executable files]
    ),
    [SAH_SELECT_BITNESS(${enableval})],
    [SAH_DEFAULT_BITNESS]
  )
])
