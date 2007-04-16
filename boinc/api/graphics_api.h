#ifndef BOINC_GRAPHICS_API_H
#define BOINC_GRAPHICS_API_H

#ifdef _WIN32
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

#ifdef __APPLE_CC__
#ifndef HAVE_GL_LIB 
#define HAVE_GL_LIB 1
#endif
#ifndef HAVE_OPENGL_GL_H
#define HAVE_OPENGL_GL_H
#endif
#include <Carbon/Carbon.h>
#endif

#ifdef HAVE_GL_H
#include <gl.h>
#elif defined(HAVE_GL_GL_H)
#include <GL/gl.h>
#elif defined(HAVE_OPENGL_GL_H)
#include <OpenGL/gl.h>
#elif defined(HAVE_MESAGL_GL_H)
#include <MesaGL/gl.h>
#else
#error No gl.h
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
extern int boinc_init_graphics();
extern int boinc_finish_graphics();

#ifdef HAVE_GL_LIB
extern GLvoid glPrint(GLuint font, const char *fmt, ...);
#endif

// Functions that must be supplied by the app
//
extern void app_graphics_render(int xs, int ys, double time_of_day);
extern void app_graphics_init();
    // called each time a window is opened;
    // called in the graphics thread
extern void app_graphics_reread_prefs();
    // called when get REREAD_PREFS message from core client.
    // called in the graphics thread
extern void app_graphics_resize(int width, int height);

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern GLenum InitGL(GLvoid);
extern GLenum ReSizeGLScene(GLsizei width, GLsizei height);
extern bool throttled_app_render(int, int, double);

#ifdef _WIN32
extern HANDLE hQuitEvent;
extern HANDLE graphics_threadh;
extern BOOL   win_loop_done;
#endif

#endif
