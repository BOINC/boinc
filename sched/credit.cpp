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

#include <cmath>

#include "boinc_db.h"
#include "error_numbers.h"

#include "sched_config.h"
#include "sched_customize.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "sched_shmem.h"
#include "sched_types.h"

#include "credit.h"

double fpops_to_credit(double fpops) {
    return fpops*COBBLESTONE_SCALE;
}

double cpu_time_to_credit(double cpu_time, double cpu_flops_sec) {
    return fpops_to_credit(cpu_time*cpu_flops_sec);
}

// Grant the host (and associated user and team)
// the given amount of credit for work that started at the given time.
// Update the user and team records,
// but not the host record (caller must update)
//
int grant_credit(DB_HOST &host, double start_time, double credit) {
    DB_USER user;
    DB_TEAM team;
    int retval;
    char buf[256];
    double now = dtime();

    // first, process the host

    update_average(
        now,
        start_time, credit, CREDIT_HALF_LIFE,
        host.expavg_credit, host.expavg_time
    );
    host.total_credit += credit;

    // then the user

    retval = user.lookup_id(host.userid);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "lookup of user %lu failed: %s\n",
            host.userid, boincerror(retval)
        );
        return retval;
    }

    update_average(
        now,
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
            "update of user %lu failed: %s\n",
            host.userid, boincerror(retval)
        );
    }

    // and finally the team

    if (user.teamid) {
        retval = team.lookup_id(user.teamid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "lookup of team %lu failed: %s\n",
                user.teamid, boincerror(retval)
            );
            return retval;
        }
        update_average(
            now,
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
                "update of team %lu failed: %s\n",
                team.id, boincerror(retval)
            );
        }
    }
    return 0;
}

int grant_credit_by_app(RESULT& result, double credit) {
    DB_CREDIT_USER cu;
    char clause1[1024], clause2[1024];
    double now = dtime();

    sprintf(clause1, "where userid=%lu and appid=%lu", result.userid, result.appid);
    int retval = cu.lookup(clause1);
    if (retval) {
        cu.clear();
        cu.userid = result.userid;
        cu.appid = result.appid;
        cu.expavg_time = dtime();
        retval = cu.insert();
        if (retval) return retval;
    }
    update_average(
        now,
        result.sent_time,
        credit, CREDIT_HALF_LIFE,
        cu.expavg, cu.expavg_time
    );
    sprintf(clause1,
        "total=total+%.15e, expavg=%.15e, expavg_time=%.15e, njobs=njobs+1",
        credit, cu.expavg, cu.expavg_time
    );
    sprintf(clause2,
        "userid=%lu and appid=%lu", result.userid, result.appid
    );
    retval = cu.update_fields_noid(clause1, clause2);
    if (retval) return retval;

    // team.  keep track of credit for zero (no team) also
    //
    DB_CREDIT_TEAM ct;
    sprintf(clause1, "where teamid=%lu and appid=%lu", result.teamid, result.appid);
    retval = ct.lookup(clause1);
    if (retval) {
        ct.clear();
        ct.teamid = result.teamid;
        ct.appid = result.appid;
        ct.expavg_time = dtime();
        retval = ct.insert();
        if (retval) return retval;
    }
    update_average(
        now,
        result.sent_time,
        credit, CREDIT_HALF_LIFE,
        ct.expavg, ct.expavg_time
    );
    sprintf(clause1,
        "total=total+%.15e, expavg=%.15e, expavg_time=%.15e, njobs=njobs+1",
        credit, ct.expavg, ct.expavg_time
    );
    sprintf(clause2,
        "teamid=%lu and appid=%lu", result.teamid, result.appid
    );
    retval = ct.update_fields_noid(clause1, clause2);
    if (retval) return retval;
    return 0;
}

///////////////////// V2 CREDIT STUFF STARTS HERE ///////////////////

// levels of confidence in a credit value
//
#define PFC_MODE_NORMAL  0
// PFC was computed in the "normal" way, i.e.
// - claimed PFC
// - app version scaling (i.e. not anonymous platform)
// - host scaling
#define PFC_MODE_APPROX  1
// PFC was approximated, but still (in the absence of cheating)
// reflects the size of the particular job
#define PFC_MODE_WU_EST  2
// PFC was set to the WU estimate.
// If this doesn't reflect the WU size, neither does the PFC estimate
// This is a last resort, and can be way off.
#define PFC_MODE_INVALID    3
// PFC exceeded max granted credit - ignore

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
    void update(APP_VERSION &av) {
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
int scale_versions(APP &app, double avg, SCHED_SHMEM *ssp) {
    char buf[256];
    int retval;

    for (int j=0; j<ssp->napp_versions; j++) {
        APP_VERSION &av = ssp->app_versions[j];
        if (av.appid != app.id) {
            continue;
        }
        if (av.pfc.n < MIN_VERSION_SAMPLES) {
            continue;
        }
        av.pfc_scale= avg/av.pfc.get_avg();

        DB_APP_VERSION dav;
        dav.id = av.id;
        sprintf(buf, "pfc_scale=%.15e", av.pfc_scale);
        retval = dav.update_field(buf);
        if (retval) {
            return retval;
        }
        if (config.debug_credit) {
            PLATFORM *p = ssp->lookup_platform_id(av.platformid);
            log_messages.printf(MSG_NORMAL,
                " updating scale factor for %lu (%s %s)\n",
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
    if (retval) {
        return retval;
    }
    return 0;
}

#define DEFAULT_GPU_SCALE   0.1
// if there are no CPU versions to compare against,
// multiply pfc_scale of GPU versions by this.
// This reflects the average lower efficiency of GPU apps compared to CPU apps.
// The observed values from SETI@home and Milkyway are 0.05 and 0.08.
// We'll be a little generous and call it 0.1

// Update app version scale factors,
// and find the min average PFC for each app.
// Called periodically from the master feeder.
//
int update_av_scales(SCHED_SHMEM *ssp) {
    int i, j, retval;
    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL, "-- updating app version scales --\n");
    }
    for (i=0; i<ssp->napps; i++) {
        APP &app = ssp->apps[i];
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL, "app %s (%lu)\n", app.name, app.id);
        }
        RSC_INFO cpu_info, gpu_info;

        // find the average PFC of CPU and GPU versions

        for (j=0; j<ssp->napp_versions; j++) {
            APP_VERSION &avr = ssp->app_versions[j];
            if (avr.appid != app.id) {
                continue;
            }
            DB_APP_VERSION av;
            retval = av.lookup_id(avr.id);
            if (retval) {
                return retval;
            }
            avr = av;       // update shared mem array
            if (plan_class_to_proc_type(av.plan_class) != PROC_TYPE_CPU) {
                if (config.debug_credit) {
                    log_messages.printf(MSG_NORMAL,
                        "add to gpu totals: (%lu %s) %g %g\n",
                        av.id, av.plan_class, av.pfc.n, av.pfc.get_avg()
                    );
                }
                gpu_info.update(av);
            } else {
                if (config.debug_credit) {
                    log_messages.printf(MSG_NORMAL,
                        "add to cpu totals: (%lu %s) %g %g\n",
                        av.id, av.plan_class, av.pfc.n, av.pfc.get_avg()
                    );
                }
                cpu_info.update(av);
            }
        }

        // If there are both CPU and GPU versions,
        // and at least 1 of each is above threshold,
        // normalize to the min of the averages
        //
        // Otherwise, if either CPU or GPU has at least
        // 2 versions above threshold, normalize to the average
        //
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
        } else if (cpu_info.nvers_thresh > 1) {
            log_messages.printf(MSG_NORMAL,
                "CPU avg: %g\n", cpu_info.avg()
            );
            scale_versions(app, cpu_info.avg(), ssp);
        } else if (gpu_info.nvers_thresh > 0) {
            log_messages.printf(MSG_NORMAL,
                "GPU avg: %g\n", gpu_info.avg()
            );
            scale_versions(app, gpu_info.avg()*DEFAULT_GPU_SCALE, ssp);
        }
    }
    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL, "-------------\n");
    }
    return 0;
}

// look up HOST_APP_VERSION record; called from validator and transitioner.
// Normally the record will exist; if not:
// look for another HOST_APP_VERSION for the same
// (host, app, platform, plan class).
// If find one, used it and update its app_version_id.
// Otherwise create a new one.
//
// This means that when a new app version is released,
// it inherits the runtime and reliability statistics of the old version.
// This is not always ideal (the new version may be faster/slower)
// but it's better than starting the statistics from scratch.
//
// (if anonymous platform, skip the above)
//
int hav_lookup(
    DB_HOST_APP_VERSION &hav, DB_ID_TYPE hostid, DB_ID_TYPE gen_avid
) {
    int retval;
    char buf[256];
    sprintf(buf, "where host_id=%lu and app_version_id=%ld", hostid, gen_avid);
    retval = hav.lookup(buf);
    if (retval != ERR_DB_NOT_FOUND) return retval;

    // Here no HOST_APP_VERSION currently exists.
    // If gen_avid is negative (anonymous platform) just make one
    //
    if (gen_avid < 0) {
        hav.clear();
        hav.host_id = hostid;
        hav.app_version_id = gen_avid;
        return hav.insert();
    }

    // otherwise try to appropriate an existing one as described above
    //
    DB_HOST_APP_VERSION hav2, best_hav;
    DB_APP_VERSION av, av2, best_av;

    retval = av.lookup_id(gen_avid);
    if (retval) return retval;

    // find the HOST_APP_VERSION w/ latest version num
    // for this (app/platform/plan class) and appropriate it
    //
    bool found = false;
    sprintf(buf, "where host_id=%lu", hostid);
    while (1) {
        retval = hav2.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) return retval;
        retval = av2.lookup_id(hav2.app_version_id);
        if (retval) continue;
        if (av2.appid != av.appid) continue;
        if (av2.platformid != av.platformid) continue;
        if (strcmp(av2.plan_class, av.plan_class)) continue;
        if (found) {
            if (av2.version_num > best_av.version_num) {
                best_av = av2;
                best_hav = hav2;
            }
        } else {
            found = true;
            best_av = av2;
            best_hav = hav2;
        }
    }
    if (found) {
        hav = best_hav;
        char query[256], where_clause[256];
        sprintf(query, "app_version_id=%ld", gen_avid);
        sprintf(where_clause,
            "host_id=%lu and app_version_id=%ld",
            hostid, best_av.id
        );
        retval = hav.update_fields_noid(query, where_clause);
        if (retval) return retval;
    } else {
        hav.clear();
        hav.host_id = hostid;
        hav.app_version_id = gen_avid;
        retval = hav.insert();
        if (retval) return retval;
    }
    return 0;
}

DB_APP_VERSION_VAL *av_lookup(
    DB_ID_TYPE id, vector<DB_APP_VERSION_VAL>& app_versions
) {
    for (unsigned int i=0; i<app_versions.size(); i++) {
        if (app_versions[i].id == id) {
            return &app_versions[i];
        }
    }
    DB_APP_VERSION_VAL av;
    int retval = av.lookup_id(id);
    if (retval) {
        return NULL;
    }
    app_versions.push_back(av);
    return &(app_versions[app_versions.size()-1]);
}

// the estimated PFC for a given WU, in the absence of any other info
//
inline double wu_estimated_pfc(WORKUNIT &wu, DB_APP &app) {
    double x = wu.rsc_fpops_est;
    if (app.min_avg_pfc) {
        x *= app.min_avg_pfc;
    }
    return x;
}
inline double wu_estimated_credit(WORKUNIT &wu, DB_APP &app) {
    return wu_estimated_pfc(wu, app)*COBBLESTONE_SCALE;
}

inline bool is_pfc_sane(double x, WORKUNIT &wu, DB_APP &app) {
    if (x > 1e4 || x < 1e-4) {
        log_messages.printf(MSG_CRITICAL,
            "Bad FLOP ratio (%f): check workunit.rsc_fpops_est for %s (app %s)\n",
            x, wu.name, app.name
        );
        return false;
    }
    return true;
}

// Compute or estimate "claimed peak FLOP count" for a completed job.
// Possibly update host_app_version records and write to DB.
// Possibly update app_version records in memory and let caller write to DB,
// to merge DB writes
//
int get_pfc(
    RESULT &r, WORKUNIT &wu, DB_APP &app,       // in
    vector<DB_APP_VERSION_VAL>&app_versions,    // in/out
    DB_HOST_APP_VERSION &hav,                   // in/out
    double &pfc,                                // out
    int &mode                                   // out
){
    DB_APP_VERSION_VAL *avp=0;

    mode = PFC_MODE_APPROX;

    // the validator can set this flag if e.g. the job exited immediately
    // and shouldn't be counted in averages
    //
    if (r.runtime_outlier) {
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%lu] runtime outlier, not updating stats\n",
                r.id
            );
        }
    }

    // transition case: there's no host_app_version record
    //
    if (!hav.host_id) {
        mode = PFC_MODE_WU_EST;
        pfc = wu_estimated_pfc(wu, app);
        return 0;
    }

    // old clients report CPU time but not elapsed time.
    //
    if (r.elapsed_time < 1e-6) {
        r.elapsed_time = r.cpu_time;
    }

    // r.flops_estimate should be positive
    // but (because of scheduler bug) it may not be.
    //
    if (r.flops_estimate <= 0) {
        r.flops_estimate = AVG_CPU_FPOPS;
        r.runtime_outlier = true;       // skip stats update
        log_messages.printf(MSG_WARNING,
            "[RESULT#%lu] result has non-pos flops_estimate\n", r.id
        );
    }

    // if benchmarks failed, client reports p_fops as 1e9
    //
    if (r.flops_estimate == 1e9) {
        r.flops_estimate = AVG_CPU_FPOPS;
        r.runtime_outlier = true;       // skip stats update
        log_messages.printf(MSG_WARNING,
            "[RESULT#%lu] result has 1e9 flops_estimate\n", r.id
        );
    }

    double raw_pfc = (r.elapsed_time * r.flops_estimate);
    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL,
            "[credit] [RESULT#%lu] raw credit: %.2f (%.2f sec, %.2f est GFLOPS)\n",
            r.id, raw_pfc*COBBLESTONE_SCALE, r.elapsed_time,
            r.flops_estimate/1e9
        );
    }

    // get app version
    //
    avp = av_lookup(r.app_version_id, app_versions);

    // Sanity check
    // If an app version scale exists, use it.  Otherwise assume 1.
    //
    double tmp_scale = (avp && avp->pfc_scale) ? (avp->pfc_scale) : 1.0;

    if (raw_pfc*tmp_scale > wu.rsc_fpops_bound) {
        // This sanity check should be unnecessary because we have a maximum
        // credit grant limit.
        // With anonymous GPU apps the sanity check often fails
        // because anonymous GPU scales are often of order 0.01.
        // That prevents PFC averages from being updated.
        // So I've removed the return statement.
        //
        pfc = wu_estimated_pfc(wu, app);
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%lu] WARNING: sanity check failed: %.2f>%.2f, return %.2f\n",
                r.id, raw_pfc*tmp_scale*COBBLESTONE_SCALE,
                wu.rsc_fpops_bound*COBBLESTONE_SCALE, pfc*COBBLESTONE_SCALE
            );
        }
//        This was a bad idea because it prevents HAV.pfc from being updated.
//        sprintf(query, "consecutive_valid=0");
//        sprintf(clause, "host_id=%d and app_version_id=%d", r.hostid, gavid);
//        retval = hav.update_fields_noid(query, clause);
//        return retval;
    }

    if (r.app_version_id < 0) {
        // anon platform
        //
        bool do_scale = true;
        if (hav.pfc.n < MIN_HOST_SAMPLES || hav.pfc.get_avg()<=0) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] anon platform, not scaling, PFC avg zero or too few samples %.0f\n",
                    r.id, hav.pfc.n
                );
            }
        }
        if (do_scale
            && app.host_scale_check
            && hav.consecutive_valid < CONS_VALID_HOST_SCALE
        ) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] anon platform, not scaling, cons valid %d\n",
                    r.id, hav.consecutive_valid
                );
            }
        }
        if (do_scale) {
            double scale = app.min_avg_pfc / hav.pfc.get_avg();
            pfc = raw_pfc * scale;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] anon platform, scaling by %g (%.2f/%.2f)\n",
                    r.id, scale, app.min_avg_pfc, hav.pfc.get_avg()
                );
            }
        } else {
            pfc = wu_estimated_pfc(wu, app);
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] not scaling, using app avg %.2f\n",
                    r.id, pfc*COBBLESTONE_SCALE
                );
            }
        }
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%lu] anon platform, returning %.2f\n",
                r.id, pfc*COBBLESTONE_SCALE
            );
        }
    } else {
        avp = av_lookup(r.app_version_id, app_versions);
        if (!avp) {
            log_messages.printf(MSG_CRITICAL,
                "get_pfc() [RESULT#%lu]: No AVP %ld!!\n", r.id, r.app_version_id
            );
            return ERR_NOT_FOUND;
        }
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%lu] [AV#%lu] normal case. %.0f sec, %.1f GFLOPS.  raw credit: %.2f\n",
                r.id, avp->id, r.elapsed_time, r.flops_estimate/1e9,
                raw_pfc*COBBLESTONE_SCALE
            );
        }

        bool do_scale = true;
        double host_scale = 0;
        if (app.host_scale_check
            && hav.consecutive_valid < CONS_VALID_HOST_SCALE
        ) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] not host scaling - cons valid %d\n",
                    r.id, hav.consecutive_valid
                );
            }
        }
        if (do_scale && (hav.pfc.n < MIN_HOST_SAMPLES || hav.pfc.get_avg()==0)) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] not host scaling - HAV PFC zero or too few samples %.0f\n",
                    r.id, hav.pfc.n
                );
            }
        }
        if (do_scale && avp->pfc.n < MIN_VERSION_SAMPLES) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] not host scaling - app_version PFC too few samples%.0f\n",
                    r.id, avp->pfc.n
                );
            }
        }
        if (do_scale && hav.pfc.get_avg() <= 0) {
            do_scale = false;
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] not host scaling - HAV PFC is zero\n",
                    r.id
                );
            }
        }
        if (do_scale) {
            host_scale = avp->pfc.get_avg() / hav.pfc.get_avg();
            if (host_scale > 10) {
                host_scale = 10;
            }
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] host scale: %.2f (%f/%f)\n",
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
                    "[credit] [RESULT#%lu] applying app version scale %.3f\n",
                    r.id, avp->pfc_scale
                );
            }
        } else {
            if (host_scale) {
                pfc *= host_scale;
            }
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] no app version scale\n",
                    r.id
                );
            }
        }
        if (config.debug_credit) {
            log_messages.printf(MSG_NORMAL,
                "[credit] [RESULT#%lu] [AV#%lu] PFC avgs with %g (%g/%g)\n",
                r.id, avp->id,
                raw_pfc/wu.rsc_fpops_est,
                raw_pfc, wu.rsc_fpops_est
            );
        }
        double x = raw_pfc / wu.rsc_fpops_est;
        if (!r.runtime_outlier && is_pfc_sane(x, wu, app)) {
            avp->pfc_samples.push_back(x);
        }
    }

    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL,
            "[credit] [RESULT#%lu] updating HAV PFC %.2f et %g turnaround %d\n",
            r.id, raw_pfc / wu.rsc_fpops_est,
            r.elapsed_time / wu.rsc_fpops_est,
            (r.received_time - r.sent_time)
        );
    }

    if (!r.runtime_outlier) {
        double x = raw_pfc / wu.rsc_fpops_est;
        if (is_pfc_sane(x, wu, app)) {
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] [HOST#%lu] before updating HAV PFC pfc.n=%f pfc.avg=%f\n",
                    r.id, hav.host_id, hav.pfc.n, hav.pfc.avg
                );
            }
            hav.pfc.update(x, HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT);
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] [HOST#%lu] after updating HAV PFC pfc.n=%f pfc.avg=%f\n",
                    r.id, hav.host_id, hav.pfc.n, hav.pfc.avg
                );
            }
        }
        hav.et.update_var(
            r.elapsed_time / wu.rsc_fpops_est,
            HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
        );
        hav.turnaround.update_var(
            (r.received_time - r.sent_time),
            HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
        );
    }

    // keep track of credit per app version
    //
    if (avp) {
        avp->credit_samples.push_back(pfc*COBBLESTONE_SCALE);
        avp->credit_times.push_back(r.sent_time);
    }

    return 0;
}

// compute the average of some numbers,
// where each value is weighted by the sum of the other values.
// (reduces the weight of large outliers)
//
double low_average(vector<double>& v) {
    int i;
    int n = v.size();
    if (n == 1) {
        return v[0];
    }
    double sum=0;
    for (i=0; i<n; i++) {
        sum += v[i];
    }
    double total=0;
    for (i=0; i<n; i++) {
        total += v[i]*(sum-v[i]);
    }
    return total/((n-1)*sum);
}

// compute the average of number weighted by proximity
// to another number
double pegged_average(vector<double>& v, double anchor) {
    int n=v.size();
    double weights=0,sum=0,w;
    int i;
    if (n==1) {
        return v[0];
    }
    for (i=0; i<n; i++) {
        w=(1.0/(0.1*anchor+fabs(anchor-v[i])));
        weights+=w;
        sum+=w*v[i];
    }
    return sum/weights;
}

double vec_min(vector<double>& v) {
    double x = v[0];
    for (unsigned int i=1; i<v.size(); i++) {
        if (v[i] < x) {
            x = v[i];
        }
    }
    return x;
}

// Called by validator when canonical result has been selected.
// For each valid result in the list:
// - calculate a peak FLOP count (PFC) and a "mode" that indicates
//   our confidence in the PFC
// - update the statistics of PFC in host_app_version and app_version
// - Compute a credit value based on a weighted average of
//   the PFCs of valid results
//   (this value can be used or ignored by the caller)
//
// This must be called exactly once for each valid result.
//
int assign_credit_set(
    WORKUNIT &wu,
    vector<RESULT>& results,
    DB_APP &app,
    vector<DB_APP_VERSION_VAL>& app_versions,
    vector<DB_HOST_APP_VERSION>& host_app_versions,
    double max_granted_credit,
    double &credit      // out
) {
    unsigned int i;
    int mode, retval;
    double pfc;
    vector<double> normal;
    vector<double> approx;

    for (i=0; i<results.size(); i++) {
        RESULT &r = results[i];
        if (r.validate_state != VALIDATE_STATE_VALID) {
            continue;
        }
        DB_HOST_APP_VERSION &hav = host_app_versions[i];
        retval = get_pfc(r, wu, app, app_versions, hav, pfc, mode);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "get_pfc() error: %s\n", boincerror(retval)
            );
            continue;
        } else {
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] [RESULT#%lu] get_pfc() returns credit %g mode %s\n",
                    r.id, pfc *COBBLESTONE_SCALE, (mode==PFC_MODE_NORMAL)?"normal":"approx"
                );
            }
        }
        if (pfc > wu.rsc_fpops_bound) {
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] PFC too high: %f > %f\n",
                    pfc*COBBLESTONE_SCALE,
                    wu.rsc_fpops_bound*COBBLESTONE_SCALE
                );
            }
            pfc = wu.rsc_fpops_bound;
        }

        // max_granted_credit trumps rsc_fpops_bound;
        // the latter may be set absurdly high
        //
        if (max_granted_credit && pfc*COBBLESTONE_SCALE > max_granted_credit) {
            log_messages.printf(MSG_CRITICAL,
                "[credit] Credit too high: %f\n", pfc*COBBLESTONE_SCALE
            );
            pfc = max_granted_credit/COBBLESTONE_SCALE;
            mode = PFC_MODE_INVALID;
        }
        switch (mode) {
        case PFC_MODE_NORMAL:
            normal.push_back(pfc);
            break;
        case PFC_MODE_INVALID:
            break;
        default:
            approx.push_back(pfc);
            break;
        }
    }

    // averaging policy: if there is more than one normal result,
    // use the "pegged average" of normal results.
    // Otherwise use the pegged_average of all results
    //
    double x;
    switch (normal.size()) {
    case 1:
        // normal has double the weight of approx
        approx.push_back(normal[0]);
        approx.push_back(normal[0]);
        // fall through
    case 0:
        if (approx.size()) {
            x = pegged_average(approx, wu_estimated_pfc(wu, app));
        } else {
            // there were only PFC_MODE_INVALID results, so we guess
            x = wu_estimated_pfc(wu, app);
        }
        break;
    default:
        x = pegged_average(normal, wu_estimated_pfc(wu, app));
        break;
    }

    x *= COBBLESTONE_SCALE;
    if (config.debug_credit) {
        log_messages.printf(MSG_NORMAL,
            "[credit] [WU#%lu] assign_credit_set: credit %g\n",
            wu.id, x
        );
    }
    credit = x;
    return 0;
}

// carefully write any app_version records that have changed;
// done at the end of every validator scan.
//
int write_modified_app_versions(vector<DB_APP_VERSION_VAL>& app_versions) {
    unsigned int i, j;
    int retval = 0;
    double now = dtime();

    for (i=0; i<app_versions.size(); i++) {
        DB_APP_VERSION_VAL &av = app_versions[i];
        if (av.pfc_samples.empty() && av.credit_samples.empty()) {
            continue;
        }
        for (int k=0; k<10; k++) {
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
                    now,
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
                    "[credit] updating app version %lu:\n", av.id
                );
            }
            if (config.debug_credit) {
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
                "pfc_n=%.15e and abs(expavg_credit-%.15e)<1e-4 and abs(pfc_scale-%.15e)<1e-6",
                pfc_n_orig, expavg_credit_orig, av.pfc_scale
            );
            retval = av.update_field(query, clause);
            if (retval) {
                break;
            }
            if (boinc_db.affected_rows() == 1) {
                break;
            }
            retval = av.lookup_id(av.id);
            if (retval) {
                break;
            }
        }
        av.pfc_samples.clear();
        av.credit_samples.clear();
        av.credit_times.clear();
        if (retval) {
            return retval;
        }
    }
    return 0;
}
