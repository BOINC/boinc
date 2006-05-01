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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "diagnostics.h"
#include "boinc_api.h"
#include "graphics_impl.h"
#include "graphics_api.h"

static BOINC_MAIN_STATE boinc_main_state;

static void init_main_state() {
    boinc_main_state.boinc_init_options_general_hook = boinc_init_options_general;
    boinc_main_state.boinc_is_standalone_hook = boinc_is_standalone;
    boinc_main_state.boinc_get_init_data_hook = boinc_get_init_data;
    boinc_main_state.set_worker_timer_hook = set_worker_timer;
    boinc_main_state.app_client_shmp = &app_client_shm;
#ifdef _WIN32
    boinc_main_state.gfx_timer_id = NULL;
#endif
}

int boinc_init_graphics(void (*worker)()) {
    int retval;
    if (!diagnostics_is_initialized()) {
        retval = boinc_init_diagnostics(BOINC_DIAG_USEDEFULATS);
        if (retval) return retval;
    }
    init_main_state();
    return boinc_init_graphics_impl(worker, &boinc_main_state);
}

int boinc_init_options_graphics(BOINC_OPTIONS& opt, void (*worker)()) {
    init_main_state();
    return boinc_init_options_graphics_impl(opt, worker, &boinc_main_state);
}

const char *BOINC_RCSID_b2ceed0813 = "$Id$";
