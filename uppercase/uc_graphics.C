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

#include "gutil.h"
#include "boinc_gl.h"
#include "graphics_api.h"
#include "txf_util.h"

float white[4] = {1., 1., 1., 1.};
TEXTURE_DESC logo;
int width, height;

static void initlights() {
   GLfloat ambient[] = {1., 1., 1., 1.0};
   GLfloat position[] = {-13.0, 6.0, 20.0, 1.0};
   GLfloat dir[] = {-1, -.5, -3, 1.0};
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
}

void app_graphics_init() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    txf_load_fonts(".");
    logo.load_image_file("logo.jpg");
    initlights();
    int viewport[4];
    get_viewport(viewport);
    int w = viewport[2];
    int h = viewport[3];
    app_graphics_resize(w,h);
}

static void draw_logo() {
    if (logo.present) {
        float pos[3] = {.2, .3, 0};
        float size[3] = {.6, .4, 0};
        logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER);
    }
}

static void draw_text() {
    static float x=0, y=0;
    static float dx=0.0003, dy=0.0007;
    char buf[256];
    x += dx;
    y += dy;
    if (x < 0 || x > .5) dx *= -1;
    if (y < 0 || y > .5) dy *= -1;
    sprintf(buf, "User: %s", uc_aid.user_name);
    txf_render_string(.1, x, y, 0, 500, white, 0, buf);
    sprintf(buf, "Team: %s", uc_aid.team_name);
    txf_render_string(.1, x, y+.1, 0, 500, white, 0, buf);
    sprintf(buf, "%% Done: %f", 100*boinc_get_fraction_done());
    txf_render_string(.1, x, y+.2, 0, 500, white, 0, buf);
}

static void draw_sphere() {
    static float x=0, y=0, z=10;
    static float dx=0.3, dy=0.2, dz=0.5;
    x += dx;
    y += dy;
    z += dz;
    if (x < -15 || x > 15) dx *= -1;
    if (y < -15 || y > 15) dy *= -1;
    if (z < 0 || z > 40) dz *= -1;
    float pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    drawSphere(pos, 4);
}

void set_viewpoint(double dist) {
    double x, y, z;
    x = 0;
    y = 3.0*dist;
    z = 11.0*dist;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        x, y, z,  // eye position
        0,-.8,0,      // where we're looking
        0.0, 1.0, 0.      // up is in positive Y direction
    );

}

static void app_init_camera(double dist) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
        45.0,       // field of view in degree
        1.0,        // aspect ratio
        1.0,        // Z near clip
        1000.0     // Z far
    );
    set_viewpoint(dist);
}

void app_graphics_render(int xs, int ys, double time_of_day) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mode_unshaded();
    mode_ortho();
    draw_logo();
    ortho_done();

    app_init_camera(10);
    scale_screen(width, height);
    GLfloat color[4] = {.7, .2, .5, 1};
    mode_shaded(color);
    draw_sphere();

    scale_screen(width, height);
    mode_unshaded();
    mode_ortho();
    draw_text();
    ortho_done();
}

void app_graphics_resize(int w, int h){
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}
void app_graphics_reread_prefs(){}
void boinc_app_mouse_move(
    int x, int y,       // new coords of cursor
    bool left,          // whether left mouse button is down
    bool middle,
    bool right
){}

void boinc_app_mouse_button(
    int x, int y,       // coords of cursor
    int which,          // which button (0/1/2)
    bool is_down        // true iff button is now down
){}

void boinc_app_key_press(
    int, int            // system-specific key encodings
){}

void boinc_app_key_release(
    int, int            // system-specific key encodings
){}

