

AC_DEFUN([SAH_GRX_LIBS],[
  AC_LANG_PUSH(C)
  sah_save_libs="$LIBS"
  AC_PATH_PROG(XLSFONTS,[xlsfonts],[/usr/X11/bin:/usr/X11R6/bin:/usr/X11R5/bin:/usr/X11R4/bin:$PATH]) 
  sah_xpath=`echo $XLSFONTS | sed 's/bin\/xlsfonts//' `
  LIBS=`echo $LIBS "-L$sah_xpath"`
  AC_CHECK_LIB([X11], [XCreateGC], [ GRXLIBS="-L$sah_xpath/lib -lX11" ; AC_DEFINE([HAVE_X11],1,[Define to 1 if you have X11 libraries])])
  AC_CHECK_LIB([Xaw], [XawTextReplace], GRXLIBS="$GRXLIBS -lXaw")
  AC_CHECK_LIB([Xext], [XShmGetImage], GRXLIBS="$GRXLIBS -lXext")
  AC_CHECK_LIB([Xmu], [XmuMakeAtom], GRXLIBS="$GRXLIBS -lXmu")
  AC_CHECK_LIB([Xt], [XtDisplay], GRXLIBS="$GRXLIBS -lXt")
  AC_CHECK_LIB([ICE], [IceProtocolVersion], GRXLIBS="$GRXLIBS -lICE")
  AC_CHECK_LIB([SM], [SmsInitialize], GRXLIBS="$GRXLIBS -lSM")
  AC_CHECK_LIB([GL], [glCallList], GRXLIBS="$GRXLIBS -lGL")
  AC_CHECK_LIB([GLU], [main], GRXLIBS="$GRXLIBS -lGLU")
  AC_CHECK_LIB([GLUT], [main], GRXLIBS="$GRXLIBS -lGLUT")
  LIBS="$sah_save_libs"
  AC_SUBST(GRXLIBS)
  AC_LANG_POP
])

AC_DEFUN([SAH_GRX_INCLUDES],[
  AC_LANG_PUSH(C++)
  AC_PATH_PROG(XLSFONTS,[xlsfonts],[/usr/X11/bin:/usr/X11R6/bin:/usr/X11R5/bin:/usr/X11R4/bin:$PATH]) 
  sah_xpath=`echo $XLSFONTS | sed 's/bin\/xlsfonts//' `
  CFLAGS=`echo $CFLAGS "-I$sah_xpath/include"`
  AC_CHECK_HEADERS([gl.h glu.h glut.h glaux.h GL/gl.h GL/glu.h GL/glut.h GL/glaux.h OpenGL/gl.h OpenGL/glu.h OpenGL/glut.h OpenGL/glaux.h GLUT/glut.h MesaGL/gl.h MesaGL/glu.h MesaGL/glut.h MesaGL/glaux.h])
  AC_LANG_POP
])

