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

#ifndef BOINC_SIM_H
#define BOINC_SIM_H

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
    int nrpcs;

    SIM_RESULTS(DUMMY_TYPE){}
    void clear() {
        static const SIM_RESULTS x(DUMMY);
        *this = x;
    }
    SIM_RESULTS() {
        clear();
    }
    void compute_figures_of_merit();
    void print(FILE* f, bool human_readable=false);
    void parse(FILE* f);
    void add(SIM_RESULTS& r);
    void divide(int);
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
    double std_dev;
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
    bool sample(double dt);
    void init(double f, double l);
    int parse(XML_PARSER&, const char*);
    RANDOM_PROCESS();
};

extern FILE* logfile;
extern bool user_active;
extern std::string html_msg;
extern SIM_RESULTS sim_results;
extern double calculate_exponential_backoff(
    int n, double MIN, double MAX
);
extern char* sim_time_string(int t);

extern bool dcf_dont_use;
extern bool dcf_stats;
extern bool cpu_sched_rr_only;
extern bool dual_dcf;
extern bool work_fetch_old;
extern bool gpus_usable;

extern RANDOM_PROCESS on_proc;
extern RANDOM_PROCESS connected_proc;
extern RANDOM_PROCESS active_proc;
extern RANDOM_PROCESS gpu_active_proc;

//#define START_TIME  946684800
    // Jan 1 2000
#define START_TIME  3600
    // should be at least an hour or so

#endif
