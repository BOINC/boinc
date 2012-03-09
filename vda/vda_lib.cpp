// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

#include <stdio.h>
#include <string.h>

#include <vector>
#include <set>
#include <algorithm>
#include <math.h>
#include <limits.h>

using std::vector;
using std::set;

#include "vda_lib.h"

META_CHUNK::META_CHUNK(
    VDA_FILE_AUX* d, META_CHUNK* par, double size,
    int coding_level, int index
) {
    dfile = d;
    parent = par;
    coding = d->policy.codings[coding_level];
    if (parent) {
        sprintf(name, "%s.%d", parent->name, index);
    } else {
        sprintf(name, "%d", index);
    }
    if (coding_level<d->policy.coding_levels-1) {
        for (int j=0; j<coding.m; j++) {
            children.push_back(new META_CHUNK(
                d,
                this,
                size/coding.n,
                coding_level+1,
                j
            ));
        }
    } else {
        for (int j=0; j<coding.m; j++) {
            children.push_back(
                new CHUNK(this, size/coding.n, j)
            );
        }
    }
}

// sort by increasing cost
//
bool compare_cost(const DATA_UNIT* d1, const DATA_UNIT* d2) {
    return d1->cost < d2->cost;
}

// sort by increase min_failures
//
bool compare_min_failures(const DATA_UNIT* d1, const DATA_UNIT* d2) {
    return d1->min_failures < d2->min_failures;
}

// Recovery logic: decide what to do in response to
// host failures and upload/download completions.
//
// One way to do this would be to store a bunch of state info
// with each node in the file's tree,
// and do things by local tree traversal.
//
// However, it's simpler to store minimal state info,
// and to reconstruct state info using
// a top-down tree traversal in response to each event.
// Actually we do 2 traversals:
// 1) plan phase:
//      We see whether each node is recoverable,
//      and if so compute its "recovery set":
//      the set of children from which it can be recovered
//      with minimal cost (i.e. network traffic).
//      Decide whether each chunk currently on the server needs to remain.
// 2) action phase
//      Based on the results of phase 1,
//      decide whether to start upload/download of chunks,
//      and whether to delete data currently on server
//
int META_CHUNK::recovery_plan() {
    vector<DATA_UNIT*> recoverable;
    vector<DATA_UNIT*> present;

    unsigned int i;
    have_unrecoverable_children = false;

    // make lists of children in various states
    //
    for (i=0; i<children.size(); i++) {
        DATA_UNIT* c = children[i];
        c->in_recovery_set = false;
        c->data_needed = false;
        c->data_now_present = false;
        int retval = c->recovery_plan();
        if (retval) return retval;
        switch (c->status) {
        case PRESENT:
            present.push_back(c);
            break;
        case RECOVERABLE:
            recoverable.push_back(c);
            break;
        case UNRECOVERABLE:
            have_unrecoverable_children = true;
            break;
        }
    }

    // based on states of children, decide what state we're in
    //
    if ((int)(present.size()) >= coding.n) {
        status = PRESENT;
        sort(present.begin(), present.end(), compare_cost);
        present.resize(coding.n);
        cost = 0;
        for (i=0; i<present.size(); i++) {
            DATA_UNIT* c= present[i];
            cost += c->cost;
            c->in_recovery_set = true;
        }
    } else if ((int)(present.size() + recoverable.size()) >= coding.n) {
        status = RECOVERABLE;
        unsigned int j = coding.n - present.size();
        sort(recoverable.begin(), recoverable.end(), compare_cost);
        cost = 0;
        for (i=0; i<present.size(); i++) {
            DATA_UNIT* c= present[i];
            c->in_recovery_set = true;
        }
        for (i=0; i<j; i++) {
            DATA_UNIT* c= recoverable[i];
            c->in_recovery_set = true;
            cost += c->cost;
        }

    } else {
        status = UNRECOVERABLE;
    }
    return 0;
}

int CHUNK::recovery_plan() {
    if (present_on_server) {
        status = PRESENT;
        cost = 0;
        min_failures = INT_MAX;
    } else if (hosts.size() > 0) {
        status = RECOVERABLE;
        cost = size;
        if ((int)(hosts.size()) < parent->dfile->policy.replication) {
            data_needed = true;
        }
        min_failures = hosts.size();
    } else {
        status = UNRECOVERABLE;
        min_failures = 0;
    }
#ifdef DEBUG_RECOVERY
    printf("chunk plan %s: status %s\n", name, status_str(status));
#endif
    return 0;
}

int META_CHUNK::recovery_action(double now) {
    unsigned int i;
    int retval;

    if (data_now_present) {
        status = PRESENT;
    }
#ifdef DEBUG_RECOVERY
    printf("meta chunk action %s state %s unrec children %d\n",
        name, status_str(status), have_unrecoverable_children
    );
#endif
    for (i=0; i<children.size(); i++) {
        DATA_UNIT* c = children[i];
#ifdef DEBUG_RECOVERY
        printf("  child %s status %s in rec set %d\n",
            c->name, status_str(c->status), c->in_recovery_set
        );
#endif
        switch (status) {
        case PRESENT:
            if (c->status == UNRECOVERABLE) {
                c->data_now_present = true;
            }
            break;
        case RECOVERABLE:
            if (c->in_recovery_set && have_unrecoverable_children) {
                c->data_needed = true;
            }
            break;
        case UNRECOVERABLE:
            break;
        }
        retval = c->recovery_action(now);
        if (retval) return retval;
    }

    // because of recovery action,
    // some of our children may have changed status and fault tolerance,
    // so ours may have changed too.
    // Recompute them.
    //
    vector<DATA_UNIT*> recoverable;
    vector<DATA_UNIT*> present;
    for (i=0; i<children.size(); i++) {
        DATA_UNIT* c = children[i];
        switch (c->status) {
        case PRESENT:
            present.push_back(c);
            break;
        case RECOVERABLE:
            recoverable.push_back(c);
            break;
        }
    }
    if ((int)(present.size()) >= coding.n) {
        status = PRESENT;
        min_failures = INT_MAX;
    } else if ((int)(present.size() + recoverable.size()) >= coding.n) {
        status = RECOVERABLE;

        // our min_failures is the least X such that some X host failures
        // would make this node unrecoverable
        //
        sort(recoverable.begin(), recoverable.end(), compare_min_failures);
        min_failures = 0;
        unsigned int k = coding.n - present.size();
            // we'd need to recover K recoverable children
        unsigned int j = recoverable.size() - k + 1;
            // a loss of J recoverable children would make this impossible

        // the loss of J recoverable children would make us unrecoverable
        // Sum the min_failures of the J children with smallest min_failures
        //
        for (i=0; i<j; i++) {
            DATA_UNIT* c = recoverable[i];
            printf("  Min failures of %s: %d\n", c->name, c->min_failures);
            min_failures += c->min_failures;
        }
        printf("  our min failures: %d\n", min_failures);
    }
    return 0;
}

bool CHUNK::download_in_progress() {
    set<VDA_CHUNK_HOST*>::iterator i;
    for (i=hosts.begin(); i!=hosts.end(); i++) {
        VDA_CHUNK_HOST* ch = *i;
        if (ch->download_in_progress()) return true;
    }
    return false;
}

int CHUNK::recovery_action(double now) {
    int retval;
    VDA_FILE_AUX* fp = parent->dfile;
    if (data_now_present) {
        present_on_server = true;
        fp->disk_usage.sample_inc(
            size,
            fp->collecting_stats(),
            now
        );
        status = PRESENT;
    }
    if (status == PRESENT && (int)(hosts.size()) < fp->policy.replication) {
        retval = assign();
        if (retval) return retval;
    }
    if (download_in_progress()) {
        data_needed = true;
    }
#ifdef DEBUG_RECOVERY
    printf("chunk action: %s data_needed %d present_on_server %d\n",
        name, data_needed, present_on_server
    );
#endif
    if (data_needed) {
        if (!present_on_server) {
            retval = start_upload();
            if (retval) return retval;
        }
    } else {
        if (present_on_server) {
            present_on_server = false;
            status = RECOVERABLE;
            min_failures = fp->policy.replication;
#ifdef EVENT_DEBUG
            printf("%s: %s replicated, removing from server\n", now_str(), name);
#endif
            parent->dfile->disk_usage.sample_inc(
                -size,
                fp->collecting_stats(),
                now
            );
        }
    }
    return 0;
}

#if 0
void META_CHUNK::decide_reconstruct() {
    if (some_child_is_unrecoverable()) {
        if (status == PRESENT) {
            need_reconstruct = true;
        } else if (status == RECOVERABLE) {
            need_present = true;
            for (i=0; i<children.size(); i++) {
                DATA_UNIT* c = children[i];
                if (c.in_recovery_set) {
                    if (c.status == PRESENT) {
                        c.keep_present = true;
                    } else {
                        c.need_present = true;
                    }
                }
            }
        }
    }
    if (needed_by_parent) {
        need_reconstruct = true;
    }
    if (need_reconstruct and !bottom_level()) {
        int n = 0;
        for (i=0; i<children.size(); i++) {
            DATA_UNIT* c = children[i];
            if (c.status == PRESENT) {
                c.needed_by_parent = true;
                n++;
                if (n == coding.n) {
                    break;
                }
            }
        }
    }
    if (keep_present) {
        int n = 0;
        for (i=0; i<children.size(); i++) {
            DATA_UNIT* c = children[i];
            if (c.status == PRESENT) {
                c.keep_present = true;
                n++;
                if (n == coding.n) {
                    break;
                }
            }
        }
    }
    if (!bottom_level()) {
        for (i=0; i<children.size(); i++) {
            META_CHUNK* c = (META_CHUNK*)children[i];
            c.decide_reconstruct();
        }
    }
}

#endif
