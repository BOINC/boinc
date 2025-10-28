// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#ifndef BOINC_FILE_NAMES_H
#define BOINC_FILE_NAMES_H

#include "client_types.h"
#include "common_defs.h"
#include "prefs.h"

extern int make_soft_link(PROJECT* project, char* link_path, char* rel_file_path);

// get the pathname (relative to client home dir) of a project file
//
extern void get_pathname(FILE_INFO* fip, char* path, int len);

// get the pathname (relative to client home dir) of the
// directory used for a particular application "slot"
//
extern void get_slot_dir(int slot, char* path, int len);

extern int make_project_dir(PROJECT&);
extern int remove_project_dir(PROJECT&);
extern int make_slot_dir(int);
extern void delete_old_slot_dirs();
extern void get_account_filename(char* master_url, char* path, int len);
extern bool is_account_file(const char*);
extern bool is_statistics_file(const char*);
extern void get_statistics_filename(char* master_url, char* path, int len);
extern bool is_image_file(const char*);

extern void get_sched_request_filename(PROJECT&, char*, int len);
extern void get_sched_reply_filename(PROJECT&, char*, int len);
extern void get_master_filename(PROJECT&, char*, int len);
extern void job_log_filename(PROJECT&, char*, int);
extern void boinc_version_dir(PROJECT&, VERSION_INFO&, char*);
extern bool is_version_dir(char*, VERSION_INFO&);
extern void send_log_after(const char* filename, double t, MIOFILE& mf);

#define ACCT_MGR_LOGIN_FILENAME     "acct_mgr_login.xml"
#define ACCT_MGR_REPLY_FILENAME     "acct_mgr_reply.xml"
#define ACCT_MGR_REQUEST_FILENAME   "acct_mgr_request.xml"
#define ACCT_MGR_URL_FILENAME       "acct_mgr_url.xml"
#define ALL_PROJECTS_LIST_FILENAME "all_projects_list.xml"
#define ALL_PROJECTS_LIST_FILENAME_TEMP "all_projects_list_temp.xml"
#define APP_CONFIG_FILE_NAME        "app_config.xml"
#define APP_INFO_FILE_NAME          "app_info.xml"
#define CA_BUNDLE_FILENAME          "ca-bundle.crt"
#define CERTIFICATE_DIRECTORY       "certificates"
#define CLIENT_AUTH_FILENAME        "client_auth.xml"
#define CLIENT_BRAND_FILENAME       "client_brand.txt"
#ifdef _WIN32
#define CLIENT_EXEC_FILENAME        "boinc.exe"
#else
#define CLIENT_EXEC_FILENAME        "boinc"
#endif
#define CLIENT_OPAQUE_FILENAME      "client_opaque.txt"
#define CONFIG_FILE                 "cc_config.xml"
#define NVC_CONFIG_FILE             "nvc_config.xml"
#define COPROC_INFO_FILENAME        "coproc_info.xml"
#define EMULATED_CPU_INFO_EXECUTABLE "detect_rosetta_cpu"
#define EMULATED_CPU_INFO_FILENAME  "emulated_cpu_info.txt"
#define CPU_BENCHMARKS_FILE_NAME    "cpu_benchmarks"
#define CREATE_ACCOUNT_FILENAME     "create_account.xml"
#define DAILY_XFER_HISTORY_FILENAME "daily_xfer_history.xml"
#define FIX_BOINC_USERS_FILENAME    "Fix_BOINC_Users"
#define RUN_PODMAN_FILENAME         "Run_Podman"
#define GET_CURRENT_VERSION_FILENAME    "get_current_version.xml"
#define GET_PROJECT_CONFIG_FILENAME "get_project_config.xml"
#define GLOBAL_PREFS_FILE_NAME      "global_prefs.xml"
#define GLOBAL_PREFS_OVERRIDE_FILE  "global_prefs_override.xml"
#define JOB_LOG_BASE                "job_log_"
#define KEYWORD_FILENAME            "keywords.xml"
#define LOGIN_TOKEN_LOOKUP_REPLY    "login_token_lookup_reply.xml"
#define LOOKUP_ACCOUNT_FILENAME     "lookup_account.xml"
#define LOOKUP_WEBSITE_FILENAME     "lookup_website.html"
#define MASTER_BASE                 "master_"
#define NOTICES_DIR                 "notices"
#ifdef __APPLE__
#define PODMAN_DIR                  "BOINC podman"
#else
#define PODMAN_DIR                  "podman"
#endif
#define PROJECTS_DIR                "projects"
#define REMOTEHOST_FILE_NAME        "remote_hosts.cfg"
#define SCHED_OP_REPLY_BASE         "sched_reply_"
#define SCHED_OP_REQUEST_BASE       "sched_request_"
#define SETPROJECTGRP_FILE_NAME     "setprojectgrp"
#define SLOTS_DIR                   "slots"
#define STATE_FILE_NEXT             "client_state_next.xml"
#define STATE_FILE_NAME             "client_state.xml"
#define STATE_FILE_PREV             "client_state_prev.xml"
#define STDERR_FILE_NAME            "stderr.txt"
#define STDOUT_FILE_NAME            "stdout.txt"
#define SWITCHER_DIR                "switcher"
#define SWITCHER_FILE_NAME          "switcher"
#define TASK_STATE_FILENAME         "boinc_task_state.xml"
#define TEMP_ACCT_FILE_NAME         "temp_acct.xml"
#define TEMP_FILE_NAME              "temp.xml"
#define TEMP_STATS_FILE_NAME        "temp_stats.xml"
#define TEMP_TIME_STATS_FILE_NAME   "temp_time_stats.xml"
#define TIME_STATS_LOG              "time_stats_log"

#endif
