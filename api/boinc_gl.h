// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef H_BOINC_GL
#define H_BOINC_GL

#if defined(_WIN32)
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glaux.h>
#  include <glut.h>

#elif defined(__APPLE_CC__)

#  include "GLUT/glut.h"

#else // !_WIN32, !__APPLE_CC__

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

# include "GL/glut.h"
/*
#  if defined(HAVE_GLUT_H)
#    include "glut.h"
#  elif defined(HAVE_GL_GLUT_H)
#    include <GL/glut.h>
#  elif defined(HAVE_OPENGL_GLUT_H)
#    include <OpenGL/glut.h>
#  elif defined(HAVE_GLUT_GLUT_H)
#    include <GLUT/glut.h>
#  endif
*/

#endif // _WIN32

#endif
