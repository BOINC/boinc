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

#ifndef _SCHED_UTIL_
#define _SCHED_UTIL_

#include <math.h>

// "average credit" uses an exponential decay so that recent
// activity is weighted more heavily.
// H is the "half-life" period: the average goes down by 1/2
// if idle for this period.
// Specifically, the weighting function W(t) is
// W(t) = exp(t/(H*log(2))*H*log(2).
// The average credit is the sum of X*W(t(X))
// over units of credit X that were granted t(X) time ago.

#define LOG2 M_LN2
    // log(2)
#define SECONDS_IN_DAY (3600*24)
#define AVG_HALF_LIFE  (SECONDS_IN_DAY*7)
#define ALPHA (LOG2/AVG_HALF_LIFE)

#define STOP_TRIGGER_FILENAME "stop_server"

#define MSG_CRITICAL 0
#define MSG_NORMAL   1
#define MSG_DEBUG    2

void write_log(int, char const*, ...);
void write_log_multiline(int msg_level, const char* p);
extern void write_pid_file(const char* filename);
extern void set_debug_level(int);
extern void check_stop_trigger();
extern void update_average(double, double, double&, double&);
extern void install_sigint_handler();
extern bool sig_int;

#endif

