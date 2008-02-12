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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _FILE_NAMES_
#define _FILE_NAMES_

#include "client_types.h"
#include "common_defs.h"
#include "prefs.h"

// get the pathname (relative to client home dir) of a project file
//
extern void get_pathname(FILE_INFO* fip, char* path, int len);
extern void get_project_dir(PROJECT*, char*, int);

// get the pathname (relative to client home dir) of the
// directory used for a particular application "slot"
//
extern void get_slot_dir(int slot, char* path, int len);

extern int make_project_dir(PROJECT&);
extern int remove_project_dir(PROJECT&);
extern int make_slot_dir(int);
extern void delete_old_slot_dirs();
extern void get_account_filename(char* master_url, char* path);
extern bool is_account_file(const char*);
extern bool is_statistics_file(const char*);
extern void get_statistics_filename(char* master_url, char* path);
extern bool is_image_file(const char*);

extern void get_sched_request_filename(PROJECT&, char*, int len);
extern void get_sched_reply_filename(PROJECT&, char*, int len);
extern void get_master_filename(PROJECT&, char*, int len);
extern void job_log_filename(PROJECT&, char*, int);
extern void boinc_version_dir(PROJECT&, VERSION_INFO&, char*);
extern bool is_version_dir(char*, VERSION_INFO&);
extern void send_log_after(const char* filename, double t, MIOFILE& mf);

#define PROJECTS_DIR                "projects"
#define SLOTS_DIR                   "slots"
#define SWITCHER_DIR                "switcher"
#define STATE_FILE_NEXT             "client_state_next.xml"
#define STATE_FILE_NAME             "client_state.xml"
#define STATE_FILE_PREV             "client_state_prev.xml"
#define GLOBAL_PREFS_FILE_NAME      "global_prefs.xml"
#define GLOBAL_PREFS_OVERRIDE_FILE  "global_prefs_override.xml"
#define MASTER_BASE                 "master_"
#define SCHED_OP_REQUEST_BASE       "sched_request_"
#define SCHED_OP_REPLY_BASE         "sched_reply_"
#define CONFIG_FILE                 "cc_config.xml"
#define TEMP_FILE_NAME              "temp.xml"
#define STDERR_FILE_NAME            "stderr.txt"
#define STDOUT_FILE_NAME            "stdout.txt"
#define CPU_BENCHMARKS_FILE_NAME    "cpu_benchmarks"
#define APP_INFO_FILE_NAME			"app_info.xml"
#define REMOTEHOST_FILE_NAME        "remote_hosts.cfg"
#define ACCT_MGR_REQUEST_FILENAME   "acct_mgr_request.xml"
#define ACCT_MGR_REPLY_FILENAME     "acct_mgr_reply.xml"
#define GUI_RPC_PASSWD_FILE         "gui_rpc_auth.cfg"
#define PROJECT_INIT_FILENAME       "project_init.xml"
#define ACCT_MGR_URL_FILENAME       "acct_mgr_url.xml"
#define ACCT_MGR_LOGIN_FILENAME     "acct_mgr_login.xml"
#define GET_PROJECT_CONFIG_FILENAME "get_project_config.xml"
#define LOOKUP_ACCOUNT_FILENAME     "lookup_account.xml"
#define CREATE_ACCOUNT_FILENAME     "create_account.xml"
#define LOOKUP_WEBSITE_FILENAME     "lookup_website.html"
#define GET_CURRENT_VERSION_FILENAME    "get_current_version.xml"
#define ALL_PROJECTS_LIST_FILENAME "all_projects_list.xml"
#define ALL_PROJECTS_LIST_FILENAME_TEMP "all_projects_list_temp.xml"
#define SWITCHER_FILE_NAME          "switcher"
#define SETPROJECTGRP_FILE_NAME     "setprojectgrp"
#define TIME_STATS_LOG              "time_stats_log"
#define JOB_LOG_BASE                "job_log_"
#define CA_BUNDLE_FILENAME          "ca-bundle.crt"
#define CLIENT_AUTH_FILENAME        "client_auth.xml"

#endif
