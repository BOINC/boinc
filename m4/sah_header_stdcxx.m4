# The contents of this file are subject to the BOINC Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://boinc.berkeley.edu/license_1.0.txt

AC_PREREQ([2.54])
	
AC_DEFUN([SAH_HEADER_STDCXX],[
  save_inc="$ac_includes_default"
  ac_includes_default="$ac_includes_default 
#define CONFIG_TEST
#include \"lib/std_fixes.h\" 
"
  sah_stdcxx_headers="algorithm bitset cassert cctype cerrno cfloat climits clocale cmath complex csetjmp csignal cstdarg cstddef cstdio cstdlib cstring ctime deque fstream functional iomanip ios iosfwd iostream istream iterator limits list locale map memory numeric ostream queue set sstream stack stdexcept streambuf string utility valarray vector"
  AC_LANG_PUSH(C++) 
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
  AC_CACHE_SAVE
  AC_LANG_POP
  ac_includes_default="$save_inc"
  CONFIG_TEST=
])

