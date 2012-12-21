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

#ifndef _USING_FCGI_
#include <cstdio>
#else
#include "boinc_fcgi.h"
#endif

#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdarg>

#ifdef SIM
const int MAX_CPUS=4096;
#else
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_send.h"
#endif

#include "edf_sim.h"
#ifdef SIM
extern FILE* logfile;
#endif

using std::vector;

#define SUMMARY 0
    // show candidate decision
#define WORKLOAD  1
    // show workload
#define DETAIL   2
    // show every step of simulation

#define TIME_SCALE 1
//#define TIME_SCALE 3600

static void log_msg(int level, const char* format, ...) {
#ifdef SIM
#else
    switch(level) {
    case SUMMARY:
        if (!config.debug_send) return;
        break;
    case WORKLOAD:
        if (!config.debug_edf_sim_workload) return;
        break;
    case DETAIL:
        if (!config.debug_edf_sim_detail) return;
        break;
    }
#endif
    va_list va;
    va_start(va, format);
#ifdef SIM
    vfprintf(logfile, format, va);
#else
    log_messages.vprintf(MSG_NORMAL, format, va);
#endif
    va_end(va);
}

bool lessthan_deadline(const IP_RESULT& p1, const IP_RESULT& p2) {
    if (p1.computation_deadline < p2.computation_deadline) return true;
    return false;
}

// run an EDF simulation, marking which results will miss
// their deadlines and when
//
void mark_edf_misses (int ncpus, vector<IP_RESULT>& ip_results){
    vector<IP_RESULT>::iterator ipp_it;
    double booked_to[MAX_CPUS];
    int j;

    log_msg(DETAIL, "[edf_detail] mark_edf_misses\n");

    // keeps track of when each cpu is next free
    //
    for (j=0; j<ncpus; j++) {
        booked_to[j] = 0;
    }

    // sort ipp_results in ascending order by deadline
    //
    sort(ip_results.begin(), ip_results.end(), lessthan_deadline);

    // simulate edf execution of all queued results
    //
    for (unsigned int i=0; i<ip_results.size(); i++) {
        IP_RESULT& r = ip_results[i];

        // find the CPU that will be free first
        //
        double lowest_booked_time = booked_to[0];
        int lowest_booked_cpu = 0;
        for (j=1; j<ncpus; j++) {
            if (booked_to[j] < lowest_booked_time) {
                lowest_booked_time = booked_to[j];
                lowest_booked_cpu = j;
            }
        }

        booked_to[lowest_booked_cpu] += r.cpu_time_remaining;
        log_msg(DETAIL, "[edf_detail]   running %s on cpu %d; finishes at %.2f\n",
            r.name, lowest_booked_cpu, booked_to[lowest_booked_cpu]/TIME_SCALE
        );
        if (booked_to[lowest_booked_cpu] > r.computation_deadline) {
            r.misses_deadline = true;
            r.estimated_completion_time = booked_to[lowest_booked_cpu];
            log_msg(DETAIL, "[edf_detail]   %s misses_deadline; est completion %.2f\n",
                r.name, booked_to[lowest_booked_cpu]/TIME_SCALE
            );
        } else {
            r.misses_deadline = false;
            log_msg(DETAIL, "[edf_detail]   %s makes deadline; est completion %.2f\n",
                r.name, booked_to[lowest_booked_cpu]/TIME_SCALE
            );
            // if result doesn't miss its deadline,
            // then the estimated_completion_time is of no use
        }
    }
}

// For each ip_result, compute computation_deadline from report_deadline,
// and determine if the deadline would be missed by simulating EDF
//
void init_ip_results(
    double work_buf_min,
    int ncpus,
    vector<IP_RESULT>& ip_results
){
    unsigned int i;

    log_msg(DETAIL,
        "[edf_detail] init_ip_results; work_buf_min %.2f ncpus %d:\n",
        work_buf_min/TIME_SCALE, ncpus
    );
    for (i=0; i<ip_results.size(); i++) {
        IP_RESULT& r = ip_results[i];
        r.computation_deadline = r.report_deadline - work_buf_min;
        log_msg(DETAIL, "[edf_detail]     %s: deadline %.2f cpu %.2f\n",
            r.name, r.computation_deadline/TIME_SCALE, r.cpu_time_remaining/TIME_SCALE
        );
    }

    // run edf simulation to determine whether any results miss their deadline
    //
    mark_edf_misses(ncpus, ip_results);
}

#if 0
// Sort test_results by computation_deadline.
// For each test result in ascending order of deadline,
// see whether adding it to the work queue would cause deadline misses
// or deadline miss delays.
// If a test result passes these checks, append it to the work queue
// for further result additions.
// Return list of new results that can be sent to the client.
//
// NOTE: should we sort by increasing deadline or by increasing slack time?
//
vector<TEMP_RESULT> REQUEST_HANDLER_WORK_SEND::find_sendable_test_results (
    vector<TEMP_RESULT> test_results,
    int ncpus,
    vector<TEMP_RESULT> ipp_results,
    double cpu_pessimism_factor // = 1 by default
) {
    //test results to send
    vector<TEMP_RESULT> sendable_test_results;
    vector<TEMP_RESULT>::iterator test_results_it;

    sort(test_results.begin(), test_results.end(), lessthan_deadline);

    // see if each test result can be added to the work queue without
    // causing deadline misses or deadline miss delays
    //
    for (test_results_it = test_results.begin();
        test_results_it != test_results.end();
        test_results_it++
    ) {
        bool failed = result_cause_deadline_miss_or_delay(
            (*test_results_it).computation_deadline*cpu_pessimism_factor,
            (*test_results_it).cpu_time_remaining,
            ncpus,
            ipp_results
        );
        if (!failed){
            // add sendable result to work queue, copying by value, so that we
            // can evaluate what happens if we add more new results to the queue
            //
            ipp_results.push_back(*test_results_it);
            sendable_test_results.push_back (*test_results_it);
        }
    }
    return (sendable_test_results);
}
#endif

// Return false if
// 1) the candidate result X would cause another result Y to miss its deadline
//    (which Y would not have otherwise missed)
// 2) X causes another result Y to miss its deadline by more than
//    it otherwise would have, or
// 3) X would miss its deadline
//
bool check_candidate (
    IP_RESULT& candidate,
    int ncpus,
    vector<IP_RESULT> ip_results        // passed by value (copy)
) {
    double booked_to[MAX_CPUS];     // keeps track of when each cpu is free
    int j;

    log_msg(DETAIL, "[edf_detail] check_candidate %s: dl %.2f cpu %.2f\n",
        candidate.name, candidate.computation_deadline/TIME_SCALE,
        candidate.cpu_time_remaining/TIME_SCALE
    );

    for (j=0; j<ncpus; j++) {
        booked_to[j] = 0;
    }
    ip_results.push_back(candidate);

    // sort ip_results in ascending order by deadline
    //
    sort(ip_results.begin(), ip_results.end(), lessthan_deadline);

    // simulate execution of all queued results and test result
    //
    for (unsigned int i=0; i<ip_results.size(); i++) {
        IP_RESULT& r = ip_results[i];

        // find the CPU that will be free first
        //
        double lowest_booked_time = booked_to[0];
        int lowest_booked_cpu = 0;
        for (j=1; j<ncpus; j++) {
            if (booked_to[j] < lowest_booked_time) {
                lowest_booked_time = booked_to[j];
                lowest_booked_cpu = j;
            }
        }
        booked_to[lowest_booked_cpu] += r.cpu_time_remaining;
        log_msg(DETAIL, "[edf_detail]   running %s on cpu %d; deadline %f, finishes at %.2f\n",
            r.name, lowest_booked_cpu, r.computation_deadline, booked_to[lowest_booked_cpu]/TIME_SCALE
        );

        // return false if completion time if > computation_deadline AND
        // result would not have missed deadline to begin with
        //
        if (booked_to[lowest_booked_cpu] > r.computation_deadline
            && !r.misses_deadline
        ) {
            log_msg(SUMMARY,
                "[send]  cand. fails; %s now misses deadline: %.2f > %.2f\n",
                r.name, booked_to[lowest_booked_cpu]/TIME_SCALE,
                r.computation_deadline/TIME_SCALE
            );
            return false;
        }
        // check a late result (i.e., one that would have missed its
        // deadline) // would be made even later
        //
        if (r.misses_deadline
            && booked_to[lowest_booked_cpu] > r.estimated_completion_time
        ){
            log_msg(SUMMARY,
                "[send]  cand. fails; late result %s would be returned even later\n",
                r.name
            );
            return false;
        }
    }
    log_msg(SUMMARY, "[send]  candidate succeeds\n");
    return true;
}

#if 0
int main() {
    vector<IP_RESULT> ip_results;
    double work_buf_min = 0;
    double cpu_pessimism_factor = 1;
    int ncpus = 1;
    bool flag;

    ip_results.push_back(IP_RESULT("R1", 5, 3));
    ip_results.push_back(IP_RESULT("R2", 5, 3));
    init_ip_results(work_buf_min, ncpus, ip_results);

    IP_RESULT c1 = IP_RESULT("C1", 10, 1);
    if (check_candidate(c1, ncpus, ip_results)) {
        printf("adding %s\n", c1.name);
        ip_results.push_back(c1);
    }

    IP_RESULT c2 = IP_RESULT("C2", 7, 2);
    if (check_candidate(c2, ncpus, ip_results)) {
        printf("adding %s\n", c2.name);
        ip_results.push_back(c2);
    }
}
#endif
