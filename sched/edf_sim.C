// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <vector>
#include <algorithm>

#include "edf_sim.h"

using std::vector;

//#define TEST
#ifdef TEST
#define INFO0 printf
#define INFO1 printf
#define INFO2 printf
#else
#define INFO0 //
#define INFO1 //
#define INFO2 //
#endif

bool lessthan_deadline(const IP_RESULT& p1, const IP_RESULT& p2) {
    if (p1.computation_deadline < p2.computation_deadline) return true;
    return false;
}

// run an EDF simulation, marking which results will miss
// their deadlines and when
//
void mark_edf_misses (int ncpus, vector<IP_RESULT>& ip_results){
    vector<IP_RESULT>::iterator ipp_it;
    double booked_to[128];
    int j;

    INFO1("mark_edf_misses\n");

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
        INFO2("  running %s on cpu %d; finishes at %f\n",
            r.name, lowest_booked_cpu, booked_to[lowest_booked_cpu]
        );
        if (booked_to[lowest_booked_cpu] > r.computation_deadline) {
	        r.misses_deadline = true;
	        r.estimated_completion_time = booked_to[lowest_booked_cpu];
	        INFO1("  %s misses_deadline; est completion %f\n",
                r.name, booked_to[lowest_booked_cpu]
            );
        } else {
	        r.misses_deadline = false;
	        INFO1("  %s makes deadline; est completion %f\n",
                r.name, booked_to[lowest_booked_cpu]
            );
	        // if result doesn't miss its deadline,
            // then the estimated_completion_time is of no use
        }
    }
}

// For each ip_result, computes computation_deadline from report_deadline,
// and determines if the deadline would be missed by simulating edf
//
void init_ip_results(
    double work_buf_min,
    int ncpus,
    vector<IP_RESULT>& ip_results
){
    unsigned int i;
    for (i=0; i<ip_results.size(); i++) {
        IP_RESULT& r = ip_results[i];
        r.computation_deadline = r.report_deadline - work_buf_min;
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
    double booked_to[128];     // keeps track of when each cpu is free
    int j;

    INFO0 ("check_candidate %s: dl %f cpu %f\n",
        candidate.name, candidate.computation_deadline,
        candidate.cpu_time_remaining
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
        INFO2("  running %s on cpu %d; finishes at %f\n",
            r.name, lowest_booked_cpu, booked_to[lowest_booked_cpu]
        );

        // return false if completion time if > computation_deadline AND
        // result would not have missed deadline to begin with
        //
        if (booked_to[lowest_booked_cpu] > r.computation_deadline
            && !r.misses_deadline
        ) {
	        INFO1 ("  %s now misses deadline: %f\n",
                r.name, booked_to[lowest_booked_cpu]
            );
            return false;
        }
        // check a late result (i.e., one that would have missed its
        // deadline) // would be made even later
        //
        if (r.misses_deadline 
            && booked_to[lowest_booked_cpu] > r.estimated_completion_time
        ){
            INFO0 ("  %s: late result to be returned even later\n", r.name);
            return false;
        }
    }
    return true;
}

#ifdef TEST
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
