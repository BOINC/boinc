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
#ifdef unix
#include <sys/time.h>
#include <sys/resource.h>
#endif

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
// - call boinc_init(APP_IN&) at startup
// - call time_to_checkpoint() often.
//   This is cheap - it returns true if time to checkpoint.
// - checkpoint as often as requested by core
// - call checkpoint_completed() when checkpoint is complete
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
    double cpu_time;		  // cpu time from previous sessions
};

struct APP_OUT {
    double percent_done;
    double cpu_time_at_checkpoint;
    bool checkpointed;              // true iff checkpointed since last call
};

void write_core_file(FILE* f, APP_IN &ai);
void parse_core_file(FILE* f, APP_IN&);
void write_init_file(FILE* f, char *file_name, int fdesc, int input_file );
void parse_init_file(FILE* f);
void boinc_init(APP_IN&);
void parse_app_file(FILE* f, APP_OUT&);
void write_app_file(FILE* f, APP_OUT&);
void boinc_poll(APP_IN&, APP_OUT&);
double boinc_cpu_time();
int boinc_resolve_link(char *file_name, char *resolved_name);

#define CORE_TO_APP_FILE    "core_to_app.xml"
#define APP_TO_CORE_FILE    "app_to_core.xml"
#define BOINC_INIT_FILE     "boinc_init.xml"

//the following are provided for implementation of the checkpoint system
int checkpoint_completed(APP_OUT& ao);
int app_completed(APP_OUT& ao);

extern bool _checkpoint;
#define time_to_checkpoint() _checkpoint
int set_timer(double period); //period is seconds spent in process
void on_timer(int not_used);
double get_cpu_time(); //return cpu time for this process

#endif
