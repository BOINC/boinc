// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// Include this file in a BOINC application to display
// a static image (JPEG, GIFF, BMP, Targa) as its graphics.
//
// The image file must be included with the workunit
//

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "boinc_gl.h"

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
void boinc_app_mouse_button(int x, int y, int which, int is_down) {
}
void boinc_app_mouse_move(int x, int y, int left, int middle, int right) {
}
void app_graphics_resize(int w, int h) {
    glViewport(0, 0, w, h);
}

void app_graphics_reread_prefs() {
}
