#ifndef BOINC_GRAPHICS_API_H
#define BOINC_GRAPHICS_API_H

#include <stdio.h>

#ifdef BOINC_APP_GRAPHICS
#ifdef __APPLE_CC__
    #include <OpenGL/gl.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <gl\gl.h>            // Header File For The OpenGL32 Library
#include <gl\glu.h>            // Header File For The GLu32 Library
#include <gl\glaux.h>        // Header File For The Glaux Library
#endif

#ifdef HAVE_GL_LIB
#include <GL/gl.h>
#include "x_opengl.h"
#endif
#endif

#include "app_ipc.h"

struct APP_OUT_GRAPHICS {
};

int boinc_init_opengl();
int boinc_finish_opengl();

#ifdef BOINC_APP_GRAPHICS
GLvoid glPrint(GLuint font, const char *fmt, ...);
GLenum InitGL(GLvoid);
GLenum ReSizeGLScene(GLsizei width, GLsizei height);
extern bool app_render(int xs, int ys, double time_of_day);
extern void app_init_gl(void);
extern void app_resize(int width, int height);
#endif

#endif
