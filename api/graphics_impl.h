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

#include "boinc_api.h"
#include "graphics_api.h"

typedef int (*BIOG_FUNC_PTR)(BOINC_OPTIONS&);
    // ptr to a function like boinc_init_options_general()

// stuff in the main program that the library need to access
//
struct BOINC_MAIN_STATE {
    BIOG_FUNC_PTR boinc_init_options_general_hook;
    bool (*boinc_is_standalone_hook)();
    int (*boinc_get_init_data_hook)(APP_INIT_DATA&);
    int (*set_worker_timer_hook)();
    APP_CLIENT_SHM* app_client_shm;
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
