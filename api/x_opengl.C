#include "x_opengl.h"
#include <cstdio>

#ifdef HAVE_GL
#include "boinc_gl.h"
#endif

#include "boinc_api.h"
#include "graphics_api.h"
#include "app_ipc.h"
#include "util.h"
#include "glut.h"

#define BOINC_WINDOW_CLASS_NAME "BOINC_app"

static bool visible = true;
static int current_graphics_mode = MODE_HIDE_GRAPHICS;
int win_loop_done;
int xpos = 100, ypos = 100; // globals
int clicked_button;
int win; 
extern void graphics_thread_init();
extern GRAPHICS_INFO gi;
static void timer_handler();
void set_mode(int mode);

void keyboardU(unsigned char key, int x, int y) {
  /* This callback is invoked when a user presses a key.
   */
  if (current_graphics_mode == MODE_FULLSCREEN) {
    set_mode(MODE_HIDE_GRAPHICS);
    // PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0); // WINDOWS DEPENDENT
    // } else {
    // boinc_app_key_release((int)wParam, (int)lParam); //CHANGE TO SPECIFICALLY RESPOND TO KEYSTROKE
  }
}

void mouse_click(int button, int state, int x, int y){
  clicked_button = button;
  if (current_graphics_mode == MODE_FULLSCREEN) {
    set_mode(MODE_HIDE_GRAPHICS);
    // PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0); // WINDOWS DEPENDENT
  } else {
    if(state)
      boinc_app_mouse_button(x, y, button, false);
    else
      boinc_app_mouse_button(x, y, button, true);
  }
}

void mouse_move(int x, int y){
  if(current_graphics_mode == MODE_FULLSCREEN) {
    set_mode(MODE_HIDE_GRAPHICS);
    // PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0);
  } else {
    boinc_app_mouse_move(x, y, false, false, false);
  }
}

void mouse_click_move(int x, int y){
  if(clicked_button == 2){
    boinc_app_mouse_move(x, y, false, false, true);
  }else if (clicked_button == 1){
    boinc_app_mouse_move(x, y, false, true, false);
  }else if (clicked_button == 0){
    boinc_app_mouse_move(x, y, true, false, false);
  }else{
    boinc_app_mouse_move(x, y, false, false, false);
  }
}

void keyboardD(unsigned char key, int x, int y) {
  /* This callback is invoked when a user presses a key.
   */
  if (current_graphics_mode == MODE_FULLSCREEN) {
    set_mode(MODE_HIDE_GRAPHICS);
    // PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0); // WINDOWS DEPENDENT
  } else {
    // boinc_app_key_press((int)wParam, (int)lParam); //CHANGE IF WANT TO RESPOND TO KEYSTROKE
  }
}

void computeFPS(){
  static int frameCount = 0;
  static int lastFrameTime = 0;

  frameCount++;
  int currentTime = glutGet(GLenum(GLUT_ELAPSED_TIME));
  if (currentTime - lastFrameTime > 1000){
    char s[30];
    sprintf(s, "%s [ FPS: %4.2f ]", aid.app_name,
	    frameCount*1000/(currentTime - lastFrameTime));
    glutSetWindowTitle(s);
    lastFrameTime = currentTime;
    frameCount = 0;
  }
}


void onIdle(){
  static double oldTime = 0;
  double currentTime = dtime();
  
  if(currentTime - oldTime > .001){
    timer_handler();
    oldTime = currentTime;
  }
  //  computeFPS();
}

static void make_new_window(int mode){
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); //DEPTH of how many? want 16
  if (mode == MODE_FULLSCREEN){
    glutFullScreen(); //TOPMOST AND POPUP?
  } else if(mode == MODE_WINDOW){
    glutInitWindowPosition(xpos, ypos);
    glutInitWindowSize(gi.xsize, gi.ysize); 
    if (!strlen(aid.app_name)) 
      strcpy(aid.app_name, "BOINC Application");
    win = glutCreateWindow(aid.app_name); 
    glutReshapeFunc(app_graphics_resize);
    glutKeyboardFunc(keyboardD);
    glutKeyboardUpFunc(keyboardU);
    glutMouseFunc(mouse_click);
    glutMotionFunc(mouse_click_move);
    glutDisplayFunc(timer_handler); 
    glutIdleFunc(onIdle);
    
    app_graphics_init();
    glEnable(GL_DEPTH_TEST);  
  }
}

void set_mode(int mode) {
  if (current_graphics_mode != mode){
    if(mode == MODE_HIDE_GRAPHICS){
      if(mode != MODE_FULLSCREEN){
	xpos = glutGet(GLUT_WINDOW_X);
	ypos = glutGet(GLUT_WINDOW_Y);
	
	gi.xsize = glutGet(GLUT_WINDOW_WIDTH);
	gi.ysize = glutGet(GLUT_WINDOW_HEIGHT);       
      }
      glutDestroyWindow(win);
    }
    make_new_window(mode);
    current_graphics_mode = mode;
  }
  
  // tell the core client that we're entering new mode
  //
  if (app_client_shm) {
    printf("in0\n");
    app_client_shm->shm->graphics_reply.send_msg(
	   	       xml_graphics_modes[current_graphics_mode]
						 );
  }
}

void* xwin_graphics_event_loop(void*){
 
  if (boinc_is_standalone()) {
    set_mode(MODE_WINDOW);
  } else {
    set_mode(MODE_HIDE_GRAPHICS);
  }
  /*  
  glutReshapeFunc(app_graphics_resize);
  glutKeyboardFunc(keyboardD);
  glutKeyboardUpFunc(keyboardU);
  glutMouseFunc(mouse_click);
  glutMotionFunc(mouse_click_move);
  glutDisplayFunc(timer_handler); 
  glutIdleFunc(onIdle);

  app_graphics_init();
  glEnable(GL_DEPTH_TEST);*/
  glutMainLoop();
}

static void timer_handler() {
  //  printf("in timer_handler\n");
  char buf[MSG_CHANNEL_SIZE];

  int width, height, new_mode, msg;
  if (app_client_shm) {
    fprintf(stderr, "In first if\n"); fflush(stderr);
    if(app_client_shm->shm->graphics_request.get_msg(buf)){
      new_mode = app_client_shm->decode_graphics_msg(buf);
      switch(new_mode) {
      case MODE_REREAD_PREFS:
	//only reread graphics prefs if we have a window open
	//
	switch(current_graphics_mode){
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
  }
  if (!visible) return;
  if (current_graphics_mode == MODE_HIDE_GRAPHICS) return;
  
  // TODO: remove width, height from API
  //
  width = glutGet(GLUT_WINDOW_WIDTH);
  height = glutGet(GLUT_WINDOW_HEIGHT);
  
  if (throttled_app_render(width, height, dtime())) {
    //    printf("swapping buffers\n");
    glutSwapBuffers();
    }
}


/*
  
BOOL VerifyPassword(HWND hwnd)
{ // Under NT, we return TRUE immediately. This lets the saver quit,
// and the system manages passwords. Under '95, we call VerifyScreenSavePwd.
// This checks the appropriate registry key and, if necessary,
// pops up a verify dialog
/*OSVERSIONINFO osv; osv.dwOSVersionInfoSize=sizeof(osv); GetVersionEx(&osv);
if (osv.dwPlatformId==VER_PLATFORM_WIN32_NT) return TRUE;
HINSTANCE hpwdcpl=::LoadLibrary("PASSWORD.CPL");
if (hpwdcpl==NULL) {return TRUE;}
typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);
VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
VerifyScreenSavePwd=
(VERIFYSCREENSAVEPWD)GetProcAddress(hpwdcpl,"VerifyScreenSavePwd");
if (VerifyScreenSavePwd==NULL)
{
  FreeLibrary(hpwdcpl);return TRUE;
}
         BOOL bres=VerifyScreenSavePwd(hwnd); FreeLibrary(hpwdcpl);
         return bres;
}
*/

#if 0
float txt_widths[256];

#endif
