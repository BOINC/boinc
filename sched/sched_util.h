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

#ifndef SCHED_UTIL_H
#define SCHED_UTIL_H

#include <math.h>
#include "util.h"

// "average credit" uses an exponential decay so that recent
// activity is weighted more heavily.
// AVG_HALF_LIFE is the "half-life" period:
// the average decreases by 1/2 if idle for this period.
//
// After a period of T, average credit is multiplied by
// exp(-T*log(2)/AHL)
//
// When new credit is granted, the average credit is incremented
// by the new credit's average rate,
// i.e. the amount divided by the time since it was started

#define LOG2 M_LN2
    // log(2)
#define SECONDS_IN_DAY (3600*24)
#define AVG_HALF_LIFE  (SECONDS_IN_DAY*7)

extern void write_pid_file(const char* filename);
extern void set_debug_level(int);
extern void check_stop_trigger();
extern void update_average(double, double, double&, double&);
extern void install_sigint_handler();
extern bool caught_sig_int;


class SchedMessages : public Messages {
    int debug_level;
    const char* v_format_kind(int kind) const;
    bool v_message_wanted(int kind) const;
public:
    enum Kind {
        CRITICAL,
        NORMAL,
        DEBUG
    };
    SchedMessages(): Messages(stderr) {}
    void set_debug_level(int new_level) { debug_level = new_level; }
};
extern SchedMessages log_messages;

#endif

