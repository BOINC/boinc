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

#ifndef _APP_IPC_
#define _APP_IPC_

// Communication between the core client and the BOINC app library.
// This code is linked into both core client and app lib.
// Includes the following:
// - shared memory (APP_CLIENT_SHM)
// - main init file
// - fd init file
// - graphics init file
// - conversion of symbolic links

// Shared memory is arranged as follows:
// 4 1K segments
// First byte of each segment is nonzero if
// segment contains unread data, remaining 1023
// bytes contain data

#define SHM_SEG_SIZE            1024
#define NUM_SEGS                4
#define CORE_APP_WORKER_SEG     0
#define APP_CORE_WORKER_SEG     1
#define CORE_APP_GFX_SEG        2
#define APP_CORE_GFX_SEG        3
#define APP_CLIENT_SHMEM_SIZE   (sizeof(char)*NUM_SEGS*SHM_SEG_SIZE)

#define DEFAULT_FRACTION_DONE_UPDATE_PERIOD     1
#define DEFAULT_CHECKPOINT_PERIOD               300

#define SHM_PREFIX          "shm_"
#define QUIT_PREFIX         "quit_"

// graphics modes of an application
//
#define MODE_UNSUPPORTED        0
#define MODE_HIDE_GRAPHICS      1
#define MODE_WINDOW             2
#define MODE_FULLSCREEN         3

// graphics messages
//
#define GRAPHICS_MSG_SET_MODE       1
#define GRAPHICS_MSG_REREAD_PREFS   2


class APP_CLIENT_SHM {
public:
    char *shm;

    bool pending_msg(int);    // returns true if a message is waiting
                              // in the specified segment
    bool get_msg(char*, int); // returns the message from the specified
                              // segment and clears pending flag
    bool send_msg(char*, int); // if there is not a message in the segment,
                              // writes specified message and sets pending flag
    bool send_graphics_msg(int seg, int msg, int mode);
    bool get_graphics_msg(int seg, int& msg, int& mode);
    void reset_msgs();        // resets all messages and clears their flags
    void reset_msg(int);      // resets specified message and clears its flag
};

// parsed version of main init file
//
struct APP_INIT_DATA {
    char app_name[256];
    char app_preferences[4096];
    char user_name[256];
    char team_name[256];
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;

    // Items below here are for implementation only
    // (not used by app writers)
    //
    double checkpoint_period;     // recommended checkpoint period
    int shm_key;
    char comm_obj_name[256];  // identifies shared memory segments, signals, etc
    double wu_cpu_time;       // cpu time from previous sessions
    double fraction_done_update_period;
};

struct GRAPHICS_INFO {
    int xsize;
    int ysize;
    double refresh_period;
};

typedef struct GRAPHICS_INFO GRAPHICS_INFO;

int write_init_data_file(FILE* f, APP_INIT_DATA&);
int parse_init_data_file(FILE* f, APP_INIT_DATA&);
int write_fd_init_file(FILE*, char*, int, bool);
int parse_fd_init_file(FILE*);
int write_graphics_file(FILE* f, GRAPHICS_INFO* gi);
int parse_graphics_file(FILE* f, GRAPHICS_INFO* gi);

#define INIT_DATA_FILE    "init_data.xml"
#define GRAPHICS_DATA_FILE    "graphics.xml"
#define FD_INIT_FILE    "fd_init.xml"

#define END_SS_MSG            "BOINC_SS_END"

#define STDERR_FILE           "stderr.txt"

extern char* xml_graphics_modes[5];
int boinc_link(const char* existing, const char* new_link);

#endif
