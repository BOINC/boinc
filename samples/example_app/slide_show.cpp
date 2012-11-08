//
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
// Copyright (C) 2012 David Coss, PhD
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

// Example graphics application that simply rotates images, like 
// a slide show. Images must be located in the subdirectory
// "slide_show_images". However, they may be shipped as a zip file,
// "slide_show_images.zip", which will be extracted if the images
// directory does not exist.

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
#include "uc2.h"
#include "diagnostics.h"
#include "filesys.h"
#include "boinc_zip.h"

#ifdef __APPLE__
#include "mac/app_icon.h"
#endif

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

using std::string;
using std::vector;
std::vector<std::string> image_paths;
size_t curr_image_idx;
int timer_counter = 0;
std::string images_dir = "slide_show_images";
std::string zip_filename = "slide_show_images.zip";

// Image path functions
int populate_image_list() {
    DIRREF dir;
    char path[MAXPATHLEN];
    std::string dirent_name,physical_path;

    boinc_resolve_filename(images_dir.c_str(),path,MAXPATHLEN);
    if (!boinc_file_exists(path)) {
        printf("missing directory '%s'\n",path);
        boinc_resolve_filename(zip_filename.c_str(),path,MAXPATHLEN);
        if (boinc_file_exists(path)) {
            char new_dir_path[MAXPATHLEN];
            boinc_resolve_filename(images_dir.c_str(),new_dir_path,MAXPATHLEN);
            printf("zipfile exists '%s'\n",path);
            boinc_zip(UNZIP_IT,path, ".");
        }
    }

    if ((dir = dir_open(images_dir.c_str())) == NULL) {
        fprintf(stderr,
            "Could not open images directory '%s'.\n",
            images_dir.c_str()
        );
        if(errno) fprintf(stderr,"Reason (%d): %s",errno,strerror(errno));
        return -1;
    }
  
    while (dir_scan(path,dir,MAXPATHLEN) == 0) {
        dirent_name = path;
#ifdef _WIN32
        dirent_name = images_dir + "\\" + dirent_name;
#else
        dirent_name = images_dir + "/" + dirent_name;
#endif
        boinc_resolve_filename_s(dirent_name.c_str(), physical_path);
        printf("%s  =>  %s\n", dirent_name.c_str(), physical_path.c_str());
        image_paths.push_back(physical_path);
    }
    dir_close(dir);

    curr_image_idx = 0;
}

void load_next_image() {
    // If there are no images, leave immediately.
    //
    if (image_paths.size() == 0) return;
  
    // If we have moved past the end of the path list,
    // reset counter.
    if (curr_image_idx >= image_paths.size()) {
        curr_image_idx = 0;
    }
    logo.load_image_file(image_paths[curr_image_idx++].c_str());
}

void timer_image_rotate() {
    timer_counter++;
    if(timer_counter >= 100) {
        load_next_image();
        timer_counter = 0;
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

static void draw_logo() {
    if (logo.present) {
        float pos[3] = {.2, .3, 0};
        float size[3] = {.6, .4, 0};
        logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER);
    }
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

    timer_image_rotate();

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

}

void boinc_app_mouse_button(int x, int y, int which, int is_down) {
}

void boinc_app_key_press(int, int){}

void boinc_app_key_release(int, int){}

void app_graphics_init() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    populate_image_list();
    load_next_image();

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

    boinc_register_timer_callback(timer_image_rotate);

    boinc_graphics_loop(argc, argv);

    boinc_finish_diag();
}
