// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

#include <vector>

#include "boinc_db.h"

#include "hostinfo.h"
#include "error_numbers.h"
#include "parse.h"

#include "sched_msgs.h"

using std::vector;

struct RSC_JOB_LIMIT {
    int base_limit;     // 0 means no limit
    int scaled_limit;   // the actual limit
    int njobs;
    bool per_proc;      // if true, scaled limit is limit*nprocs

    int parse(XML_PARSER&, const char* end_tag);

    inline void reset(int nprocs) {
        njobs = 0;
        if (per_proc) {
            scaled_limit = base_limit*nprocs;
        } else {
            scaled_limit = base_limit;
        }
    }

    inline bool exceeded() {
        return (scaled_limit && njobs >= scaled_limit);
    }

    inline void register_job() {
        njobs++;
    }
};

struct JOB_LIMIT {
    char app_name[256];
    RSC_JOB_LIMIT total;
    RSC_JOB_LIMIT cpu;
    RSC_JOB_LIMIT gpu;

    int parse(XML_PARSER&, const char* end_tag);

    inline void reset(HOST_INFO& h) {
        total.reset(1);
        cpu.reset(h.p_ncpus);
        gpu.reset(h.coprocs.ndevs());
    }

    inline bool exceeded(bool is_gpu) {
        if (total.exceeded()) return true;
        if (is_gpu) {
            if (gpu.exceeded()) return true;
        } else {
            if (cpu.exceeded()) return true;
        }
        return false;
    }

    inline void register_job(bool is_gpu) {
        total.register_job();
        if (is_gpu) {
            gpu.register_job();
        } else {
            cpu.register_job();
        }
    }
};

struct JOB_LIMITS {
    JOB_LIMIT project_limits;      // project-wide limits
    vector<JOB_LIMIT> app_limits;  // per-app limits

    int parse(XML_PARSER&, const char* end_tag);

    // called at start of each request
    //
    inline void reset(HOST_INFO& h) {
        project_limits.reset(h);
        for (unsigned int i=0; i<app_limits.size(); i++) {
            app_limits[i].reset(h);
        }
    }

    inline JOB_LIMIT* lookup_app(char* name) {
        for (unsigned int i=0; i<app_limits.size(); i++) {
            JOB_LIMIT* jlp = &app_limits[i];
            if (!strcmp(name, jlp->app_name)) {
                return jlp;
            }
        }
        return NULL;
    }

    inline bool exceeded(APP& app, bool is_gpu) {
        if (project_limits.exceeded(is_gpu)) return true;
        JOB_LIMIT* jlp = lookup_app(app.name);
        if (jlp) {
            if (jlp->exceeded(is_gpu)) return true;
        }
        return false;
    }

    inline void register_job(APP& app, bool is_gpu) {
        project_limits.register_job(is_gpu);
        JOB_LIMIT* jlp = lookup_app(app.name);
        if (jlp) {
            jlp->register_job(is_gpu);
        }
    }
};
