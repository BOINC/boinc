# SETI_BOINC is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2, or (at your option) any later
# version.

AC_DEFUN([SAH_LIBEXT],[
  AC_MSG_CHECKING(library extension)
  if test -n `echo $AR | grep ar` 
  then
    LIBEXT=a
  else
    LIBEXT=lib
  fi
  AC_MSG_RESULT($LIBEXT)
  AC_SUBST(LIBEXT)
])

AC_DEFUN([SAH_DLLEXT],[
  AC_MSG_CHECKING(shared object extension)
  case "${host}" in
    *-*-emx* | *-*-mingw* | *-*-cygwin* | *-*-rmx*)
      DLLEXT="dll"
      ;;
    *-*-darwin*)
      DLLEXT="dylib"
      ;;
    *)
      DLLEXT="so"
      ;;
  esac
  AC_MSG_RESULT($DLLEXT)
  AC_DEFINE_UNQUOTED(DLLEXT,".${DLLEXT}",[Define to the platform's shared library extension])
  AC_SUBST(DLLEXT)
])
