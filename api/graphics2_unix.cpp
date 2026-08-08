// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// unix-specific graphics stuff
//
#include "config.h"
#include <cstdlib>
#include <cstdio>
#include <csetjmp>
#include <unistd.h>
#include <pthread.h>
#include <csignal>
#include <cstring>
#include "x_opengl.h"

#include "app_ipc.h"
#include "filesys.h"
#include "str_replace.h"
#include "util.h"

#include "boinc_gl.h"
#include "boinc_glut.h"
#include "boinc_api.h"
#include "graphics2.h"
#include "diagnostics.h"

#define TIMER_INTERVAL_MSEC 30

static int xpos = 100, ypos = 100, width = 600, height = 400;
static int clicked_button;
static int win=0;
static int checkparentcounter=0;

#ifdef __APPLE__
static bool need_show = false;

// Set to true to draw both directly to window and also to offscreen buffer
bool debugSharedOffscreenBuffer = false;
#endif

bool fullscreen;

void boinc_close_window_and_quit(const char* p) {
    char buf[256];
    fprintf(stderr, "%s Quitting: %s\n", boinc_msg_prefix(buf, sizeof(buf)), p);
    exit(0);
}

void boinc_close_window_and_quit_aux() {
    boinc_close_window_and_quit("GLUT close");
}

// This callback is invoked when a user presses a key.
//
void keyboardD(unsigned char key, int /*x*/, int /*y*/) {
    if (fullscreen) {
        boinc_close_window_and_quit("key down");
    } else {
        boinc_app_key_press((int) key, 0);
    }
}

void keyboardU(unsigned char key, int /*x*/, int /*y*/) {
    if (fullscreen) {
        boinc_close_window_and_quit("key up");
    } else {
        boinc_app_key_release((int) key, 0);
    }
}

void mouse_click(int button, int state, int x, int y){
    clicked_button = button;
    if (fullscreen) {
        boinc_close_window_and_quit("mouse click");
    } else {
        if (state) {
            boinc_app_mouse_button(x, y, button, false);
        } else {
            boinc_app_mouse_button(x, y, button, true);
        }
    }
}

void mouse_click_move(int x, int y){
    if (fullscreen) {
        boinc_close_window_and_quit("mouse move");
    } else if (clicked_button == 2){
        boinc_app_mouse_move(x, y, false, false, true);
    } else if (clicked_button == 1){
        boinc_app_mouse_move(x, y, false, true, false);
    } else if (clicked_button == 0){
        boinc_app_mouse_move(x, y, true, false, false);
    } else{
        boinc_app_mouse_move(x, y, false, false, false);
    }
}

static void maybe_render() {
    int new_xpos, new_ypos, new_width, new_height;
    static int size_changed = 0;

    new_xpos = glutGet(GLUT_WINDOW_X);
    new_ypos = glutGet(GLUT_WINDOW_Y);
    new_width = glutGet(GLUT_WINDOW_WIDTH);
    new_height = glutGet(GLUT_WINDOW_HEIGHT);


    if (throttled_app_render(new_width, new_height, dtime())) {
#ifdef __APPLE__
        if (UseSharedOffscreenBuffer() && !debugSharedOffscreenBuffer) {
            return; // Don't waste cycles drawing to hidden window on screen
        }
#endif
        glutSwapBuffers();
        if (! fullscreen) {
            // If user has changed window size, wait until it stops
            // changing and then write the new dimensions to file
            if ((new_xpos != xpos) || (new_ypos != ypos) ||
                (new_width != width) || (new_height != height)
                ) {
                    size_changed = 1;
                    xpos = new_xpos;
                    ypos = new_ypos;
                    width = new_width;
                    height = new_height;

                } else {
                    if (size_changed && (++size_changed > 10)) {
                        size_changed = 0;
#ifdef __APPLE__
                        ClearDocumentEditedDot();
#endif
                        FILE *f = boinc_fopen("gfx_info", "w");
                        if (f) {
                            // ToDo: change this to XML
                            fprintf(f, "%d %d %d %d\n", xpos, ypos, width, height);
                            fclose(f);
                        }
                    }
                }               // End if (new size != previous size) else
            }                   // End if (! fullscreen)
#ifdef __APPLE__
        MacGLUTFix(fullscreen);
        if (need_show) {
            glutShowWindow();
            need_show = false;
        }
#endif
    }
}

static void make_window(const char* title) {
    char window_title[256];
    if (title) {
        strlcpy(window_title, title, sizeof(window_title));
    } else {
        get_window_title(window_title, 256);
    }

    win = glutCreateWindow(window_title);
    glutReshapeFunc(app_graphics_resize);
    glutKeyboardFunc(keyboardD);
    glutKeyboardUpFunc(keyboardU);
    glutMouseFunc(mouse_click);
    glutMotionFunc(mouse_click_move);
    glutDisplayFunc(maybe_render);
    glEnable(GL_DEPTH_TEST);

    app_graphics_init();

#ifdef __APPLE__
    glutWMCloseFunc(boinc_close_window_and_quit_aux);   // Enable the window's close box
    BringAppToFront();
    // Show window only after a successful call to throttled_app_render();
    // this avoids momentary display of old image when screensaver restarts
    // which made image appear to "jump."
    need_show = true;
#endif

    if (fullscreen)  {
        glutFullScreen();
    }
}

static void boinc_glut_init(int *argc, char** argv) {
    win = 0;

    FILE *f = boinc_fopen("gfx_info", "r");
    if (f) {
        // ToDo: change this to XML parsing
        int n = fscanf(f, "%d %d %d %d\n", &xpos, &ypos, &width, &height);
        fclose(f);
        if (n != 4) {
            fprintf(stderr, "failed to parse gfx_info");
        }
    }

    glutInit (argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA);
    glutInitWindowPosition(xpos, ypos);
    glutInitWindowSize(width, height);
}

static void timer_handler(int) {
    maybe_render();
    // When running under a V5 client, the worker app launches the graphics app
    // so this code kills the graphics when the worker application exits.
    // Under a V6 client, the Manager or Screensaver launched the graphics app
    // so this code kills the graphics when the Manager or Screensaver exits.
    if (--checkparentcounter < 0) {
        // Check approximately twice per second if parent process still running
        checkparentcounter = 500 / TIMER_INTERVAL_MSEC;
        if (getppid() == 1) {
            // Quit graphics application if parent process no longer running
            boinc_close_window_and_quit("parent dead");
        }
    }
    glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);
}

void boinc_graphics_loop(int argc, char** argv, const char* title) {
    if (!diagnostics_is_initialized()) {
        boinc_init_graphics_diagnostics(BOINC_DIAG_DEFAULTS);
    }

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--fullscreen")) {
            fullscreen = true;
        }
    }
#ifdef __APPLE__
    char dir [MAXPATHLEN];
    getcwd(dir, MAXPATHLEN);

    if (fullscreen) {
        pass_BOINC_gfx_lib_version_to_ss();
    }
#endif
    boinc_glut_init(&argc, argv);
    make_window(title);
    glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);
#ifdef __APPLE__
    // Apparently glut changed our working directory in OS 10.3.9
    chdir(dir);
#endif
    glutMainLoop();
}

#ifdef __APPLE__

bool UseSharedOffscreenBuffer() {
    static bool alreadyTested = false;
    static bool needSharedGfxBuffer = false;

//return true;    // FOR TESTING ONLY
    if (alreadyTested) {
        return needSharedGfxBuffer;
    }
    alreadyTested = true;
    if (fullscreen) {
        SInt32 major = -1;
        SInt32 minor = -1;
        char vers[100], *p1 = NULL;
        FILE *f;
        vers[0] = '\0';
        f = popen("sw_vers -productVersion", "r");
        if (f) {
            fscanf(f, "%s", vers);
            pclose(f);
        }
        if (vers[0] == '\0') {
            fprintf(stderr, "popen(\"sw_vers -productVersion\" failed\n");
            fflush(stderr);
            return false;
        }
        // Extract the major system version number
        major = atoi(vers);
        if (major > 10) {   // OS 11.0 or later
            needSharedGfxBuffer = true;
            return true;
        }
        // Extract the minor system version number
        p1 = strchr(vers, '.');
        minor = atoi(p1+1);
        if (minor > 12) {   // OS 10.13 or later
            needSharedGfxBuffer = true;
            return true;
        }
    }
    return false;
}

#include "shmem.h"

// struct ss_shmem_data must be kept in sync in these files:
// screensaver.cpp
// gfx_switcher.cpp
// gfx_cleanup.mm
// graphics2_unix.cpp
struct ss_shmem_data {
    pid_t gfx_pid;
    int gfx_slot;
    int major_version;
    int minor_version;
    int release;
};

void pass_BOINC_gfx_lib_version_to_ss() {
    struct ss_shmem_data* ss_shmem = NULL;
    char userName[64];
    char shmem_name[MAXPATHLEN];

    strlcpy(userName, getenv("USER"), sizeof(userName));
    snprintf(shmem_name, sizeof(shmem_name), "/tmp/boinc_ss_%s", userName);
    attach_shmem_mmap(shmem_name, (void**)&ss_shmem);

    if (ss_shmem) {
        ss_shmem->major_version = BOINC_MAJOR_VERSION;
        ss_shmem->minor_version = BOINC_MINOR_VERSION;
        ss_shmem->release = BOINC_RELEASE;
    }
}

#endif
