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

#include <vector>
#include <string>

#include "hostinfo.h"

// Communication between the core client and the BOINC app library.
// This code is linked into both core client and app lib.
//
// Some apps may involve separate "coordinator" and "worker" programs.
// The coordinator runs one or more worker programs in sequence,
// and don't do work themselves.
//
// Includes the following:
// - shared memory (APP_CLIENT_SHM)
// - main init file
// - fd init file
// - graphics init file
// - conversion of symbolic links

// Shared memory is a set of MSG_CHANNELs.
// First byte of a channel is nonzero if
// the channel contains an unread data.
// This is set by the sender and cleared by the receiver.
// The sender doesn't write if the flag is set.
// Remaining 1023 bytes contain data.
//
#define MSG_CHANNEL_SIZE 1024

struct MSG_CHANNEL {
    char buf[MSG_CHANNEL_SIZE];
    bool get_msg(char*);    // returns a message and clears pending flag
    bool send_msg(char*);   // if there is not a message in the segment,
                            // writes specified message and sets pending flag
    void send_msg_overwrite(char*);
                            // write message, overwriting any msg already there
};

struct SHARED_MEM {
    MSG_CHANNEL process_control_request;
        // core->app
        // <quit/>
        // <suspend/>
        // <resume/>
    MSG_CHANNEL process_control_reply;
        // app->core
        // same as above
    MSG_CHANNEL graphics_request;
        // core->app
        // request a graphics mode:
        // <mode_hide_graphics/>
        // ...
        // <mode_blankscreen/>
    MSG_CHANNEL graphics_reply;
        // app->core
        // same as above
    MSG_CHANNEL heartbeat;
        // core->app
        // <heartbeat/>             sent every second, even while app is suspended
        // <disable_heartbeat/>     disables heartbeat mechanism
        // <enable_heartbeat/>      enable heartbeat mechanism (default)
    MSG_CHANNEL app_status;
        // app->core
        // status message every second, of the form
        // <current_cpu_time>...
        // <checkpoint_cpu_time>...
        // <working_set_size>...
        // <fraction_done> ...
    MSG_CHANNEL trickle_up;
        // app->core
        // <have_new_trickle_up/>
    MSG_CHANNEL trickle_down;
        // core->app
        // <have_new_trickle_down/>
};

struct MSG_QUEUE {
    std::vector<std::string> msgs;
    char name[256];
    void msg_queue_send(char*, MSG_CHANNEL& channel);
    void msg_queue_poll(MSG_CHANNEL& channel);
};

#define DEFAULT_FRACTION_DONE_UPDATE_PERIOD     1
#define DEFAULT_CHECKPOINT_PERIOD               300

#define SHM_PREFIX          "shm_"
#define QUIT_PREFIX         "quit_"

// graphics messages
//
#define MODE_UNSUPPORTED        0
#define MODE_HIDE_GRAPHICS      1
#define MODE_WINDOW             2
#define MODE_FULLSCREEN         3
#define MODE_BLANKSCREEN        4
#define MODE_REREAD_PREFS       5

#define NGRAPHICS_MSGS  6

#include <cstdio>

class APP_CLIENT_SHM {
public:
    SHARED_MEM *shm;

    int decode_graphics_msg(char*);
    void reset_msgs();        // resets all messages and clears their flags

    APP_CLIENT_SHM();
};

#ifdef _WIN32
    typedef char SHMEM_SEG_NAME[256];
#else
    typedef int SHMEM_SEG_NAME;
#endif

// parsed version of main init file
//
struct APP_INIT_DATA {
    int core_version;
    char app_name[256];
    char project_preferences[65536];
    char user_name[256];
    char team_name[256];
    char project_dir[256];
    char boinc_dir[256];
    char wu_name[256];
    char authenticator[256];
    int slot;
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    HOST_INFO host_info;

    // Items below here are for implementation only
    // (not used by app developers)
    //
    double checkpoint_period;     // recommended checkpoint period
    SHMEM_SEG_NAME shmem_seg_name;
    double wu_cpu_time;       // cpu time from previous episodes
    double fraction_done_update_period;
        // the following 2 are used for compound apps,
        // where each stage of the computation is a fixed
        // fraction of the total.
    double fraction_done_start;
    double fraction_done_end;
};

struct GRAPHICS_INFO {
    int xsize;
    int ysize;
    double refresh_period;
};

typedef struct GRAPHICS_INFO GRAPHICS_INFO;

int write_init_data_file(FILE* f, APP_INIT_DATA&);
int parse_init_data_file(FILE* f, APP_INIT_DATA&);
int write_graphics_file(FILE* f, GRAPHICS_INFO* gi);
int parse_graphics_file(FILE* f, GRAPHICS_INFO* gi);

// filenames used in the slot directory
//
#define INIT_DATA_FILE    "init_data.xml"
#define GRAPHICS_DATA_FILE    "graphics.xml"
#define BOINC_FINISH_CALLED_FILE "boinc_finish_called"
#define TRICKLE_UP_FILENAME "trickle_up.xml"
#define STDERR_FILE           "stderr.txt"
#define STDOUT_FILE           "stdout.txt"
#define LOCKFILE               "boinc_lockfile"

extern char* xml_graphics_modes[NGRAPHICS_MSGS];
int boinc_link(const char* existing, const char* new_link);

#endif
