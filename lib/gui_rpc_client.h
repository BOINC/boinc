// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// a C++ interface to BOINC GUI RPC

#ifndef _GUI_RPC_CLIENT_H_
#define _GUI_RPC_CLIENT_H_

#if !defined(_WIN32) || defined (__CYGWIN__)
#include <cstdio>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <locale.h>
#endif

#include "miofile.h"
#include "prefs.h"
#include "hostinfo.h"
#include "common_defs.h"
#include "notice.h"
#include "network.h"
#include "cc_config.h"

struct GUI_URL {
    std::string name;
    std::string description;
    std::string url;

    int parse(XML_PARSER&);
    void print();
};

// statistics at a specific day
//
struct DAILY_STATS {
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double day;

    int parse(XML_PARSER&);
};


struct PROJECT_LIST_ENTRY {
    std::string name;
    std::string url;
    std::string general_area;
    std::string specific_area;
    std::string description;
    std::string home;       // sponsoring organization
    std::string image;      // URL of logo
    std::vector<std::string> platforms;
        // platforms supported by project, or empty

    PROJECT_LIST_ENTRY();
    ~PROJECT_LIST_ENTRY();

    int parse(XML_PARSER&);
    void clear();
};

struct AM_LIST_ENTRY {
    std::string name;
    std::string url;
    std::string description;
    std::string image;

    AM_LIST_ENTRY();
    ~AM_LIST_ENTRY();

    int parse(XML_PARSER&);
    void clear();
};

struct ALL_PROJECTS_LIST {
    std::vector<PROJECT_LIST_ENTRY*> projects;
    std::vector<AM_LIST_ENTRY*> account_managers;

    ALL_PROJECTS_LIST();
    ~ALL_PROJECTS_LIST();

    void clear();
    void shuffle();
};

struct RSC_DESC {
    double backoff_time;
    double backoff_interval;
    bool no_rsc_ams;
    bool no_rsc_apps;
    bool no_rsc_pref;
    bool no_rsc_config;

    void clear();
};

struct PROJECT {
    char master_url[256];
    double resource_share;
    std::string project_name;
    std::string user_name;
    std::string team_name;
    int hostid;
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
    double download_backoff;
    double upload_backoff;

    RSC_DESC rsc_desc_cpu;
    RSC_DESC rsc_desc_nvidia;
    RSC_DESC rsc_desc_ati;

    double sched_priority;

    double duration_correction_factor;

    bool anonymous_platform;
    bool master_url_fetch_pending; // need to fetch and parse the master URL
    int sched_rpc_pending;      // need to contact scheduling server
        // encodes the reason for the request
    bool non_cpu_intensive;
    bool suspended_via_gui;
    bool dont_request_more_work;
    bool scheduler_rpc_in_progress;
    bool attached_via_acct_mgr;
    bool detach_when_done;
    bool ended;
    bool trickle_up_pending;
    double project_files_downloaded_time;
        // when the last project file download was finished
        // (i.e. the time when ALL project files were finished downloading)
    double last_rpc_time;
        // when the last successful scheduler RPC finished
    std::vector<DAILY_STATS> statistics; // credit data over the last x days
    char venue[256];

    // NOTE: if you add any data items above,
    // update parse(), and clear() to include them!!

    PROJECT();
    ~PROJECT();

    int parse(XML_PARSER&);
    void print();
    void print_disk_usage();
    void clear();
    void get_name(std::string&);

    // temp - keep track of whether or not this record needs to be deleted
    bool flag_for_delete;
};

struct APP {
    char name[256];
    char user_friendly_name[256];
    PROJECT* project;

    APP();
    ~APP();

    int parse(XML_PARSER&);
    void print();
    void clear();
};

struct APP_VERSION {
    char app_name[256];
    int version_num;
    char platform[64];
    char plan_class[64];
    double avg_ncpus;
    double ncudas;
    double natis;
    double gpu_ram;
    double flops;
    APP* app;
    PROJECT* project;

    APP_VERSION();
    ~APP_VERSION();

    int parse(XML_PARSER&);
    int parse_coproc(XML_PARSER&);
    void print();
    void clear();
};

struct WORKUNIT {
    char name[256];
    char app_name[256];
    int version_num;    // backwards compat
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;
    PROJECT* project;
    APP* app;

    WORKUNIT();
    ~WORKUNIT();

    int parse(XML_PARSER&);
    void print();
    void clear();
};

struct RESULT {
    char name[256];
    char wu_name[256];
    char project_url[256];
    int version_num;
    char plan_class[64];
    double report_deadline;
    double received_time;
    bool ready_to_report;
    bool got_server_ack;
    double final_cpu_time;
    double final_elapsed_time;
    int state;
    int scheduler_state;
    int exit_status;
    int signal;
    //std::string stderr_out;
    bool suspended_via_gui;
    bool project_suspended_via_gui;
    bool coproc_missing;
    bool scheduler_wait;
    char scheduler_wait_reason[256];
    bool network_wait;

    // the following defined if active
    bool active_task;
    int active_task_state;
    int app_version_num;
    int slot;
    int pid;
    double checkpoint_cpu_time;
    double current_cpu_time;
    double fraction_done;
    double elapsed_time;
    double swap_size;
    double working_set_size_smoothed;
    double estimated_cpu_time_remaining;
        // actually, estimated elapsed time remaining
    bool too_large;
    bool needs_shmem;
    bool edf_scheduled;
    char graphics_exec_path[MAXPATHLEN];
    char web_graphics_url[256];
    char remote_desktop_addr[256];
    char slot_path[MAXPATHLEN];
        // only present if graphics_exec_path is
    char resources[256];

    APP* app;
    WORKUNIT* wup;
    PROJECT* project;
    APP_VERSION* avp;

    RESULT();
    ~RESULT();

    int parse(XML_PARSER&);
    void print();
    void clear();
};

struct FILE_TRANSFER {
    std::string name;
    std::string project_url;
    std::string project_name;
    double nbytes;              // total # of bytes to be transferred
    bool uploaded;
    bool is_upload;
    bool generated_locally;     // deprecated; for compatibility w/ old clients
    bool sticky;
    bool pers_xfer_active;
    bool xfer_active;
    int num_retries;
    double first_request_time;
    double next_request_time;
    int status;
    double time_so_far;
    double bytes_xferred;
    double file_offset;
    double xfer_speed;
    std::string hostname;
    double project_backoff;
    PROJECT* project;

    FILE_TRANSFER();
    ~FILE_TRANSFER();

    int parse(XML_PARSER&);
    void print();
    void clear();
};

struct MESSAGE {
    std::string project;
    int priority;
    int seqno;
    int timestamp;
    std::string body;

    MESSAGE();
    ~MESSAGE();

    int parse(XML_PARSER&);
    void print();
    void clear();
};

// should match up with PROXY_INFO in proxy_info.h
//
struct GR_PROXY_INFO {
    bool use_http_proxy;
    bool use_http_authentication;
    std::string http_server_name;
    int http_server_port;
    std::string http_user_name;
    std::string http_user_passwd;

    bool use_socks_proxy;
    std::string socks_server_name;
    int socks_server_port;
    std::string socks5_user_name;
    std::string socks5_user_passwd;

	std::string noproxy_hosts;

    GR_PROXY_INFO();
    ~GR_PROXY_INFO();

    int parse(XML_PARSER&);
    void print();
    void clear();
};

struct CC_STATE {
    std::vector<PROJECT*> projects;
    std::vector<APP*> apps;
    std::vector<APP_VERSION*> app_versions;
    std::vector<WORKUNIT*> wus;
    std::vector<RESULT*> results;
    std::vector<std::string> platforms;
        // platforms supported by client
    GLOBAL_PREFS global_prefs;  // working prefs, i.e. network + override
    VERSION_INFO version_info;  // populated only if talking to pre-5.6 CC
    bool executing_as_daemon;   // true if client is running as a service / daemon
    HOST_INFO host_info;
    bool have_nvidia;           // redundant; include for compat (set by <have_cuda/>)
    bool have_ati;              // redundant; include for compat

    CC_STATE();
    ~CC_STATE();

    PROJECT* lookup_project(const char* url);
    APP* lookup_app(PROJECT*, const char* name);
    APP_VERSION* lookup_app_version(PROJECT*, APP*, int, char* plan_class);
    APP_VERSION* lookup_app_version_old(PROJECT*, APP*, int);
    WORKUNIT* lookup_wu(PROJECT*, const char* name);
    RESULT* lookup_result(PROJECT*, const char* name);
    RESULT* lookup_result(const char* url, const char* name);

    void print();
    void clear();
    int parse(XML_PARSER&);
};

struct PROJECTS {
    std::vector<PROJECT*> projects;

    PROJECTS(){}
    ~PROJECTS();

    void print();
    void clear();
};

struct DISK_USAGE {
    std::vector<PROJECT*> projects;
    double d_total;
    double d_free;
    double d_boinc;     // amount used by BOINC itself, not projects
    double d_allowed;   // amount BOINC is allowed to use, total

    DISK_USAGE(){clear();}
    ~DISK_USAGE();

    void print();
    void clear();
};

struct RESULTS {
    std::vector<RESULT*> results;

    RESULTS(){}
    ~RESULTS();

    void print();
    void clear();
};

struct FILE_TRANSFERS {
    std::vector<FILE_TRANSFER*> file_transfers;

    FILE_TRANSFERS();
    ~FILE_TRANSFERS();

    void print();
    void clear();
};

struct MESSAGES {
    std::vector<MESSAGE*> messages;

    MESSAGES();
    ~MESSAGES();

    void print();
    void clear();
};

struct NOTICES {
    bool complete;
        // whether vector contains all notices, or just new ones
    std::vector<NOTICE*> notices;

    NOTICES();
    ~NOTICES();

    void print();
    void clear();
};

struct ACCT_MGR_INFO {
    std::string acct_mgr_name;
    std::string acct_mgr_url;
    bool have_credentials;
    bool cookie_required;
    std::string cookie_failure_url;
    
    ACCT_MGR_INFO();
    ~ACCT_MGR_INFO(){}

    int parse(XML_PARSER&);
    void clear();
};

struct PROJECT_ATTACH_REPLY {
    int error_num;
    std::vector<std::string>messages;

    PROJECT_ATTACH_REPLY();
    ~PROJECT_ATTACH_REPLY(){}

    int parse(XML_PARSER&);
    void clear();
};

struct ACCT_MGR_RPC_REPLY {
    int error_num;
    std::vector<std::string>messages;

    ACCT_MGR_RPC_REPLY();
    ~ACCT_MGR_RPC_REPLY(){}

    int parse(XML_PARSER&);
    void clear();
};

struct PROJECT_INIT_STATUS {
    std::string url;
    std::string name;
    std::string team_name;
    bool has_account_key;

    PROJECT_INIT_STATUS();
    ~PROJECT_INIT_STATUS(){}

    int parse(XML_PARSER&);
    void clear();
};

struct PROJECT_CONFIG {
    int error_num;
    std::string name;
    std::string master_url;
    int local_revision;     // SVN changeset# of server software
    int min_passwd_length;
    bool account_manager;
    bool uses_username;     // true for WCG
    bool account_creation_disabled;
    bool client_account_creation_disabled;  // must create account on web
    bool sched_stopped;         // scheduler disabled
    bool web_stopped;           // DB-driven web functions disabled
    int min_client_version;
	std::string error_msg;
    std::string terms_of_use;
        // if present, show this text in an "accept terms of use?" dialog
        // before allowing attachment to continue.
    std::vector<std::string> platforms;
        // platforms supported by project, or empty

    PROJECT_CONFIG();
    ~PROJECT_CONFIG();

    int parse(XML_PARSER&);
    void clear();
    void print();
};

struct ACCOUNT_IN {
    std::string url;
    std::string email_addr;
        // the account identifier (email address or user name)
    std::string user_name;
    std::string passwd;
    std::string team_name;

    ACCOUNT_IN();
    ~ACCOUNT_IN();

    void clear();
};

struct ACCOUNT_OUT {
    int error_num;
	std::string error_msg;
    std::string authenticator;

    ACCOUNT_OUT();
    ~ACCOUNT_OUT();

    int parse(XML_PARSER&);
    void clear();
    void print();
};

struct CC_STATUS {
    int network_status;         // values: NETWORK_STATUS_*
    bool ams_password_error;
    bool manager_must_quit;
    int task_suspend_reason;    // bitmap, see common_defs.h
    int task_mode;              // always/auto/never; see common_defs.h
    int task_mode_perm;			// same, but permanent version
	double task_mode_delay;		// time until perm becomes actual
    int gpu_suspend_reason;
    int gpu_mode;
    int gpu_mode_perm;
	double gpu_mode_delay;
    int network_suspend_reason;
    int network_mode;
    int network_mode_perm;
	double network_mode_delay;
    bool disallow_attach;
    bool simple_gui_only;

    CC_STATUS();
    ~CC_STATUS();

    int parse(XML_PARSER&);
    void clear();
    void print();
};

struct SIMPLE_GUI_INFO {
    std::vector<PROJECT*> projects;
    std::vector<RESULT*> results;
    void print();
};

struct DAILY_XFER {
    int when;
    double up;
    double down;

    int parse(XML_PARSER&);
};

struct DAILY_XFER_HISTORY {
    std::vector <DAILY_XFER> daily_xfers;
    int parse(XML_PARSER&);
    void print();
};

struct RPC_CLIENT {
    int sock;
    double start_time;
    double timeout;
    bool retry;
    sockaddr_storage addr;

    int send_request(const char*);
    int get_reply(char*&);
    RPC_CLIENT();
    ~RPC_CLIENT();
    int get_ip_addr(const char* host, int port);
    int init(const char* host, int port=0);
    int init_asynch(
        const char* host, double timeout, bool retry, int port=GUI_RPC_PORT
    );
        // timeout == how long to wait until give up
        //    If the caller (i.e. BOINC Manager) just launched the core client,
        //    this should be large enough to allow the process to
        //    run and open its listening socket (e.g. 60 sec)
        //    If connecting to a remote client, it should be large enough
        //    for the user to deal with a "personal firewall" popup
        //    (e.g. 60 sec)
        // retry: if true, keep retrying until succeed or timeout.
        //    Use this if just launched the core client.
    int init_poll();
    void close();
    int authorize(const char* passwd);
    int exchange_versions(VERSION_INFO&);
    int get_state(CC_STATE&);
    int get_results(RESULTS&, bool active_only = false);
    int get_file_transfers(FILE_TRANSFERS&);
    int get_simple_gui_info(SIMPLE_GUI_INFO&);
    int get_project_status(PROJECTS&);
    int get_all_projects_list(ALL_PROJECTS_LIST&);
    int get_disk_usage(DISK_USAGE&);
    int project_op(PROJECT&, const char* op);
    int set_run_mode(int mode, double duration);
        // if duration is zero, change is permanent.
        // otherwise, after duration expires,
        // restore last permanent mode
    int set_gpu_mode(int mode, double duration);
    int set_network_mode(int mode, double duration);
    int get_screensaver_tasks(int& suspend_reason, RESULTS&);
    int run_benchmarks();
    int set_proxy_settings(GR_PROXY_INFO&);
    int get_proxy_settings(GR_PROXY_INFO&);
    int get_messages(int seqno, MESSAGES&, bool translatable=false);
    int get_message_count(int& seqno);
    int get_notices(int seqno, NOTICES&);
    int get_notices_public(int seqno, NOTICES&);
    int file_transfer_op(FILE_TRANSFER&, const char*);
    int result_op(RESULT&, const char*);
    int get_host_info(HOST_INFO&);
    int quit();
    int acct_mgr_info(ACCT_MGR_INFO&);
    const char* mode_name(int mode);
    int get_statistics(PROJECTS&);
    int network_available();
    int get_project_init_status(PROJECT_INIT_STATUS& pis);

    // the following are asynch operations.
    // Make the first call to start the op,
    // call the second one periodically until it returns zero.
    // TODO: do project update
    //
    int get_project_config(std::string url);
    int get_project_config_poll(PROJECT_CONFIG&);
    int lookup_account(ACCOUNT_IN&);
    int lookup_account_poll(ACCOUNT_OUT&);
    int create_account(ACCOUNT_IN&);
    int create_account_poll(ACCOUNT_OUT&);
    int project_attach(
        const char* url, const char* auth, const char* project_name
    );
    int project_attach_from_file();
    int project_attach_poll(PROJECT_ATTACH_REPLY&);
    int acct_mgr_rpc(
        const char* url, const char* name, const char* passwd,
        bool use_config_file=false
    );
    int acct_mgr_rpc_poll(ACCT_MGR_RPC_REPLY&);

    int get_newer_version(std::string&, std::string&);
    int read_global_prefs_override();
    int read_cc_config();
    int get_cc_status(CC_STATUS&);
    int get_global_prefs_file(std::string&);
    int get_global_prefs_working(std::string&);
    int get_global_prefs_working_struct(GLOBAL_PREFS&, GLOBAL_PREFS_MASK&);
    int get_global_prefs_override(std::string&);
    int set_global_prefs_override(std::string&);
    int get_global_prefs_override_struct(GLOBAL_PREFS&, GLOBAL_PREFS_MASK&);
    int set_global_prefs_override_struct(GLOBAL_PREFS&, GLOBAL_PREFS_MASK&);
    int get_cc_config(CONFIG& config, LOG_FLAGS& log_flags);
    int set_cc_config(CONFIG& config, LOG_FLAGS& log_flags);
    int get_daily_xfer_history(DAILY_XFER_HISTORY&);
};

struct RPC {
    char* mbuf;
    MIOFILE fin;
    XML_PARSER xp;
    RPC_CLIENT* rpc_client;

    RPC(RPC_CLIENT*);
    ~RPC();
    int do_rpc(const char*);
    int parse_reply();
};

// We recommend using the XCode project under OS 10.5 to compile 
// the BOINC library, but some projects still use config & make, 
// so the following compatibility code avoids compiler errors when 
// building libboinc.a using config & make on system OS 10.3.9 or 
// with the OS 10.3.9 SDK (but using config & make is not recommended.)
//
#if defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4) && (!defined(BUILDING_MANAGER))
#define NO_PER_THREAD_LOCALE 1
#endif

// uselocal() API should be available on UNIX, Fedora & Ubuntu.
// For any platforms which do not support setting locale on a 
// per-thread basis, add code here similar to the following sample:
//#if defined(__UNIVAC__)
//#define NO_PER_THREAD_LOCALE 1
//#endif
#if defined(__HAIKU__)
#define NO_PER_THREAD_LOCALE 1
#endif


#ifdef NO_PER_THREAD_LOCALE  
    // Use this code for any platforms which do not support 
    // setting locale on a per-thread basis (see comment above)
 struct SET_LOCALE {
    std::string locale;
    inline SET_LOCALE() {
        locale = setlocale(LC_ALL, NULL);
        setlocale(LC_ALL, "C");
    }
    inline ~SET_LOCALE() {
        setlocale(LC_ALL, locale.c_str());
    }
};

#elif defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4)
// uselocale() is not available in OS 10.3.9 so use weak linking
#include <xlocale.h>
extern int		freelocale(locale_t) __attribute__((weak_import));
extern locale_t	newlocale(int, __const char *, locale_t) __attribute__((weak_import));
extern locale_t	uselocale(locale_t) __attribute__((weak_import));

struct SET_LOCALE {
    locale_t old_locale, RPC_locale;
    std::string locale;
    inline SET_LOCALE() {
        if (uselocale == NULL) {
            locale = setlocale(LC_ALL, NULL);
            setlocale(LC_ALL, "C");
        }
    }
    inline ~SET_LOCALE() {
        if (uselocale == NULL) {
            setlocale(LC_ALL, locale.c_str());
        }
    }
};

#else

struct SET_LOCALE {
    // Don't need to juggle locales if we have per-thread locale
    inline SET_LOCALE() {
    }
    inline ~SET_LOCALE() {
    }
};
#endif

extern int read_gui_rpc_password(char*);

#endif /* _GUI_RPC_CLIENT_H_ */
