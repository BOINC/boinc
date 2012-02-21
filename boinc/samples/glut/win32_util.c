
/* Copyright (c) Nate Robins, 1997. */

/* portions Copyright (c) Mark Kilgard, 1997, 1998. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */


#include "glutint.h"
#include "glutstroke.h"
#include "glutbitmap.h"

extern StrokeFontRec glutStrokeRoman;

/* To get around the fact that Microsoft DLLs only allow functions
   to be exported and now data addresses (as Unix DSOs support), the
   GLUT API constants such as GLUT_STROKE_ROMAN have to get passed
   through a case statement to get mapped to the actual data structure
   address. */
void*
__glutFont(void *font)
{
  switch (*reinterpret_cast<size_t *>(&font)) {
#ifdef __MINGW32__
  case 0:
#else
  case (size_t)GLUT_STROKE_ROMAN:
#endif
    return &glutStrokeRoman;
  }
  return &glutStrokeRoman;
}

int
__glutGetTransparentPixel(Display * dpy, XVisualInfo * vinfo)
{
  /* the transparent pixel on Win32 is always index number 0.  So if
     we put this routine in this file, we can avoid compiling the
     whole of layerutil.c which is where this routine normally comes
     from. */
  return 0;
}

void
__glutAdjustCoords(Window parent, int* x, int* y, int* width, int* height)
{
  RECT rect;

  /* adjust the window rectangle because Win32 thinks that the x, y,
     width & height are the WHOLE window (including decorations),
     whereas GLUT treats the x, y, width & height as only the CLIENT
     area of the window. */
  rect.left = *x; rect.top = *y;
  rect.right = *x + *width; rect.bottom = *y + *height;

  /* must adjust the coordinates according to the correct style
     because depending on the style, there may or may not be
     borders. */
  AdjustWindowRect(&rect, WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		   (parent ? WS_CHILD : WS_OVERLAPPEDWINDOW),
		   FALSE);
  /* FALSE in the third parameter = window has no menu bar */

  /* readjust if the x and y are offscreen */
  if(rect.left < 0) {
    *x = 0;
  } else {
    *x = rect.left;
  }
  
  if(rect.top < 0) {
    *y = 0;
  } else {
    *y = rect.top;
  }

  *width = rect.right - rect.left;	/* adjusted width */
  *height = rect.bottom - rect.top;	/* adjusted height */
}

