dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/ax_check_glut.html
dnl
AC_DEFUN([AX_CHECK_GLUT],
[AC_REQUIRE([AX_CHECK_GLU])dnl
AC_REQUIRE([AC_PATH_XTRA])dnl

if test "X$with_apple_opengl_framework" = "Xyes"; then
  GLUT_CFLAGS="${GLU_CFLAGS}"
  GLUT_LIBS="-framework GLUT -lobjc ${GL_LIBS}"
else
  GLUT_CFLAGS=${GLU_CFLAGS}
  GLUT_LIBS=${GLU_LIBS}

  #
  # If X is present, assume GLUT depends on it.
  #
  if test "X${no_x}" != "Xyes"; then
    GLUT_LIBS="${X_PRE_LIBS} -lXmu -lXi ${X_EXTRA_LIBS} ${GLUT_LIBS}"
  fi
  #
  # If were running under windows assume we need GDI32 and WinMM
  #
  if echo $host_os | grep -E '^mingw|^winnt' > /dev/null ; then
    GLUT_LIBS="${GLUT_LIBS} -lgdi32 -lwinmm"
  fi

  AC_LANG_PUSH(C)

  ax_save_CPPFLAGS="${CPPFLAGS}"
  CPPFLAGS="${GLUT_CFLAGS} ${CPPFLAGS}"

  AC_CACHE_CHECK([for GLUT library], [ax_cv_check_glut_libglut],
  [ax_cv_check_glut_libglut="no"
  ax_save_LIBS="${LIBS}"
  LIBS=""
  ax_check_libs="-lglut32 -lglut -lfreeglut_static -lfreeglut"
  for ax_lib in ${ax_check_libs}; do
    if test X$ax_compiler_ms = Xyes; then
      ax_try_lib=`echo $ax_lib | sed -e 's/^-l//' -e 's/$/.lib/'`
    else
      ax_try_lib="${ax_lib}"
    fi
    LIBS="${ax_try_lib} ${GLUT_LIBS} ${ax_save_LIBS}"
    AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[
#define FREEGLUT_STATIC 1
# if HAVE_WINDOWS_H && (defined(_WIN32) || defined(CYGWIN_USE_WIN32))
#   include <windows.h>
# endif
# include <GL/glut.h>]],
                     [[glutMainLoop()]])],
    [ax_cv_check_glut_libglut="${ax_try_lib}"; break])

  done
  LIBS=${ax_save_LIBS}
  ])
  CPPFLAGS="${ax_save_CPPFLAGS}"
  AC_LANG_POP(C)

  if test "X${ax_cv_check_glut_libglut}" = "Xno"; then
    no_glut="yes"
    GLUT_CFLAGS=""
    GLUT_LIBS=""
  else
    GLUT_LIBS="${ax_cv_check_glut_libglut} ${GLUT_LIBS}"
  fi
fi

AC_SUBST([GLUT_CFLAGS])
AC_SUBST([GLUT_LIBS])
])dnl
