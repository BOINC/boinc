// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef H_BOINC_GL
#define H_BOINC_GL


#if defined(_WIN32)
#  include <GL/gl.h>
#  include <GL/glu.h>
#if !defined(__MINGW32__) && !defined(__CYGWIN32__)
#  include <GL/glaux.h>
#else
#  include <GL/glext.h>
#endif

#elif defined(__APPLE_CC__)

#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>


#else // !_WIN32, !__APPLE_CC__
#include "config.h"

#  if defined(HAVE_GL_H)
#    include <gl.h>
#  elif defined(HAVE_GL_GL_H)
#    include <GL/gl.h>
#  elif defined(HAVE_OPENGL_GL_H)
#    include <OpenGL/gl.h>
#  endif

#  if defined(HAVE_GLX_H)
#    include <glx.h>
#  elif defined(HAVE_GL_GLX_H)
#    include <GL/glx.h>
#  endif

#  if defined(HAVE_GLU_H)
#    include <glu.h>
#  elif defined(HAVE_GL_GLU_H)
#    include <GL/glu.h>
#  elif defined(HAVE_OPENGL_GLU_H)
#    include <OpenGL/glu.h>
#  endif

#endif // _WIN32

#endif
