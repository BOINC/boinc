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

#ifndef BOINC_GRAPHICS_API_H
#define BOINC_GRAPHICS_API_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*WORKER_FUNC_PTR)();

extern int boinc_init_graphics(WORKER_FUNC_PTR);

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
extern void boinc_app_mouse_button(int x, int y, int which, bool is_down);
extern void boinc_app_mouse_move(int x, int y, bool left, bool middle, bool right);
extern void boinc_app_key_press(int, int);
extern void boinc_app_key_release(int, int);

#ifdef __cplusplus
} // extern "C"
#endif

// C++ API follows here 

#if defined __cplusplus
#include "boinc_api.h"

extern int boinc_init_options_graphics(BOINC_OPTIONS&, WORKER_FUNC_PTR);

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern bool throttled_app_render(int, int, double);

#ifdef _WIN32
extern HANDLE hQuitEvent;
extern HANDLE graphics_threadh;
#endif // WIN32


#endif // C++ API

#endif // double-inclusion protection
