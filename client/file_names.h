// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "client_types.h"
#include "prefs.h"

// get the pathname (relative to client home dir) of a project file
//
extern void get_pathname(FILE_INFO* fip, char* path);

// get the pathname (relative to client home dir) of the
// directory used for a particular application "slot"
//
extern void get_slot_dir(int slot, char* path);

extern int make_project_dir(PROJECT&);
extern int remove_project_dir(PROJECT&);
extern int make_slot_dir(int);
extern void get_account_filename(char* master_url, char* path);
extern bool is_account_file(char*);
extern void escape_project_url(char *in, char* out);

#define PROJECTS_DIR                "projects"
#define SLOTS_DIR                   "slots"
#define STATE_FILE_TEMP             "state_file_temp.xml"
#define STATE_FILE_NAME             "client_state.xml"
#define GLOBAL_PREFS_FILE_NAME      "global_prefs.xml"
#define MASTER_FILE_NAME            "master.html"
#define SCHED_OP_REQUEST_FILE       "sched_request.xml"
#define SCHED_OP_RESULT_FILE        "sched_reply.xml"
#define LOG_FLAGS_FILE              "log_flags.xml"
#define TEMP_FILE_NAME              "temp.xml"
#define STDERR_FILE_NAME            "stderr.txt"
#define STDOUT_FILE_NAME            "stdout.txt"
#define CPU_BENCHMARKS_FILE_NAME    "cpu_benchmarks.xml"
#define LOCK_FILE_NAME              "lockfile"
#define INI_FILE_NAME               "boinc.ini"
#define LANGUAGE_FILE_NAME          "language.ini"
#define LIST_STATE_FILE_NAME        "list.ini"
