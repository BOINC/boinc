// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOINC_BOINC_GL_H
#define BOINC_BOINC_GL_H

#if defined(_WIN32)
#  include <GL/gl.h>
#  include <GL/glu.h>
#if defined(__MINGW32__) || defined(__CYGWIN32__)
#  include <GL/glext.h>
#endif

#elif defined(__APPLE_CC__)

#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>

#else // !_WIN32, !__APPLE_CC__
#include "config.h"

#  if HAVE_GL_H
#    include <gl.h>
#  elif HAVE_GL_GL_H
#    include <GL/gl.h>
#  elif HAVE_OPENGL_GL_H
#    include <OpenGL/gl.h>
#  endif

#  if HAVE_GLX_H
#    include <glx.h>
#  elif HAVE_GL_GLX_H
#    include <GL/glx.h>
#  endif

#  if HAVE_GLU_H
#    include <glu.h>
#  elif HAVE_GL_GLU_H
#    include <GL/glu.h>
#  elif HAVE_OPENGL_GLU_H
#    include <OpenGL/glu.h>
#  endif

#endif // _WIN32

#endif
