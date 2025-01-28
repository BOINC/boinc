// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// utility functions for BOINC server programs

#include "config.h"
#include <ctime>

#include "boinc_db.h"
#include "sched_util.h"

void compute_avg_turnaround(HOST& host, double turnaround) {
    double new_avg;
    if (host.avg_turnaround == 0) {
        new_avg = turnaround;
    } else {
        new_avg = .7*host.avg_turnaround + .3*turnaround;
    }
    host.avg_turnaround = new_avg;
}

int PERF_INFO::get_from_db() {
    int retval;
    long n;
    DB_HOST host;

    host_fpops_mean = 2.2e9;
    host_fpops_stddev = .7e9;
    host_fpops_50_percentile = 3.3e9;
    host_fpops_95_percentile = 3.3e9;

    retval = host.count(n);
    if (retval) return retval;
    if (n < 10) return 0;
    retval = host.fpops_mean(host_fpops_mean);
    retval = host.fpops_stddev(host_fpops_stddev);
    retval = host.fpops_percentile(50, host_fpops_50_percentile);
    retval = host.fpops_percentile(95, host_fpops_95_percentile);
    return 0;
}

int count_results(char* query, long& n) {
    DB_RESULT result;
    return result.count(n, query);
}

int count_workunits(long& n, const char* query) {
    DB_WORKUNIT workunit;
    return workunit.count(n, query);
}

int count_unsent_results(long& n, DB_ID_TYPE appid, int size_class) {
    char query[1024], buf[256];
    sprintf(query, "where server_state<=%d", RESULT_SERVER_STATE_UNSENT);
    if (appid) {
        sprintf(buf, " and appid=%lu", appid);
        strcat(query, buf);
    }
    if (size_class >= 0) {
        sprintf(buf, " and size_class=%d", size_class);
        strcat(query, buf);
    }
    return count_results(query, n);
}

// Arrange that further results for this workunit
// will be sent only to the specific host(s).
// This could be used, for example, so that late workunits
// are sent only to cloud or cluster resources
//
static int restrict_wu(WORKUNIT& _wu, DB_ID_TYPE id, int assign_type) {
    DB_RESULT result;
    DB_ASSIGNMENT asg;
    DB_WORKUNIT wu;
    wu = _wu;
    char buf[256];
    int retval;

    // mark unsent results as DIDNT_NEED
    //
    sprintf(buf, "where workunitid=%lu and server_state=%d",
        wu.id, RESULT_SERVER_STATE_UNSENT
    );
    while (!result.enumerate(buf)) {
        char buf2[256];
        sprintf(buf2, "server_state=%d, outcome=%d",
            RESULT_SERVER_STATE_OVER,
            RESULT_OUTCOME_DIDNT_NEED
        );
        result.update_field(buf2);
    }

    // mark the WU as TRANSITION_NO_NEW_RESULTS
    //
    sprintf(buf, "transitioner_flags=%d", TRANSITION_NO_NEW_RESULTS);
    retval = wu.update_field(buf);
    if (retval) return retval;

    // create an assignment record
    //
    asg.clear();
    asg.create_time = time(0);
    asg.target_id = id;
    asg.target_type = assign_type;
    asg.multi = 0;
    asg.workunitid = wu.id;
    retval = asg.insert();
    return retval;
}

int restrict_wu_to_user(WORKUNIT& wu, DB_ID_TYPE userid) {
    return restrict_wu(wu, userid, ASSIGN_USER);
}

int restrict_wu_to_host(WORKUNIT& wu, DB_ID_TYPE hostid) {
    return restrict_wu(wu, hostid, ASSIGN_HOST);
}

// return the min transition time.
// This lets you check if the transitioner is backed up.
//
int min_transition_time(double& x) {
    return boinc_db.get_double("select min(transition_time) from workunit", x);
}
