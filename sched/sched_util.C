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

using namespace std;

#include <strings.h>

#include "parse.h"
#include "util.h"
#include "main.h"
#include "sched_util.h"
#include "server_types.h"

void write_log(char* p) {
    fprintf(stderr, "%s: %s", timestamp(), p);
}

void check_stop_trigger() {
    FILE* f = fopen(STOP_TRIGGER_FILENAME, "r");
    if (!f) return;
    exit(0);
}

// update an exponential average of credit per second.
//
void update_average(double credit_assigned_time, double credit, double& avg, double& avg_time) {
    time_t now = time(0);

    // decrease existing average according to how long it's been
    // since it was computed
    //
    if (avg_time) {
        double deltat = now - avg_time;
        avg *= exp(-deltat*ALPHA);
    }
    if (credit_assigned_time) {
        double deltat = now - credit_assigned_time;
        // Add (credit)/(number of days to return result) to credit, which
        // is the average number of cobblestones per day
        avg += credit/(deltat/86400);
    }
    avg_time = now;
}

