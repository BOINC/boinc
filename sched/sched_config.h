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

#ifndef _SCHED_CONFIG_
#define _SCHED_CONFIG_

#include <fstream>
using std::istream;

// parsed version of server configuration file
//
class SCHED_CONFIG {
public:
    char db_name[256];
    char db_user[256];
    char db_passwd[256];
    char db_host[256];
    int shmem_key;
    char key_dir[256];
    char download_url[256];
    char download_dir[256];
    char upload_url[256];
    char upload_dir[256];
    bool one_result_per_user_per_wu;
    bool trickle_down;
    int min_sendwork_interval;
    int max_wus_to_send;
    bool non_cpu_intensive;

    int parse(istream& f);
    int parse_file(char* dir=".");
};

// get the project's home directory
// (assumed to be the parent of the CWD)
//
void get_project_dir(char*, int);

#endif
