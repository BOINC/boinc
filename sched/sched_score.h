// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// job dispatch using a score-based approach.  See sched_score.cpp

#include "buda.h"

struct JOB {
    int index;          // index into shared-mem job array
    DB_ID_TYPE result_id;
    double score;
    APP* app;
    BEST_APP_VERSION* bavp;
    BUDA_VARIANT* buda_variant;
    HOST_USAGE host_usage;
        // if is_buda, usage returned by plan class func for chosen variant
        // else a copy of bavp->host_usage

    JOB();
    bool get_score(int);
};

extern void send_work_score();
