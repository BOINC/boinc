#ifndef __glutint_h__
#define __glutint_h__

/* Copyright (c) Mark J. Kilgard, 1994, 1997, 1998. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#if defined(_WIN32)
#include "glutwin32.h"
#else
#ifdef __sgi
#define SUPPORT_FORTRAN
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#endif

#include "glut.h"


#endif /* __glutint_h__ */
