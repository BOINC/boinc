#ifndef BOINC_GRAPHICS_API_H
#define BOINC_GRAPHICS_API_H

/*----------------------------------------------------------------------
 * PURE ANSI-C API follows here 
 */
/* to allow prototypes using 'bool' in ANSI-C */
#if (!defined __cplusplus) && (!defined bool)
#  define bool int
#endif

#ifdef __cplusplus
extern "C" {
#endif

  extern int boinc_init_graphics(void (*worker)());

  // Functions that must be supplied by the app
  // application needs to define mouse, keyboard handlers
  //
  extern void app_graphics_render(int xs, int ys, double time_of_day);
  extern void app_graphics_init(void);
  // called each time a window is opened;
  // called in the graphics thread 
  extern void app_graphics_reread_prefs(void);
  // called when get REREAD_PREFS message from core client.
  // called in the graphics thread
  extern void app_graphics_resize(int width, int height);
  extern void boinc_app_mouse_button(int x, int y, int which, bool is_down);
  extern void boinc_app_mouse_move(int x, int y, bool left, bool middle, bool right);
  extern void boinc_app_key_press(int, int);
  extern void boinc_app_key_release(int, int);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* ----------------------------------------------------------------------
 * C++ API follows here 
 */
#if defined __cplusplus
#include "boinc_api.h"

extern int boinc_init_options_graphics(BOINC_OPTIONS&, void (*worker)());

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern bool throttled_app_render(int, int, double);

#ifdef _WIN32
extern HANDLE hQuitEvent;
extern HANDLE graphics_threadh;
extern BOOL   win_loop_done;
#endif /* WIN32 */


#endif /* C++ API */

#endif /* double-inclusion protection */
