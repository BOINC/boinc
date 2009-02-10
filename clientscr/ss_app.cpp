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

// Example graphics application, paired with uc2.C
// This demonstrates:
// - using shared memory to communicate with the worker app
// - reading XML preferences by which users can customize graphics
//   (in this case, select colors)
// - handle mouse input (in this case, to zoom and rotate)
// - draw text and 3D objects using OpenGL

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <math.h>
#endif
#include <string>
#include <vector>
#ifdef __APPLE__
#include "mac_app_icon.h"
#include "boinc_api.h"
#include <sys/socket.h>
#endif

#include "diagnostics.h"
#include "gutil.h"
#include "boinc_gl.h"
#include "graphics2.h"
#include "txf_util.h"
#include "network.h"
#include "gui_rpc_client.h"
#include "app_ipc.h"

using std::string;
using std::vector;

float white[4] = {1., 1., 1., 1.};
TEXTURE_DESC logo;
int width, height;      // window dimensions
bool mouse_down = false;
int mouse_x, mouse_y;
double pitch_angle, roll_angle, viewpoint_distance=10;
float color[4] = {.7, .2, .5, 1};
    // the color of the 3D object.
    // Can be changed using preferences

RPC_CLIENT rpc;
CC_STATE cc_state;

struct APP_SLIDES {
    string name;
    int index;
    double switch_time;
    vector<TEXTURE_DESC> slides;
    APP_SLIDES(string n): name(n), index(0), switch_time(0) {}
};

struct PROJECT_IMAGES {
    string url;
    TEXTURE_DESC icon;
    vector<APP_SLIDES> app_slides;
};

vector<PROJECT_IMAGES> project_images;

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

static void draw_logo(float alpha) {
    if (logo.present) {
        float pos[3] = {.2, .3, 0};
        float size[3] = {.6, .4, 0};
        logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER, alpha);
    }
}

void icon_path(PROJECT* p, char* buf) {
    char dir[256];
    url_to_project_dir((char*)p->master_url.c_str(), dir);
    sprintf(buf, "%s/stat_icon", dir);
}

void slideshow(PROJECT* p) {
    char dir[256], buf[256];
    int i;

    url_to_project_dir((char*)p->master_url.c_str(), dir);
    for (i=0; i<99; i++) {
        sprintf(buf, "%s/slideshow_%02d", dir, i);
    }
}

PROJECT_IMAGES* get_project_images(PROJECT* p) {
    unsigned int i;
    char dir[256], path[256], filename[256];

    for (i=0; i<project_images.size(); i++) {
        PROJECT_IMAGES& pi = project_images[i];
        if (pi.url == p->master_url) return &pi;
    }
    PROJECT_IMAGES pim;
    pim.url = p->master_url;
    url_to_project_dir((char*)p->master_url.c_str(), dir);
    sprintf(path, "%s/stat_icon", dir);
    boinc_resolve_filename(path, filename, 256);
    pim.icon.load_image_file(filename);
    for (i=0; i<cc_state.apps.size(); i++) {
        APP& app = *cc_state.apps[i];
        if (app.project != p) continue;
        APP_SLIDES as(app.name);
        for (int j=0; j<99; j++) {
            sprintf(path, "%s/slideshow_%s_%02d", dir, app.name.c_str(), j);
            boinc_resolve_filename(path, filename, 256);
            TEXTURE_DESC td;
            int retval = td.load_image_file(filename);
            if (retval) break;
            as.slides.push_back(td);
        }
        pim.app_slides.push_back(as);
    }
    project_images.push_back(pim);
    return &(project_images.back());
}

void show_result(RESULT* r, float x, float& y, float alpha) {
    PROGRESS progress;
    char buf[256];
    float prog_pos[] = {x, y, 0};
    float prog_c[] = {.5, .4, .1, alpha/2};
    float prog_ci[] = {.1, .8, .2, alpha};
    txf_render_string(.1, x, y, 0, 8., white, 0, (char*)r->app->user_friendly_name.c_str());
    y -= 3;
    progress.init(prog_pos, 10., 1., 0.8, prog_c, prog_ci);
    progress.draw(r->fraction_done);
    mode_unshaded();
    sprintf(buf, "%.2f%%", r->fraction_done*100);
    txf_render_string(.1, x+15, y, 0, 8., white, 0, buf);
    y -= 3;
}

void show_coords() {
    int i;
    char buf[256];
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float x = (float)i;
        txf_render_string(.1, x, 0, 0, 10., white, 0, buf);
    }
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float y = (float)i;
        txf_render_string(.1, 0, y, 0, 10., white, 0, buf);
    }
}

void show_project(PROJECT* p, float x, float& y, float alpha) {
    unsigned int i;
    PROJECT_IMAGES* pim = get_project_images(p);
    txf_render_string(.1, x, y, 0, 5., white, 0, (char*)p->project_name.c_str());
    if (pim->icon.present) {
        float pos[3] = {x, y, 1};
        float size[2] = {3., 3.};
        pim->icon.draw(pos, size, 0, 0);
    }
    y -= 3;
    for (i=0; i<cc_state.results.size(); i++) {
        RESULT* r = cc_state.results[i];
        if (r->project != p) continue;
        if (!r->active_task) continue;
        if (r->active_task_state != PROCESS_EXECUTING) continue;
        show_result(r, x, y, alpha);
    }
}

void show_projects(float alpha) {
    float x=-45, y=30;
    unsigned int i;
    for (i=0; i<cc_state.projects.size(); i++) {
        PROJECT* p = cc_state.projects[i];
        show_project(p, x, y, alpha);
        y -= 2;
    }
}

int update_data(double t) {
    static double state_time = 0;
    if (state_time < t) {
        int retval = rpc.get_state(cc_state);
        if (retval) return retval;
        state_time = t;
    }
    return 0;
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

void app_graphics_render(int xs, int ys, double t) {
    int retval = update_data(t);
    if (retval) {
        boinc_close_window_and_quit("RPC failed");
    }

    static float alpha=1;
    static float dalpha = -.01;

    white[3] = alpha;
    alpha += dalpha;
    if (alpha < 0) {
        alpha = 0;
        dalpha = -dalpha;
    }
    if (alpha > 1) {
        alpha = 1;
        dalpha = -dalpha;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw logo first - it's in background
    //
    mode_unshaded();
    mode_ortho();
    draw_logo(alpha);
    ortho_done();

    // draw 3D objects
    //
    init_camera(viewpoint_distance);
    scale_screen(width, height);
    //mode_shaded(color);

    // draw text on top
    //
    //mode_unshaded();
    //mode_ortho();
    show_projects(alpha);
    show_coords();
    //ortho_done();


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
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    txf_load_fonts(".");
    logo.load_image_file("boinc_logo_black.jpg");
    init_lights();
}

int main(int argc, char** argv) {
    int retval;

#ifdef _WIN32
    WinsockInitialize();
#endif
    retval = rpc.init("localhost");
    if (retval) exit(retval);

#ifdef __APPLE__
    setMacIcon(argv[0], MacAppIconData, sizeof(MacAppIconData));
#endif
    boinc_graphics_loop(argc, argv);
    boinc_finish_diag();
#ifdef _WIN32
    WinsockCleanup();
#endif
}
