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

// Code that is included in the shared-library part of a graphics app,
// but NOT in libboinc_graphics_api.a (used by monolithic apps)

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include <cstring>
#include "config.h"
#include "app_ipc.h"
#include "graphics_impl.h"

int boinc_get_init_data(APP_INIT_DATA& app_init_data) {
    return g_bmsp->boinc_get_init_data_hook(app_init_data);
}

int boinc_is_standalone() {
    return g_bmsp->boinc_is_standalone_hook();
}

// The following is a duplicate of function in boinc_api.C

#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#ifndef _WIN32
// block SIGALRM, so that the worker thread will be forced to handle it
//
void block_sigalrm() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}
#endif
