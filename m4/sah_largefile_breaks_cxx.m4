# SETI_BOINC is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2, or (at your option) any later
# version.

AC_DEFUN([SAH_LARGEFILE_BREAKS_CXX],[
  AC_MSG_CHECKING([whether largefile support breaks C++])
  AC_LANG_PUSH(C++)
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[
        #define CONFIG_TEST
        #include <iostream>
      ]],
      [[
        std::cout << 1.0 << std::endl;
      ]])],
    [tmp_res="no"],
    [AC_DEFINE([LARGEFILE_BREAKS_CXX],[1],
    ["Define to 1 if largefile support causes missing symbols in C++"] )
    tmp_res="yes"
    sah_cxx_includes=`echo "#include  \"${ac_aux_dir}/lib/std_fixes.h\"" ; echo $sah_cxx_includes`
    ]
  )
  AC_MSG_RESULT($tmp_res)
  AC_LANG_POP
])

