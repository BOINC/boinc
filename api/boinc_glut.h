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

#ifndef BOINC_BOINC_GLUT_H
#define BOINC_BOINC_GLUT_H


#ifdef _WIN32
#if HAVE_GL_GLUT_H
#  include <GL/glut.h>
#else
#  include<glut.h>
#endif

#elif defined(__APPLE_CC__)

#  include "GLUT/glut.h"

#else // !_WIN32, !__APPLE_CC__

#include "config.h"

#  if HAVE_GLUT_H
#    include "glut.h"
#  elif HAVE_GL_GLUT_H
#    include <GL/glut.h>
#  elif HAVE_OPENGL_GLUT_H
#    include <OpenGL/glut.h>
#  elif HAVE_GLUT_GLUT_H
#    include <GLUT/glut.h>
#  endif

#endif // _WIN32

#endif
