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

using namespace std;

#include <strings.h>
#include <csignal>
#include <cstdarg>
#include <unistd.h>

#include "parse.h"
#include "util.h"
#include "main.h"
#include "sched_util.h"
#include "server_types.h"

const char* STOP_TRIGGER_FILENAME = "../stop_servers";

void write_pid_file(const char* filename) {
    FILE* fpid = fopen(filename, "w");
    if (!fpid) {
        log_messages.printf(SchedMessages::NORMAL, "Couldn't write pid\n");
        return;
    }
    fprintf(fpid, "%d\n", (int)getpid());
    fclose(fpid);
}

// caught_sig_int will be set to true if SIGINT is caught.
bool caught_sig_int = false;
static void sigint_handler(int) {
    fprintf(stderr, "SIGINT\n");
    caught_sig_int = true;
}

void install_sigint_handler() {
    signal(SIGINT, sigint_handler);
    // SIGINT is now default again so hitting ^C again will kill the program.
}

void check_stop_trigger() {
    if (caught_sig_int) {
        log_messages.printf(SchedMessages::CRITICAL, "Quitting due to SIGINT\n");
        exit(0);
    }
    FILE* f = fopen(STOP_TRIGGER_FILENAME, "r");
    if (f) {
        log_messages.printf(SchedMessages::NORMAL, "Quitting due to stop trigger\n");
        exit(0);
    }
}

bool is_stopfile_present() {
    FILE* f = fopen(STOP_TRIGGER_FILENAME, "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}


// decay an exponential average of credit per day,
// and possibly add an increment for new credit
//
void update_average(
    double credit_assigned_time,        // when work was started for new credit
                                        // (or zero if no new credit)
    double credit,                      // amount of new credit
    double& avg,                        // average credit per day (in and out)
    double& avg_time                    // when average was last computed
) {
    time_t now = time(0);

    // decrease existing average according to how long it's been
    // since it was computed
    //
    if (avg_time) {
        double deltat = now - avg_time;
        avg *= exp(-deltat*LOG2/AVG_HALF_LIFE);
    }
    if (credit_assigned_time) {
        double deltat = now - credit_assigned_time;
        // Add (credit)/(number of days to return result) to credit,
        // which is the average number of cobblestones per day
        //
        avg += credit/(deltat/SECONDS_IN_DAY);
    }
    avg_time = now;
}

