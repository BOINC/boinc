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

// "average credit" uses an exponential decay so that recent
// activity is weighted more heavily.
// CREDIT_HALF_LIFE is the "half-life" period:
// the average decreases by 1/2 if idle for this period.
//
#define SECONDS_IN_DAY (3600*24)
#define CREDIT_HALF_LIFE  (SECONDS_IN_DAY*7)

extern void write_pid_file(const char* filename);
extern void set_debug_level(int);
extern void check_stop_trigger();
extern bool is_stopfile_present();
extern void install_stop_signal_handler();
extern bool caught_stop_signal;

#endif
