// Include this file in a BOINC application to display
// a static image (JPEG, GIFF, BMP, Targa) as its graphics.
//
// The image file must be included with the workunit
//

#ifdef _WIN32
#include "stdafx.h"
#include <gl/gl.h>           // Header File For The OpenGL32 Library
#include <gl/glu.h>          // Header File For The GLu32 Library
#include <gl/glaux.h>        // Header File For The Glaux Library
#include <glut/glut.h>
#endif

#include "gutil.h"

#define FILENAME "background"

TEXTURE_DESC background;

void app_graphics_render(int xs, int ys, double time_of_day) {
    float pos[3] = {0, 0, 0};
    float size[3] = {1, 1, 0};
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mode_ortho();
    mode_unshaded();
    glColor4d(1,1,1,1);
    background.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER);
    ortho_done();
    glFlush();
}

void app_graphics_init() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// Black Background
    glClearDepth(1.0f);					// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);				// The Type Of Depth Testing To Do

    char filename[256];
    boinc_resolve_filename(FILENAME, filename, sizeof(filename));
    background.load_image_file(filename);
}
void boinc_app_key_press(int, int) {
}

void boinc_app_key_release(int, int) {
}
void boinc_app_mouse_button(int x, int y, int which, bool is_down) {
}
void boinc_app_mouse_move(int x, int y, bool left, bool middle, bool right) {
}
void app_graphics_resize(int w, int h) {
    glViewport(0, 0, w, h);
}