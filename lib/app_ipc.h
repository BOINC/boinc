#ifndef _APP_IPC_
#define _APP_IPC_

// Communication between the core client and the BOINC app library.
// This code is linked into both core client and app lib.
// Includes the following:
// - shared memory (APP_CLIENT_SHM)
// - main init file
// - fd init file
// - graphics init file

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

class APP_CLIENT_SHM {
public:
    char *shm;

    bool pending_msg(int);    // returns true a message is waiting
                              // in the specified segment
    bool get_msg(char *,int);  // returns the message from the specified
                              // segment and resets the message flag
    bool send_msg(char *,int); // if there is not already a message in the segment,
                              // writes specified message and sets message flag
    bool send_graphics_mode_msg(int seg, int mode);
    bool get_graphics_mode_msg(int seg, int& mode);
    void reset_msgs();        // resets all messages and clears their flags
    void reset_msg(int);      // resets specified message and clears its flag
};

struct APP_INIT_DATA {
    char app_preferences[4096];
    char user_name[256];
    char team_name[256];
    char comm_obj_name[256];  // name to identify shared memory segments, signals, etc
    double wu_cpu_time;       // cpu time from previous sessions
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double checkpoint_period;     // recommended checkpoint period
    int shm_key;
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
int write_fd_init_file(FILE*, char*, int, int);
int parse_fd_init_file(FILE*);
int write_graphics_file(FILE* f, GRAPHICS_INFO* gi);
int parse_graphics_file(FILE* f, GRAPHICS_INFO* gi);

#define INIT_DATA_FILE    "init_data.xml"
#define GRAPHICS_DATA_FILE    "graphics.xml"
#define FD_INIT_FILE    "fd_init.xml"

// possible graphics modes of an application
//
#define MODE_UNSUPPORTED        0
#define MODE_HIDE_GRAPHICS      1
#define MODE_WINDOW             2
#define MODE_FULLSCREEN         3

#define END_SS_MSG            "BOINC_SS_END"

#define STDERR_FILE           "stderr.txt"

extern char* xml_graphics_modes[5];

#endif
