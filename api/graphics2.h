#ifndef _GRAPHICS2_H_
#define _GRAPHICS2_H_

#ifdef __cplusplus
extern "C" {
#endif

// Functions that must be supplied by the app
//
extern void app_graphics_render(int xs, int ys, double time_of_day);
extern void app_graphics_init(void);
    // called each time a window is opened;
    // called in the graphics thread
extern void app_graphics_reread_prefs(void);
    // called when get REREAD_PREFS message from core client.
    // called in the graphics thread
extern void app_graphics_resize(int width, int height);
extern void boinc_app_mouse_button(int x, int y, int which, int is_down);
extern void boinc_app_mouse_move(int x, int y, int left, int middle, int right);
extern void boinc_app_key_press(int, int);
extern void boinc_app_key_release(int, int);

// C++ API follows here
#ifdef __cplusplus
} // end extern "C"

extern void boinc_graphics_loop(int, char**);
extern void* boinc_graphics_make_shmem(char*, int);
extern void* boinc_graphics_get_shmem(char*);

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern void get_window_title(char* buf, int len);
extern bool throttled_app_render(int, int, double);

#endif // C++ API


#endif
