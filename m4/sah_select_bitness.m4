AC_DEFUN([SAH_DEFAULT_BITNESS],[
  if test -z "${COMPILER_MODEL_BITS}"; then
    AC_LANG_PUSH([C])
      AC_MSG_CHECKING([default bitness of C compiler])
      echo "int main() { return 0; }" >conftest.$ac_ext
      AC_REQUIRE_CPP
      ${CC} ${CFLAGS} ${CPPFLAGS} -fno-lto -c conftest.$ac_ext 2>&AS_MESSAGE_LOG_FD >&AS_MESSAGE_LOG_FD
      COMPILER_MODEL_BITS=32
      if test -f conftest.${OBJEXT}; then
        OBJDUMP_TEST="$(${OBJDUMP} --file-headers conftest.${OBJEXT} | grep 'file format')"
        FILE_TEST="$(file conftest.${OBJEXT})"
        if test -n "$(echo $OBJDUMP_TEST | grep 64)" -o -n "$(echo $FILE_TEST | grep -i 64-bit)"; then
          COMPILER_MODEL_BITS=64
        #else
          #if test -n "$(echo $OBJDUMP_TEST | grep 16)"; then
            #COMPILER_MODEL_BITS=16
          #fi
        fi
      fi
      /bin/rm -f conftest.$ac_ext conftest.${OBJEXT}
      AC_MSG_RESULT([$COMPILER_MODEL_BITS])
    AC_LANG_POP([C])

    AC_LANG_PUSH([C++])
      AC_MSG_CHECKING([default bitness of C++ compiler])
      echo "int main() { return 0; }" >conftest.$ac_ext
      AC_REQUIRE_CPP
      ${CXX} ${CXXFLAGS} ${CPPFLAGS} -fno-lto -c conftest.$ac_ext 2>&AS_MESSAGE_LOG_FD >&AS_MESSAGE_LOG_FD
      if test -f conftest.${OBJEXT}; then
        OBJDUMP_TEST="$(${OBJDUMP} --file-headers conftest.${OBJEXT} | grep 'file format')"
        FILE_TEST="$(file conftest.${OBJEXT})"
        if test -n "$(echo $OBJDUMP_TEST | grep 32)" -o -n "$(echo $FILE_TEST | grep -i 32-bit)"; then
          if test "${COMPILER_MODEL_BITS}" != "32"; then
            AC_MSG_ERROR([32 but not same as bitness in C])
          fi
        fi
        if test -n "$(echo $OBJDUMP_TEST | grep 64)" -o -n "$(echo $FILE_TEST | grep -i 64-bit)"; then
          if test "${COMPILER_MODEL_BITS}" != "64"; then
            AC_MSG_ERROR([64 but not same as bitness in C])
          fi
        fi
      fi
      /bin/rm -f conftest.$ac_ext conftest.${OBJEXT}
      AC_MSG_RESULT([$COMPILER_MODEL_BITS])
    AC_LANG_POP([C++])
  fi
])

AC_DEFUN([SAH_SELECT_BITNESS],[
  SAH_DEFAULT_BITNESS

  AC_LANG_PUSH([C])
    AC_MSG_CHECKING([if C compiler can use -m$1])
    if test "$1" != "${COMPILER_MODEL_BITS}"; then
      echo "int main() { return 0; }" >conftest.$ac_ext
      AC_REQUIRE_CPP
      ${CC} ${CFLAGS} ${CPPFLAGS} -m$1 -fno-lto -c conftest.$ac_ext 2>&AS_MESSAGE_LOG_FD >&AS_MESSAGE_LOG_FD
      if test -f conftest.${OBJEXT}; then
        FILE_TEST="$(file conftest.${OBJEXT})"
        if test -n "$(echo $FILE_TEST | grep -i $1-bit)"; then
          CFLAGS="${CFLAGS} -m$1"
          AC_MSG_RESULT([ok use $1])
        else
          AC_MSG_ERROR([failed still $COMPILER_MODEL_BITS])
        fi
      fi
      /bin/rm -f conftest.$ac_ext conftest.${OBJEXT}
    else
      AC_MSG_RESULT([ok use $1])
    fi
  AC_LANG_POP([C])

  AC_LANG_PUSH([C++])
    AC_MSG_CHECKING([if C++ compiler can use -m$1])
    if test "$1" != "${COMPILER_MODEL_BITS}"; then
      echo "int main() { return 0; }" >conftest.$ac_ext
      AC_REQUIRE_CPP
      ${CXX} ${CXXFLAGS} ${CPPFLAGS} -m$1 -fno-lto -c conftest.$ac_ext 2>&AS_MESSAGE_LOG_FD >&AS_MESSAGE_LOG_FD
      if test -f conftest.${OBJEXT}; then
        FILE_TEST="$(file conftest.${OBJEXT})"
        if test -n "$(echo $FILE_TEST | grep -i $1-bit)"; then
          CXXFLAGS="${CXXFLAGS} -m$1"
          AC_MSG_RESULT([ok use $1])
        else
          AC_MSG_ERROR([failed still $COMPILER_MODEL_BITS])
        fi
      fi
      /bin/rm -f conftest.$ac_ext conftest.${OBJEXT}
    else
      AC_MSG_RESULT([ok use $1])
    fi
  AC_LANG_POP([C++])

  COMPILER_MODEL_BITS=$1
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
