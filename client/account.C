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

#include <stdio.h>

#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"

#include "account.h"

int write_account_file(char* master_url, char* authenticator) {
    char path[256];
    FILE* f;

    get_account_filename(master_url, path);
    f = fopen(path, "w");
    if (!f) return ERR_FOPEN;

    fprintf(f,
        "<account>\n"
        "    <master_url>%s</master_url>\n"
        "    <authenticator>%s</authenticator>\n"
        "    <resource_share>1</resource_share>"
        "</account>\n",
        master_url,
        authenticator
    );
    fclose(f);
    return 0;
}

int CLIENT_STATE::parse_account_files() {
    DIRREF dir;
    char name[256];
    PROJECT* project;
    FILE* f;

    dir = dir_open(".");
    while (dir_scan(name, dir) == 0) {
        if (is_account_file(name)) {
            f = fopen(name, "r");
            if (!f) continue;
            project = new PROJECT;
            project->parse_account(f);
            projects.push_back(project);
            fclose(f);
        }
    }
    dir_close(dir);
    return 0;
}

