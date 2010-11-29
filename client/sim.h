// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _SIM_H_
#define _SIM_H_

#include <vector>

using std::vector;

struct SIM_RESULTS {
    double flops_used;      // based on peak flops
    double flops_wasted;
    int nresults_met_deadline;
    int nresults_missed_deadline;
    double share_violation;
    double monotony;
    double wasted_frac;
    double idle_frac;

    void compute();
    void print(FILE* f, const char* title=0);
    void parse(FILE* f);
    void add(SIM_RESULTS& r);
    void divide(int);
    void clear();
};

struct PROJECT_RESULTS {
    double flops_used;      // based on peak flops
    double flops_wasted;
    int nresults_met_deadline;
    int nresults_missed_deadline;

    PROJECT_RESULTS() {
        flops_used = 0;
        flops_wasted = 0;
        nresults_met_deadline = 0;
        nresults_missed_deadline = 0;
    }
};

struct NORMAL_DIST {
    double mean;
    double stdev;
    int parse(XML_PARSER&, const char* end_tag);
    double sample();
};

struct UNIFORM_DIST {
    double lo;
    double hi;
    int parse(XML_PARSER&, const char* end_tag);
    double sample();
};

class RANDOM_PROCESS {
    double last_time;
    double time_left;
    bool value;
    double off_lambda;
public:
    double frac;
    double lambda;
    int parse(XML_PARSER&, const char* end_tag);
    bool sample(double);
    void init(double);
    RANDOM_PROCESS();
};

extern FILE* logfile;
extern bool user_active;
extern SIM_RESULTS sim_results;
extern double calculate_exponential_backoff(
    int n, double MIN, double MAX
);

extern bool dcf_dont_use;
extern bool dcf_stats;
extern bool cpu_sched_rr_only;
extern bool dual_dcf;
extern bool work_fetch_old;
extern bool gpus_usable;

#define START_TIME  946684800
    // Jan 1 2000

#endif
