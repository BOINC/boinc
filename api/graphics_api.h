#ifndef BOINC_GRAPHICS_API_H
#define BOINC_GRAPHICS_API_H

#ifdef __APPLE_CC__
#ifndef HAVE_GL_LIB 
#define HAVE_GL_LIB 1
#endif
#ifndef HAVE_OPENGL_GL_H
#define HAVE_OPENGL_GL_H
#endif
#include <Carbon/Carbon.h>
#endif

#ifdef _WIN32
#include <windows.h>
#ifndef HAVE_GL_LIB
#define HAVE_GL_LIB 1
#endif
#ifndef HAVE_GL_GL_H
#define HAVE_GL_GL_H 1
#endif
#ifndef HAVE_GL_GLU_H
#define HAVE_GL_GLU_H 1
#endif
#ifndef HAVE_GL_GLAUX_H
#define HAVE_GL_GLAUX_H 1
#endif
#endif

#ifdef HAVE_GL_H
#include <gl.h>
#elif defined(HAVE_GL_GL_H)
#include <GL/gl.h>
#elif defined(HAVE_OPENGL_GL_H)
#include <OpenGL/gl.h>
#elif defined(HAVE_MESAGL_GL_H)
#include <MesaGL/gl.h>
#endif
#ifdef HAVE_GLU_H
#include <glu.h>
#elif defined(HAVE_GL_GLU_H)
#include <GL/glu.h>
#elif defined(HAVE_OPENGL_GLU_H)
#include <OpenGL/glu.h>
#elif defined(HAVE_MESAGL_GLU_H)
#include <MesaGL/glu.h>
#endif
#ifdef HAVE_GLUT_H
#include <glut.h>
#elif defined(HAVE_GL_GLUT_H)
#include <GL/glut.h>
#elif defined(HAVE_OPENGL_GLUT_H)
#include <OpenGL/glut.h>
#elif defined(HAVE_MESAGL_GLUT_H)
#include <MesaGL/glut.h>
#elif defined(HAVE_GLUT_GLUT_H)
#include <GLUT/glut.h>
#endif
#ifdef HAVE_GLAUX_H
#include <glaux.h>
#elif defined(HAVE_GL_GLAUX_H)
#include <GL/glaux.h>
#elif defined(HAVE_OPENGL_GLAUX_H)
#include <OpenGL/glaux.h>
#elif defined(HAVE_MESAGL_GLAUX_H)
#include <MesaGL/glaux.h>
#endif

#if defined(HAVE_GL_LIB) && defined(HAVE_X11)
#include "x_opengl.h"
#endif

// The API (functions called by the app)
extern int boinc_init_opengl();
extern int boinc_finish_opengl();

#ifdef HAVE_GL_LIB
extern GLvoid glPrint(GLuint font, const char *fmt, ...);
#endif

// Functions that must be supplied by the app
//
extern void app_render(int xs, int ys, double time_of_day);
extern void app_init_gl(void);
extern void app_resize(int width, int height);

// Implementation stuff
//
#ifdef HAVE_GL_LIB
extern GLenum InitGL(GLvoid);
extern GLenum ReSizeGLScene(GLsizei width, GLsizei height);
#endif
extern bool throttled_app_render(int, int, double);

#ifdef _WIN32
extern HANDLE hQuitEvent;
extern HANDLE graphics_threadh;
extern BOOL    win_loop_done;
#endif

#endif
