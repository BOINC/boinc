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

// graphics code for upper_case

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <math.h>
#include "config.h"
#endif

#include "parse.h"
#include "gutil.h"
#include "boinc_gl.h"
#include "graphics_api.h"
#include "txf_util.h"

float white[4] = {1., 1., 1., 1.};
TEXTURE_DESC logo;
int width, height;      // window dimensions
APP_INIT_DATA uc_aid;
bool mouse_down = false;
int mouse_x, mouse_y;
double pitch_angle, roll_angle, viewpoint_distance=10;
float color[4] = {.7, .2, .5, 1};

static void parse_project_prefs(char* buf) {
    char cs[256];
    COLOR c;
    double hue;
    if (parse_str(buf, "<color_scheme>", cs, 256)) {
        if (!strcmp(cs, "Tahiti Sunset")) {
            hue = .9;
        } else if (!strcmp(cs, "Desert Sands")) {
            hue = .1;
        } else {
            hue = .5;
        }
        HLStoRGB(hue, .5, .5, c);
        color[0] = c.r;
        color[1] = c.g;
        color[2] = c.b;
        color[3] = 1;
    }
}

// set up lighting model
//
static void init_lights() {
   GLfloat ambient[] = {1., 1., 1., 1.0};
   GLfloat position[] = {-13.0, 6.0, 20.0, 1.0};
   GLfloat dir[] = {-1, -.5, -3, 1.0};
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
}

void app_graphics_init() {
    char path[256];
    int viewport[4];

    boinc_get_init_data(uc_aid);
    parse_project_prefs(uc_aid.project_preferences);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    txf_load_fonts(".");
    boinc_resolve_filename("logo.jpg", path, sizeof(path));
    logo.load_image_file(path);
    init_lights();
    get_viewport(viewport);
    app_graphics_resize(viewport[2], viewport[3]);
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

static void draw_3d_stuff() {
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
    drawCylinder(false, pos, 6, 6);
}

void set_viewpoint(double dist) {
    double x, y, z;
    x = 0;
    y = 3.0*dist;
    z = 11.0*dist;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        x, y, z,        // eye position
        0,-.8,0,        // where we're looking
        0.0, 1.0, 0.    // up is in positive Y direction
    );
    glRotated(pitch_angle, 1., 0., 0);
    glRotated(roll_angle, 0., 1., 0);
}

static void app_init_camera(double dist) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
        45.0,       // field of view in degree
        1.0,        // aspect ratio
        1.0,        // Z near clip
        1000.0      // Z far
    );
    set_viewpoint(dist);
}

void app_graphics_render(int xs, int ys, double time_of_day) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mode_unshaded();
    mode_ortho();
    draw_logo();
    ortho_done();

    app_init_camera(viewpoint_distance);

    scale_screen(width, height);
    mode_shaded(color);
    draw_3d_stuff();

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

void boinc_app_mouse_move(int x, int y, bool left, bool middle, bool right) {
    if (left) {
        pitch_angle += (y-mouse_y)*.1;
        roll_angle += (x-mouse_x)*.1;
        mouse_y = y;
        mouse_x = x;
    } else if (right) {
        double d = (y-mouse_y);
        viewpoint_distance *= exp(d/100.);
        mouse_y = y;
        mouse_x = x;
    } else {
        mouse_down = false;
    }
}

void boinc_app_mouse_button(int x, int y, int which, bool is_down) {
    if (is_down) {
        mouse_down = true;
        mouse_x = x;
        mouse_y = y;
    } else {
        mouse_down = false;
    }
}

void boinc_app_key_press(int, int){}

void boinc_app_key_release(int, int){}

