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
#include "graphics_lib.h"

// for prototype of boinc_init_options_general(), boinc_init, boinc_finish
#include "boinc_api.h"  

#define BOINC_STRLEN 512

// Many applications will NOT want to call this routine.  Instead
// they'll want to cut and paste this code into their own application.
// This is because they will want to use 'handle' below to resolve
// functions that are included in the apps shared library, which
// communicate information about the work in progress to the
// app_graphics_render() routine, and other graphics routines.


// This routine never returns. If a problem arises, it calls
// boinc_finish(nonzero).
//
// First argument: worker function
//
// Second argument: argv[0] from command line arguments.  This is the
// executable name, and is used to derive the shared object library
// name: executable_name.so

int boinc_init_graphics_lib(void (*worker)(), char* argv0) {

  // name of shared object library: same as executable_name.so
  char graphics_lib[BOINC_STRLEN];
  // boinc-resolved version of the same
  char resolved_name[BOINC_STRLEN];
  char *ptr;
  void *handle;
  int retval;
  char *errormsg;

  // DAVID -- WARNING -- TO BE USABLE IN C YOU NEED TO GIVE
  // boinc_init_options_general() A PURE C PROTOTYPE NOT C++!  THIS
  // CURRENTLY WON'T WORK FOR E@h.
  int (*boinc_init_graphics_impl_hook)(void (*worker)(), int (*init_options)(BOINC_OPTIONS& opt));

  /* figure out name of executable, and append .so */
  if ((ptr = strrchr(argv0, '/')))
    ptr++;
  else
    ptr = argv0;
  strncat(graphics_lib, ".so", BOINC_STRLEN);
  graphics_lib[BOINC_STRLEN-1]='\0';
  
  /* boinc-resolve library name: it could be a XML symlink */
  if (boinc_resolve_filename(graphics_lib, resolved_name, BOINC_STRLEN)) {
#if 0
    // for debugging
    fprintf(stderr, "Unable to boinc_resolve name of shared object file %s\n", graphics_lib);
#endif
    goto no_graphics;
  }
  
  // now get handle for shared library
  if (!(handle = dlopen(resolved_name,  RTLD_NOW))) {
#if 0
    // for debugging
    errormsg=dlerror();
    fprintf (stderr, "dlopen() failed: %s\nNo graphics.\n", ?errormsg:errormsg:"");
#endif
      goto no_graphics;
    }
    
    if (!(boinc_init_graphics_impl_hook = dlsym(handle,"boinc_init_graphics_impl"))) {
#if 0
      // for debugging
      errormsg=dlerror();
      fprintf(stderr, "dlsym() couldn't find boinc_init_graphics_impl in %s\n", resolved_name);
#endif
      goto no_graphics;
    }

#if 0
    // Applications that wish to make use of functions in the shared
    // library, to communicate data from the worker function to the
    // graphics rendinging functions should paste them in there:
    myfunction1_hook=dlsym("myfunction1");
    myfunction2_hook=dlsym("myfunction2");
    myfunction3_hook=dlsym("myfunction3");
#endif

    // this should never return
    retval = boinc_init_graphics_impl_hook(worker, boinc_init_options_general);
    
    if (retval) {
#if 0
      fprintf(stderr,"boinc_init_graphics_impl() returned %d: unable to create worker thread\n", retval);
#endif
    }
    
    boinc_finish(1+retval);
   
 no_graphics:
    // unable to resolve the shared object file, or unable to resolve
    // dependencies on machine, or unable to find needed symbol in
    // library
    boinc_init();
    worker();
    
    // worker() should call boinc_finish so we should NEVER get here!
    boinc_finish(1);
}

