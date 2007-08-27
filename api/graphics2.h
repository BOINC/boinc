#ifndef _GRAPHICS2_H_
#define _GRAPHICS2_H_

// Functions that must be supplied by the app
//
extern void app_graphics_render(int xs, int ys, double time_of_day);
extern void app_graphics_init(void);
extern void app_graphics_reread_prefs(void);
    // called when get REREAD_PREFS message from core client.
extern void app_graphics_resize(int width, int height);
extern void boinc_app_mouse_button(int x, int y, int which, int is_down);
extern void boinc_app_mouse_move(int x, int y, int left, int middle, int right);
extern void boinc_app_key_press(int, int);
extern void boinc_app_key_release(int, int);

// Functions that the app can call
//
extern void boinc_graphics_loop(int, char**);
extern void* boinc_graphics_make_shmem(char*, int);

#ifdef USE_FILE_MAPPED_SHMEM
// Use boinc_graphics_make_shmem() instead of boinc_graphics_get_shmem()
#else
extern void* boinc_graphics_get_shmem(char*);
#endif

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern void get_window_title(char* buf, int len);
extern bool throttled_app_render(int, int, double);

#endif
