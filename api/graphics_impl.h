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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "boinc_api.h"
#include "graphics_api.h"

typedef int (*BIOG_FUNC_PTR)(BOINC_OPTIONS&);
    // ptr to a function like boinc_init_options_general()

// stuff in the main program that the library need to access
//
struct BOINC_MAIN_STATE {
    BIOG_FUNC_PTR boinc_init_options_general_hook;
    int (*boinc_is_standalone_hook)();
    int (*boinc_get_init_data_hook)(APP_INIT_DATA&);
    int (*set_worker_timer_hook)();
    int (*start_timer_thread_hook)();
    APP_CLIENT_SHM** app_client_shmp;
#ifdef _WIN32
    UINT_PTR gfx_timer_id;
#endif
};

extern int boinc_init_graphics_impl(
    WORKER_FUNC_PTR worker, BOINC_MAIN_STATE*
);

// This extern C is needed to make this code work correctly,
// even in a 100% C++ context.
// This is because we need to dlsym() resolve this function.
// That does not work unless the symbol is in the library in UNMANGLED form.
// See http://www.isotton.com/howtos/C++-dlopen-mini-HOWTO/C++-dlopen-mini-HOWTO.html
// for some additional discussion.

extern "C" {
    extern int boinc_init_options_graphics_impl(
        BOINC_OPTIONS& opt,
        WORKER_FUNC_PTR _worker_main,
        BOINC_MAIN_STATE*
    );
}

extern BOINC_MAIN_STATE* g_bmsp;
extern void get_window_title(APP_INIT_DATA& aid, char* buf, int len);
