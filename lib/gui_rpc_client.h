// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// a C++ interface to BOINC GUI RPC

#ifndef _WIN32
#include <stdio.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "miofile.h"
#include "prefs.h"
#include "hostinfo.h"

#define GUI_RPC_PORT                                1043
#define GUI_RPC_PORT_ALT                            31416

#define RUN_MODE_ALWAYS                             0
#define RUN_MODE_NEVER                              1
#define RUN_MODE_AUTO                               2

#define RESULT_NEW                                  0
#define RESULT_FILES_DOWNLOADING                    1
#define RESULT_FILES_DOWNLOADED                     2
#define RESULT_COMPUTE_ERROR                        3
#define RESULT_FILES_UPLOADING                      4
#define RESULT_FILES_UPLOADED                       5

#define CPU_SCHED_UNINITIALIZED                     0
#define CPU_SCHED_PREEMPTED                         1
#define CPU_SCHED_SCHEDULED                         2

// see client/ss_logic.h for explanation
//
#define SS_STATUS_ENABLED                           1
#define SS_STATUS_RESTARTREQUEST                    2
#define SS_STATUS_BLANKED                           3
#define SS_STATUS_BOINCSUSPENDED                    4
#define SS_STATUS_NOAPPSEXECUTING                   6
#define SS_STATUS_NOGRAPHICSAPPSEXECUTING           7
#define SS_STATUS_QUIT                              8
#define SS_STATUS_NOPROJECTSDETECTED                9

// These MUST match the constants in client/client_msgs.h

#define MSG_PRIORITY_INFO               1
    // show message in black
#define MSG_PRIORITY_ERROR              2
    // show message in red
#define MSG_PRIORITY_ALERT_INFO              4
    // show message in a modal dialog
#define MSG_PRIORITY_ALERT_ERROR              5
    // show error message in a modal dialog

struct GUI_URL {
    std::string name;
    std::string description;
    std::string url;

    int parse(MIOFILE&);
    void print();
};

// statistics at a specific day
//
struct STATISTIC {
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double day;
};


class PROJECT {
public:
    std::string master_url;
    double resource_share;
    std::string project_name;
    std::string user_name;
    std::string team_name;
    std::vector<GUI_URL> gui_urls;
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;      // as reported by server
    double host_expavg_credit;     // as reported by server
    double disk_usage;
    int nrpc_failures;          // # of consecutive times we've failed to
                                // contact all scheduling servers
    int master_fetch_failures;
    double min_rpc_time;           // earliest time to contact any server

    bool master_url_fetch_pending; // need to fetch and parse the master URL
    bool sched_rpc_pending;     // contact scheduling server for preferences
    bool tentative;             // master URL and account ID not confirmed
    bool non_cpu_intensive;
    bool suspended_via_gui;
    bool dont_request_more_work;

    PROJECT();
    ~PROJECT();

    int parse(MIOFILE&);
    void print();
    void clear();
    void get_name(std::string&);

    // statistic of the last x days
    std::vector<STATISTIC> statistics;
};

class APP {
public:
    std::string name;
    PROJECT* project;

    APP();
    ~APP();

    int parse(MIOFILE&);
    void print();
    void clear();
};

class APP_VERSION {
public:
    std::string app_name;
    int version_num;
    APP* app;
    PROJECT* project;

    APP_VERSION();
    ~APP_VERSION();

    int parse(MIOFILE&);
    void print();
    void clear();
};

class WORKUNIT {
public:
    std::string name;
    std::string app_name;
    int version_num;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;

    WORKUNIT();
    ~WORKUNIT();

    int parse(MIOFILE&);
    void print();
    void clear();
};

class RESULT {
public:
    std::string name;
    std::string wu_name;
    std::string project_url;
    int report_deadline;
    bool ready_to_report;
    bool got_server_ack;
    double final_cpu_time;
    int state;
    int scheduler_state;
    int exit_status;
    int signal;
    std::string stderr_out;
    bool suspended_via_gui;
    bool aborted_via_gui;

    // the following defined if active
    bool active_task;
    int active_task_state;
    int app_version_num;
    double checkpoint_cpu_time;
    double current_cpu_time;
    double fraction_done;
    double vm_bytes;
    double rss_bytes;
    double estimated_cpu_time_remaining;
    bool supports_graphics;
    int graphics_mode_acked;

    APP* app;
    WORKUNIT* wup;
    PROJECT* project;

    RESULT();
    ~RESULT();

    int parse(MIOFILE&);
    void print();
    void clear();
};

class FILE_TRANSFER {
public:
    std::string name;
    std::string project_url;
    std::string project_name;
    double nbytes;
    bool generated_locally;
    bool uploaded;
    bool upload_when_present;
    bool sticky;
    bool pers_xfer_active;
    bool xfer_active;
    int num_retries;
    int first_request_time;
    int next_request_time;
    int status;
    double time_so_far;
    double bytes_xferred;
    double file_offset;
    double xfer_speed;
    std::string hostname;
    PROJECT* project;

    FILE_TRANSFER();
    ~FILE_TRANSFER();

    int parse(MIOFILE&);
    void print();
    void clear();
};

class MESSAGE {
public:
    std::string project;
    int priority;
    int seqno;
    int timestamp;
    std::string body;

    MESSAGE();
    ~MESSAGE();

    int parse(MIOFILE&);
    void print();
    void clear();
};

class GR_PROXY_INFO {
public:
    bool use_http_proxy;
    bool use_socks_proxy;
    bool use_http_authentication;
    int socks_version;
    std::string socks_server_name;
    std::string http_server_name;
    int socks_server_port;
    int http_server_port;
    std::string http_user_name;
    std::string http_user_passwd;
    std::string socks5_user_name;
    std::string socks5_user_passwd;

    GR_PROXY_INFO();
    ~GR_PROXY_INFO();

    int parse(MIOFILE&);
    void print();
    void clear();
};

#if 0
struct HOST_INFO {
    int timezone;    // local STANDARD time - UTC time (in seconds) 
    char domain_name[256];
    char serialnum[256];
    char ip_addr[256];

    int p_ncpus;
    char p_vendor[256];
    char p_model[256];
    double p_fpops;
    double p_iops;
    double p_membw;
    int p_fpop_err;
    int p_iop_err;
    int p_membw_err;
    double p_calculated; //needs to be initialized to zero

    char os_name[256];
    char os_version[256];

    double m_nbytes;
    double m_cache;
    double m_swap;

    double d_total;
    double d_free;

    HOST_INFO();
    ~HOST_INFO();

    int parse(MIOFILE&);
    void print();
    void clear();
};
#endif

class CC_STATE {
public:
    std::vector<PROJECT*> projects;
    std::vector<APP*> apps;
    std::vector<APP_VERSION*> app_versions;
    std::vector<WORKUNIT*> wus;
    std::vector<RESULT*> results;

    GLOBAL_PREFS global_prefs;

    CC_STATE();
    ~CC_STATE();

    PROJECT* lookup_project(std::string&);
    APP* lookup_app(std::string&, std::string&);
    APP* lookup_app(PROJECT*, std::string&);
    APP_VERSION* lookup_app_version(std::string&, std::string&, int);
    APP_VERSION* lookup_app_version(PROJECT*, std::string&, int);
    WORKUNIT* lookup_wu(std::string&, std::string&);
    WORKUNIT* lookup_wu(PROJECT*, std::string&);
    RESULT* lookup_result(std::string&, std::string&);
    RESULT* lookup_result(PROJECT*, std::string&);

    void print();
    void clear();
};

class PROJECTS {
public:
    std::vector<PROJECT*> projects;

    PROJECTS();
    ~PROJECTS();

    void print();
    void clear();
};

class RESULTS {
public:
    std::vector<RESULT*> results;

    RESULTS();
    ~RESULTS();

    void print();
    void clear();
};

class FILE_TRANSFERS {
public:
    std::vector<FILE_TRANSFER*> file_transfers;

    FILE_TRANSFERS();
    ~FILE_TRANSFERS();

    void print();
    void clear();
};

class MESSAGES {
public:
    std::vector<MESSAGE*> messages;

    MESSAGES();
    ~MESSAGES();

    void print();
    void clear();
};

struct DISPLAY_INFO {
    char window_station[256];   // windows
    char desktop[256];          // windows
    char display[256];          // X11

    DISPLAY_INFO();
    void print_str(char*);
};

struct ACCT_MGR_INFO {
    std::string acct_mgr_name;
    std::string acct_mgr_url;
    std::string login_name;
    std::string password;

    int parse(MIOFILE&);
};

struct PROJECT_CONFIG {
    int uses_email_id;
    std::string name;
    int min_passwd_length;

    int parse(MIOFILE&);
};

struct ACCOUNT_IN {
    std::string url;
    std::string email_addr;
    std::string user_name;
    std::string passwd_hash;
};

struct ACCOUNT_OUT {
    int error_num;
    std::string authenticator;

    int parse(MIOFILE&);
};

class RPC_CLIENT {
public:
    int sock;
    int client_version;
    bool tried_alt_port;
    double start_time;
    double timeout;
    bool retry;
    sockaddr_in addr;

    int send_request(const char*);
    int get_reply(char*&);
    RPC_CLIENT();
    ~RPC_CLIENT();
    int init(const char* host);
    int init_asynch(const char* host, double timeout, bool retry);
        // timeout == how long to wait until give up
        //    If the caller (i.e. BOINC Manager) just launched the core client,
        //    this should be large enough to allow the process to
        //    run and open its listening socket (e.g. 60 sec)
        //    If connecting to a remote client, it should be large enough
        //    for the user to deal with a "personal firewall" popup
        //    (e.g. 60 sec)
        // retry: if true, keep retrying (alternating between ports)
        //    until succeed or timeout.
        //    Use this if just launched the core client.
    int init_poll();
    void close();
    int authorize(const char* passwd);
    int get_state(CC_STATE&);
    int get_results(RESULTS&);
    int get_file_transfers(FILE_TRANSFERS&);
    int get_project_status(PROJECTS&);
    int get_disk_usage(PROJECTS&);
    int show_graphics(
        const char* project, const char* result_name, bool full_screen,
        DISPLAY_INFO&
    );
    int project_op(PROJECT&, const char* op);
    int project_attach(const char* url, const char* auth);
    int set_run_mode(int mode);
    int get_run_mode(int& mode);
    int set_network_mode(int mode);
    int get_network_mode(int& mode);
    int get_activity_state(bool& activities_suspended, bool& network_suspended);
    int get_screensaver_mode(int& status);
    int set_screensaver_mode(
        bool enabled, double blank_time, DISPLAY_INFO&
    );
    int run_benchmarks();
    int set_proxy_settings(GR_PROXY_INFO&);
    int get_proxy_settings(GR_PROXY_INFO&);
    int get_messages(int seqno, MESSAGES&);
    int file_transfer_op(FILE_TRANSFER&, const char*);
    int result_op(RESULT&, const char*);
    int get_host_info(HOST_INFO&);
    int quit();
    int acct_mgr_rpc(const char* url, const char* name, const char* passwd);
    int acct_mgr_info(ACCT_MGR_INFO&);
    const char* mode_name(int mode);
    int get_statistics(PROJECTS&);
    int network_query(int&);
    int network_available();

    // the following are asynch operations.
    // Make the first call to start the op,
    // call the second one periodically until it returns zero.
    // TODO: do project update and account manager RPC this way too
    //
    int get_project_config(std::string url);
    int get_project_config_poll(PROJECT_CONFIG&);
    int lookup_account(ACCOUNT_IN&);
    int lookup_account_poll(ACCOUNT_OUT&);
    int create_account(ACCOUNT_IN&);
    int create_account_poll(ACCOUNT_OUT&);
};

struct RPC {
    char* mbuf;
    MIOFILE fin;
    RPC_CLIENT* rpc_client;

    RPC(RPC_CLIENT*);
    ~RPC();
    int do_rpc(const char*);
    int parse_reply();
};
