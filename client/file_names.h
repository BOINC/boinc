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

#ifndef _FILE_NAMES_
#define _FILE_NAMES_

#include "client_types.h"
#include "prefs.h"

// get the pathname (relative to client home dir) of a project file
//
extern void get_pathname(FILE_INFO* fip, char* path);
extern void get_project_dir(PROJECT*, char*);

// get the pathname (relative to client home dir) of the
// directory used for a particular application "slot"
//
extern void get_slot_dir(int slot, char* path);

extern int make_project_dir(PROJECT&);
extern int remove_project_dir(PROJECT&);
extern int make_slot_dir(int);
extern void get_account_filename(char* master_url, char* path);
extern bool is_account_file(const char*);
extern void escape_project_url(char *in, char* out);
extern int check_unique_instance();
extern void get_sched_request_filename(PROJECT&, char*);
extern void get_sched_reply_filename(PROJECT&, char*);

#define PROJECTS_DIR                "projects"
#define SLOTS_DIR                   "slots"
#define STATE_FILE_NEXT             "client_state_next.xml"
#define STATE_FILE_NAME             "client_state.xml"
#define STATE_FILE_PREV             "client_state_prev.xml"
#define GLOBAL_PREFS_FILE_NAME      "global_prefs.xml"
#define MASTER_FILE_NAME            "master.html"
#define SCHED_OP_REQUEST_BASE       "sched_request_"
#define SCHED_OP_REPLY_BASE        "sched_reply_"
#define LOG_FLAGS_FILE              "log_flags.xml"
#define TEMP_FILE_NAME              "temp.xml"
#define STDERR_FILE_NAME            "stderr.txt"
#define STDOUT_FILE_NAME            "stdout.txt"
#define CPU_BENCHMARKS_FILE_NAME    "cpu_benchmarks"
#define LOCK_FILE_NAME              "lockfile"
#define INI_FILE_NAME               "boinc.ini"
#define LANGUAGE_FILE_NAME          "language.ini"
#define LIST_STATE_FILE_NAME        "list.ini"
#define APP_INFO_FILE_NAME			"app_info.xml"
#define REMOTEHOST_FILE_NAME        "remote_hosts.cfg"
#define FILE_LIST_NAME              "file_list.xml"
#define ACCT_MGR_REPLY_FILENAME     "acct_mgr_reply.xml"

#endif
