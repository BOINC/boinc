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

// read "in", convert to UC, write to "out"
// command line options:
// -run_slow: sleep 1 second after each character, useful for debugging
// -cpu_time N: use about N CPU seconds
// -signal:   raise SIGHUP signal (for testing signal handler)
// -exit:     exit with status -10 (for testing exit handler)
//

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#ifndef _WIN32
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#endif

#define BOINC_APP_GRAPHICS

#ifdef BOINC_APP_GRAPHICS
#include "gutil.h"
#include "boinc_gl.h"
#include "graphics_api.h"
#include "txf_util.h"
#endif

#include "diagnostics.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

using std::string;

#define CHECKPOINT_FILE "upper_case_state"
#define INPUT_FILENAME "in"
#define OUTPUT_FILENAME "out"

bool run_slow;
bool raise_signal;
bool random_exit;
double cpu_time=20;
APP_INIT_DATA uc_aid;

int do_checkpoint(MFILE& mf, int nchars) {
    int retval;
    string resolved_name;

    FILE* f = fopen("temp", "w");
    if (!f) return 1;
    fprintf(f, "%d", nchars);
    fclose(f);

    fprintf(stderr, "APP: upper_case checkpointing\n");

    retval = mf.flush();
    if (retval) return retval;
    boinc_resolve_filename_s(CHECKPOINT_FILE, resolved_name);
    retval = boinc_rename("temp", resolved_name.c_str());
    if (retval) return retval;

    return 0;
}

static void use_some_cpu() {
    double j = 3.14159;
    int i, n = 0;
    for (i=0; i<20000000; i++) {
        n++;
        j *= n+j-3.14159;
        j /= (float)n;
    }
}

#ifdef _WIN32

extern int main(int argc, char** argv);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif

void worker() {
    int c, nchars = 0, retval;
    double fsize;
    char resolved_name[512];
    MFILE out;
    FILE* state, *in;

    boinc_get_init_data(uc_aid);

    boinc_resolve_filename(INPUT_FILENAME, resolved_name, sizeof(resolved_name));
    in = boinc_fopen(resolved_name, "r");
    if (in == NULL) {
        fprintf(
            stderr,
            "Couldn't find input file, resolved name %s.\n",
            resolved_name
        );
        exit(-1);
    }

    file_size(resolved_name, fsize);

    boinc_resolve_filename(CHECKPOINT_FILE, resolved_name, sizeof(resolved_name));
    state = boinc_fopen(resolved_name, "r");
    if (state) {
        fscanf(state, "%d", &nchars);
        fseek(in, nchars, SEEK_SET);
        boinc_resolve_filename(OUTPUT_FILENAME, resolved_name, sizeof(resolved_name));
        retval = out.open(resolved_name, "a");
    } else {
        boinc_resolve_filename(OUTPUT_FILENAME, resolved_name, sizeof(resolved_name));
        retval = out.open(resolved_name, "w");
    }
    if (retval) {
        fprintf(stderr, "APP: upper_case output open failed:\n");
        fprintf(stderr, "resolved name %s, retval %d\n", resolved_name, retval);
        perror("open");
        exit(1);
    }
    while (1) {
        c = fgetc(in);

        if (c == EOF) break;
        c = toupper(c);
        out._putchar(c);
        nchars++;

        if (run_slow) {
            boinc_sleep(1.);
        }

#ifdef HAVE_SIGNAL_H
        if (raise_signal) {
            raise(SIGHUP);
        }
#endif
        if (random_exit) {
            if (drand() < 0.05) {
                exit(-10);
            }
        }

        int flag = boinc_time_to_checkpoint();
        if (flag) {
            retval = do_checkpoint(out, nchars);
            if (retval) {
                fprintf(stderr, "APP: upper_case checkpoint failed %d\n", retval);
                exit(1);
            }
            boinc_checkpoint_completed();
        }

        boinc_fraction_done(nchars/fsize);
    }
    retval = out.flush();
    if (retval) {
        fprintf(stderr, "APP: upper_case flush failed %d\n", retval);
        exit(1);
    }
    if (cpu_time) {
        double start = dtime();
        while (1) {
            double e = dtime()-start;
            if (e > cpu_time) break;
            boinc_fraction_done(e/cpu_time);
            use_some_cpu();
        }
    }

    if (random_exit) exit(-10);
    fprintf(stderr, "APP: upper_case ending, wrote %d chars\n", nchars);

    boinc_finish(0);
}

int main(int argc, char **argv) {
    int i, retval;

    boinc_init_diagnostics(
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_REDIRECTSTDERR
    );

    // NOTE: if you change output here, remember to change the output that
    // test_uc.py pattern-matches against.

    for (i=0; i<argc; i++) {
        fprintf(stderr, "APP: upper_case: argv[%d] is %s\n", i, argv[i]);
        if (!strcmp(argv[i], "-run_slow")) run_slow = true;
        if (!strcmp(argv[i], "-cpu_time")) {
            cpu_time = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "-signal")) raise_signal = true;
        if (!strcmp(argv[i], "-exit")) random_exit = true;
    }

    fprintf(stderr, "APP: upper_case: starting, argc %d\n", argc);

#ifdef BOINC_APP_GRAPHICS
    retval = boinc_init_graphics(worker);
    if (retval) exit(retval);
#else
    retval = boinc_init();
    if (retval) exit(retval);
    worker();
#endif
}

#ifdef BOINC_APP_GRAPHICS

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

#endif

const char *BOINC_RCSID_33ac47a071 = "$Id$";
