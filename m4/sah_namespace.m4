# The contents of this file are subject to the BOINC Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://boinc.berkeley.edu/license_1.0.txt
#
# Revision Log:
# $Log$
# Revision 1.1  2003/12/11 18:38:24  korpela
# Added checked macro files into boinc
#
# Revision 1.5  2003/12/03 23:46:11  korpela
# Fixed "sah_namespaces.m4" not to rely on "tr".
#
#

AC_PREREQ([2.54])


AC_DEFUN([SAH_LC_TO_DEFN],[
  $1=`echo $2 | $AWK '{split($[]1,a,"(");print a[[ 1 ]]}' | $as_tr_cpp`
])

AC_DEFUN([SAH_NS_TO_DEFN],[
  $1=`echo $2 | $as_tr_cpp`
])

AC_DEFUN([SAH_CHECK_NAMESPACES],[
  AC_LANG_PUSH(C++)
  AC_MSG_CHECKING(for C++ namespaces)
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([
      #define CONFIG_TEST
      namespace foo {
        int foo(void) { return(0); }
      }
    ],[
      return foo::foo();
    ])
  ], [AC_DEFINE(HAVE_NAMESPACES,[1],[Define if your C++ compiler supports namespaces]) sah_cv_have_namespaces="yes"], [sah_cv_have_namespaces="no"])
  AC_MSG_RESULT($sah_cv_have_namespaces)
  AC_LANG_POP
])

AC_DEFUN([SAH_FUNCS_IN_NAMESPACE],[
  AC_LANG_PUSH(C++)
  for func_name in $1 
  do
    func_name=m4_quote($func_name)
    t_ns=m4_quote($2)
    AC_MSG_CHECKING([$func_name in namespace $t_ns])
    SAH_LC_TO_DEFN(ac_func_upper,[$func_name]) 
    SAH_LC_TO_DEFN(ac_namespace_upper,[$t_ns]) 
    ac_uc_defn=`echo HAVE_"$ac_namespace_upper"_$ac_func_upper`
    AC_LINK_IFELSE(
	[AC_LANG_PROGRAM([[
	          #define CONFIG_TEST
	          $sah_cxx_includes
		]],
		[[
		 $2::$func_name ;
		]])],
        [AC_DEFINE_UNQUOTED([$ac_uc_defn], [1], 
	["Define to 1 if $func_name is in namespace $t_ns::"  ] ) tmp_res=yes],
	[tmp_res=no])
    eval sah_cv_func_$2_$ac_func_upper=$tmp_res
    AC_MSG_RESULT($tmp_res)
  done
  AC_LANG_POP
])


