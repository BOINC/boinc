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

// a C++ interface to BOINC GUI RPC

#ifndef _WIN32
#include <stdio.h>
#include <string>
#include <vector>
#endif

#include "miofile.h"

#define GUI_RPC_PORT 31416

#define RUN_MODE_ALWAYS 0
#define RUN_MODE_NEVER  1
#define RUN_MODE_AUTO   2

struct GUI_URL {
    std::string name;
    std::string description;
    std::string url;

    int parse(MIOFILE&);
    void print();
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
    int min_rpc_time;           // earliest time to contact any server

    bool master_url_fetch_pending; // need to fetch and parse the master URL
    bool sched_rpc_pending;     // contact scheduling server for preferences
    bool tentative;             // master URL and account ID not confirmed
    bool suspended_via_gui;

    PROJECT();
    ~PROJECT();

    int parse(MIOFILE&);
    void print();
    void clear();
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
    // the following defined if active
    bool active_task;
    int active_task_state;
    int app_version_num;
    double checkpoint_cpu_time;
    double current_cpu_time;
    double fraction_done;
    double vm_size;
    double estimated_cpu_time_remaining;
    bool suspended_via_gui;
    bool supports_graphics;

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

class PROXY_INFO {
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

    PROXY_INFO();
    ~PROXY_INFO();

    int parse(MIOFILE&);
    void print();
    void clear();
};

struct HOST_INFO {
    int timezone;    // seconds added to local time to get UTC
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
    void clear();
};

class CC_STATE {
public:
    std::vector<PROJECT*> projects;
    std::vector<APP*> apps;
    std::vector<APP_VERSION*> app_versions;
    std::vector<WORKUNIT*> wus;
    std::vector<RESULT*> results;

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

class RPC_CLIENT {
    int sock;
public:
    int send_request(char*);
    int get_reply(char*&);

public:

    RPC_CLIENT();
    ~RPC_CLIENT();
    int init(char* host);
    void close();
    int get_state(CC_STATE&);
    int get_results(RESULTS&);
    int get_file_transfers(FILE_TRANSFERS&);
    int get_project_status(PROJECTS&);
    int get_disk_usage(PROJECTS&);
    int show_graphics(const char* project, const char* result_name, bool full_screen);
    int project_op(PROJECT&, char* op);
    int project_attach(char* url, char* auth);
    int set_run_mode(int mode);
    int get_run_mode(int& mode);
    int set_network_mode(int mode);
    int get_network_mode(int& mode);
    int run_benchmarks();
    int set_proxy_settings(PROXY_INFO&);
    int get_proxy_settings(PROXY_INFO&);
    int get_messages(int seqno, MESSAGES&);
    int file_transfer_op(FILE_TRANSFER&, char*);
    int result_op(RESULT&, char*);
    int get_host_info(HOST_INFO&);
    char* mode_name(int mode);
};

struct RPC {
    char* mbuf;
    MIOFILE fin;
    RPC_CLIENT* rpc_client;

    RPC(RPC_CLIENT*);
    ~RPC();
    int do_rpc(char*);
};
