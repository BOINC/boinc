#ifndef BOINC_GRAPHICS_API_H
#define BOINC_GRAPHICS_API_H

// The API (functions called by the app)
extern int boinc_init_graphics();
extern int boinc_finish_graphics();

// Functions that must be supplied by the app
//
extern void app_graphics_render(int xs, int ys, double time_of_day);
extern void app_graphics_init();
    // called each time a window is opened;
    // called in the graphics thread
extern void app_graphics_reread_prefs();
    // called when get REREAD_PREFS message from core client.
    // called in the graphics thread
extern void app_graphics_resize(int width, int height);

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern bool throttled_app_render(int, int, double);

#ifdef _WIN32
extern HANDLE hQuitEvent;
extern HANDLE graphics_threadh;
extern BOOL   win_loop_done;
#endif

#endif
