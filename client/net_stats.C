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

#include <string.h>

#include "math.h"
#include "parse.h"
#include "time.h"

#include "error_numbers.h"
#include "net_stats.h"

// Is this a reasonable cutoff?
#define SMALL_FILE_CUTOFF 32000

NET_STATS::NET_STATS() {
    last_update_up = 0;
    last_update_down = 0;
    bwup = 0;
    bwdown = 0;
}

// Update network statistics
void NET_STATS::update(bool is_upload, double nbytes, double nsecs) {
    double w1, w2, bw;

    // ignore small files since their transfer times don't
    // reflect steady-state behavior

    if (nbytes < SMALL_FILE_CUTOFF) return;

    // The weight of the new item is a function of its size

    w1 = 1 - exp(-nbytes/1.e7);
    w2 = 1 - w1;
    bw = nbytes/nsecs;

    if (is_upload) {
        if (!last_update_up) {
            bwup = bw;
        } else {
            bwup = w1*bw + w2*bwup;
        }
        last_update_up = time(0);
    } else {
        if (!last_update_down) {
            bwdown = bw;
        } else {
            bwdown = w1*bw + w2*bwdown;
        }
        last_update_down = time(0);
    }
}

// Write XML based network statistics
//
int NET_STATS::write(FILE* out, bool to_server) {
    if(out==NULL) {
        fprintf(stderr, "error: NET_STATS.write: unexpected NULL pointer out\n");
        return ERR_NULL;
    }
    fprintf(out,
        "<net_stats>\n"
        "    <bwup>%f</bwup>\n"
        "    <bwdown>%f</bwdown>\n",
        bwup,
        bwdown
    );
    if (!to_server) {
        fprintf(out,
            "    <last_update_up>%d</last_update_up>\n"
            "    <last_update_down>%d</last_update_down>\n",
            last_update_up, last_update_down
        );
    }
    fprintf(out, "</net_stats>\n");
    return 0;
}

// Read XML based network statistics
//
int NET_STATS::parse(FILE* in) {
    char buf[256];
    if(in==NULL) {
        fprintf(stderr, "error: NET_STATS.parse: unexpected NULL pointer in\n");
        return ERR_NULL;
    }
    memset(this, 0, sizeof(NET_STATS));
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</net_stats>")) return 0;
        else if (parse_double(buf, "<bwup>", bwup)) continue;
        else if (parse_double(buf, "<bwdown>", bwdown)) continue;
        else if (parse_int(buf, "<last_update_up>", last_update_up)) continue;
        else if (parse_int(buf, "<last_update_down>", last_update_down)) continue;
        else fprintf(stderr, "NET_STATS::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}
