// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// Example graphics application, paired with uc2.C
// This demonstrates:
// - using shared memory to communicate with the worker app
// - reading XML preferences by which users can customize graphics
//   (in this case, select colors)
// - handle mouse input (in this case, to zoom and rotate)
// - draw text and 3D objects using OpenGL
//
// - Expects TrueType font 0 (by default, LiberationSans-Regular.ttf) 
//   to be in the current directory.  
// - Must be linked with api/ttfont.cpp, libfreetype.a and libftgl.a.
//   (libfreetype.a may also require linking with -lz and -lbz2.)
//   See comments at top of api/ttfont.cpp for more information.
//

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <math.h>
#endif

#include "parse.h"
#include "util.h"
#include "gutil.h"
#include "boinc_gl.h"
#include "app_ipc.h"
#include "boinc_api.h"
#include "graphics2.h"
#include "ttfont.h"
#include "uc2.h"
#include "diagnostics.h"

#ifdef __APPLE__
#include "mac/app_icon.h"
#endif

using TTFont::ttf_render_string;
using TTFont::ttf_load_fonts;

float white[4] = {1., 1., 1., 1.};
TEXTURE_DESC logo;
int width, height;      // window dimensions
APP_INIT_DATA uc_aid;
bool mouse_down = false;
int mouse_x, mouse_y;
double pitch_angle, roll_angle, viewpoint_distance=10;
float color[4] = {.7, .2, .5, 1};
    // the color of the 3D object.
    // Can be changed using preferences
UC_SHMEM* shmem = NULL;

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
    if (y < 0 || y > .4) dy *= -1;
    double fd = 0, cpu=0, dt;
    if (shmem) {
        fd = shmem->fraction_done;
        cpu = shmem->cpu_time;
    }
    sprintf(buf, "User: %s", uc_aid.user_name);
    ttf_render_string(x, y, 0, 500, white, buf);
    sprintf(buf, "Team: %s", uc_aid.team_name);
    ttf_render_string(x, y+.1, 0, 500, white, buf);
    sprintf(buf, "%% Done: %f", 100*fd);
    ttf_render_string(x, y+.2, 0, 500, white, buf);
    sprintf(buf, "CPU time: %f", cpu); 
    ttf_render_string(x, y+.3, 0, 500, white, buf);
    if (shmem) {
        dt = dtime() - shmem->update_time;
        if (dt > 10) {
            boinc_close_window_and_quit("shmem not updated");
        } else if (dt > 5) {
            ttf_render_string(0, 0, 0, 500, white, (char*)"App not running - exiting in 5 seconds");
        } else if (shmem->status.suspended) {
            ttf_render_string(0, 0, 0, 500, white, (char*)"App suspended");
        }
    } else {
        ttf_render_string(0, 0, 0, 500, white, (char*)"No shared mem");
    }
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

static void init_camera(double dist) {
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
    // boinc_graphics_get_shmem() must be called after 
    // boinc_parse_init_data_file()
    // Put this in the main loop to allow retries if the 
    // worker application has not yet created shared memory
    //
    if (shmem == NULL) {
        shmem = (UC_SHMEM*)boinc_graphics_get_shmem("uppercase");
    }
    if (shmem) {
        shmem->countdown = 5;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw logo first - it's in background
    //
    mode_unshaded();
    mode_ortho();
    draw_logo();
    ortho_done();

    // draw 3D objects
    //
    init_camera(viewpoint_distance);
    scale_screen(width, height);
    mode_shaded(color);
    draw_3d_stuff();

    // draw text on top
    //
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

// mouse drag w/ left button rotates 3D objects;
// mouse draw w/ right button zooms 3D objects
//
void boinc_app_mouse_move(int x, int y, int left, int middle, int right) {
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

void boinc_app_mouse_button(int x, int y, int which, int is_down) {
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

void app_graphics_init() {
    char path[256];

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    ttf_load_fonts(".");

    boinc_resolve_filename((char*)"logo.jpg", path, sizeof(path));
    logo.load_image_file(path);

    init_lights();
}

static void parse_project_prefs(char* buf) {
    char cs[256];
    COLOR c;
    double hue;
    double max_frames_sec, max_gfx_cpu_pct;
    if (!buf) return;
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
    if (parse_double(buf, "<max_frames_sec>", max_frames_sec)) {
        boinc_max_fps = max_frames_sec;
    }
    if (parse_double(buf, "<max_gfx_cpu_pct>", max_gfx_cpu_pct)) {
        boinc_max_gfx_cpu_frac = max_gfx_cpu_pct/100;
    }
}

int main(int argc, char** argv) {
    boinc_init_graphics_diagnostics(BOINC_DIAG_DEFAULTS);

#ifdef __APPLE__
    setMacIcon(argv[0], MacAppIconData, sizeof(MacAppIconData));
#endif

    boinc_parse_init_data_file();
    boinc_get_init_data(uc_aid);
    if (uc_aid.project_preferences) {
        parse_project_prefs(uc_aid.project_preferences);
    }
    boinc_graphics_loop(argc, argv);

    boinc_finish_diag();
}
