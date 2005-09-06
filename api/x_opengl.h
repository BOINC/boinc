#ifndef BOINC_X_OPENGL_H
#define BOINC_X_OPENGL_H

#ifdef __cplusplus
extern "C" {
#endif

extern int xwin_glut_is_initialized();  

#ifdef __APPLE__
extern void MacGLUTFix(bool isScreenSaver);  
extern void BringAppToFront(void);
extern void HideThisApp(void);
#endif

#ifdef __cplusplus
}
#endif

extern void xwin_graphics_event_loop();

#endif
