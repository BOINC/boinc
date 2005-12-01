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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef H_BOINC_GL
#define H_BOINC_GL


#if defined(_WIN32)
#  include <GL/gl.h>
#  include <GL/glu.h>
#ifndef __MINGW32__
#  include <GL/glaux.h>
#else
#  include <GL/glext.h>
#endif
#  include <glut.h>

#elif defined(__APPLE_CC__)

#  include "GLUT/glut.h"

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

#  if defined(HAVE_GLUT_H)
#    include "glut.h"
#  elif defined(HAVE_GL_GLUT_H)
#    include <GL/glut.h>
#  elif defined(HAVE_OPENGL_GLUT_H)
#    include <OpenGL/glut.h>
#  elif defined(HAVE_GLUT_GLUT_H)
#    include <GLUT/glut.h>
#  endif

#endif // _WIN32

#endif
