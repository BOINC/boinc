dnl  These functions still require wxWidgets.m4

AC_DEFUN([BOINC_OPTIONS_WXWIDGETS],[

   AC_ARG_ENABLE(unicode,
     AS_HELP_STRING([--enable-unicode/--disable-unicode],
                   [enable/disable building the manager with unicode support]),
     [enable_unicode="$enableval"],
     [])

   WX_CONFIG_OPTIONS
dnl When building with '--with-wxwidgets=PATH', skip checking for cross-compile
   cross_compiling_orig=$cross_compiling
   if test "x$wx_config_name" != x ; then
     cross_compiling=no
   fi
     WX_CONFIG_CHECK([$1],
       [_ac_cv_have_wxwidgets=yes],
       [AC_MSG_ERROR([No suitable wxWidgets library found.])],
       [std], ,
       [webview])
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
         ==> building with ${nprf} libraries.

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
       WX_CONFIG_CHECK($1, wxWin=1, wxWin=0, [std], ${ac_cv_wxwidgets_options}, [webview])
     fi
   fi

   AC_MSG_CHECKING([if wxWidgets uses the GTK+ toolkit])
   gtkver=none
   AM_CONDITIONAL([GUI_GTK], echo $wx_default_config | grep -i ^gtk 2>&1 >/dev/null)
   if echo $wx_default_config | grep -i gtk 2>&1 >/dev/null ; then
     case ${wx_default_config} in
        gtk3-*|*-gtk3-*)  gtkver=gtk+-3.0
	         ;;
        gtk2-*|*-gtk2-*)  gtkver=gtk+-2.0
	         ;;
        gtk-*|*-gtk-*)    gtkver=gtk+
	         ;;
      esac
      GTK_CFLAGS="`pkg-config --cflags $gtkver`"
      if test "x$GTK_LIBS" = "x" ; then
        GTK_LIBS="`pkg-config --libs $gtkver`"
      fi
   fi
   AC_SUBST([GTK_CFLAGS])
   AC_SUBST([GTK_LIBS])
   AC_MSG_RESULT([$gtkver])
   cross_compiling=$cross_compiling_orig
])
