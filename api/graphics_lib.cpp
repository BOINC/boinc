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

// An interface to BOINC graphics in which
// the graphics code lives in a separate shared library.
// This lets you make applications that work whether or not
// the host has X11 and OpenGL libraries.
//
// This file is the code that's part of the main program

// DEPRECATED

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "config.h"
#include <cstring>
#include <dlfcn.h>

#include "boinc_api.h"
#include "graphics_api.h"
#include "graphics_impl.h"
#include "graphics_lib.h"

static BOINC_MAIN_STATE boinc_main_state;

void* graphics_lib_handle=NULL;

typedef int (*BIOGI_FUNC_PTR)(BOINC_OPTIONS&, WORKER_FUNC_PTR, BOINC_MAIN_STATE*);
    // ptr to a function like boinc_init_options_graphics_impl()

// This routine never returns.
// If a problem arises, it calls boinc_finish(nonzero).
//
// First argument: worker function
//
// Second argument: argv[0] from command line arguments.
// This is the executable name, and is used to derive
// the shared object library name: executable_name.so

int boinc_init_graphics_lib(WORKER_FUNC_PTR worker, char* argv0) {
    BOINC_OPTIONS opt;
    boinc_options_defaults(opt);
    return boinc_init_options_graphics_lib(opt, worker, argv0);
}

int boinc_init_options_graphics_lib(
    BOINC_OPTIONS& opt, WORKER_FUNC_PTR worker, char* argv0
) {
    char graphics_lib[MAXPATHLEN];
    char resolved_name[MAXPATHLEN];
    char *ptr;
    int retval;
    char *errormsg;
    BIOGI_FUNC_PTR boinc_init_options_graphics_impl_hook;

    boinc_main_state.boinc_init_options_general_hook = boinc_init_options_general;
    boinc_main_state.boinc_is_standalone_hook = boinc_is_standalone;
    boinc_main_state.boinc_get_init_data_hook = boinc_get_init_data;
    boinc_main_state.start_timer_thread_hook = start_timer_thread;
    boinc_main_state.app_client_shmp = &app_client_shm;

    // figure out name of executable, and append .so
    //
    if ((ptr = strrchr(argv0, '/'))) {
        ptr++;
    } else {
        ptr = argv0;
    }
    strlcpy(graphics_lib, ptr, sizeof(graphics_lib));
    strlcat(graphics_lib, ".so", sizeof(graphics_lib));

    // boinc-resolve library name: it could be a XML symlink
    //
    if (boinc_resolve_filename(graphics_lib, resolved_name, MAXPATHLEN)) {
        fprintf(stderr,
            "Unable to boinc_resolve name of shared object file %s\n",
            graphics_lib
        );
        goto no_graphics;
    }

    // if it's not a symlink, put "./" in front of it
    //
    if (!strcmp(graphics_lib, resolved_name)) {
        snprintf(resolved_name, sizeof(resolved_name), "./%s", graphics_lib);
    }

    // get handle for shared library.
    // This handle is a global variable, so it can be declared 'extern'
    // in worker() and thus worker() has access to functions
    // from within this shared library
    //
    graphics_lib_handle = dlopen(resolved_name,  RTLD_NOW);
    if (!graphics_lib_handle) {
        errormsg = (char*)dlerror();
        fprintf(stderr,
            "dlopen() failed: %s\nNo graphics.\n", errormsg?errormsg:""
        );
        goto no_graphics;
    }

    // use handle from shared library to resolve the 'initialize
    // graphics' routine from shared library
    //
    boinc_init_options_graphics_impl_hook = (BIOGI_FUNC_PTR) dlsym(
        graphics_lib_handle,
        "boinc_init_options_graphics_impl"
    );
    if (!boinc_init_options_graphics_impl_hook) {
        errormsg = (char*)dlerror();
        fprintf(stderr,
            "dlsym(): no boinc_init_options_graphics_impl() in %s\n%s\n",
            resolved_name, errormsg?errormsg:""
        );
        goto no_graphics;
    }

    // here's where we start the graphics thread and the worker thread.
    // Normally this function should not return.
    //
    retval = boinc_init_options_graphics_impl_hook(
        opt, worker, &boinc_main_state
    );

    if (retval) {
        fprintf(stderr,
            "boinc_init_options_graphics_impl() returned %d: unable to create worker thread\n",
            retval
        );
    }

    boinc_finish(retval);
    // never get here...
    return 1;

no_graphics:
    // unable to resolve the shared object file, or unable to resolve
    // library dependencies on machine (eg, no X11, no GL libraries,
    // etc) or unable to find needed symbol in library
    //
    boinc_init_options(&opt);
    worker();

    // worker() should call boinc_finish so we should NEVER get here!
    //
    boinc_finish(1);
    // never get here...
    return 1;
}

bool boinc_graphics_possible() {
#ifdef _WIN32
  // ???? should not be here
#elif defined(__APPLE__)
  // ???? should not be here
#else
    if (!getenv("DISPLAY")) return false;
#endif
    return true;
}
