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
#include <string.h>

#include "error_numbers.h"
#include "file_names.h"
#include "parse.h"

#include "log_flags.h"

LOG_FLAGS log_flags;

LOG_FLAGS::LOG_FLAGS() {
    task = file_xfer = sched_ops = state_debug = false;
    task_debug = file_xfer_debug = sched_op_debug = false;
    http_debug = time_debug = net_xfer_debug = false;
}

// Parse log flag preferences
//
int LOG_FLAGS::parse(FILE* in) {
    char buf[256];
    if(in==NULL) {
        fprintf(stderr, "error: LOG_FLAGS.parse: unexpected NULL pointer in\n");
        return ERR_NULL;
    }
    fgets(buf, 256, in);
    if (!match_tag(buf, "<log_flags>")) return ERR_XML_PARSE;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</log_flags>")) return 0;
        else if (match_tag(buf, "<task/>")) {
            task = true;
            continue;
        }
        else if (match_tag(buf, "<file_xfer/>")) {
            file_xfer = true;
            continue;
        }
        else if (match_tag(buf, "<sched_ops/>")) {
            sched_ops = true;
            continue;
        }
        else if (match_tag(buf, "<state_debug/>")) {
            state_debug = true;
            continue;
        }
        else if (match_tag(buf, "<task_debug/>")) {
            task_debug = true;
            continue;
        }
        else if (match_tag(buf, "<file_xfer_debug/>")) {
            file_xfer_debug = true;
            continue;
        }
        else if (match_tag(buf, "<sched_op_debug/>")) {
            sched_op_debug = true;
            continue;
        }
        else if (match_tag(buf, "<http_debug/>")) {
            http_debug = true;
            continue;
        }
        else if (match_tag(buf, "<time_debug/>")) {
            time_debug = true;
            continue;
        }
        else if (match_tag(buf, "<net_xfer_debug/>")) {
            net_xfer_debug = true;
            continue;
        }
        else fprintf(stderr, "LOG_FLAGS::parse: unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

void read_log_flags() {
    FILE* f;

    f = fopen(LOG_FLAGS_FILE, "r");
    if (f) {
        log_flags.parse(f);
        fclose(f);
    }
}
