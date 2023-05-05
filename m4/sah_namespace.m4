# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2005 University of California
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# either version 2.1 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# To view the GNU Lesser General Public License visit
# http://www.gnu.org/copyleft/lesser.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
# Revision Log:
# $Log$
# Revision 1.4  2005/12/24 20:08:03  rwalton
# *** empty log message ***
#
# Revision 1.3  2005/05/10 15:43:56  korpela
# Fixed bug in caching of SAH_FUNCS_IN_NAMESPACE results.
#
# Revision 1.2  2005/05/06 00:31:05  korpela
# Added caching of results to SAH_CHECK_NAMESPACES and SAH_FUNCS_IN_NAMESPACE
#
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
  AC_CACHE_CHECK([for C++ namespaces],
    [sah_cv_have_namespaces],[
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
        #define CONFIG_TEST
        namespace foo {
          int foo(void) { return(0); }
        }
      ],[
        return foo::foo();
      ])
    ], [sah_cv_have_namespaces="yes"], [sah_cv_have_namespaces="no"])
  ])
  if test "${sah_cv_have_namespaces}" = "yes" ; then
    AC_DEFINE(HAVE_NAMESPACES,[1],[Define if your C++ compiler supports namespaces])
  fi
  AC_LANG_POP(C++)
])

AC_DEFUN([SAH_FUNCS_IN_NAMESPACE],[
  AC_LANG_PUSH(C++)
  for func_name in $1
  do
    func_name=m4_quote($func_name)
    t_ns=m4_quote($2)
    SAH_LC_TO_DEFN(ac_func_upper,[$func_name])
    SAH_LC_TO_DEFN(ac_namespace_upper,[$t_ns])
    ac_uc_defn=`echo HAVE_"$ac_namespace_upper"_$ac_func_upper`
    AC_CACHE_CHECK([for $func_name in namespace $t_ns],
      [sah_cv_func_$2_$ac_func_upper],[
      AC_LINK_IFELSE(
	[AC_LANG_PROGRAM([[
	          #define CONFIG_TEST
	          $sah_cxx_includes
		]],
		[[
		 $2::$func_name ;
		]])],
        [
	 eval sah_cv_func_$2_$ac_func_upper="yes"
	],
        [
	 eval sah_cv_func_$2_$ac_func_upper="no"
	]
      )
    ])
    if test "`eval echo '${'sah_cv_func_$2_$ac_func_upper'}'`" = "yes" ; then
        AC_DEFINE_UNQUOTED([$ac_uc_defn], [1],
	["Define to 1 if $func_name is in namespace $t_ns::"  ])
    fi
  done
  AC_LANG_POP(C++)
])


