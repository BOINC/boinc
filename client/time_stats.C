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
#include <time.h>
#include <math.h>

#include "parse.h"
#include "error_numbers.h"

#include "time_stats.h"

// exponential decay constant.
// The last 30 days have a weight of 1/e;
// everything before that has a weight of (1-1/e)

#define ALPHA (3600.*24*7*30)

TIME_STATS::TIME_STATS() {
    first = true;
    last_update = 0;
    on_frac = 1;
    connected_frac = 1;
    active_frac = 1;
}

void TIME_STATS::update(bool is_connected, bool is_active) {
    int now = time(0), dt;
    double w1, w2;

    if (last_update == 0) {
        // this is the first time this client has executed.
        // Assume that its state has always been what it is now

        on_frac = 1;
        connected_frac = is_connected?1:0;
        active_frac = is_active?1:0;
        first = false;
        last_update = now;
    } else {
        dt = now - last_update;
        if (dt <= 0) return;
        w1 = 1 - exp(-dt/ALPHA);
        w2 = 1 - w1;
        if (first) {
            // the client has just started; this is the first call.
            // The client has been off (and disconnected and inactive)
            // since the last time it ran.

            on_frac *= w2;
            connected_frac *= w2;
            active_frac *= w2;
            first = false;
        } else {
            on_frac = w1 + w2*on_frac;
            connected_frac *= w2;
            if (is_connected) connected_frac += w1;
            active_frac *= w2;
            if (is_active) active_frac += w1;
        }
        last_update = now;
    }
}

int TIME_STATS::write(FILE* out, bool to_server) {
    fprintf(out,
        "<time_stats>\n"
        "    <on_frac>%f</on_frac>\n"
        "    <connected_frac>%f</connected_frac>\n"
        "    <active_frac>%f</active_frac>\n",
        on_frac,
        connected_frac,
        active_frac
    );
    if (!to_server) {
        fprintf(out,
            "    <last_update>%d</last_update>\n",
            last_update
        );
    }
    fprintf(out, "</time_stats>\n");
    return 0;
}

int TIME_STATS::parse(FILE* in) {
    char buf[256];

    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</time_stats>")) return 0;
        else if (parse_int(buf, "<last_update>", last_update)) continue;
        else if (parse_double(buf, "<on_frac>", on_frac)) continue;
        else if (parse_double(buf, "<connected_frac>", connected_frac)) continue;
        else if (parse_double(buf, "<active_frac>", active_frac)) continue;
        else fprintf(stderr, "TIME_STATS:: parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}
