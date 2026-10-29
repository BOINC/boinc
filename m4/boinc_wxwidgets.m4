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
     boinc_wxwidgets_options="${ac_cv_wxwidgets_options}"
     wx_default_config="`$WX_CONFIG ${boinc_wxwidgets_options} --selected-config`"
     if test -n "$PKG_CONFIG" ; then
       case ${wx_default_config} in
         gtk2-*|*-gtk2-*)
           if $PKG_CONFIG --exists gtk+-2.0 >/dev/null 2>&1 ; then
             :
           elif $PKG_CONFIG --exists gtk+-3.0 >/dev/null 2>&1 && $WX_CONFIG ${boinc_wxwidgets_options} --toolkit=gtk3 --selected-config >/dev/null 2>&1 ; then
             boinc_wxwidgets_options="${boinc_wxwidgets_options} --toolkit=gtk3"
           fi
           ;;
         gtk3-*|*-gtk3-*)
           if $PKG_CONFIG --exists gtk+-3.0 >/dev/null 2>&1 ; then
             :
           elif $PKG_CONFIG --exists gtk+-2.0 >/dev/null 2>&1 && $WX_CONFIG ${boinc_wxwidgets_options} --toolkit=gtk2 --selected-config >/dev/null 2>&1 ; then
             boinc_wxwidgets_options="${boinc_wxwidgets_options} --toolkit=gtk2"
           fi
           ;;
       esac
     fi
     wx_default_config="`$WX_CONFIG ${boinc_wxwidgets_options} --selected-config`"
     AC_MSG_CHECKING([wxWidgets config to use])
     AC_MSG_RESULT([$wx_default_config])
     if test "x${boinc_wxwidgets_options}" != "x" ; then
       WX_CONFIG_CHECK([$1], [wxWin=1], [AC_MSG_ERROR([No suitable wxWidgets library found.])], [std], [${boinc_wxwidgets_options}], [webview])
     fi
   fi

   AC_MSG_CHECKING([if wxWidgets uses the GTK+ toolkit])
   gtkver=none
   AM_CONDITIONAL([GUI_GTK], echo $wx_default_config | grep -i gtk 2>&1 >/dev/null)
   if echo $wx_default_config | grep -i gtk 2>&1 >/dev/null ; then
     case ${wx_default_config} in
        gtk3-*|*-gtk3-*)  gtkver=gtk+-3.0
	         ;;
        gtk2-*|*-gtk2-*)  gtkver=gtk+-2.0
	         ;;
        gtk-*|*-gtk-*)    gtkver=gtk+
	         ;;
      esac
   fi
   AC_MSG_RESULT([$gtkver])
   if test "x$gtkver" != "xnone" ; then
     PKG_CHECK_MODULES([GTK], [$gtkver])
   fi
   AC_SUBST([GTK_CFLAGS])
   AC_SUBST([GTK_LIBS])
   cross_compiling=$cross_compiling_orig

   AC_LANG_PUSH(C++)
   save_cppflags="${CPPFLAGS}"
   save_cxxflags="${CXXFLAGS}"
   save_libs="${LIBS}"
   if test "${enable_debug}" = "yes" ; then
     CPPFLAGS="${CPPFLAGS} ${WX_CPPFLAGS} ${GTK_CFLAGS} -D_DEBUG -DDEBUG"
     CXXFLAGS="${CXXFLAGS} ${WX_CXXFLAGS} ${GTK_CFLAGS} -D_DEBUG -DDEBUG"
   else
     CPPFLAGS="${CPPFLAGS} ${WX_CPPFLAGS} ${GTK_CFLAGS} -DNDEBUG"
     CXXFLAGS="${CXXFLAGS} ${WX_CXXFLAGS} ${GTK_CFLAGS} -DNDEBUG"
   fi
   LIBS="${WX_LIBS} ${GTK_LIBS} ${LIBS}"
   AC_MSG_CHECKING([if wxWidgets is built with webview support])
   AC_COMPILE_IFELSE(
     [AC_LANG_PROGRAM([[
       #include <wx/wx.h>
       #if !wxUSE_WEBVIEW
       #error wxUSE_WEBVIEW is not enabled
       #endif
     ]], [[]])],
     [wx_use_webview=yes],
     [wx_use_webview=no]
   )
   AC_MSG_RESULT([$wx_use_webview])
   if test "x${wx_use_webview}" = "xyes" ; then
     AC_MSG_CHECKING([if wxWidgets webview works])
     AC_LINK_IFELSE(
       [AC_LANG_PROGRAM([[
         #include <wx/wx.h>
         #include <wx/webview.h>
         #include <wx/webviewarchivehandler.h>
         #include <wx/webviewfshandler.h>
       ]], [[
         wxWebView::New();
       ]])],
       [AC_MSG_RESULT([yes])],
       [
         AC_MSG_RESULT([no])
         AC_MSG_ERROR([
================================================================================
ERROR: wxWidgets is built with webview support (wxUSE_WEBVIEW=1), but the
       wxWidgets webview library or headers could not be compiled/linked.

  On Debian/Ubuntu, install the matching wxWidgets webview development package
  (e.g., libwxgtk-webview3.2-dev or libwxgtk-webview3.0-gtk3-dev) and re-run
  configure.
================================================================================
         ])
       ]
     )
   fi
   CPPFLAGS="${save_cppflags}"
   CXXFLAGS="${save_cxxflags}"
   LIBS="${save_libs}"
   AC_LANG_POP(C++)
])
