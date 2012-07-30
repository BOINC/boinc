// DEPRECATED

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

#include "config.h"
#include <cstdlib>
#include <cstdio>    
#include <csetjmp>    
#include <unistd.h> 
#include <pthread.h>
#include <cstring>
#include <csignal>
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
#define TIMER_INTERVAL_SUSPENDED_MSEC 1000

static bool visible = true;
static int current_graphics_mode = MODE_HIDE_GRAPHICS;
static int acked_graphics_mode;
static int xpos = 100, ypos = 100;
static int win_width = 600, win_height = 400;
static int clicked_button;
static int win=0;
static bool glut_is_initialized = false;
static bool suspend_render = false;

// freeglut extensions, not in GL/glut.h.
//

// glutGet(GLUT_VERSION) returns the freeglut version.
// returns (major*10000)+(minor*100)+maint level
// NOTE - make sure freeglut is initialized before
//        getting the version.
// NOTE - this isn't valid in GLUT which makes it doubly
//        useful, the -1 return code says its _not_
//        running freeglut so makes for a useful way
//        we can tell at runtime which library
//        is in use.
//
#ifndef GLUT_VERSION
#define GLUT_VERSION 0x01FC
#endif

// glutGet(GLUT_INIT_STATE) returns freegluts state.  Determines
// when to call glutInit again to reinitialize it.
//
#ifndef GLUT_INIT_STATE
#define GLUT_INIT_STATE 0x007C
#endif

// Simple defines for checking freeglut states
//

// Check whether freeglut is initialized.  Necessary
// since it deinitializes itself when the window is
// closed.  Only works with freeglut, although glut
// will return -1 and write a warning message to stdout
//
#define FREEGLUT_IS_INITIALIZED ((bool)(glutGet(GLUT_INIT_STATE)==1))


// Check whether a window exists. Necessary as glutDestroyWindow
// causes SIGABRT, so we hide and reshow windows instead
// of creating and deleting them.  (Talking freeglut here)
// note - make sure glutInit is called before checking,
// otherwise freeglut will SIGABRT.
//
#define GLUT_HAVE_WINDOW ((bool)(glutGetWindow()!=0))


// Runtime tag to tell we're using freeglut so can take precautions
// and avoid SIGABRTs.  Initialize the flag to 'true' and the
// init code in boinc_glut_init() will check whether we have
// openglut or freeglut, resetting the variable as necessary.
//
// Linux systems ship with freeglut, so default is 'true' and
// its checked.
//
// If running freeglut, also get the version.
//
#ifdef __APPLE__
static bool glut_is_freeglut = false;   // Avoid warning message to stderr from glutGet(GLUT_VERSION) 
static bool need_show = false;
#else
static bool glut_is_freeglut = true;
#endif
static int glut_version = 0;


// State variables for freeglut windows.  Since freeglut has
// problems wiht glutDestroyWindow, keep it open and just
// hide/show it as necessary.
//
static int fg_window_state;   // values same as mode
static bool fg_window_is_fullscreen; 

static jmp_buf jbuf;    // longjump/setjump for exit/signal handler
static struct sigaction original_signal_handler; // store previous ABRT signal-handler
static void set_mode(int mode);
static void restart(void);
static void boinc_glut_init(void);

static APP_INIT_DATA aid;

extern pthread_t graphics_thread;    // thread info
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

bool debug = false;

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

// maybe_render() can be called directly by GLUT when a window is 
// uncovered, so we let that happen even if suspend_render is true.
static void maybe_render() {
    int width, height;
    BOINC_STATUS boinc_status;

    if (visible && (current_graphics_mode != MODE_HIDE_GRAPHICS)) {
        width = glutGet(GLUT_WINDOW_WIDTH);
        height = glutGet(GLUT_WINDOW_HEIGHT);
        if (throttled_app_render(width, height, dtime())) {
            glutSwapBuffers();
#ifdef __APPLE__
            switch (current_graphics_mode) {
            case MODE_WINDOW:
                MacGLUTFix(false);
                if (need_show) {
                    glutShowWindow();
                    need_show = false;
                }
                break;
            case MODE_FULLSCREEN:
            case MODE_BLANKSCREEN:
                MacGLUTFix(true);
                if (need_show) {
                    glutShowWindow();
                    need_show = false;
                }
                break;
            }
#endif
            // Always render once after app is suspended in case app displays
            //  different graphics when suspended, then stop rendering
            boinc_get_status(&boinc_status);
            if (boinc_status.suspended)
                suspend_render = true;
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

    if (glut_is_initialized && glut_is_freeglut) {
        if (!FREEGLUT_IS_INITIALIZED) {
            glut_is_initialized = false;
            fg_window_is_fullscreen = false;
	        fg_window_state = 0;
        }
    }
    if (!glut_is_initialized)  {
        boinc_glut_init();
    }

    if (debug) fprintf(stderr, "make_new_window(): now calling glutCreateWindow(%s)...\n", aid.app_name);
    char window_title[256];
    get_window_title(aid, window_title, 256);

    // just show the window if its hidden
    // if it used to be fullscreen (before
    // it was hidden, reset size and position
    // to defaults
    //
    bool have_window = false;
    if (glut_is_freeglut && GLUT_HAVE_WINDOW) {
          have_window = true;
          glutShowWindow();
          if (fg_window_is_fullscreen) {
             glutPositionWindow(xpos, ypos);
             glutReshapeWindow(600, 400);
             fg_window_is_fullscreen = false;
          }
	  fg_window_state = MODE_WINDOW;
    }
    
#ifdef __APPLE__
    if (win)
        have_window = true;
#endif

    if (!have_window) {
        win = glutCreateWindow(window_title); 
        if (debug) fprintf(stderr, "glutCreateWindow() succeeded. win = %d\n", win);

        glutReshapeFunc(app_graphics_resize);
        glutKeyboardFunc(keyboardD);
        glutKeyboardUpFunc(keyboardU);
        glutMouseFunc(mouse_click);
        glutMotionFunc(mouse_click_move);
        glutDisplayFunc(maybe_render); 
        glEnable(GL_DEPTH_TEST);
  
        app_graphics_init();
    }
  
#ifdef __APPLE__
    glutWMCloseFunc(CloseWindow);   // Enable the window's close box
    BringAppToFront();
    // Show window only after a successful call to throttled_app_render(); 
    // this avoids momentary display of old image when screensaver restarts 
    // which made image appear to "jump."
    need_show = true;
#endif

    if (mode == MODE_FULLSCREEN)  {
        glutFullScreen();
    }

    return;
}

// Initialize GLUT.
// Using GLUT, this should only called once,
// even if the window is closed and opened.
// Using freeGLUT, its called to reinit freeglut and
// the graphics window as freeglut 'deinitialized' itself
// when the window is destroyed
//
// note - this is called from make_new_window() when a new
// window is to be created.
//
static void boinc_glut_init() {
    const char* args[2] = {"screensaver", NULL};
    int one=1;
    static bool first = true;
    
    win = 0;
    if (debug) fprintf(stderr, "Calling glutInit()... \n");
    glutInit (&one, (char**)args);
    if (debug) fprintf(stderr, "survived glutInit(). \n");

    // figure out whether we're running GLUT or freeglut.
    // 
    // note - glutGet(GLUT_VERSION) is supported in freeglut,
    //        other GLUT  returns -1 and outputs a warning
    //        message to stderr. 
    //      - must be after glutInit or will get SIGABRT
    //
    if (first) {
        first = false;
        if (glut_is_freeglut) {
            glut_version = glutGet(GLUT_VERSION);
            if (glut_version == -1) {
                glut_version = 0;
                glut_is_freeglut = false;
           } else {
               int major = glut_version/10000;
               int minor = (glut_version -(major*10000))/100;
               int maint = glut_version - (major*10000) - (minor * 100);
               if (debug) fprintf(stderr, "Running freeGLUT %d.%d.%d.\n", major, minor, maint);
           }
       }
    }

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
    glutInitWindowPosition(xpos, ypos);
    glutInitWindowSize(600, 400); 
    g_bmsp->boinc_get_init_data_hook(aid);
    if (!strlen(aid.app_name))  {
        strcpy(aid.app_name, "BOINC Application");
    }

    glut_is_initialized = true;
    if (glut_is_freeglut) {
       fg_window_state = 0;
    }

    return;

}

// Destroy current window if any
//
// Note - glutDestroyWindow results in SIGABRT in freeglut, so
//        instead of closing the window, just hide it.  And
//        before hiding it, make sure there is a window,
//        user could have closed it.
//
void KillWindow() {
   if (win) {
#ifdef __APPLE__
        if (current_graphics_mode == MODE_WINDOW) {
            win_width = glutGet(GLUT_WINDOW_WIDTH);
            win_height = glutGet(GLUT_WINDOW_HEIGHT);
            xpos = glutGet(GLUT_WINDOW_X);
            ypos = glutGet(GLUT_WINDOW_Y);
        } else {
            // If fullscreen, resize now to avoid ugly flash if we subsequently 
            // redisplay as MODE_WINDOW.
            glutPositionWindow(xpos, ypos);
            glutReshapeWindow(win_width, win_height);
        }
        
        // On Intel Macs (but not on PowerPC Macs) GLUT's destuctors often crash when 
        // glutDestroyWindow() is called.  So far, this has only been reported for
        // SETI@home. Since it doesn't occur on PowerPC Macs, we suspect a bug in GLUT.  
        // To work around this, we just hide the window instead.  Though this does not 
        // free up RAM and VM used by the graphics, glutDestroyWindow() doesn't free 
        // them either (surprisingly), so there is no additional penalty for doing it 
        // this way.
        glutHideWindow();
#else
      if (glut_is_freeglut && FREEGLUT_IS_INITIALIZED && GLUT_HAVE_WINDOW) {
         glutHideWindow();
      } else {
         int oldwin = win;
         win = 0;	// set this to 0 FIRST to avoid recursion if the following call fails.
         glutDestroyWindow(oldwin);
      }
#endif
   } 
}

void set_mode(int mode) {
    if (mode == current_graphics_mode) {
        // Bring graphics window to front whenever user presses "Show graphics"
        if (mode == MODE_WINDOW) {
#ifdef __APPLE__
            BringAppToFront();
#else
            if (glut_is_freeglut && FREEGLUT_IS_INITIALIZED && (GLUT_HAVE_WINDOW > 0)) {
//              glutPopWindow();
       	        glutShowWindow();
            }
#endif 
       }
        return;
    }

    if (debug) fprintf(stderr, "set_mode(%d): current_mode = %d.\n", mode, current_graphics_mode);
    if (glut_is_initialized) {
        if (debug) fprintf(stderr, "Calling KillWindow(): win = %d\n", win);
        KillWindow();
        if (debug) fprintf(stderr, "KillWindow() survived.\n");
    }
    
    if (mode != MODE_HIDE_GRAPHICS) {
        if (debug) fprintf(stderr, "set_mode(): Calling make_new_window(%d)\n", mode); 
        make_new_window(mode);
        if (debug) fprintf(stderr, "make_new_window() survived.\n");
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
    BOINC_STATUS boinc_status;

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
		    setenv("DISPLAY",m.display,1);
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
    boinc_get_status(&boinc_status);
    if (! boinc_status.suspended)
        suspend_render = false;
    if (suspend_render) {
        glutTimerFunc(TIMER_INTERVAL_SUSPENDED_MSEC, timer_handler, 0);
    } else {
        maybe_render();
        glutTimerFunc(TIMER_INTERVAL_MSEC, timer_handler, 0);
    }

    return;
}

// exit handler: really only meant to handle exits() called by GLUT...
//
void restart() {
    // don't do anything except when exit is called from the graphics-thread
    if (!pthread_equal(pthread_self(), graphics_thread))  {
        if (debug) fprintf(stderr, "exit() was called from worker-thread\n");
        return;
    }
    if (debug) fprintf(stderr, "restart: exit() called from graphics-thread.\n");

    // if we are standalone and glut was initialized,
    // we assume user pressed 'close', and we exit the app
    //
    // 
    if (glut_is_initialized ) {
        if (boinc_is_standalone()) {
            if (debug) fprintf(stderr,
                "Assuming user pressed 'close'... means we're exiting now.\n"
            );
            if (boinc_delete_file(LOCKFILE) != 0) {
                perror ("Failed to remove lockfile..\n");
            }
            return;
        }
    }

    // re-install the  exit-handler to catch glut's notorious exits()...
    //
    atexit(restart);

    // jump back to entry-point in xwin_graphics_event_loop();
    //
    if (debug) fprintf(stderr, "restart: Jumping back to event_loop.\n");
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
        if (debug) fprintf(stderr, "Caught SIGABRT in graphics thread. Jumping back to xwin_graphics_event_loop()...\n");
        // jump back to entry-point in xwin_graphics_event_loop()
        longjmp(jbuf, JUMP_ABORT);
    } else {
        // In non-graphics thread: use original signal handler
        //
        fprintf(stderr, "Caught SIGABRT in non-graphics thread\n");
        if (debug) fprintf(stderr, "Caught SIGABRT in non-graphics thread. Trying to call previous ABRT-handler...\n");
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

    if (debug) fprintf(stderr, "Direct call to xwin_graphics_event_loop()\n");

    struct sigaction sa = {0};
    memset(&sa, 0, sizeof(sa));
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

    // If using freeglut, check to see if we need to
    // reinitialize glut and graphics.
    // We're only resetting flags here, actual reinit function
    // is called in make_new_window().
    //
    if (glut_is_initialized && glut_is_freeglut) {
        if (1 == (glut_is_initialized = FREEGLUT_IS_INITIALIZED)) {
	       if (!GLUT_HAVE_WINDOW) {
	          win = 0; 
	       }
	    }
    }

    if (boinc_is_standalone()) {
        if (restarted) {
            while(1) {
                sleep(1);    // assuming glutInit() failed: put graphics-thread to sleep
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
                if (debug) fprintf(stderr,
                    "Graphics-thread now waiting for client-message...\n"
                );
                wait_for_initial_message();
                if (debug) fprintf(stderr, "got a graphics-message from client... \n");
                timer_handler(0);
            }
        } else
            // here glut has been initialized previously
            // probably the user pressed window-'close'
            //
            set_mode(MODE_HIDE_GRAPHICS);    // close any previously open windows
    }

    // ok we should be ready & initialized by now to call glutMainLoop()
    //
    if (debug) fprintf(stderr, "now calling glutMainLoop()...\n");
    glutMainLoop();
    if (debug) fprintf(stderr, "glutMainLoop() returned!! This should never happen...\n");
}


const char *BOINC_RCSID_c457a14644 = "$Id$";
