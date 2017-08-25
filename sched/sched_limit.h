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

#ifndef BOINC_SCHED_LIMIT_H
#define BOINC_SCHED_LIMIT_H

#include <vector>

#include "boinc_db.h"

#include "hostinfo.h"
#include "error_numbers.h"
#include "parse.h"

#include "sched_msgs.h"

using std::vector;

// represents a limit on # of jobs in progress for a given processor type
//
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

    inline bool any_limit() {
        return (base_limit != 0);
    }

    void print_log(const char*);

    RSC_JOB_LIMIT() {
        base_limit = 0;
        scaled_limit = 0;
        njobs = 0;
        per_proc = false;
    }
};

// represents limits for a given app (or overall, if app_name is empty)
//
struct JOB_LIMIT {
    char app_name[256];
    RSC_JOB_LIMIT total;
    RSC_JOB_LIMIT proc_type_limits[NPROC_TYPES];

    int parse(XML_PARSER&, const char* end_tag);

    inline void reset(int ninstances[]) {
        total.reset(1);
        for (int i=0; i<NPROC_TYPES; i++) {
            proc_type_limits[i].reset(ninstances[i]);
        }
    }

    inline bool exceeded(int proc_type) {
        if (total.exceeded()) return true;
        if (proc_type_limits[proc_type].exceeded()) return true;
        return false;
    }

    inline void register_job(int proc_type) {
        total.register_job();
        proc_type_limits[proc_type].register_job();
    }

    inline bool any_limit() {
        if (total.any_limit()) return true;
        for (int i=0; i<NPROC_TYPES; i++) {
            if (proc_type_limits[i].any_limit()) return true;
        }
        return false;
    }

    void print_log();
};

// combined limits, overall and per app
//
struct JOB_LIMITS {
    JOB_LIMIT project_limits;      // project-wide limits
    vector<JOB_LIMIT> app_limits;  // per-app limits

    int parse(XML_PARSER&, const char* end_tag);
    void print_log();

    // called at start of each request
    //
    inline void reset(int ninstances[]) {
        project_limits.reset(ninstances);
        for (unsigned int i=0; i<app_limits.size(); i++) {
            app_limits[i].reset(ninstances);
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

    inline bool exceeded(APP* app, int proc_type) {
        if (project_limits.exceeded(proc_type)) return true;
        if (app) {
            JOB_LIMIT* jlp = lookup_app(app->name);
            if (jlp) {
                if (jlp->exceeded(proc_type)) return true;
            }
        }
        return false;
    }

    inline void register_job(APP* app, int proc_type) {
        project_limits.register_job(proc_type);
        if (app) {
            JOB_LIMIT* jlp = lookup_app(app->name);
            if (jlp) {
                jlp->register_job(proc_type);
            }
        }
    }
};

#endif
