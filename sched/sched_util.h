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

extern void write_log(char*);
extern void check_stop_trigger();
extern int get_team_credit(TEAM&);
extern int update_teams();
extern void update_average(double, double, double&, double&);

#endif

