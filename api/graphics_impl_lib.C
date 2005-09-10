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

// Code that is included in the shared-library part of a graphics app,
// but NOT in libboinc_graphics_api.a (used by monolithic apps)

#include "app_ipc.h"
#include "graphics_impl.h"

int boinc_get_init_data(APP_INIT_DATA& app_init_data) {
    return g_bmsp->boinc_get_init_data_hook(app_init_data);
}

bool boinc_is_standalone() {
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

const char *BOINC_RCSID_9886dee259 = "$Id$";
