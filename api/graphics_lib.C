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

int boinc_init_graphics_lib(void (*worker)(), char* libname) {
    dlopen(libname, RTLD_GLOBAL);
    // look up boinc_init_graphics_impl; call it
}
