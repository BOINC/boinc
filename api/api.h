// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include <stdio.h>

#ifndef BOINC_API
#define BOINC_API

// MFILE supports a primitive form of checkpointing.
// Write all your output (and restart file) to MFILEs.
// The output is buffered in memory.
// Then close all the MFILEs;
// all the buffers will be flushed to disk, almost atomically.

class MFILE {
    char* buf;
    int len;
    FILE* f;
public:
    int open(char* path, char* mode);
    int _putchar(char);
    int puts(char*);
    int printf(char* format, ...);
    size_t write(const void *,size_t,size_t);
    int close();
    int flush();
};

// An application that wants to be well-behaved should do the following:
//
// - call boinc_init() at startup
// - call boinc_time() periodically.
//   This is cheap - it gets the time of day.
// - checkpoint as often as requested by core
// - boinc_poll(): 
//   Call this as often as requested by core

struct APP_IN_GRAPHICS {
    int xsize;
    int ysize;
    double refresh_period;
    char shmem_seg_name[32];
};

struct APP_OUT_GRAPHICS {
};

struct APP_IN {
    char app_preferences[4096];
    APP_IN_GRAPHICS graphics;
    double checkpoint_period;     // recommended checkpoint period
    double poll_period;           // recommended poll period
};

struct APP_OUT {
    double percent_done;
    double cpu_time_at_checkpoint;
    bool checkpointed;              // true iff checkpointed since last call
};

void write_core_file(FILE* f, APP_IN &ai);
void boinc_init(APP_IN&);
double boinc_time();
void boinc_poll(APP_IN&, APP_OUT&);
double boinc_cpu_time();

#define CORE_TO_APP_FILE    "core_to_app.xml"
#define APP_TO_CORE_FILE    "app_to_core.xml"

#endif
