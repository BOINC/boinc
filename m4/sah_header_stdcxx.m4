# The contents of this file are subject to the BOINC Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://boinc.berkeley.edu/license_1.0.txt

AC_PREREQ([2.54])
	
AC_DEFUN([SAH_HEADER_STDCXX],[
  save_inc="$ac_includes_default"
  ac_includes_default="
#define CONFIG_TEST
#include \"lib/std_fixes.h\"
$ac_includes_default 
"

  sah_stdcxx_headers="algorithm bitset cassert cctype cerrno cfloat climits clocale cmath complex csetjmp csignal cstdarg cstddef cstdio cstdlib cstring ctime deque fstream functional iomanip ios iosfwd iostream istream iterator limits list locale map memory numeric ostream queue set sstream stack stdexcept streambuf string utility valarray vector"
  AC_LANG_PUSH(C++) 
  dnl First we'll check to see if they are all here in order to save time.
  AC_MSG_CHECKING([standard C++ headers])
  tmp_includes=
  for header in $sah_stdcxx_headers
  do
    tmp_includes="$tmp_includes 
#include <$header>
"
  done
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([[
$ac_includes_default
$tmp_includes
      ]],
      []
    )],
    [
    ac_includes_default="${ac_includes_default}
${tmp_includes}
"
    sah_cxx_includes=${tmp_includes}
    AC_MSG_RESULT(yes)
    for header in $sah_stdcxx_headers
    do
      eval ac_cv_header_${header}=yes
      ac_uc_defn=HAVE_`echo ${header} | sed -e 's/[^a-zA-Z0-9_]/_/g' \
          -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/'`
      AC_DEFINE_UNQUOTED([${ac_uc_defn}],1,[Define to 1 if you have the ${header} header])
      AC_CACHE_CHECK([for C++ header <${header}>],[ac_cv_header_${header}])
    done
    ],
    [
    ac_includes_default="$save_inc"
    AC_MSG_RESULT(no)
    AC_CHECK_HEADERS([$sah_stdcxx_headers])
    for header in $sah_stdcxx_headers 
    do
      eval tmp_var=\$ac_cv_header_${header}
      if test "$tmp_var" = "yes"
      then 
        sah_cxx_includes="$sah_cxx_includes 
#include <$header>
"
      fi
    done
    ]
  )
  ac_includes_default="$save_inc"
  AC_CACHE_SAVE
  AC_LANG_POP
  CONFIG_TEST=
])

