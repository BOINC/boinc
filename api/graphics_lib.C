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

// An interface to BOINC graphics in which
// the graphics code lives in a separate shared library.
// This lets you make applications that work whether or not
// the host has X11 and OpenGL libraries.

#include <dlfcn.h>

#include "boinc_api.h"  
#include "graphics_lib.h"

#define BOINC_STRLEN 512

void* graphics_lib_handle;

typedef int (*BIOGI_FUNC)(BOINC_OPTIONS&, void(*worker)(), int (*)(BOINC_OPTIONS&));

// This routine never returns.
// If a problem arises, it calls boinc_finish(nonzero).
//
// First argument: worker function
//
// Second argument: argv[0] from command line arguments.
// This is the executable name, and is used to derive
// the shared object library name: executable_name.so

int boinc_init_graphics_lib(void (*worker)(), char* argv0) {
    BOINC_OPTIONS opt;
    options_defaults(opt);
    return boinc_init_options_graphics_lib(opt, worker, argv0);
}

int boinc_init_options_graphics_lib(
    BOINC_OPTIONS& opt, void (*worker)(), char* argv0
) {
    char graphics_lib[BOINC_STRLEN];
    char resolved_name[BOINC_STRLEN];
    char *ptr;
    void *handle;
    int retval;
    char *errormsg;

    BIOGI_FUNC boinc_init_options_graphics_impl_hook;

    // figure out name of executable, and append .so
    //
    if ((ptr = strrchr(argv0, '/'))) {
        ptr++;
    } else {
        ptr = argv0;
    }
    strcpy(graphics_lib, ptr);
    strncat(graphics_lib, ".so", BOINC_STRLEN);
    graphics_lib[BOINC_STRLEN-1] = 0;
  
    // boinc-resolve library name: it could be a XML symlink
    //
    if (boinc_resolve_filename(graphics_lib, resolved_name, BOINC_STRLEN)) {
        fprintf(stderr,
            "Unable to boinc_resolve name of shared object file %s\n",
            graphics_lib
        );
        goto no_graphics;
    }
  
    // get handle for shared library
    //
    graphics_lib_handle = dlopen(resolved_name,  RTLD_NOW);
    if (!graphics_lib_handle) {
        errormsg = dlerror();
        fprintf(stderr,
            "dlopen() failed: %s\nNo graphics.\n", errormsg?errormsg:""
        );
        goto no_graphics;
    }
    
    boinc_init_options_graphics_impl_hook = (BIOGI_FUNC) dlsym(
        graphics_lib_handle,
        "boinc_init_options_graphics_impl"
    );
    if (!boinc_init_options_graphics_impl_hook) {
        errormsg = dlerror();
        fprintf(stderr,
            "dlsym() couldn't find boinc_init_options_graphics_impl in %s\n",
            resolved_name
        );
        goto no_graphics;
    }

    retval = boinc_init_options_graphics_impl_hook(
        opt, worker, boinc_init_options_general
    );
    
    if (retval) {
        fprintf(stderr,
            "boinc_init_options_graphics_impl() returned %d: unable to create worker thread\n",
            retval
        );
    }
    
    boinc_finish(retval);
   
no_graphics:
    // unable to resolve the shared object file, or unable to resolve
    // dependencies on machine, or unable to find needed symbol in library
    //
    boinc_init_options(opt);
    worker();
    
    // worker() should call boinc_finish so we should NEVER get here!
    //
    boinc_finish(1);
    return 1;
}

