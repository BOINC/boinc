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

#ifndef _BOINC_API_
#define _BOINC_API_

#include <stdio.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include "app_ipc.h"

// MFILE supports a primitive form of checkpointing.
// Write all your output (and restart file) to MFILEs.
// The output is buffered in memory.
// Then close or flush all the MFILEs;
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
    size_t write(const void *, size_t size, size_t nitems);
    int close();
    int flush();
    long tell() const;
};

extern int boinc_init(bool standalone = false);
extern int boinc_get_init_data(APP_INIT_DATA&);
extern int boinc_finish(int);
extern int boinc_resolve_filename(char*, char*, int len);
extern bool boinc_time_to_checkpoint();
extern int boinc_checkpoint_completed();
extern int boinc_fraction_done(double);
//extern void boinc_mask();
//extern void boinc_unmask();
extern int boinc_child_start();
extern int boinc_child_done(double);
bool boinc_is_standalone();

/////////// API ENDS HERE - IMPLEMENTATION STUFF FOLLOWS

extern int boinc_cpu_time(double&, double&);
    // CPU time and memory usage for this process
extern int boinc_install_signal_handlers();
extern int update_app_progress(double, double, double, double);

extern int set_timer(double period);
extern void setup_shared_mem();
extern void cleanup_shared_mem();

extern APP_CLIENT_SHM *app_client_shm;

#endif
