// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#ifndef BOINC_GRAPHICS_API_H
#define BOINC_GRAPHICS_API_H

// PURE ANSI-C API follows here 
// to allow prototypes using 'bool' in ANSI-C
#if (!defined __cplusplus) && (!defined bool)
#if ((defined(_MSC_VER)) && (_MSC_VER > 1020))
#define bool char
#else
#define bool int
#endif // defined(_MSC_VER) && (_MSC_VER > 1020)
#endif // (!defined __cplusplus) && (!defined bool)

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
extern BOOL   win_loop_done;
#endif // WIN32


#endif // C++ API

#endif // double-inclusion protection
