#include "config.h"
#include <stdlib.h>
#include <stdio.h>    
#include <setjmp.h>    
#include <unistd.h> 
#include <pthread.h> 
#include <signal.h>
#include "x_opengl.h"

#include "app_ipc.h"
#include "util.h"
#include "graphics_api.h"

#include "boinc_gl.h"
#include "boinc_glut.h"

#define TIMER_INTERVAL_MSEC 30

static int xpos = 100, ypos = 100;
static int win_width = 600, win_height = 400;
static int clicked_button;
static int win=0;

static APP_INIT_DATA aid;

bool fullscreen;

static void get_window_title(APP_INIT_DATA& aid, char* buf, int len) {
    if (aid.app_version) {
        snprintf(buf, len,
            "%s version %.2f [workunit: %s]",
            aid.app_name, aid.app_version/100.0, aid.wu_name
        );
    } else {
        snprintf(buf, len,
            "%s [workunit: %s]",
            aid.app_name, aid.wu_name
        );
    }
}

static void close_window() {
    exit(0);
}

// This callback is invoked when a user presses a key.
//
void keyboardD(unsigned char key, int /*x*/, int /*y*/) {
    if (fullscreen) {
        close_window();
    } else {
        boinc_app_key_press((int) key, 0);
    }
}

void keyboardU(unsigned char key, int /*x*/, int /*y*/) {
    if (fullscreen) {
        close_window();
    } else {
        boinc_app_key_release((int) key, 0);
    }
}

void mouse_click(int button, int state, int x, int y){
    clicked_button = button;
    if (fullscreen) {
        close_window();
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
        close_window();
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
    int width, height;
    width = glutGet(GLUT_WINDOW_WIDTH);
    height = glutGet(GLUT_WINDOW_HEIGHT);
    if (throttled_app_render(width, height, dtime())) {
        glutSwapBuffers();
#ifdef __APPLE__
        MacGLUTFix(fullscreen);
        if (need_show) {
            glutShowWindow();
            need_show = false;
        }
#endif
    }
}

static void make_window() {
    char window_title[256];
    get_window_title(aid, window_title, 256);

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
    glutWMCloseFunc(CloseWindow);   // Enable the window's close box
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

static void boinc_glut_init() {
    const char* args[2] = {"screensaver", NULL};
    int one=1;
    
    win = 0;
    glutInit (&one, (char**)args);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
    glutInitWindowPosition(xpos, ypos);
    glutInitWindowSize(600, 400); 
}

static void timer_handler(int) {
    maybe_render();
    glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);
}

void boinc_graphics(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--fullscreen")) {
            fullscreen = true;
        }
    }
    boinc_get_init_data(aid);
    if (!strlen(aid.app_name))  {
        strcpy(aid.app_name, "BOINC Application");
    }
    boinc_glut_init();
    make_window();
    glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);      
    glutMainLoop();
}
