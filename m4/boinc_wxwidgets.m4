dnl  These functions still require wxWidgets.m4
   
AC_DEFUN([BOINC_OPTIONS_WXWIDGETS],[

   AC_ARG_ENABLE(unicode, 
     AS_HELP_STRING([--enable-unicode/--disable-unicode],
                   [enable/disable building the manager with unicode support]),
     [enable_unicode="$enableval"],
     [])

   AM_OPTIONS_WXCONFIG
     AM_PATH_WXCONFIG($1, [_ac_cv_have_wxwidgets=yes], [_ac_cv_have_wxwidgets=no])
   AC_CACHE_CHECK([if wxWidgets works],[ac_cv_have_wxwidgets],
    [ac_cv_have_wxwidgets="${_ac_cv_have_wxwidgets}"])
   AC_CACHE_SAVE
   if test "x$ac_cv_path_WX_CONFIG_PATH" != "x" ; then
     WX_CONFIG="$ac_cv_path_WX_CONFIG_PATH"
   elif test "x$WX_CONFIG_PATH" != "x" ; then
     WX_CONFIG="$WX_CONFIG_PATH"
   fi
   if test "x${ac_cv_have_wxwidgets}" = "xyes" ; then
dnl Find the default wxWidgets options.
     wx_default_config="`$WX_CONFIG --selected-config`"
     AC_MSG_CHECKING(for default wxWidgets config)
     AC_MSG_RESULT($wx_default_config)
     AC_CACHE_CHECK([for wxwidgets options],[ac_cv_wxwidgets_options],[
       ac_cv_wxwidgets_options=""
       if test "x${enable_client_release}" = xyes ; then
         if $WX_CONFIG $ac_cv_wxwidgets_options --static=yes --selected-config 2>&1 >/dev/null ; then
           ac_cv_wxwidgets_options="${ac_cv_wxwidgets_options} --static=yes"
         else
           AC_MSG_WARN(
[===============================================================================
WARNING: No static libraries for wxWidgets are installed.
         ==> building with dynamic libraries.

  You requested a release build, but configure is unable to find static 
  wxWidgets libraries.  This may be OK if your release will be installed with
  a packaging system that supports dependencies.  Be sure to include the 
  wxWidgets dynamic library package as a dependency.  
  
  If not, you have several options.  You can install the wxWidgets static 
  libraries on your system and rerun configure.  You can link with uninstalled
  wxWidgets static libraries by using the --with-wxdir=PATH option to 
  configure.  Or you can require that people using your build manually install
  the wxWidgets dynamic libraries.
===============================================================================
])
           ac_cv_wxwidgets_options="${ac_cv_wxwidgets_options} --static=no"
         fi
       fi
       wx_default_config="`$WX_CONFIG ${ac_cv_wxwidgets_options} --selected-config`"
       if test "x${enable_unicode}" = x ; then
         isuc="`echo ${wx_default_config} | grep unicode`" 
	 if test "x${isuc}" = x ; then
	   enable_unicode=no
	 else
	   enable_unicode=yes
	 fi
       fi
	   
       if test "x${enable_unicode}" != x ; then
         if $WX_CONFIG ${ac_cv_wxwidgets_options} --unicode=${enable_unicode} --selected-config 2>&1 >/dev/null ; then
           ac_cv_wxwidgets_options="${ac_cv_wxwidgets_options} --unicode=${enable_unicode}"
         else
	   if test "x${enable_unicode}" = xno ; then
	     uprf="ascii"
	     nprf="unicode"
	   else
	     uprf="unicode"
	     nprf="ascii"
	   fi  
           AC_MSG_WARN([
===============================================================================
WARNING: No ${uprf} libraries for wxWidgets are installed.
         ==> building with nprf libraries.  
	 
  You requested a ${uprf} build, but configure is unable to find ${uprf} 
  wxWidgets libraries.  We will build with the default ${nprf} libraries.
===============================================================================
])
           ac_cv_wxwidgets_options="${ac_cv_wxwidgets_options}"
         fi
       fi
       wx_default_config="`$WX_CONFIG ${ac_cv_wxwidgets_options} --selected-config`"
       if test "x${enable_wx_debug}" != x ; then
         if $WX_CONFIG ${ac_cv_wxwidgets_options} --debug=${enable_debug} --selected-config 2>&1 >/dev/null ; then
           ac_cv_wxwidgets_options="${ac_cv_wxwidgets_options} --debug=${enable_debug}"
         else
	   if test "x${enable_debug}" = xno ; then
	     uprf="non-debug"
	     nprf="debug"
	   else
	     uprf="debug"
	     nprf="non-debug"
	   fi  
           AC_MSG_WARN([
===============================================================================
WARNING: No ${uprf} libraries for wxWidgets are installed.
         ==> building with nprf libraries.  
	 
  You requested a ${uprf} build, but configure is unable to find ${uprf} 
  wxWidgets libraries.  We will build with the default ${nprf} libraries.
===============================================================================
])
           ac_cv_wxwidgets_options="${ac_cv_wxwidgets_options}"
         fi
       fi
     ])
     wx_default_config="`$WX_CONFIG ${ac_cv_wxwidgets_options} --selected-config`"
     AC_MSG_CHECKING([wxWidgets config to use])
     AC_MSG_RESULT([$wx_default_config])
     if test "x${ac_cv_wxwidgets_options}" != "x" ; then
       AM_PATH_WXCONFIG($1, wxWin=1, wxWin=0, ${ac_cv_wxwidgets_options})
     fi
   fi
])
     
