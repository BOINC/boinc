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
extern void boinc_app_mouse_button(int x, int y, int which, int is_down);
extern void boinc_app_mouse_move(int x, int y, int left, int middle, int right);
extern void boinc_app_key_press(int, int);
extern void boinc_app_key_release(int, int);
extern void boinc_suspend_graphics_thread();
extern void boinc_resume_graphics_thread();

// C++ API follows here 
#ifdef __cplusplus
} // end extern "C"

#include "boinc_api.h"

extern int boinc_init_options_graphics(BOINC_OPTIONS&, WORKER_FUNC_PTR);
extern bool boinc_graphics_possible();

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
