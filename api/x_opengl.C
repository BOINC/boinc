#include <stdlib.h>
#include <stdio.h>    
#include <unistd.h> 
#include "x_opengl.h"

#include "boinc_gl.h"
#include "boinc_api.h"
#include "graphics_api.h"
#include "app_ipc.h"
#include "util.h"

#define BOINC_WINDOW_CLASS_NAME "BOINC_app"

#define TIMER_INTERVAL_MSEC 30

static bool visible = true;
static int current_graphics_mode = MODE_HIDE_GRAPHICS;
static int acked_graphics_mode;
static int xpos = 100, ypos = 100;
static int clicked_button;
static int win=0;
extern int userclose;
extern void graphics_thread_init();
//extern GRAPHICS_INFO gi;
static void set_mode(int mode);

void keyboardU(unsigned char key, int x, int y) {
    // This callback is invoked when a user presses a key.
    //
    if (current_graphics_mode == MODE_FULLSCREEN) {
        set_mode(MODE_HIDE_GRAPHICS);
    }
}

void mouse_click(int button, int state, int x, int y){
    clicked_button = button;
    if (current_graphics_mode == MODE_FULLSCREEN) {
        set_mode(MODE_HIDE_GRAPHICS);
    } else {
        if (state) {
            boinc_app_mouse_button(x, y, button, false);
        } else {
            boinc_app_mouse_button(x, y, button, true);
        }
    }
}

void mouse_click_move(int x, int y){
    if (current_graphics_mode == MODE_FULLSCREEN){
        set_mode(MODE_HIDE_GRAPHICS);
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

void keyboardD(unsigned char key, int x, int y) {
    // This callback is invoked when a user presses a key.
    //
    if (current_graphics_mode == MODE_FULLSCREEN) {
        set_mode(MODE_HIDE_GRAPHICS);
    }
}

#if 0
void onIdle(){
    static double oldTime = 0;
    double currentTime = dtime();
    
    if (userclose == 1){
        userclose = 0;
        set_mode(MODE_HIDE_GRAPHICS);
    }
    if (currentTime - oldTime > .001){
        timer_handler();
        oldTime = currentTime;
    }
}
#endif

static void maybe_render() {
    int width, height;
    if (visible && (current_graphics_mode != MODE_HIDE_GRAPHICS)) {
        width = glutGet(GLUT_WINDOW_WIDTH);
        height = glutGet(GLUT_WINDOW_HEIGHT);
        if (throttled_app_render(width, height, dtime())) {
            glutSwapBuffers();
        }
    }
}

static void close_func() {
    if (boinc_is_standalone()) {
        exit(0);
    } else {
        set_mode(MODE_HIDE_GRAPHICS);
    }
}

static void make_new_window(int mode){
    if (mode == MODE_WINDOW || mode == MODE_FULLSCREEN){
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
        glutInitWindowPosition(xpos, ypos);
        glutInitWindowSize(600, 400); 
        boinc_get_init_data(aid);
        if (!strlen(aid.app_name)) {
            strcpy(aid.app_name, "BOINC Application");
        }
        win = glutCreateWindow(aid.app_name); 

        glutReshapeFunc(app_graphics_resize);
        glutKeyboardFunc(keyboardD);
        glutKeyboardUpFunc(keyboardU);
        glutMouseFunc(mouse_click);
        glutMotionFunc(mouse_click_move);
        glutDisplayFunc(maybe_render); 
        
        app_graphics_init();
        glEnable(GL_DEPTH_TEST);    
        
        if (mode == MODE_FULLSCREEN) {
            glutFullScreen();
        }
    }
}

void KillWindow() {
    if (win) {
        glutDestroyWindow(win);
        win = 0;
    }
}

void set_mode(int mode) {
    if (mode == current_graphics_mode) return;
    KillWindow();
    current_graphics_mode = mode;
    if (mode != MODE_HIDE_GRAPHICS) {
        make_new_window(mode);
    }
}

static void wait_for_initial_message() {
    app_client_shm->shm->graphics_reply.send_msg(
        xml_graphics_modes[MODE_HIDE_GRAPHICS]
    );
    acked_graphics_mode = MODE_HIDE_GRAPHICS;
    while (1) {
        if (app_client_shm->shm->graphics_request.has_msg()) {
            break;
        }
        sleep(1);
    }
}

static void timer_handler(int) {
    char buf[MSG_CHANNEL_SIZE];

    if (userclose) {
        close_func();
        userclose = false;
    }
    int new_mode;
    if (app_client_shm) {
        if (app_client_shm->shm->graphics_request.get_msg(buf)) {
            new_mode = app_client_shm->decode_graphics_msg(buf);
            switch (new_mode) {
            case MODE_REREAD_PREFS:
                //only reread graphics prefs if we have a window open
                //
                switch (current_graphics_mode){
                case MODE_WINDOW:
                case MODE_FULLSCREEN:
                    app_graphics_reread_prefs();
                    break;
                }
                break;
            case MODE_HIDE_GRAPHICS:
            case MODE_WINDOW:
            case MODE_FULLSCREEN:
            case MODE_BLANKSCREEN:
                set_mode(new_mode);
                break;
            }
        }
        if (acked_graphics_mode != current_graphics_mode) {
            bool sent = app_client_shm->shm->graphics_reply.send_msg(
                xml_graphics_modes[current_graphics_mode]
            );
            if (sent) acked_graphics_mode = current_graphics_mode;
        }
    }
    maybe_render();
    glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);
}

void xwin_graphics_event_loop(){
    if (boinc_is_standalone()) {
        set_mode(MODE_WINDOW);
    } else {
        wait_for_initial_message();
        timer_handler(0);
    }
    glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);
    glutMainLoop();
}

