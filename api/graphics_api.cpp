// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// DEPRECATED - DO NOT USE

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include <cstring>
#include "diagnostics.h"
#include "boinc_api.h"
#include "graphics_impl.h"
#include "graphics_api.h"

static BOINC_MAIN_STATE boinc_main_state;

static void init_main_state() {
    boinc_main_state.boinc_init_options_general_hook = boinc_init_options_general;
    boinc_main_state.boinc_is_standalone_hook = boinc_is_standalone;
    boinc_main_state.boinc_get_init_data_hook = boinc_get_init_data;
    boinc_main_state.start_timer_thread_hook = start_timer_thread;
    boinc_main_state.app_client_shmp = &app_client_shm;
#ifdef _WIN32
    boinc_main_state.gfx_timer_id = (UINT_PTR)NULL;
#endif
}

void boinc_suspend_graphics_thread() {
#ifdef _WIN32
  if (graphics_threadh) SuspendThread(graphics_threadh);
#endif
}

void boinc_resume_graphics_thread() {
#ifdef _WIN32
  if (graphics_threadh) ResumeThread(graphics_threadh);
#endif
}

int boinc_init_graphics(void (*worker)()) {
    int retval;
    if (!diagnostics_is_initialized()) {
        retval = boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);
        if (retval) return retval;
    }
    init_main_state();
    return boinc_init_graphics_impl(worker, &boinc_main_state);
}

int boinc_init_options_graphics(BOINC_OPTIONS& opt, void (*worker)()) {
    int retval;
    if (!diagnostics_is_initialized()) {
        retval = boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);
        if (retval) return retval;
    }
    init_main_state();
    return boinc_init_options_graphics_impl(opt, worker, &boinc_main_state);
}

bool boinc_graphics_possible() {
#ifdef _WIN32
#if 0
    // Attempt to load the dlls that are required to display graphics, if
    // any of them fail do not start the application in graphics mode.
    if (FAILED(__HrLoadAllImportsForDll("GDI32.dll"))) {
       fprintf(stderr, "Failed to load GDI32.DLL\n" );
       return false;
    }
    if (FAILED(__HrLoadAllImportsForDll("OPENGL32.dll"))) {
       fprintf( stderr, "Failed to load OPENGL32.DLL\n" );
       return false;
    }
    if (FAILED(__HrLoadAllImportsForDll("GLU32.dll"))) {
       fprintf( stderr, "Failed to load GLU32.DLL\n" );
       return false;
    }
#endif
#elif defined(__APPLE__)
#else
    if (!getenv("DISPLAY")) return false;
#endif
    return true;
}
