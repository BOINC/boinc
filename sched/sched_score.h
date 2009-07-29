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

#include <list>

struct JOB {
    int index;
    double score;
    double est_time;
    double disk_usage;
    APP* app;
    BEST_APP_VERSION* bavp;

    bool get_score();
};

struct JOB_SET {
    double work_req;
    double est_time;
    double disk_usage;
    double disk_limit;
    int max_jobs;
    std::list<JOB> jobs;     // sorted high to low

    JOB_SET() {
        work_req = g_request->work_req_seconds;
        est_time = 0;
        disk_usage = 0;
        disk_limit = g_wreq->disk_available;
        max_jobs = g_wreq->max_jobs_per_rpc;

        int n = g_wreq->max_jobs_on_host - g_wreq->njobs_on_host;
        if (n < 0) n = 0;
        if (n < max_jobs) max_jobs = n;
    }
    void add_job(JOB&);
    double higher_score_disk_usage(double);
    double lowest_score();
    inline bool request_satisfied() {
        return est_time >= work_req;
    }
    void send();
};

extern void send_work_matchmaker();
