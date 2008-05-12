// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
//

// Replace the following with your own code.
// WARNING: after doing this, you must prevent this file from
// being overwritten the next time you update BOINC source code.
// You can either:
// 1) write-protect this file, or
// 2) put this in a differently-named file and change the Makefile.am
//    (you need to prevent that from being overwritten too)
// In either case, put your version under source-code control, e.g. SVN

#include "sched_msgs.h"
#include "sched_plan.h"

// return the number of usable CPUs, taking prefs into account.
// If prefs limit apply, set bounded to true.
//
static void get_ncpus(SCHEDULER_REQUEST& sreq, int& ncpus, bool& bounded) {
    ncpus = sreq.host.p_ncpus;
    bounded = false;
    if (sreq.global_prefs.max_ncpus_pct && sreq.global_prefs.max_ncpus_pct < 100) {
        bounded = true;
        ncpus = (int)((ncpus*sreq.global_prefs.max_ncpus_pct)/100.);
    }
}

bool app_plan(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu) {
    if (!strcmp(plan_class, "mt")) {
        // the following is for an app that can use anywhere
        // from 1 to 64 threads, can control this exactly,
        // and whose speedup is .95N
        // (so on a uniprocessor, we'll use a sequential app
        // if one is available)
        //
        int ncpus, nthreads;
        bool bounded;

        get_ncpus(sreq, ncpus, bounded);
        nthreads = ncpus;
        if (nthreads > 64) nthreads = 64;
        hu.avg_ncpus = nthreads;
        hu.max_ncpus = nthreads;
        sprintf(hu.cmdline, "--nthreads %d", nthreads);
        hu.flops = 0.95*sreq.host.p_fpops*nthreads;
        return true;
    } else if (!strcmp(plan_class, "cuda")) {
        // the following is for an app that uses a CUDA GPU
        // and some CPU also, and gets 50 GFLOPS total
        //
        for (unsigned int i=0; i<sreq.coprocs.coprocs.size(); i++) {
            COPROC* cp = sreq.coprocs.coprocs[i];
            if (!strcmp(cp->type, "CUDA")) {
                COPROC* cu = new COPROC (cp->type);
                cu->count = 1;
                hu.coprocs.coprocs.push_back(cu);
                double x = 1e9/sreq.host.p_fpops;
                if (x > 1) x = 1;
                hu.avg_ncpus = x;
                hu.max_ncpus = x;
                hu.flops = 5e11;
                return true;
            }
        }
        return false;
    }
    log_messages.printf(MSG_CRITICAL,
        "Unknown plan class: %s\n", plan_class
    );
    return false;
}
