
AC_DEFUN([BOINC_CHECK_GTK],[
  AC_MSG_CHECKING([if wxWidgets uses GTK])
  loc_uses_gtk="`echo $WX_LIBS | grep gtk`"
  if test "x${loc_uses_gtk}" = "x" ; then
    AC_MSG_RESULT(no)
  else
    AC_MSG_RESULT(yes)
    AC_PATH_PROG(PKGCONFIG,[$mypkgconfig pkg-config])
    AC_MSG_CHECKING([if gtk uses pkg-config])
    gtk_module="`$PKGCONFIG --list-all | grep -E '^gtk\+?-' | head -1 | awk '{print $[]1}'`"
    if test "x${gtk_module}" != x; then
      GTK_CONFIG="$PKGCONFIG ${gtk_module}"
      AC_MSG_RESULT([yes])
    else
      GTK_CONFIG="no"
      AC_MSG_RESULT([no])
    fi
    if test "x${GTK_CONFIG}" = "xno" ; then
      AC_PATH_PROG(GTK_CONFIG,[gtk-config],no)
    fi
    if test "x${GTK_CONFIG}" != "xno" ; then
      gtk_config_flags="--libs"
      GTK_DYNLIBS="`$GTK_CONFIG ${gtk_config_flags}`"
      if test "x${enable_client_release}" = "xyes" ; then
        gtk_config_flags="--libs --static"
      fi
      AC_MSG_CHECKING([GTK libraries])
      GTK_LIBS=`$GTK_CONFIG ${gtk_config_flags}`
      if test "x$GTK_LIBS" = "x" ; then
        GTK_LIBS="$GTK_DYNLIBS"
      fi
      AC_MSG_RESULT([$GTK_LIBS])
    else
      GTK_LIBS=
      AC_MSG_RESULT(none)
    fi
  fi
  AC_SUBST([GTK_LIBS])
])
