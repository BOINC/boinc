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
// Portions created by the SETI@home project are Copyright (C) 2002, 2003
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#include <cstdio>
#include <cstdlib>

#include "config.h"
#include "sched_util.h"

int main() {
    CONFIG config;
    int i, retval;
    char* p;

    write_log(MSG_NORMAL, "Starting servers.\n");

    retval = config.parse_file();
    if (retval) {
        write_log(MSG_CRITICAL, "Can't read config\n");
        exit(1);
    }

    for (i=0; i<20; i++) {
        p = config.start_commands[i];
        if (!p) break;
        write_log(MSG_NORMAL, "Executing: %s\n", p);
        system(p);
    }
}
