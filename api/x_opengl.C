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
#include "filesys.h"

#include "boinc_gl.h"
#include "boinc_glut.h"
#include "graphics_api.h"
#include "graphics_impl.h"

#define BOINC_WINDOW_CLASS_NAME "BOINC_app"

#define TIMER_INTERVAL_MSEC 30

static bool visible = true;
static int current_graphics_mode = MODE_HIDE_GRAPHICS;
static int acked_graphics_mode;
static int xpos = 100, ypos = 100;
static int clicked_button;
static int win=0;
static bool glut_is_initialized = false;
static jmp_buf jbuf;	// longjump/setjump for exit/signal handler
static struct sigaction original_signal_handler; // store previous ABRT signal-handler
static void set_mode(int mode);
static void restart(void);
static void boinc_glut_init(void);

static APP_INIT_DATA aid;

extern pthread_t graphics_thread;	// thread info
extern pthread_t worker_thread;

// possible longjmp-values to signal from where we jumped:
//
enum { 
  JUMP_NONE = 0,
  JUMP_EXIT,
  JUMP_ABORT,
  JUMP_LAST
};
// 1= exit caught by atexit, 2 = signal caught by handler
       
int xwin_glut_is_initialized() {
    return glut_is_initialized;
}

void app_debug_msg (const char *fmt, ...);

// This callback is invoked when a user presses a key.
//
void keyboardD(unsigned char key, int /*x*/, int /*y*/) {
    if (current_graphics_mode == MODE_FULLSCREEN) {
        set_mode(MODE_HIDE_GRAPHICS);
    } else {
        boinc_app_key_press((int) key, 0);
    }
}

void keyboardU(unsigned char key, int /*x*/, int /*y*/) {
    if (current_graphics_mode == MODE_FULLSCREEN) {
        set_mode(MODE_HIDE_GRAPHICS);
    } else {
        boinc_app_key_release((int) key, 0);
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

static void maybe_render() {
    int width, height;
    if (visible && (current_graphics_mode != MODE_HIDE_GRAPHICS)) {
        width = glutGet(GLUT_WINDOW_WIDTH);
        height = glutGet(GLUT_WINDOW_HEIGHT);
        if (throttled_app_render(width, height, dtime())) {
            glutSwapBuffers();
#ifdef __APPLE__
            switch (current_graphics_mode) {
            case MODE_WINDOW:
                MacGLUTFix(false);
                break;
            case MODE_FULLSCREEN:
            case MODE_BLANKSCREEN:
                MacGLUTFix(true);
                break;
            }
#endif
        }
    }
}

void CloseWindow() {
  set_mode(MODE_HIDE_GRAPHICS);
}

static void make_new_window(int mode) {
    if ( (mode != MODE_WINDOW) &&  (mode != MODE_FULLSCREEN) ) {
        // nothing to be done here
        return;
    }

    if (!glut_is_initialized)  {
        boinc_glut_init();
    }

    app_debug_msg("make_new_window(): now calling glutCreateWindow(%s)...\n", aid.app_name);
    char window_title[256];
    get_window_title(aid, window_title, 256);
    win = glutCreateWindow(window_title); 
    app_debug_msg("glutCreateWindow() succeeded. win = %d\n", win);

    // installing callbacks for the current window
    glutReshapeFunc(app_graphics_resize);
    glutKeyboardFunc(keyboardD);
    glutKeyboardUpFunc(keyboardU);
    glutMouseFunc(mouse_click);
    glutMotionFunc(mouse_click_move);
    glutDisplayFunc(maybe_render); 
    glEnable(GL_DEPTH_TEST);    
#ifdef __APPLE__
    glutWMCloseFunc(CloseWindow);   // Enable the window's close box
    BringAppToFront();
#endif
    app_graphics_init();

    if (mode == MODE_FULLSCREEN)  {
        glutFullScreen();
    }

    return;
}

// initialized glut and screensaver-graphics
// this should only called once, even if restarted by user-exit
//
static void boinc_glut_init() {
    const char* args[2] = {"screensaver", NULL};
    int one=1;
    app_debug_msg("Calling glutInit()... \n");
    glutInit (&one, (char**)args);
    app_debug_msg("...survived glutInit(). \n");
     
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
    glutInitWindowPosition(xpos, ypos);
    glutInitWindowSize(600, 400); 
    g_bmsp->boinc_get_init_data_hook(aid);
    if (!strlen(aid.app_name))  {
        strcpy(aid.app_name, "BOINC Application");
    }

    glut_is_initialized = true;

    return;

}

// Destroy current window if any
//
void KillWindow() {
    if (win) {
      int oldwin = win;
      win = 0;	// set this to 0 FIRST to avoid recursion if the following call fails.
      glutDestroyWindow(oldwin);
    }
}

void set_mode(int mode) {
    if (mode == current_graphics_mode) {
#ifdef __APPLE__
        // Bring graphics window to front whenever user presses "Show graphics"
        if (mode == MODE_WINDOW)
            BringAppToFront();
#endif
        return;
    }

    app_debug_msg("set_mode(%d): current_mode = %d.\n", mode, current_graphics_mode);
    if (glut_is_initialized) {
        app_debug_msg("Calling KillWindow(): win = %d\n", win);
        KillWindow();
        app_debug_msg("...KillWindow() survived.\n");
    }
    
    if (mode != MODE_HIDE_GRAPHICS) {
        app_debug_msg("set_mode(): Calling make_new_window(%d)\n", mode);
        make_new_window(mode);
        app_debug_msg("...make_new_window() survived.\n");
    }
#ifdef __APPLE__
    else
        HideThisApp();
#endif

    current_graphics_mode = mode;

    return;
}

static void wait_for_initial_message() {
    (*g_bmsp->app_client_shmp)->shm->graphics_reply.send_msg(
        xml_graphics_modes[MODE_HIDE_GRAPHICS]
    );
    acked_graphics_mode = MODE_HIDE_GRAPHICS;
    while (1) {
        if ((*g_bmsp->app_client_shmp)->shm->graphics_request.has_msg()) {
            break;
        }
        sleep(1);
    }
    return;
}

static void timer_handler(int) {
    char buf[MSG_CHANNEL_SIZE];
    GRAPHICS_MSG m;

    if (*g_bmsp->app_client_shmp) {
        if ((*g_bmsp->app_client_shmp)->shm->graphics_request.get_msg(buf)) {
            (*g_bmsp->app_client_shmp)->decode_graphics_msg(buf, m);
            switch (m.mode) {
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
                if (strlen(m.display)) {
                    sprintf(buf, "DISPLAY=%s", m.display);
                    putenv(buf);
                }
                set_mode(m.mode);
                break;
            }
        }
        if (acked_graphics_mode != current_graphics_mode) {
            bool sent = (*g_bmsp->app_client_shmp)->shm->graphics_reply.send_msg(
                xml_graphics_modes[current_graphics_mode]
            );
            if (sent) acked_graphics_mode = current_graphics_mode;
        }
    }
    maybe_render();
    glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);

    return;
}

// exit handler: really only meant to handle exits() called by GLUT...
//
void restart() {
    // don't do anything except when exit is called from the graphics-thread
    if (!pthread_equal(pthread_self(), graphics_thread))  {
        app_debug_msg ("exit() was called from worker-thread\n");
        return;
    }
    app_debug_msg ("exit() called from graphics-thread.\n");

    // if we are standalone and glut was initialized,
    // we assume user pressed 'close', and we exit the app
    //
    // 
    if (glut_is_initialized ) {
#ifdef __APPLE__
	if (boinc_is_standalone())
            app_debug_msg("Assuming user pressed 'close'... means we're exiting now.\n");
	else
            app_debug_msg("Assuming user pressed 'quit'... means we're exiting now.\n");

	if (boinc_delete_file(LOCKFILE) != 0)
            perror ("Failed to remove lockfile..\n");
	return;
#else
        if (boinc_is_standalone())
        {
            app_debug_msg("Assuming user pressed 'close'... means we're exiting now.\n");
            if (boinc_delete_file(LOCKFILE) != 0)
                perror ("Failed to remove lockfile..\n");
            return;
        }
#endif
    }

    // re-install the  exit-handler to catch glut's notorious exits()...
    //
    atexit(restart);

    // jump back to entry-point in xwin_graphics_event_loop();
    //
    app_debug_msg( "Jumping back to event_loop.\n");
    longjmp(jbuf, JUMP_EXIT);
}

// deal with ABORT's, typically raised by GLUT
// If we are in the graphics thread, we just return back
// into xwin_graphics_event_loop.  If we are in the main thread, then
// restore the old signal handler and raise SIGABORT.
//
void restart_sig(int /*signal_number*/) {
    if (pthread_equal(pthread_self(), graphics_thread)) {
        // alternative approach is for the signal hander to call exit().
        fprintf(stderr, "Caught SIGABRT in graphics thread\n");
        app_debug_msg ("Caught SIGABRT in graphics thread. Jumping back to xwin_graphics_event_loop()...\n");
        // jump back to entry-point in xwin_graphics_event_loop()
        longjmp(jbuf, JUMP_ABORT);
    } else {
        // In non-graphics thread: use original signal handler
        //
        fprintf(stderr, "Caught SIGABRT in non-graphics thread\n");
        app_debug_msg("Caught SIGABRT in non-graphics thread. Trying to call previous ABRT-handler...\n");
        if (sigaction(SIGABRT, &original_signal_handler, NULL)) {
	        perror("Unable to restore SIGABRT signal handler in non-graphics thread\n");
	        // what to do? call abort(3)??  call exit(nonzero)???
	        // Here we just return.
	    } else {
            // we could conceivably try to call the old signal handler
            // directly.  But this means checking how its flags/masks
            // are set, making sure it's not NULL (for no action) and so
            // on.  This seems simpler and more robust. On the other
            // hand it may take some time for the signal to be
            // delivered, and during that time, what's going to be
            // executing?
            raise(SIGABRT);
        }
    }
}


// graphics thread main-function
// NOTE: this should only be called *ONCE* by an application
//
void xwin_graphics_event_loop() {
    int restarted = 0;

    app_debug_msg ("Direct call to xwin_graphics_event_loop()\n");

    struct sigaction sa;
    sa.sa_handler = restart_sig;
    sa.sa_flags = SA_RESTART;

    // install signal handler to catch abort() from GLUT.  Note that
    // signal handlers are global to ALL threads, so the signal
    // handler that we record here in &original_signal_handler is the
    // previous one that applied to BOTH threads
    if (sigaction(SIGABRT, &sa, &original_signal_handler)) {
        perror("unable to install signal handler to catch SIGABORT");
    }

    // handle glut's notorious exits()..
    atexit(restart);

    // THIS IS THE re-entry point at exit or ABORT in the graphics-thread
    restarted = setjmp(jbuf);

    // if we came here from an ABORT-signal thrown in this thread,
    // we put this thread to sleep
    //
    if (restarted == JUMP_ABORT) {
        while(1)  {
            sleep(1);
        }
    }

    if (boinc_is_standalone()) {
        if (restarted) {
            while(1) {
                sleep(1);	// assuming glutInit() failed: put graphics-thread to sleep
            }
        } else {
	        // open the graphics-window
            set_mode(MODE_WINDOW);
            glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);      
        }
    } else {
        if (!glut_is_initialized) {
#ifdef __APPLE__
                setMacPList();
#endif
	        set_mode(MODE_HIDE_GRAPHICS);
	        while ( current_graphics_mode == MODE_HIDE_GRAPHICS ) {
	            app_debug_msg ("Graphics-thread now waiting for client-message...\n");
	            wait_for_initial_message();
	            app_debug_msg ("got a graphics-message from client... \n");
	            timer_handler(0);
	        }
	    } else
            // here glut has been initialized previously
            // probably the user pressed window-'close'
            //
            set_mode(MODE_HIDE_GRAPHICS);	// close any previously open windows
    }

    // ok we should be ready & initialized by now to call glutMainLoop()
    //
    app_debug_msg ("now calling glutMainLoop()...\n");
    glutMainLoop();
    app_debug_msg("...glutMainLoop() returned!! This should never happen...\n");
}

#ifndef _DEBUG
void app_debug_msg (const char* /*fmt*/, ...) {
    return;
}
#else
void app_debug_msg (const char* fmt, ...) {
    va_list args;
    char buffer[5000+1];
    static char *boinc_slotdir = NULL;
    va_start (args, fmt);

    if (boinc_slotdir == NULL) {
        char *tmp, *ptr;
        if ((tmp = getcwd(NULL, 0)) == NULL) {
	        perror ("failed to get working directory using getcwd()");
	        boinc_slotdir = (char*)calloc(1, 20);
	        strcpy( boinc_slotdir, "[unknown]");
	    } else {
	        if ( (ptr = strrchr(tmp, '/')) == NULL) {
	            ptr = tmp;
            } else {
                ptr ++;
            }
	        boinc_slotdir = (char*)calloc(1, strlen(ptr)+1);
	        strcpy(boinc_slotdir, ptr);
	        free (tmp);
	    }
    }
  
    vsnprintf (buffer, 5000, fmt, args);
    fprintf (stdout, "APP '%s' DEBUG: ", boinc_slotdir);
    fprintf (stdout, buffer);
    fflush (stdout);
  
    va_end (args);
}
#endif




const char *BOINC_RCSID_c457a14644 = "$Id$";
