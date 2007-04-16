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
    // NOTE: this be the same name as used by the "start" script
const int STOP_SIGNAL = SIGHUP;
    // NOTE: this be the same signal as used by the "start" script

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
bool caught_stop_signal = false;
static void stop_signal_handler(int) {
    fprintf(stderr, "GOT STOP SIGNAL\n");
    caught_stop_signal = true;
}

void install_stop_signal_handler() {
    signal(STOP_SIGNAL, stop_signal_handler);
    // handler is now default again so hitting ^C again will kill the program.
}

void check_stop_trigger() {
    if (caught_stop_signal) {
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
    double now = dtime();

    if (avg_time) {
        double diff = now - avg_time;
        double diff_days = diff/SECONDS_IN_DAY;
        double weight = exp(-diff*LOG2/AVG_HALF_LIFE);
        avg *= weight;
        avg += (1-weight)*(credit/diff_days);
    } else {
        double dd = (now - credit_assigned_time)/SECONDS_IN_DAY;
        avg = credit/dd;
    }
    avg_time = now;
}

