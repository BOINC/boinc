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
#include <sys/stat.h>

#include "file_names.h"

void get_pathname(FILE_INFO* fip, char* path) {
    PROJECT* p = fip->project;
    sprintf(path, "%s/%s", p->domain, fip->name);
}

void get_slot_dir(int slot, char* path) {
    sprintf(path, "slots/%d", slot);
}

int make_project_dir(PROJECT& p) {
    mkdir(p.domain, 0777);
    return 0;
}

int make_slot_dir(int slot) {
    char buf[256];
    mkdir("slots", 0777);
    get_slot_dir(slot, buf);
    mkdir(buf, 0777);
    return 0;
}
