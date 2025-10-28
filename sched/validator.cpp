// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// validator - check and validate results, and grant credit
//
// Must be linked with two functions
// check_set(): find a canonical result from a set of results
// check_pair(): compare a result with a canonical result
//
// We recommend that you use the versions of these in validate_util2.cpp,
// in which case you have to supply 3 simpler functions
// init_result()
// compare_results()
// cleanup_result()

//  --app appname
//  [-d N] [--debug_level N]    log verbosity (1=least, 4=most)
//  [--one_pass_N_WU N]         Validate only N WU in one pass, then exit
//  [--one_pass]                make one pass through WU table, then exit
//  [--mod n i]                 process only WUs with (id mod n) == i
//  [--max_granted_credit X]    limit maximum granted credit to X
//  [--update_credited_job]     add userid/wuid pair to credited_job table
//
// credit options.
// The default is to grant credit using an adaptive scheme
// that provides devices neutrality.
// This only works if job sizes are fairly uniform.
//
//  [--no_credit]               don't grant credit
//                              Use this, e.g., if using trickles for credit
//  [--post_assigned_credit]    init_result() must set result.claimed_credit
//  [--credit_from_wu]          get credit from workunit.canonical_credit
//  [--credit_from_runtime]     grant credit based on runtime
//                              use this if big variation between job sizes
//  [--wu_id n]                 Validate WU n (debugging)
//  [--check_punitive]          check for results with long-term failure,
//                              punish host

#include "config.h"
#include <unistd.h>
#include <climits>
#include <cmath>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <string>
#include <signal.h>

#include "boinc_db.h"
#include "util.h"
#include "str_replace.h"
#include "str_util.h"
#include "error_numbers.h"
#include "svn_version.h"
#include "common_defs.h"

#include "credit.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "validator.h"
#include "validate_util.h"
#include "validate_util2.h"
#ifdef GCL_SIMULATOR
#include "gcl_simulator.h"
#endif

using std::vector;

#define LOCKFILE "validate.out"
#define PIDFILE  "validate.pid"

#define SELECT_LIMIT    1000
#define SLEEP_PERIOD    5

int sleep_interval = SLEEP_PERIOD;

typedef enum {
    NEVER,
    DELAYED,
    IMMEDIATE,
    NO_CHANGE
} TRANSITION_TIME;

char app_name[256];
DB_APP app;
DB_APP* g_app = &app;
int wu_id_modulus=0;
int wu_id_remainder=0;
int wu_id_min=0;
int wu_id_max=0;
int one_pass_N_WU=0;
bool one_pass = false;
double max_granted_credit = 200 * 1000 * 365;
    // limit credit to 1 TeraFLOP-year
bool update_credited_job = false;
bool credit_from_wu = false;
bool credit_from_runtime = false;
bool post_assigned_credit = false;
bool no_credit = false;
bool dry_run = false;
bool check_punitive = false;
int wu_id = 0;
int g_argc;
char **g_argv;

WORKUNIT* g_wup;
vector<DB_APP_VERSION_VAL> app_versions;
    // cache of app_versions; the PFC statistics of these are
    // updated in memory, and periodically flushed to the DB

bool is_unreplicated(WORKUNIT& wu) {
    return (wu.target_nresults == 1 && app.target_nresults > 1);
}

// Here when a result has been validated.
// - update consecutive_valid
// - udpdate turnaround stats
// - insert credited_job record if needed
//
int is_valid(
    DB_HOST& host, RESULT& result, WORKUNIT& wu, DB_HOST_APP_VERSION& hav
) {
    DB_CREDITED_JOB credited_job;
    int retval;

    double turnaround = result.received_time - result.sent_time;
    compute_avg_turnaround(host, turnaround);

    // increment daily quota
    //
    hav.max_jobs_per_day++;

    // increment consecutive_valid, but only if unreplicated
    //
    if (!is_unreplicated(wu)) {
        hav.consecutive_valid++;
        log_messages.printf(MSG_DEBUG,
            "[HAV#%lu] consecutive valid now %d\n",
            hav.app_version_id, hav.consecutive_valid
        );
    }

    if (update_credited_job) {
        credited_job.userid = host.userid;
        credited_job.workunitid = long(wu.opaque);
        if (dry_run) {
            log_messages.printf(MSG_NORMAL, "DB not updated (dry run)\n");
        } else {
            retval = credited_job.insert();
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%lu] Warning: credited_job insert failed (userid: %lu workunit: %f err: %s)\n",
                    result.id, host.userid, wu.opaque, boincerror(retval)
                );
            } else {
                log_messages.printf(MSG_DEBUG,
                    "[RESULT#%lu %s] added credited_job record [WU#%lu OPAQUE#%f USER#%lu]\n",
                    result.id, result.name, wu.id, wu.opaque, host.userid
                );
            }
        }
    }

    return 0;
}

static inline void is_invalid(DB_HOST_APP_VERSION& hav) {
    hav.consecutive_valid = 0;
    if (hav.max_jobs_per_day > config.daily_result_quota) {
        hav.max_jobs_per_day--;
    }
}

// check for results with long-term failure; punish those hosts.
//
void scan_punitive(vector<VALIDATOR_ITEM>& items) {
    void* data=NULL;
    char buf[256];

    for (unsigned int i=0; i<items.size(); i++) {
        RESULT& result = items[i].res;
        if (result.server_state != RESULT_SERVER_STATE_OVER) continue;
        if (result.outcome != RESULT_OUTCOME_CLIENT_ERROR) continue;
        if (init_result(result, data) == VAL_RESULT_LONG_TERM_FAIL) {
            DB_HOST_APP_VERSION hav, hav2;
            sprintf(buf, "where host_id=%ld and app_version_id=%ld",
                result.hostid, result.app_version_id
            );
            int retval = hav.lookup(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "scan_punitive(): can't find HAV for results %ld",
                    result.id
                );
                continue;
            }
            hav2 = hav;
            hav2.max_jobs_per_day = 1;
            hav2.n_jobs_today = 1;
            hav2.update_scheduler(hav);
                // update_scheduler() updates the above fields
        }
        if (data) {
            cleanup_result(result, data);
            data = NULL;
        }
    }
}

// handle a workunit which has new results
//
int handle_wu(
    DB_VALIDATOR_ITEM_SET& validator, vector<VALIDATOR_ITEM>& items
) {
    int canonical_result_index = -1;
    bool update_result, retry;
    TRANSITION_TIME transition_time = NO_CHANGE;
    int retval = 0, x;
    DB_ID_TYPE canonicalid = 0;
    double credit = 0;
    unsigned int i;

    WORKUNIT& wu = items[0].wu;
    g_wup = &wu;

    if (check_punitive) {
        scan_punitive(items);
    }

    if (wu.canonical_resultid) {
        log_messages.printf(MSG_NORMAL,
            "[WU#%lu %s] Already has canonical result %lu\n",
            wu.id, wu.name, wu.canonical_resultid
        );

        // Here if WU already has a canonical result.
        // Get unchecked results and see if they match the canonical result
        //
        for (i=0; i<items.size(); i++) {
            RESULT& result = items[i].res;

            if (result.id == wu.canonical_resultid) {
                canonical_result_index = i;
            }
        }
        if (canonical_result_index == -1) {
            log_messages.printf(MSG_CRITICAL,
                "[WU#%lu %s] Can't find canonical result %lu\n",
                wu.id, wu.name, wu.canonical_resultid
            );
            return 0;
        }

        RESULT& canonical_result = items[canonical_result_index].res;

        // scan this WU's results, and check the unchecked ones
        //
        for (i=0; i<items.size(); i++) {
            RESULT& result = items[i].res;

            if (result.server_state != RESULT_SERVER_STATE_OVER) continue;
            if (result.outcome !=  RESULT_OUTCOME_SUCCESS) continue;
            switch (result.validate_state) {
            case VALIDATE_STATE_INIT:
            case VALIDATE_STATE_INCONCLUSIVE:
                break;
            default:
                continue;
            }
            log_messages.printf(MSG_NORMAL,
                 "[WU#%lu] handle_wu(): testing result %lu\n",
                 wu.id, result.id
             );

            check_pair(result, canonical_result, retry);
            if (retry) {
                // this usually means an NFS mount has failed;
                // arrange to try again later.
                //
                transition_time = DELAYED;
                goto leave;
            }
            update_result = false;

            if (result.outcome == RESULT_OUTCOME_VALIDATE_ERROR) {
                update_result = true;
            }

            // this might be last result, so let transitioner
            // trigger file delete etc. if needed
            //
            transition_time = IMMEDIATE;

            DB_HOST host;
            retval = host.lookup_id(result.hostid);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%lu] lookup of host %lu failed: %s\n",
                    result.id, result.hostid, boincerror(retval)
                );
                continue;
            }
            HOST host_initial = host;

            bool update_hav = false;
            DB_HOST_APP_VERSION hav;
            retval = hav_lookup(hav, result.hostid,
                generalized_app_version_id(result.app_version_id, result.appid)
            );
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%lu %s] hav_lookup returned %d\n",
                    result.id, result.name, retval
                );
                hav.host_id = 0;
            }
            DB_HOST_APP_VERSION hav_orig = hav;
            vector<DB_HOST_APP_VERSION> havv;
            havv.push_back(hav);

            vector<RESULT> rv;
            switch (result.validate_state) {
            case VALIDATE_STATE_VALID:
                update_result = true;
                update_hav = true;
                log_messages.printf(MSG_NORMAL,
                    "[RESULT#%lu %s] pair_check() matched: setting result to valid\n",
                    result.id, result.name
                );
                retval = is_valid(host, result, wu, havv[0]);
                if (retval) {
                    log_messages.printf(MSG_NORMAL,
                        "[RESULT#%lu %s] is_valid() error: %s\n",
                        result.id, result.name, boincerror(retval)
                    );
                }
                // do credit computation, but grant credit of canonical result
                //
                rv.push_back(result);
                assign_credit_set(
                    wu, rv, app, app_versions, havv,
                    max_granted_credit, credit
                );
                if (!no_credit) {
                    result.granted_credit = canonical_result.granted_credit;
                    grant_credit(host, result.sent_time, result.granted_credit);
                    if (config.credit_by_app) {
                        grant_credit_by_app(result, result.granted_credit);
                    }
                }
                break;
            case VALIDATE_STATE_INVALID:
                update_result = true;
                update_hav = true;
                log_messages.printf(MSG_NORMAL,
                    "[RESULT#%lu %s] pair_check() didn't match: setting result to invalid\n",
                    result.id, result.name
                );
                is_invalid(havv[0]);
            }
            if (hav.host_id && update_hav) {
                if (dry_run) {
                    log_messages.printf(MSG_NORMAL, "DB not updated (dry run)\n");
                } else {
                    log_messages.printf(MSG_NORMAL,
                        "[HOST#%lu AV#%lu] [outlier=%d] Updating HAV in DB.  pfc.n=%f->%f\n",
                        havv[0].host_id, havv[0].app_version_id,
                        result.runtime_outlier, hav_orig.pfc.n, havv[0].pfc.n
                    );
                    retval=havv[0].update_validator(hav_orig);
                    if (retval) {
                        log_messages.printf(MSG_CRITICAL,
                            "[HOST#%lu AV%lu] hav.update_validator() failed: %s\n",
                            hav.host_id, hav.app_version_id, boincerror(retval)
                        );
                    }
                }
            }
            host.update_diff_validator(host_initial);
            if (update_result) {
                log_messages.printf(MSG_NORMAL,
                    "[RESULT#%lu %s] granted_credit %f\n",
                    result.id, result.name, result.granted_credit
                );
                if (dry_run) {
                    log_messages.printf(MSG_NORMAL, "DB not updated (dry run)\n");
                } else {
                    retval = validator.update_result(result);
                    if (retval) {
                        log_messages.printf(MSG_CRITICAL,
                            "[RESULT#%lu %s] Can't update result: %s\n",
                            result.id, result.name, boincerror(retval)
                        );
                    }
                }
            }
        }
    } else {
        // Here if WU doesn't have a canonical result yet.
        // Try to get one

        vector<RESULT> viable_results;
        vector<DB_HOST_APP_VERSION> host_app_versions, host_app_versions_orig;

        log_messages.printf(MSG_NORMAL,
            "[WU#%lu %s] handle_wu(): No canonical result yet\n",
            wu.id, wu.name
        );

        // make a vector of the "viable" (i.e. possibly canonical) results,
        // and a parallel vector of host_app_versions
        //
        for (i=0; i<items.size(); i++) {
            RESULT& result = items[i].res;

            if (result.server_state != RESULT_SERVER_STATE_OVER) continue;
            if (result.outcome != RESULT_OUTCOME_SUCCESS) continue;
            if (result.validate_state == VALIDATE_STATE_INVALID) continue;

            viable_results.push_back(result);
            DB_HOST_APP_VERSION hav;
            retval = hav_lookup(hav, result.hostid,
                generalized_app_version_id(result.app_version_id, result.appid)
            );
            if (retval) {
                hav.host_id=0;   // flag that it's missing
            }
            host_app_versions.push_back(hav);
            host_app_versions_orig.push_back(hav);
        }

        log_messages.printf(MSG_DEBUG,
            "[WU#%lu %s] Found %d viable results\n",
            wu.id, wu.name, (int)viable_results.size()
        );
        if (viable_results.size() >= (unsigned int)wu.min_quorum) {
            log_messages.printf(MSG_DEBUG,
                "[WU#%lu %s] Enough for quorum, checking set.\n",
                wu.id, wu.name
            );

            double dummy;
            retval = check_set(viable_results, wu, canonicalid, dummy, retry);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[WU#%lu %s] check_set() error: %s\n",
                    wu.id, wu.name, boincerror(retval)
                );
                return retval;
            }
            if (retry) transition_time = DELAYED;

            // if we found a canonical instance, decide on credit
            //
            if (canonicalid) {
                // do the credit calculation, to update statistics,
                // even if we're granting credit a different way
                //
                if (!credit_from_runtime) {
                    retval = assign_credit_set(
                        wu, viable_results, app, app_versions,
                        host_app_versions, max_granted_credit, credit
                    );
                    if (retval) {
                        log_messages.printf(MSG_CRITICAL,
                            "[WU#%lu %s] assign_credit_set(): %s\n",
                            wu.id, wu.name, boincerror(retval)
                        );
                        transition_time = DELAYED;
                        goto leave;
                    }
                }

                if (credit_from_wu) {
                    credit = wu.canonical_credit;
                    if (credit == 0) {
                        log_messages.printf(MSG_CRITICAL,
                            "[WU#%lu %s] credit not specified in WU\n",
                            wu.id, wu.name
                        );
                    }
                } else if (credit_from_runtime) {
                    // take the average of results whose runtime-based credit
                    // is within range
                    //
                    vector<double> cc;
                    for (i=0; i<viable_results.size(); i++) {
                        RESULT& result = viable_results[i];
                        if (result.flops_estimate == 1e9) {
                            result.flops_estimate = AVG_CPU_FPOPS;
                        }
                        double runtime = result.elapsed_time;
                        double pfc = result.flops_estimate * runtime;
                        if (pfc > wu.rsc_fpops_bound*2) {
                            pfc = wu.rsc_fpops_bound;
                        }
                        double c = pfc * COBBLESTONE_SCALE;
                        if (c <=0 || c > max_granted_credit) {
                            log_messages.printf(MSG_CRITICAL,
                                "[WU#%lu %s] credit out of range: %f\n",
                                wu.id, wu.name, c
                            );
                        } else {
                            cc.push_back(c);
                        }

                        log_messages.printf(MSG_NORMAL,
                            "[WU#%lu][RESULT#%lu] credit_from_runtime %.2f = %.0fs * %.2fGFLOPS\n",
                            wu.id, result.id,
                            c, runtime, result.flops_estimate/1e9
                        );
                    }
                    if (cc.size()) {
                        credit = low_average(cc);
                        log_messages.printf(MSG_DEBUG,
                            "[WU#%lu %s] credit from runtime: %f\n",
                            wu.id, wu.name, credit
                        );
                    } else {
                        log_messages.printf(MSG_CRITICAL,
                            "[WU#%lu %s] credit from runtime: no results have valid credit\n",
                            wu.id, wu.name
                        );
                        credit = 0;
                    }
                } else if (post_assigned_credit) {
                    credit = 0;
                    for (i=0; i<viable_results.size(); i++) {
                        RESULT& result = viable_results[i];
                        if (result.id != canonicalid) {
                            continue;
                        }
                        credit = result.claimed_credit;
                        break;
                    }
                    if (credit == 0) {
                        log_messages.printf(MSG_CRITICAL,
                            "[WU#%lu %s] post-assigned credit missing\n",
                            wu.id, wu.name
                        );
                    }
                } else if (no_credit) {
                    credit = 0;
                }
                if (max_granted_credit && credit>max_granted_credit) {
                    credit = max_granted_credit;
                }
            }

            // scan the viable results.
            // update as needed,
            // and count the # of results that are still viable
            // (some may now have outcome VALIDATE_ERROR,
            // or validate_state INVALID)
            //
            int n_viable_results = 0;
            for (i=0; i<viable_results.size(); i++) {
                RESULT& result = viable_results[i];
                DB_HOST_APP_VERSION& hav = host_app_versions[i];
                DB_HOST_APP_VERSION& hav_orig = host_app_versions_orig[i];

                update_result = false;
                bool update_host = false;

                if (result.outcome != RESULT_OUTCOME_SUCCESS
                    || result.validate_state == VALIDATE_STATE_INVALID
                ) {
                    transition_time = IMMEDIATE;
                    update_result = true;
                } else {
                    n_viable_results++;
                }

                DB_HOST host;
                HOST host_initial;
                switch (result.validate_state) {
                case VALIDATE_STATE_VALID:
                case VALIDATE_STATE_INVALID:
                    retval = host.lookup_id(result.hostid);
                    if (retval) {
                        log_messages.printf(MSG_CRITICAL,
                            "[RESULT#%lu] lookup of host %lu: %s\n",
                            result.id, result.hostid, boincerror(retval)
                        );
                        continue;
                    }
                    host_initial = host;
                }

                switch (result.validate_state) {
                case VALIDATE_STATE_VALID:
                    update_result = true;
                    update_host = true;
                    retval = is_valid(host, result, wu, host_app_versions[i]);
                    if (retval) {
                        log_messages.printf(MSG_DEBUG,
                            "[RESULT#%lu %s] is_valid() failed: %s\n",
                            result.id, result.name, boincerror(retval)
                        );
                    }
                    if (!no_credit) {
                        result.granted_credit = credit;
                        grant_credit(host, result.sent_time, credit);
                        log_messages.printf(MSG_NORMAL,
                            "[RESULT#%lu %s] Valid; granted %f credit [HOST#%lu]\n",
                            result.id, result.name, result.granted_credit,
                            result.hostid
                        );
                        if (config.credit_by_app) {
                            grant_credit_by_app(result, credit);
                        }
                    }
                    break;
                case VALIDATE_STATE_INVALID:
                    update_result = true;
                    update_host = true;
                    log_messages.printf(MSG_NORMAL,
                        "[RESULT#%lu %s] Invalid [HOST#%lu]\n",
                        result.id, result.name, result.hostid
                    );
                    is_invalid(host_app_versions[i]);
                    break;
                case VALIDATE_STATE_INIT:
                    log_messages.printf(MSG_NORMAL,
                        "[RESULT#%lu %s] Inconclusive [HOST#%lu]\n",
                        result.id, result.name, result.hostid
                    );
                    result.validate_state = VALIDATE_STATE_INCONCLUSIVE;
                    update_result = true;
                    break;
                }

                if (dry_run) {
                    log_messages.printf(MSG_NORMAL, "DB not updated (dry run)\n");
                } else {
                    if (hav.host_id) {
                        log_messages.printf(MSG_NORMAL,
                            "[HOST#%lu AV#%lu] [outlier=%d] Updating HAV in DB.  pfc.n=%f->%f\n",
                            hav.host_id, hav.app_version_id,
                            result.runtime_outlier, hav_orig.pfc.n, hav.pfc.n
                        );
                        retval = hav.update_validator(hav_orig);
                        if (retval) {
                            log_messages.printf(MSG_CRITICAL,
                                "[HOST#%lu AV%lu] hav.update_validator() failed: %s\n",
                                hav.host_id, hav.app_version_id, boincerror(retval)
                            );
                        }
                    }
                    if (update_host) {
                        retval = host.update_diff_validator(host_initial);
                        if (retval) {
                            log_messages.printf(MSG_CRITICAL,
                                "[HOST#%lu] host.update_diff_validator() failed: %s\n",
                                host.id, boincerror(retval)
                            );
                        }
                    }
                    if (update_result) {
                        retval = validator.update_result(result);
                        if (retval) {
                            log_messages.printf(MSG_CRITICAL,
                                "[RESULT#%lu %s] result.update() failed: %s\n",
                                result.id, result.name, boincerror(retval)
                            );
                        }
                    }
                }
            }

            if (canonicalid) {
                // if we found a canonical result,
                // trigger the assimilator, but do NOT trigger
                // the transitioner - doing so creates a race condition
                //
                transition_time = NEVER;
                log_messages.printf(MSG_DEBUG,
                    "[WU#%lu %s] Found a canonical result: id=%lu\n",
                    wu.id, wu.name, canonicalid
                );
                wu.canonical_resultid = canonicalid;
                wu.canonical_credit = credit;
                wu.assimilate_state = ASSIMILATE_READY;

                // don't need to send any more results
                //
                for (i=0; i<items.size(); i++) {
                    RESULT& result = items[i].res;

                    if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
                        continue;
                    }

                    result.server_state = RESULT_SERVER_STATE_OVER;
                    result.outcome = RESULT_OUTCOME_DIDNT_NEED;
                    if (dry_run) {
                        log_messages.printf(MSG_NORMAL, "DB not updated (dry run)\n");
                    } else {
                        retval = validator.update_result(result);
                        if (retval) {
                            log_messages.printf(MSG_CRITICAL,
                                "[RESULT#%lu %s] result.update() failed: %s\n",
                                result.id, result.name, boincerror(retval)
                            );
                        }
                    }
                }
            } else {
                // here if no consensus.

                // check if #viable results is too large
                //
                if (n_viable_results > wu.max_success_results) {
                    wu.error_mask |= WU_ERROR_TOO_MANY_SUCCESS_RESULTS;
                    transition_time = IMMEDIATE;
                }

                // if #viable results >= target_nresults,
                // we need more results, so bump target_nresults
                // NOTE: n_viable_results should never be > target_nresults,
                // but accommodate that if it should happen
                //
                if (n_viable_results >= wu.target_nresults) {
                    wu.target_nresults = n_viable_results+1;
                    transition_time = IMMEDIATE;
                }
            }
        }
    }

leave:

    switch (transition_time) {
    case IMMEDIATE:
        wu.transition_time = time(0);
        break;
    case DELAYED:
        x = time(0) + 6*3600;
        if (x < wu.transition_time) wu.transition_time = x;
        break;
    case NEVER:
        wu.transition_time = INT_MAX;
        break;
    case NO_CHANGE:
        break;
    }

    wu.need_validate = 0;

    if (dry_run) {
        log_messages.printf(MSG_NORMAL, "DB not updated (dry run)\n");
    } else {
        retval = validator.update_workunit(wu);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[WU#%lu %s] update_workunit() failed: %s\n",
                wu.id, wu.name, boincerror(retval)
            );
            return retval;
        }
    }
    return 0;
}

// make one pass through the workunits with need_validate set.
// return true if there were any
//
bool do_validate_scan() {
    DB_VALIDATOR_ITEM_SET validator;
    std::vector<VALIDATOR_ITEM> items;
    bool found=false;
    int retval, i=0;

    // loop over entries that need to be checked
    //
    while (1) {
        if (wu_id) {
            // kludge to tell enumerate to return a given WU
            wu_id_modulus = 1;
            wu_id_remainder = wu_id;
        }
        retval = validator.enumerate(
            app.id, SELECT_LIMIT, wu_id_modulus, wu_id_remainder,
            wu_id_min, wu_id_max, items
        );
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_DEBUG,
                    "DB connection lost, exiting\n"
                );
                exit(0);
            }
            break;
        }
        retval = handle_wu(validator, items);
        if (!retval) found = true;
        if (++i == one_pass_N_WU) break;
        if (wu_id) break;
        if (dry_run) break;  // otherwise it will enumerate forever
    }
    return found;
}

int main_loop() {
    int retval;
    bool did_something;
    char buf[256];

    sprintf(buf, "where name='%s'", app_name);

    while (1) {
        check_stop_daemons();

        // look up app within the loop,
        // in case its min_avg_pfc has been changed by the feeder
        //
        retval = app.lookup(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
            exit(1);
        }
        did_something = do_validate_scan();
        if (!did_something) {
            write_modified_app_versions(app_versions);
            if (one_pass) break;
#ifdef GCL_SIMULATOR
            char nameforsim[64];
            sprintf(nameforsim, "validator%i", app.id);
            continue_simulation(nameforsim);
            signal(SIGUSR2, simulator_signal_handler);
            pause();
#else
            daemon_sleep(sleep_interval);
#endif
        }
        if (one_pass) break;
    }
    return 0;
}

void usage(char* name) {
    fprintf(stderr,
        "This program is a 'validator'; it handles completed tasks.\n"
        "Normally it is run as a daemon from config.xml.\n"
        "See: https://github.com/BOINC/boinc/wiki/BackendPrograms\n\n"
    );

    fprintf(stderr, "usage: %s [options]\n"
        "    Options:\n"
        "    --app name                 Process tasks for the given application\n"
        "    [--one_pass_N_WU N]        Validate at most N WUs, then exit\n"
        "    [--one_pass]               Make one pass through WU table, then exit\n"
        "    [--dry_run]                Don't update db, just write logs (for debugging)\n"
        "    [--mod n i]                Process only WUs with (id mod n) == i\n"
        "    [--max_wu_id n]            Process only WUs with id <= n\n"
        "    [--min_wu_id n]            Process only WUs with id >= n\n"
        "    [--max_granted_credit X]   Grant no more than this amount of credit to a result\n"
        "    [--update_credited_job]    Add record to credited_job table after granting credit\n"
        "    [--credit_from_wu]         Credit is specified in WU XML\n"
        "    [--credit_from_runtime X]  Grant credit based on runtime (max X seconds)and estimated FLOPS\n"
        "    [--no_credit]              Don't grant credit\n"
        "    [--check_punitive]         Check failed results and reduce the daily quota to one.\n"
        "    [--sleep_interval n]       Set sleep-interval to n\n"
        "    [--wu_id n]                Process WU with given ID\n"
        "    [-d level|--debug_level n] Set log verbosity level\n"
        "    [-h|--help]                Print this usage information and exit\n"
        "    [-v|--version]             Print version information and exit\n"
        "\n",
        name
    );
    validate_handler_usage();

}

// For use by project-supplied routines check_set() and check_pair()
//
int debug_level=0;

int main(int argc, char** argv) {
    int i, retval;

    if (argc > 1) {
      if (is_arg(argv[1], "h") || is_arg(argv[1], "help")) {
        usage(argv[0]);
        exit(0);
      } else if (is_arg(argv[1], "v") || is_arg(argv[1], "version")) {
        printf("%s\n", SVN_VERSION);
        exit(0);
      }
    }

    check_stop_daemons();

    int j=1;
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass_N_WU")) {
            one_pass_N_WU = atoi(argv[++i]);
            one_pass = true;
        } else if (is_arg(argv[i], "sleep_interval")) {
            sleep_interval = atoi(argv[++i]);
        } else if (is_arg(argv[i], "dry_run")) {
            dry_run = true;
        } else if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "app")) {
            safe_strcpy(app_name, argv[++i]);
        } else if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            debug_level = atoi(argv[++i]);
            log_messages.set_debug_level(debug_level);
            if (debug_level == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "mod")) {
            wu_id_modulus = atoi(argv[++i]);
            wu_id_remainder = atoi(argv[++i]);
        } else if (is_arg(argv[i], "min_wu_id")) {
            wu_id_min = atoi(argv[++i]);
        } else if (is_arg(argv[i], "max_wu_id")) {
            wu_id_max = atoi(argv[++i]);
        } else if (is_arg(argv[i], "max_granted_credit")) {
            max_granted_credit = atof(argv[++i]);
        } else if (is_arg(argv[i], "update_credited_job")) {
            update_credited_job = true;
        } else if (is_arg(argv[i], "credit_from_wu")) {
            credit_from_wu = true;
        } else if (is_arg(argv[i], "post_assigned_credit")) {
            post_assigned_credit = true;
        } else if (is_arg(argv[i], "credit_from_runtime")) {
            credit_from_runtime = true;
        } else if (is_arg(argv[i], "no_credit")) {
            no_credit = true;
        } else if (is_arg(argv[i], "wu_id")) {
            wu_id = atoi(argv[++i]);
            one_pass = true;
        } else if (is_arg(argv[i], "check_punitive")) {
            check_punitive = true;
        } else {
            // unknown arg - pass to handler
            argv[j++] = argv[i];
        }
    }

    if (app_name[0] == 0) {
        log_messages.printf(MSG_CRITICAL,
            "must use '--app' to specify an application\n"
        );
        usage(argv[0]);
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open failed: %s\n", boinc_db.error_string()
        );
        exit(1);
    }

    if (credit_from_runtime) {
        if (max_granted_credit == 0) {
            log_messages.printf(MSG_CRITICAL,
                "if use credit_from_runtime, must specify max credit\n"
            );
            exit(1);
        }
        log_messages.printf(MSG_NORMAL,
            "using credit from runtime\n"
        );
    }

    if (max_granted_credit) {
        log_messages.printf(MSG_NORMAL,
            "max_granted_credit: %f\n", max_granted_credit
        );
    }

    if (wu_id_modulus) {
        log_messages.printf(MSG_NORMAL,
            "Modulus %d, remainder %d\n", wu_id_modulus, wu_id_remainder
        );
    }
    if (wu_id_min) {
        log_messages.printf(MSG_NORMAL,
            "min wu id %d\n", wu_id_min
        );
    }
    if (wu_id_max) {
        log_messages.printf(MSG_NORMAL,
            "max wu id %d\n", wu_id_max
        );
    }

    argv[j] = 0;
    retval = validate_handler_init(j, argv);
    if (retval) exit(1);

    log_messages.printf(MSG_NORMAL,
        "Starting validator, debug level %d\n", log_messages.debug_level
    );

    install_stop_signal_handler();

    main_loop();
}
