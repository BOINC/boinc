#ifndef __glutwin32_h__
#define __glutwin32_h__

/* Copyright (c) Nate Robins, 1997. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include "win32_x11.h"
#include "win32_glx.h"

/* Private routines from win32_util.c */
extern void *__glutFont(void *font);
extern int __glutGetTransparentPixel(Display *dpy, XVisualInfo *vinfo);
extern void __glutAdjustCoords(Window parent, int *x, int *y, int *width, int *height);

#endif /* __glutwin32_h__ */
