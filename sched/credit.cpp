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
// Modify claimed credit based on the historical granted credit if
// the project is configured to do this
//

// functions related to the computation and granting of credit
// Note: this is credit.cpp rather than sched_credit.cpp
// because you might grant credit e.g. from a trickle handler

#include <math.h>

#include "boinc_db.h"
#include "error_numbers.h"

#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "sched_shmem.h"
#include "sched_types.h"

#include "credit.h"

// TODO: delete
double fpops_to_credit(double fpops, double intops) {
    // TODO: use fp_weight if specified in config file
    double fpc = (fpops/1e9)*COBBLESTONE_FACTOR/SECONDS_PER_DAY;
    double intc = (intops/1e9)*COBBLESTONE_FACTOR/SECONDS_PER_DAY;
    return std::max(fpc, intc);
}

// TODO: delete
double credit_multiplier(int appid, time_t create_time) {
    DB_CREDIT_MULTIPLIER mult;
    mult.get_nearest(appid, create_time);
    return mult.multiplier;
}

// TODO: delete
static void modify_credit_rating(HOST& host) {
    double new_claimed_credit = 0;
    double percent_difference = 0;
    // The percent difference between claim and history
    double difference_weight = 1;
    // The weight to be applied based on the difference between claim and
    // history
    double credit_weight = 1;
    // The weight to be applied based on how much credit the host has earned
    // (hosts that are new do not have accurate histories so they shouldn't
    // have much weight)
    double combined_weight = 1;

    // Only modify if the credit_per_cpu_sec is established
    // and the option is enabled
    //
    if (host.credit_per_cpu_sec > 0
        && config.granted_credit_weight > 0.0
        && config.granted_credit_weight <= 1.0
    ) {

        // Calculate the difference between claimed credit and the hosts
        // historical granted credit history
        percent_difference=host.claimed_credit_per_cpu_sec-host.credit_per_cpu_sec;
        percent_difference = fabs(percent_difference/host.credit_per_cpu_sec);

        // A study on World Community Grid determined that 50% of hosts
        // claimed within 10% of their historical credit per cpu sec.
        // These hosts should not have their credit modified.
        //
        if (percent_difference < 0.1) {
             log_messages.printf(MSG_DEBUG,
                 "[HOSTID:%d] Claimed credit %.1lf not "
                 "modified.  Percent Difference %.4lf\n", host.id,
                 host.claimed_credit_per_cpu_sec*86400, percent_difference
             );
            return;
        }

        // The study also determined that 95% of hosts claim within
        // 50% of their historical credit per cpu sec.
        // Computers claiming above 10% but below 50% should have their
        // credit adjusted based on their history
        // Computers claiming more than 50% above should use their
        // historical value.
        if (percent_difference < .5) {
            // weight based on variance from historical credit
            difference_weight = 1-(0.5-percent_difference)/0.4;
        } else {
            difference_weight = 1;
        }

        // A weight also needs to be calculated based upon the amount of
        // credit awarded to a host.  This is becuase hosts without much
        // credit awarded do not yet have an accurate history so the weight
        // should be limited for these hosts.
        if (config.granted_credit_ramp_up) {
            credit_weight=config.granted_credit_ramp_up - host.total_credit;
            credit_weight=credit_weight/config.granted_credit_ramp_up;
            if (credit_weight < 0) credit_weight = 0;
            credit_weight = 1 - credit_weight;
        }

        // Compute the combined weight
        combined_weight = credit_weight*difference_weight*config.granted_credit_weight;
        log_messages.printf(MSG_DEBUG, "[HOSTID:%d] Weight details: "
            "diff_weight=%.4lf credit_weight=%.4lf config_weight=%.4lf\n",
            host.id, difference_weight, credit_weight,
            config.granted_credit_weight
        );

        // Compute the new value for claimed credit
        new_claimed_credit=(1-combined_weight)*host.claimed_credit_per_cpu_sec;
        new_claimed_credit=new_claimed_credit+combined_weight*host.credit_per_cpu_sec;

        if (new_claimed_credit < host.claimed_credit_per_cpu_sec) {
            log_messages.printf(MSG_DEBUG,
                "[HOSTID:%d] Modified claimed credit "
                 "(lowered) original: %.1lf new: %.1lf historical: %.1lf "
                 "combined weight: %.4lf\n", host.id,
                 host.claimed_credit_per_cpu_sec*86400,
                 new_claimed_credit*86400, host.credit_per_cpu_sec*86400,
                 combined_weight
            );
        } else {
            log_messages.printf(MSG_DEBUG,
                "[HOSTID:%d] Modified claimed credit "
                "(increased) original: %.1lf new: %.1lf historical: %.1lf "
                "combined weight: %.4lf\n", host.id,
                host.claimed_credit_per_cpu_sec*86400,
                new_claimed_credit*86400, host.credit_per_cpu_sec*86400,
                combined_weight
            );
        }

        host.claimed_credit_per_cpu_sec = new_claimed_credit;
    }
}

// TODO: delete
// somewhat arbitrary formula for credit as a function of CPU time.
// Could also include terms for RAM size, network speed etc.
//
void compute_credit_rating(HOST& host) {
    double fpw, intw, scale, x;
    if (config.use_benchmark_weights) {

        fpw = config.fp_benchmark_weight;
        intw = 1. - fpw;

        // FP benchmark is 2x int benchmark, on average.
        // Compute a scaling factor the gives the same credit per day
        // no matter how benchmarks are weighted
        //
        scale = 1.5 / (2*intw + fpw);
    } else {
        fpw = .5;
        intw = .5;
        scale = 1;
    }
    x = fpw*fabs(host.p_fpops) + intw*fabs(host.p_iops);
    x /= 1e9;
    x *= COBBLESTONE_FACTOR;
    x /= SECONDS_PER_DAY;
    x *= scale;
    host.claimed_credit_per_cpu_sec  = x;

    if (config.granted_credit_weight) {
        modify_credit_rating(host);
    }
}

// TODO: delete
// This function should be called from the validator whenever credit
// is granted to a host.  It's purpose is to track the average credit
// per cpu time for that host.
//
// It updates an exponentially-decaying estimate of credit_per_cpu_sec
// Note that this does NOT decay with time, but instead decays with
// total credits earned.  If a host stops earning credits, then this
// quantity stops decaying.  So credit_per_cpu_sec must NOT be
// periodically decayed using the update_stats utility or similar
// methods.
//
// The intended purpose is for cross-project credit comparisons on
// BOINC statistics pages, for hosts attached to multiple machines.
// One day people will write PhD theses on how to normalize credit
// values to equalize them across projects.  I hope this will be done
// according to "Allen's principle": "Credits granted by a project
// should be normalized so that, averaged across all hosts attached to
// multiple projects, projects grant equal credit per cpu second."
// This principle ensures that (on average) participants will choose
// projects based on merit, not based on credits.  It also ensures
// that (on average) host machines migrate to the projects for which
// they are best suited.
//
// For cross-project comparison the value of credit_per_cpu_sec should
// be exported in the statistics file host_id.gz, which is written by
// the code in db_dump.C.
//
// Algorithm: credits_per_cpu_second should be updated each time that
// a host is granted credit, according to:
//
//     CREDIT_AVERAGE_CONST = 500           [see Note 5]
//     MAX_CREDIT_PER_CPU_SEC = 0.1         [see Note 6]
//
//     e = tanh(granted_credit/CREDIT_AVERAGE_CONST)
//     if (e < 0) then e = 0
//     if (e > 1) then e = 1
//     if (credit_per_cpu_sec <= 0) then e = 1
//     if (cpu_time <= 0) then e = 0        [see Note 4]
//     if (granted_credit <= 0) then e = 0  [see Note 3]
//
//     rate = granted_credit/cpu_time
//     if (rate < 0) rate = 0
//     if (rate > MAX_CREDIT_PER_CPU_SEC) rate = MAX_CREDIT_PER_CPU_SEC
//
//     credit_per_cpu_sec = e * rate + (1 - e) * credit_per_cpu_sec

// Note 0: all quantities above should be treated as real numbers
// Note 1: cpu_time is measured in seconds
// Note 2: When a host is created, the initial value of
//         credit_per_cpu_sec, should be zero.
// Note 3: If a host has done invalid work (granted_credit==0) we have
//         chosen not to include it.  One might argue that the
//         boundary case granted_credit==0 should be treated the same
//         as granted_credit>0.  However the goal here is not to
//         identify cpus whose host machines sometimes produce
//         rubbish.  It is to get a measure of how effectively the cpu
//         runs the application code.
// Note 4: e==0 means 'DO NOT include the first term on the rhs of the
//         equation defining credit_per_cpu_sec' which is equivalent
//         to 'DO NOT update credit_per_cpu_sec'.
// Note 5: CREDIT_AVERAGE_CONST determines the exponential decay
//         credit used in averaging credit_per_cpu_sec.  It may be
//         changed at any time, even if the project database has
//         already been populated with non-zero values of
//         credit_per_cpu_sec.
// Note 6: Typical VERY FAST cpus have credit_per_cpu_sec of around
//         0.02.  This is a safety mechanism designed to prevent
//         trouble if a client or host has reported absurd values (due
//         to a bug in client or server software or by cheating).  In
//         five years when cpus are five time faster, please increase
//         the value of R.  You may also want to increase the value of
//         CREDIT_AVERAGE_CONST.
//
//         Nonzero return value: host exceeded the max allowed
//         credit/cpu_sec.
//
int update_credit_per_cpu_sec(
    double  granted_credit,     // credit granted for this work
    double  cpu_time,           // cpu time (seconds) used for this work
    double& credit_per_cpu_sec  // (average) credit per cpu second
) {
    int retval = 0;

    // Either of these values may be freely changed in the future.
    // When CPUs get much faster one must increase the 'sanity-check'
    // value of max_credit_per_cpu_sec.  At that time it would also
    // make sense to proportionally increase the credit_average_const.
    //
    const double credit_average_const = 500;
    const double max_credit_per_cpu_sec = 0.07;

    double e = tanh(granted_credit/credit_average_const);
    if (e <= 0.0 || cpu_time == 0.0 || granted_credit == 0.0) return retval;
    if (e > 1.0 || credit_per_cpu_sec == 0.0) e = 1.0;

    double rate =  granted_credit/cpu_time;
    if (rate < 0.0) rate = 0.0;
    if (rate > max_credit_per_cpu_sec) {
        rate = max_credit_per_cpu_sec;
        retval = 1;
    }

    credit_per_cpu_sec = e * rate + (1.0 - e) * credit_per_cpu_sec;

    return retval;
}

// Grant the host (and associated user and team)
// the given amount of credit for work that started at the given time.
// Update the user and team records,
// but not the host record (caller must update)
//
int grant_credit(
    DB_HOST& host, double start_time, double cpu_time, double credit
) {
    DB_USER user;
    DB_TEAM team;
    int retval;
    char buf[256];

    // first, process the host

    update_average(
        start_time, credit, CREDIT_HALF_LIFE,
        host.expavg_credit, host.expavg_time
    );

    // then the user

    retval = user.lookup_id(host.userid);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "lookup of user %d failed %d\n",
            host.userid, retval
        );
        return retval;
    }

    update_average(
        start_time, credit, CREDIT_HALF_LIFE,
        user.expavg_credit, user.expavg_time
    );
    sprintf(
        buf, "total_credit=total_credit+%.15e, expavg_credit=%.15e, expavg_time=%.15e",
        credit,  user.expavg_credit, user.expavg_time
    );
    retval = user.update_field(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "update of user %d failed %d\n",
             host.userid, retval
        );
    }

    // and finally the team

    if (user.teamid) {
        retval = team.lookup_id(user.teamid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "lookup of team %d failed %d\n",
                user.teamid, retval
            );
            return retval;
        }
        update_average(
            start_time, credit, CREDIT_HALF_LIFE,
            team.expavg_credit, team.expavg_time
        );
        sprintf(buf,
            "total_credit=total_credit+%.15e, expavg_credit=%.15e, expavg_time=%.15e",
            credit,  team.expavg_credit, team.expavg_time
        );
        retval = team.update_field(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "update of team %d failed %d\n",
                team.id, retval
            );
        }
    }
    return 0;
}

///////////////////// V2 CREDIT STUFF STARTS HERE ///////////////////


// parameters for maintaining averages.
// per-host averages respond faster to change

#define HAV_AVG_THRESH  20
#define HAV_AVG_WEIGHT  .01
#define HAV_AVG_LIMIT   10

#define AV_AVG_THRESH   100
#define AV_AVG_WEIGHT   .001
#define AV_AVG_LIMIT    10

#define PFC_MODE_NORMAL  0
    // PFC was computed in the "normal" way,
    // i.e. not anon platform, and reflects version scaling
#define PFC_MODE_APPROX  1
    // PFC was crudely approximated

// used in the computation of AV scale factors
//
struct RSC_INFO {
    double pfc_sum;
    double pfc_n;
    int nvers_thresh;   // # app versions w/ lots of samples
    int nvers_total;

    RSC_INFO() {
        pfc_sum = 0;
        pfc_n = 0;
        nvers_thresh = 0;
        nvers_total = 0;
    }
    void update(APP_VERSION& av) {
        nvers_total++;
        if (av.pfc.n > MIN_VERSION_SAMPLES) {
            nvers_thresh++;
            pfc_sum += av.pfc.get_avg() * av.pfc.n;
            pfc_n += av.pfc.n;
        }
    }
    double avg() {
        return pfc_sum/pfc_n;
    }
};

// "avg" is the average PFC for this app
// over CPU versions or GPU versions, whichever is lowest.
// Update the pfc_scale of this app's versions in the DB,
// and update app.min_avg_pfc
//
int scale_versions(APP& app, double avg, SCHED_SHMEM* ssp) {
    char buf[256];
    int retval;

    for (int j=0; j<ssp->napp_versions; j++) {
        APP_VERSION& av = ssp->app_versions[j];
        if (av.appid != app.id) continue;
        if (av.pfc.n < MIN_VERSION_SAMPLES) continue;
        av.pfc_scale= avg/av.pfc.get_avg();

        DB_APP_VERSION dav;
        dav.id = av.id;
        sprintf(buf, "pfc_scale=%.15e", av.pfc_scale);
        retval = dav.update_field(buf);
        if (retval) return retval;
        if (config.debug_credit) {
            PLATFORM* p = ssp->lookup_platform_id(av.platformid);
            log_messages.printf(MSG_NORMAL,
                " updating scale factor for %d (%s %s)\n",
                av.id, p->name, av.plan_class
            );
            log_messages.printf(MSG_NORMAL,
                "  n: %g avg PFC: %g new scale: %g\n",
                av.pfc.n, av.pfc.get_avg(), av.pfc_scale
            );
        }
    }
    app.min_avg_pfc = avg;
    DB_APP da;
    da.id = app.id;
    sprintf(buf, "min_avg_pfc=%.15e", avg);
    retval = da.update_field(buf);
    if (retval) return retval;
    return 0;
}

// Update app version scale factors,
// and find the min average PFC for each app.
// Called periodically from the master feeder.
//
int update_av_scales(SCHED_SHMEM* ssp) {
    int i, j, retval;
    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL, "-- updating app version scales --\n");
    }
    for (i=0; i<ssp->napps; i++) {
        APP& app = ssp->apps[i];
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL, "app %s (%d)\n", app.name, app.id);
        }
        RSC_INFO cpu_info, gpu_info;

        // find the average PFC of CPU and GPU versions

        for (j=0; j<ssp->napp_versions; j++) {
            APP_VERSION& avr = ssp->app_versions[j];
            if (avr.appid != app.id) continue;
            DB_APP_VERSION av;
            retval = av.lookup_id(avr.id);
            if (retval) return retval;
            avr = av;       // update shared mem array
            if (strstr(av.plan_class, "cuda") || strstr(av.plan_class, "ati")) {
                if (config.debug_credit) {
                    log_messages.printf(MSG_NORMAL,
                        "add to gpu totals: (%d %s) %g %g\n",
                        av.id, av.plan_class, av.pfc.n, av.pfc.get_avg()
                    );
                }
                gpu_info.update(av);
            } else {
                if (config.debug_credit) {
                    log_messages.printf(MSG_NORMAL,
                        "add to cpu totals: (%d %s) %g %g\n",
                        av.id, av.plan_class, av.pfc.n, av.pfc.get_avg()
                    );
                }
                cpu_info.update(av);
            }
        }

        // If there are only CPU or only GPU versions,
        // and at least 2 are above threshold, normalize to the average
        //
        // If there are both, and at least 1 of each is above threshold,
        // normalize to the min of the averages
        //
        if (cpu_info.nvers_total) {
            if (gpu_info.nvers_total) {
                if (cpu_info.nvers_thresh && gpu_info.nvers_thresh) {
                    if (config.debug_credit) {
                        log_messages.printf(MSG_NORMAL,
                            "CPU avg: %g; GPU avg: %g\n",
                            cpu_info.avg(), gpu_info.avg()
                        );
                    }
                    scale_versions(app,
                        cpu_info.avg()<gpu_info.avg()?cpu_info.avg():gpu_info.avg(),
                        ssp
                    );
                }
            } else {
                if (cpu_info.nvers_thresh > 1) {
                    log_messages.printf(MSG_NORMAL,
                        "CPU avg: %g\n", cpu_info.avg()
                    );
                    scale_versions(app, cpu_info.avg(), ssp);
                }
            }
        } else {
            if (gpu_info.nvers_thresh > 1) {
                log_messages.printf(MSG_NORMAL,
                    "GPU avg: %g\n", gpu_info.avg()
                );
                scale_versions(app, gpu_info.avg(), ssp);
            }
        }


    }
    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL, "-------------\n");
    }
    return 0;
}

// look up or create a HOST_APP_VERSION record
//
int hav_lookup(DB_HOST_APP_VERSION& hav, int hostid, int avid) {
    int retval;
    char buf[256];
    sprintf(buf, "where host_id=%d and app_version_id=%d", hostid, avid);
    retval = hav.lookup(buf);
    if (retval == ERR_DB_NOT_FOUND) {
        hav.clear();
        hav.host_id = hostid;
        hav.app_version_id = avid;
        retval = hav.insert();
        if (retval) return retval;
    }
    return retval;
}

DB_APP_VERSION* av_lookup(int id, vector<DB_APP_VERSION>& app_versions) {
    for (unsigned int i=0; i<app_versions.size(); i++) {
        if (app_versions[i].id == id) {
            return &app_versions[i];
        }
    }
    DB_APP_VERSION av;
    int retval = av.lookup_id(id);
    if (retval) return NULL;
    app_versions.push_back(av);
    return &(app_versions[app_versions.size()-1]);
}

// called from the validator (get_pfc()).
// Do a non-careful update.
//
static int write_hav(DB_HOST_APP_VERSION& hav) {
    char set_clause[8192], where_clause[8192];
    sprintf(set_clause,
        "pfc_n=%.15e, "
        "pfc_avg=%.15e, "
        "et_n=%.15e, "
        "et_avg=%.15e, "
        "et_q=%.15e, "
        "et_var=%.15e, "
        "turnaround_n=%.15e, "
        "turnaround_avg=%.15e, "
        "turnaround_q=%.15e, "
        "turnaround_var=%.15e",
        hav.pfc.n,
        hav.pfc.avg,
        hav.et.n,
        hav.et.avg,
        hav.et.q,
        hav.et.var,
        hav.turnaround.n,
        hav.turnaround.avg,
        hav.turnaround.q,
        hav.turnaround.var
    );
    if (hav.host_scale_time < dtime()) {
        strcat (set_clause, ", scale_probation=0");
    }
    sprintf(where_clause,
        "host_id=%d and app_version_id=%d ",
        hav.host_id, hav.app_version_id
    );
    return hav.update_fields_noid(set_clause, where_clause);
}

// Compute or estimate "claimed peak FLOP count".
// Possibly update host_app_version records and write to DB.
// Possibly update app_version records in memory and let caller write to DB,
// to merge DB writes
//
int get_pfc(
    RESULT& r, WORKUNIT& wu, DB_APP& app,       // in
    vector<DB_APP_VERSION>&app_versions,        // in/out
    double& pfc, int& mode                      // out
) {
    DB_HOST_APP_VERSION hav;
    DB_APP_VERSION* avp=0;
    int retval;

    mode = PFC_MODE_APPROX;

    // is result from old scheduler that didn't set r.app_version_id correctly?
    // if so, use WU estimate (this is a transient condition)
    //
    if (r.app_version_id == 0 || r.app_version_id == 1) {
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%d] app_version_id is %d: returning WU default %f\n",
                r.id, r.app_version_id, wu.rsc_fpops_est
            );
        }
        pfc = wu.rsc_fpops_est;
        return 0;
    }

    // temporary kludge for SETI@home:
    // if GPU initialization fails the app falls back to CPU.
    //
    if (strstr(r.stderr_out, "Device Emulation (CPU)")) {
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%d] CUDA app fell back to CPU\n",
                r.id
            );
        }
        pfc = wu.rsc_fpops_est;
        return 0;
    }

    int gavid = generalized_app_version_id(r.app_version_id, r.appid);
    retval = hav_lookup(hav, r.hostid, gavid);
    if (retval) return retval;

    // old clients report CPU time but not elapsed time.
    // Use HOST_APP_VERSION.et to track distribution of CPU time.
    //
    if (!r.elapsed_time) {
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%d] old client (elapsed time not reported)\n",
                r.id
            );
        }
        hav.et.update_var(
            r.cpu_time/wu.rsc_fpops_est,
            HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
        );
        pfc = app.min_avg_pfc*wu.rsc_fpops_est;
        bool do_scale = true;
        if (hav.et.n < MIN_HOST_SAMPLES) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] old client: not scaling - too few samples %f\n",
                    r.id, hav.et.n
                );
            }
        }
        if (do_scale && dtime() < hav.host_scale_time) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] old client: not scaling - scale probation\n",
                    r.id
                );
            }
        }
        if (do_scale) {
            double s = r.cpu_time/(hav.et.get_avg()*wu.rsc_fpops_est);
            pfc *= s;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] old client: scaling (based on CPU time) by %g, return %f\n",
                    r.id, s, pfc
                );
            }
        }
        write_hav(hav);
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%d] old client: returning PFC %f\n",
                r.id, pfc
            );
        }
        return 0;
    }

    double raw_pfc = (r.elapsed_time * r.flops_estimate);

    // Sanity check
    //
    if (raw_pfc > wu.rsc_fpops_bound) {
        char query[256], clause[256];
        if (app.min_avg_pfc) {
            pfc = app.min_avg_pfc * wu.rsc_fpops_est;
        } else {
            pfc = wu.rsc_fpops_est;
        }
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "{credit] [RESULT#%d] get_pfc: sanity check failed: %f>%f, return %f\n",
                r.id, raw_pfc, wu.rsc_fpops_bound, pfc
            );
        }
        sprintf(query, "scale_probation=1 and error_rate=%f", ERROR_RATE_INIT);
        sprintf(clause, "host_id=%d and app_version_id=%d", r.hostid, gavid);
        retval = hav.update_fields_noid(query, clause);
        return retval;
    }

    if (r.app_version_id < 0) {
        // anon platform
        //
        if (app.min_avg_pfc) {
            bool do_scale = true;
            if (hav.pfc.n < MIN_HOST_SAMPLES) {
                do_scale = false;
                if (config.debug_credit) {
                    log_messages.printf(MSG_NORMAL,
                        "[credit] [RESULT#%d] get_pfc: anon platform, not scaling, too few samples %f\n",
                        r.id, hav.pfc.n
                    );
                }
            }
            if (do_scale && dtime() < hav.host_scale_time) {
                do_scale = false;
                if (config.debug_credit) {
                    log_messages.printf(MSG_NORMAL,
                        "[credit] [RESULT#%d] get_pfc: anon platform, not scaling, host probation\n",
                        r.id
                    );
                }
            }
            if (do_scale) {
                double scale = app.min_avg_pfc / hav.pfc.get_avg();
                pfc = raw_pfc * scale;
                if (config.debug_credit) {
                    log_messages.printf(MSG_NORMAL,
                        "[credit] [RESULT#%d] get_pfc: anon platform, scaling by %g (%g/%g)\n",
                        r.id, scale, app.min_avg_pfc , hav.pfc.get_avg()
                    );
                }
            } else {
                pfc = app.min_avg_pfc * wu.rsc_fpops_est;
            }
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] get_pfc: anon platform, returning %f\n",
                    r.id, pfc
                );
            }
        } else {
            pfc = wu.rsc_fpops_est;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] get_pfc: anon platform, no app.min_avg_pfc; returning %f\n",
                    r.id, pfc
                );
            }
        }
    } else {
        avp = av_lookup(r.app_version_id, app_versions);
        if (!avp) {
            log_messages.printf(MSG_CRITICAL,
                "get_pfc() [RESULT#%d]: No AVP %d!!\n", r.id, r.app_version_id
            );
            return ERR_NOT_FOUND;
        }
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%d] normal case. sec: %.0f GFLOPS: %.0f raw PFC: %fG\n",
                r.id, r.elapsed_time, r.flops_estimate/1e9, raw_pfc/1e9
            );
        }

        bool do_scale = true;
        double host_scale = 0;
        if (dtime() < hav.host_scale_time) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] not host scaling - host probation\n",
                    r.id
                );
            }
        }
        if (do_scale && hav.pfc.n < MIN_HOST_SAMPLES) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] not host scaling - HAV PFC too few samples %f\n",
                    r.id, hav.pfc.n
                );
            }
        }
        if (do_scale && avp->pfc.n < MIN_VERSION_SAMPLES) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] not host scaling - app_version PFC too few samples%f\n",
                    r.id, avp->pfc.n
                );
            }
        }
        if (do_scale && hav.pfc.get_avg() == 0) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] not host scaling - HAV PFC is zero\n",
                    r.id
                );
            }
        }
        if (do_scale) {
            host_scale = avp->pfc.get_avg()/hav.pfc.get_avg();
            if (host_scale > 10) host_scale = 10;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] host scale: %g (%g/%g)\n",
                    r.id, host_scale, avp->pfc.get_avg(), hav.pfc.get_avg()
                );
            }
        }

        pfc = raw_pfc;
        if (avp->pfc_scale) {
            pfc *= avp->pfc_scale;
            if (host_scale) {
                pfc *= host_scale;
                mode = PFC_MODE_NORMAL;
            }
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] applying app version scale %f\n",
                    r.id, avp->pfc_scale
                );
            }
        } else {
            if (host_scale) {
                pfc *= host_scale;
            }
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] no app version scale\n",
                    r.id
                );
            }
        }
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%d] get_pfc(): updating app version %d PFC avgs with %g (%g/%g)\n",
                r.id, avp->id, pfc/wu.rsc_fpops_est, pfc, wu.rsc_fpops_est
            );
        }
        avp->pfc_samples.push_back(raw_pfc/wu.rsc_fpops_est);
    }

    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL,
            "[credit] [RESULT#%d] updating HAV PFC %f et %.15e turnaround %d\n",
            r.id, raw_pfc/wu.rsc_fpops_est,
            r.elapsed_time/wu.rsc_fpops_est,
            (r.received_time - r.sent_time)
        );
    }
                
    hav.pfc.update(
        raw_pfc/wu.rsc_fpops_est,
        HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
    );
    hav.et.update_var(
        r.elapsed_time/wu.rsc_fpops_est,
        HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
    );
    hav.turnaround.update_var(
        (r.received_time - r.sent_time),
        HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
    );

    write_hav(hav);

    // keep track of credit per app version
    //
    if (avp) {
        avp->credit_samples.push_back(pfc*COBBLESTONE_SCALE);
        avp->credit_times.push_back(r.sent_time);
    }

    return 0;
}

// Called by validator when canonical result has been selected.
// Compute credit for valid instances, store in result.granted_credit
// and return as credit
//
int assign_credit_set(
    WORKUNIT& wu, vector<RESULT>& results,
    DB_APP& app, vector<DB_APP_VERSION>& app_versions,
    double max_granted_credit, double& credit
) {
    unsigned int i;
    int n_normal=0, n_total=0, mode, retval;
    double sum_normal=0, sum_total=0, pfc;
    char query[256];

    for (i=0; i<results.size(); i++) {
        RESULT& r = results[i];
        if (r.validate_state != VALIDATE_STATE_VALID) continue;
        retval= get_pfc(r, wu, app, app_versions, pfc, mode);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "get_pfc() error: %d\n", retval);
            return retval;
        } else {
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%d] get_pfc() returns pfc %g (credit %g) mode %s\n",
                    r.id, pfc, pfc*COBBLESTONE_SCALE, (mode==PFC_MODE_NORMAL)?"normal":"approx"
                );
            }
        }
        if (max_granted_credit && pfc*COBBLESTONE_SCALE > max_granted_credit) {
            log_messages.printf(MSG_NORMAL,
                "Credit too high: %f\n", pfc*COBBLESTONE_SCALE
            );
            exit(1);
        }
        if (mode == PFC_MODE_NORMAL) {
            sum_normal += pfc;
            n_normal++;
        }
        sum_total += pfc;
        n_total++;
    }

    // averaging policy: if there is least one normal result,
    // use the average of normal results.
    // Otherwise use the average of all results
    //
    double x;
    if (n_normal) {
        x = sum_normal/n_normal;
    } else {
        x = sum_total/n_total;
    }
    x *= COBBLESTONE_SCALE;
    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL,
            "[credit] [WU#%d] assign_credit_set: credit %g\n",
            wu.id, x
        );
    }
    for (i=0; i<results.size(); i++) {
        RESULT& r = results[i];
        if (r.validate_state != VALIDATE_STATE_VALID) continue;
        r.granted_credit = x;
    }
    credit = x;
    return 0;
}

// A job has errored or timed out; put (host/app_version) on probation
// Called from transitioner
//
int host_scale_probation(
    DB_HOST& host, int appid, int app_version_id, double latency_bound
) {
    DB_HOST_APP_VERSION hav;
    char query[256], clause[512];

    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL,
            "[credit] [HOST#%d] Imposing scale probation\n", host.id
        );
    }
    sprintf(query,
        "scale_probation=1, pfc_n=0, pfc_avg=0, host_scale_time=%f",
        dtime()+latency_bound
    );
    sprintf(clause,
        "host_id=%d and app_version_id=%d",
        host.id, generalized_app_version_id(appid, app_version_id)
    );
    return hav.update_fields_noid(query, clause);
}

// carefully write any app_version records that have changed;
// done at the end of every validator scan.
//
int write_modified_app_versions(vector<DB_APP_VERSION>& app_versions) {
    unsigned int i, j;
    int retval = 0;

    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL,
            "[credit] start write_modified_app_versions()\n"
        );
    }
    for (i=0; i<app_versions.size(); i++) {
        DB_APP_VERSION& av = app_versions[i];
        if (av.pfc_samples.empty() && av.credit_samples.empty()) {
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] skipping app version %d - no change\n", av.id
                );
            }
            continue;
        }
        while (1) {
            double pfc_n_orig = av.pfc.n;
            double expavg_credit_orig = av.expavg_credit;

            for (j=0; j<av.pfc_samples.size(); j++) {
                av.pfc.update(
                    av.pfc_samples[j],
                    AV_AVG_THRESH, AV_AVG_WEIGHT, AV_AVG_LIMIT
                );
            }
            for (j=0; j<av.credit_samples.size(); j++) {
                update_average(
                    av.credit_times[j], av.credit_samples[j], CREDIT_HALF_LIFE,
                    av.expavg_credit, av.expavg_time
                );
            }
            char query[512], clause[512];
            sprintf(query,
                "pfc_n=%.15e, pfc_avg=%.15e, expavg_credit=%.15e, expavg_time=%f",
                av.pfc.n,
                av.pfc.avg,
                av.expavg_credit,
                av.expavg_time
            );
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] updating app version %d:\n", av.id
                );
                log_messages.printf(MSG_NORMAL,
                    "[credit] pfc.n = %f, pfc.avg = %f, expavg_credit = %f, expavg_time=%f\n",
                    av.pfc.n,
                    av.pfc.avg,
                    av.expavg_credit,
                    av.expavg_time
                );
            }
            // if pfc_scale has changed (from feeder) reread it
            //
            sprintf(clause,
                "pfc_n=%.15e and abs(expavg_credit-%.15e)<1e4 and abs(pfc_scale-%.15e)<1e6",
                pfc_n_orig, expavg_credit_orig, av.pfc_scale
            );
            retval = av.update_field(query, clause);
            if (retval) break;
            if (boinc_db.affected_rows() == 1) break;
            retval = av.lookup_id(av.id);
            if (retval) break;
        }
        av.pfc_samples.clear();
        av.credit_samples.clear();
        av.credit_times.clear();
        if (retval) return retval;
    }
    return 0;
}
