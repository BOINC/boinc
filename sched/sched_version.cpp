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

#include "boinc_db.h"

#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_config.h"
#include "sched_customize.h"
#include "sched_types.h"
#include "sched_util.h"
#include "credit.h"

#include "sched_version.h"

inline void dont_need_message(
    const char* p, APP_VERSION* avp, CLIENT_APP_VERSION* cavp
) {
    if (!config.debug_version_select) return;
    if (avp) {
        log_messages.printf(MSG_NORMAL,
            "[version] [AV#%d] Don't need %s jobs, skipping\n",
            avp->id, p
        );
    } else if (cavp) {
        log_messages.printf(MSG_NORMAL,
            "[version] Don't need %s jobs, skipping anonymous version %d for %s (%s)\n",
            p, cavp->version_num, cavp->app_name, cavp->plan_class
        );
    }
}

// for new-style requests, check that the app version uses a
// resource for which we need work
//
bool need_this_resource(
    HOST_USAGE& host_usage, APP_VERSION* avp, CLIENT_APP_VERSION* cavp
) {
    if (!g_wreq->rsc_spec_request) {
        return true;
    }
    int pt = host_usage.proc_type;
    if (!g_wreq->need_proc_type(pt)) {
        dont_need_message(proc_type_name(pt), avp, cavp);
        return false;
    }
    return true;
}

static DB_HOST_APP_VERSION* lookup_host_app_version(int gavid) {
    for (unsigned int i=0; i<g_wreq->host_app_versions.size(); i++) {
        DB_HOST_APP_VERSION& hav = g_wreq->host_app_versions[i];
        if (hav.app_version_id == gavid) return &hav;
    }
    return NULL;
}

static inline bool app_version_is_trusted(int gavid) {
    DB_HOST_APP_VERSION* havp = lookup_host_app_version(gavid);
    if (!havp) return false;
    return havp->trusted;
}

static inline bool app_version_is_reliable(int gavid) {
    DB_HOST_APP_VERSION* havp = lookup_host_app_version(gavid);
    if (!havp) return false;
    return havp->reliable;
}

inline int host_usage_to_gavid(HOST_USAGE& hu, APP& app) {
    return app.id*1000000 - hu.resource_type();
}

// scale daily quota by # processors and/or by config.gpu_multiplier
//
inline int scaled_max_jobs_per_day(DB_HOST_APP_VERSION& hav, HOST_USAGE& hu) {
    int n = hav.max_jobs_per_day;
    if (hu.proc_type == PROC_TYPE_CPU) {
        if (g_reply->host.p_ncpus) {
            n *= g_reply->host.p_ncpus;
        }
    } else {
        COPROC* cp = g_request->coprocs.type_to_coproc(hu.proc_type);
        if (cp->count) {
            n *= cp->count;
        }
        if (config.gpu_multiplier) {
            n *= config.gpu_multiplier;
        }
    }
    if (config.debug_quota) {
        log_messages.printf(MSG_NORMAL,
            "[quota] [AV#%d] scaled max jobs per day: %d\n",
            hav.app_version_id,
            n
        );
    }
    return n;
}

inline bool daily_quota_exceeded(int gavid, HOST_USAGE& hu) {
    DB_HOST_APP_VERSION* havp = lookup_host_app_version(gavid);
    if (!havp) return false;
    int q = scaled_max_jobs_per_day(*havp, hu);
    if (havp->n_jobs_today >= q) {
        if (config.debug_quota) {
            log_messages.printf(MSG_NORMAL,
                "[quota] [AV#%d] daily quota exceeded: %d >= %d\n",
                gavid, havp->n_jobs_today, q
            );
        }
        havp->daily_quota_exceeded = true;
        return true;
    }
    return false;
}

// scan through client's anonymous apps and pick the best one
//
CLIENT_APP_VERSION* get_app_version_anonymous(
    APP& app, bool need_64b, bool reliable_only
) {
    unsigned int i;
    CLIENT_APP_VERSION* best = NULL;
    bool found = false;
    char message[256];

    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] get_app_version_anonymous: app %s%s\n",
            app.name, reliable_only?" (reliable only)":""
        );
    }
    for (i=0; i<g_request->client_app_versions.size(); i++) {
        CLIENT_APP_VERSION& cav = g_request->client_app_versions[i];
        if (!cav.app) continue;
        if (cav.app->id != app.id) {
            continue;
        }
        if (need_64b && !is_64b_platform(cav.platform)) {
            continue;
        }
        int gavid = host_usage_to_gavid(cav.host_usage, app);
        if (reliable_only && !app_version_is_reliable(gavid)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] %d %s not reliable\n",
                    cav.version_num, cav.plan_class
                );
            }
            continue;
        }
        if (daily_quota_exceeded(gavid, cav.host_usage)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] %d %s daily quota exceeded\n",
                    cav.version_num, cav.plan_class
                );
            }
            continue;
        }
        if (cav.version_num < app.min_version) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] %d %s version < min version\n",
                    cav.version_num, cav.plan_class
                );
            }
            continue;
        }
        found = true;
        if (!need_this_resource(cav.host_usage, NULL, &cav)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] %d %s don't need resource\n",
                    cav.version_num, cav.plan_class
                );
            }
            continue;
        }
        if (best) {
            if (cav.host_usage.projected_flops > best->host_usage.projected_flops) {
                best = &cav;
            }
        } else {
            best = &cav;
        }
    }
    if (!best) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Didn't find anonymous platform app for %s\n",
                app.name
            );
        }
    }
    if (!found) {
        sprintf(message,
            "%s %s.",
            _("Your app_info.xml file doesn't have a usable version of"),
            app.user_friendly_name
        );
        add_no_work_message(message);
    }
    return best;
}

#define ET_RATIO_LIMIT  250.
    // if the FLOPS estimate based on elapsed time
    // exceeds projected_flops by more than this factor, cap it.
    // The host may have received a bunch of short jobs recently

#define GPU_CPU_RATIO   10.
    // a conservative estimate of the ratio of a typical GPU to CPU

// input:
// cav.host_usage.projected_flops
//      This is the <flops> specified in app_info.xml
//      If not specified there, it's a conservative estimate
//      (CPU speed * (ncpus + 10*ngpus))
//      In either case, this value will be used by the client
//      to estimate job runtime and runtime limit
//          est runtime = wu.rsc_fpops_est/x
//          runtime limit = wu.rsc_fpops_bound/x
//      x may be way off from the actual speed.
//      So to get accurate runtime est, we need to adjust wu.rsc_fpops_est
//
// output:
// cav.host_usage.projected_flops
//      An estimate of the actual FLOPS the app will get,
//      based on elapsed time history (if possible).
//      This is used by the scheduler to estimate runtime.
// cav.rsc_fpops_scale
//      wu.rsc_fpops_est and wu.rsc_fpops_bound will be scaled by this
//
// called at start of send_work().
//
void estimate_flops_anon_platform() {
    unsigned int i;
    for (i=0; i<g_request->client_app_versions.size(); i++) {
        CLIENT_APP_VERSION& cav = g_request->client_app_versions[i];
        if (!cav.app) continue;

        cav.rsc_fpops_scale = 1;

        if (cav.host_usage.avg_ncpus == 0
            && cav.host_usage.proc_type == PROC_TYPE_CPU
        ) {
            cav.host_usage.avg_ncpus = 1;
        }

        // if projected_flops is missing, make a wild guess
        // Note: 6.12+ clients supply a project FLOPS,
        // even if the user didn't
        //
        if (cav.host_usage.projected_flops == 0) {
            cav.host_usage.projected_flops = g_reply->host.p_fpops;
        }

        // If data is available, estimate FLOPS based on average elapsed time
        //
        DB_HOST_APP_VERSION* havp = gavid_to_havp(
            generalized_app_version_id(
                cav.host_usage.resource_type(), cav.app->id
            )
        );
        if (havp
            && (havp->et.n > MIN_HOST_SAMPLES)
            && (havp->et.get_avg() > 0)
        ) {
            double new_flops = 1./havp->et.get_avg();

            // cap this at ET_RATIO_LIMIT*projected,
            // in case we've had a bunch of short jobs recently
            //
            if (new_flops > ET_RATIO_LIMIT*cav.host_usage.projected_flops) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] (%s) capping new_flops; %.1fG > %.0f*%.1fG\n",
                        cav.plan_class, new_flops/1e9,
                        ET_RATIO_LIMIT,
                        cav.host_usage.projected_flops/1e9
                    );
                }
                new_flops = ET_RATIO_LIMIT*cav.host_usage.projected_flops;
            }
            cav.rsc_fpops_scale = cav.host_usage.projected_flops/new_flops;
            cav.host_usage.projected_flops = new_flops;
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] (%s) setting projected flops to %fG based on ET\n",
                    cav.plan_class, new_flops/1e9
                );
                log_messages.printf(MSG_NORMAL,
                    "[version] setting rsc_fpops_scale to %g\n",
                    cav.rsc_fpops_scale
                );
            }
        } else {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] (%s) using client-supplied flops %fG\n",
                    cav.plan_class, cav.host_usage.projected_flops/1e9
                );
            }
        }
    }
}

// compute HOST_USAGE::projected_flops as best we can:
// 1) if we have statistics for (host, app version) elapsed time,
//    use those.
// 2) if we have statistics for app version elapsed time, use those.
// 3) else use a conservative estimate (p_fpops*(cpus+gpus))
//    This prevents jobs from aborting with "time limit exceeded"
//    even if the estimate supplied by the plan class function is way off
//
void estimate_flops(HOST_USAGE& hu, APP_VERSION& av) {
    DB_HOST_APP_VERSION* havp = gavid_to_havp(av.id);
    if (havp && havp->et.n > MIN_HOST_SAMPLES) {
        double new_flops = 1./havp->et.get_avg();
        // cap this at ET_RATIO_LIMIT*projected,
        // in case we've had a bunch of short jobs recently
        //
        if (new_flops > ET_RATIO_LIMIT*hu.projected_flops) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] (%s) capping new_flops; %.1fG > %.0f*%.1fG\n",
                    av.plan_class, new_flops/1e9,
                    ET_RATIO_LIMIT,
                    hu.projected_flops/1e9
                );
            }
            new_flops = ET_RATIO_LIMIT*hu.projected_flops;
        }
        hu.projected_flops = new_flops;

        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] [AV#%d] (%s) setting projected flops based on host elapsed time avg: %.2fG\n",
                av.id, av.plan_class, hu.projected_flops/1e9
            );
        }
    } else {
        if (av.pfc.n > MIN_VERSION_SAMPLES) {
            hu.projected_flops = hu.peak_flops/av.pfc.get_avg();
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [AV#%d] (%s) adjusting projected flops based on PFC avg: %.2fG\n",
                    av.id, av.plan_class, hu.projected_flops/1e9
                );
            }
        } else {
            hu.projected_flops = g_reply->host.p_fpops * (hu.avg_ncpus + GPU_CPU_RATIO*hu.gpu_usage);
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [AV#%d] (%s) using conservative projected flops: %.2fG\n",
                    av.id, av.plan_class, hu.projected_flops/1e9
                );
            }
        }
    }
}

// return a string describing an app version
//
static void app_version_desc(BEST_APP_VERSION& bav, char* buf) {
    if (!bav.present) {
        strcpy(buf, "none");
        return;
    }
    if (bav.cavp) {
        sprintf(buf, "anonymous platform (%s)", proc_type_name(bav.host_usage.proc_type));
    } else {
        sprintf(buf, "[AV#%d]", bav.avp->id);
    }
}

// different OSs have different max user address space for 32 bit apps
//
static double max_32b_address_space() {
    if (strstr(g_request->platform.name, "windows")) {
        return 2*GIGA;
    } else if (strstr(g_request->platform.name, "linux")) {
        return 3*GIGA;
    } else if (strstr(g_request->platform.name, "darwin")) {
        return 4*GIGA;
    } else if (strstr(g_request->platform.name, "solaris")) {
        return 4*GIGA;
    } else if (strstr(g_request->platform.name, "anonymous")) {
        // problem case.  assume windows
        return 2*GIGA;
    }
    return 2*GIGA;
}

// The WU is already committed to an app version.
// - check if this host supports that platform
// - if plan class, check if this host can handle it
// - check if we need work for the resource
//
static BEST_APP_VERSION* check_homogeneous_app_version(
    WORKUNIT& wu, bool /* reliable_only */
    // TODO: enforce reliable_only
) {
    static BEST_APP_VERSION bav;

    bool found;
    APP_VERSION *avp = ssp->lookup_app_version(wu.app_version_id);
    if (!avp) {
        // If the app version is not in shmem,
        // it's been superceded or deprecated.
        // Use it anyway.
        // Keep an array of such app versions in
        // SCHEDULER_REPLY::old_app_versions
        //
        found = false;
        for (unsigned int i=0; i<g_reply->old_app_versions.size(); i++) {
            APP_VERSION& av = g_reply->old_app_versions[i];
            if (av.id == wu.app_version_id) {
                avp = &av;
                found = true;
                break;
            }
        }
        if (!found) {
            DB_APP_VERSION av;
            int retval = av.lookup_id(wu.app_version_id);
            if (retval) return NULL;
            g_reply->old_app_versions.push_back(av);
            avp = &(g_reply->old_app_versions.back());
        }
    }

    // see if this host supports the version's platform
    //
    found = false;
    for (unsigned int i=0; i<g_request->platforms.list.size(); i++) {
        PLATFORM* p = g_request->platforms.list[i];
        if (p->id == avp->platformid) {
            found = true;
            bav.avp = avp;
            break;
        }
    }
    if (!found) return NULL;

    // and see if it supports the plan class
    //
    if (strlen(avp->plan_class)) {
        if (!app_plan(*g_request, avp->plan_class, bav.host_usage)) {
            return NULL;
        }
    } else {
        bav.host_usage.sequential_app(capped_host_fpops());
    }

    // and see if the client is asking for this resource
    //
    if (!need_this_resource(bav.host_usage, avp, NULL)) {
        return NULL;
    }
    return &bav;
}

// return the app version with greatest projected FLOPS
// for the given job and host, or NULL if none is available
//
// check_req: if set, return only app versions that use resources
//  for which the work request is nonzero.
//  This check is not done for:
//    - assigned jobs
//    - resent jobs
// reliable_only: use only versions for which this host is "reliable"
//
// We "memoize" the results, maintaining an array g_wreq->best_app_versions
// that maps app ID to the best app version (or NULL).
//
BEST_APP_VERSION* get_app_version(
    WORKUNIT& wu, bool check_req, bool reliable_only
) {
    unsigned int i;
    int j;
    BEST_APP_VERSION* bavp;
    char buf[256];
    bool job_needs_64b = (wu.rsc_memory_bound > max_32b_address_space());

    if (config.debug_version_select) {
        if (job_needs_64b) {
            log_messages.printf(MSG_NORMAL,
                "[version] job needs 64-bit app version: mem bnd %f\n",
                wu.rsc_memory_bound
            );
        }
    }

    APP* app = ssp->lookup_app(wu.appid);
    if (!app) {
        log_messages.printf(MSG_CRITICAL,
            "WU refers to nonexistent app: %d\n", wu.appid
        );
        return NULL;
    }

    // handle the case where we're using homogeneous app version
    // and the WU is already committed to an app version
    //
    if (app->homogeneous_app_version && wu.app_version_id) {
        return check_homogeneous_app_version(wu, reliable_only);
    }

    // see if app is already in memoized array
    //
    std::vector<BEST_APP_VERSION*>::iterator bavi;
    bavi = g_wreq->best_app_versions.begin();
    while (bavi != g_wreq->best_app_versions.end()) {
        bavp = *bavi;
        if (bavp->appid == wu.appid && (job_needs_64b == bavp->for_64b_jobs)) {
            if (!bavp->present) {
#if 0
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] returning cached NULL\n"
                    );
                }
#endif
                return NULL;
            }

            // if we're at the jobs-in-progress limit for this
            // app and resource type, fall through and find another version
            //
            if (config.max_jobs_in_progress.exceeded(
                app, bavp->host_usage.uses_gpu())
            ) {
                if (config.debug_version_select) {
                    app_version_desc(*bavp, buf);
                    log_messages.printf(MSG_NORMAL,
                        "[version] %s: max jobs in progress exceeded\n", buf
                    );
                }
                g_wreq->best_app_versions.erase(bavi);
                break;
            }

            // if we previously chose an app version but don't need more work
            // for that processor type, fall through and find another version
            //
            if (check_req && g_wreq->rsc_spec_request) {
                int pt = bavp->host_usage.proc_type;
                if (!g_wreq->need_proc_type(pt)) {
                    if (config.debug_version_select) {
                        log_messages.printf(MSG_NORMAL,
                            "[version] have %s version but no more %s work needed\n",
                            proc_type_name(pt),
                            proc_type_name(pt)
                        );
                    }
                    g_wreq->best_app_versions.erase(bavi);
                    break;
                }
            }

            if (config.debug_version_select) {
                app_version_desc(*bavp, buf);
                log_messages.printf(MSG_NORMAL,
                    "[version] returning cached version: %s\n", buf
                );
            }
            return bavp;
        }
        bavi++;
    }

    // here if app was not in memoized array,
    // or we couldn't use the app version there.

    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] looking for version of %s\n",
            app->name
        );
    }

    bavp = new BEST_APP_VERSION;
    bavp->appid = wu.appid;
    bavp->for_64b_jobs = job_needs_64b;
    if (g_wreq->anonymous_platform) {
        CLIENT_APP_VERSION* cavp = get_app_version_anonymous(
            *app, job_needs_64b, reliable_only
        );
        if (!cavp) {
            bavp->present = false;
        } else {
            bavp->present = true;
            bavp->host_usage = cavp->host_usage;
            bavp->cavp = cavp;
            int gavid = host_usage_to_gavid(cavp->host_usage, *app);
            bavp->reliable = app_version_is_reliable(gavid);
            bavp->trusted = app_version_is_trusted(gavid);
            if (config.debug_version_select) {
                app_version_desc(*bavp, buf);
                log_messages.printf(MSG_NORMAL, "[version] using %s\n", buf);
            }
        }
        g_wreq->best_app_versions.push_back(bavp);
        if (!bavp->present) return NULL;
        return bavp;
    }

    // Go through the client's platforms,
    // and scan the app versions for each platform.
    // Pick the one with highest expected FLOPS
    //
    // if config.prefer_primary_platform is set:
    // stop scanning platforms once we find a feasible version 

    bavp->host_usage.projected_flops = 0;
    bavp->avp = NULL;
    for (i=0; i<g_request->platforms.list.size(); i++) {
        bool found_feasible_version = false;
        PLATFORM* p = g_request->platforms.list[i];
        if (job_needs_64b && !is_64b_platform(p->name)) {
            continue;
        }
        for (j=0; j<ssp->napp_versions; j++) {
            HOST_USAGE host_usage;
            APP_VERSION& av = ssp->app_versions[j];
            if (av.appid != wu.appid) continue;
            if (av.platformid != p->id) continue;

            if (strlen(av.plan_class)) {
                if (!app_plan(*g_request, av.plan_class, host_usage)) {
                    if (config.debug_version_select) {
                        log_messages.printf(MSG_NORMAL,
                            "[version] [AV#%d] app_plan() returned false\n",
                            av.id
                        );
                    }
                    continue;
                }
                if (!g_request->client_cap_plan_class) {
                    if (!host_usage.is_sequential_app()) {
                        if (config.debug_version_select) {
                            log_messages.printf(MSG_NORMAL,
                                "[version] [AV#%d] client %d lacks plan class capability\n",
                                av.id, g_request->core_client_version
                            );
                        }
                        continue;
                    }
                }
            } else {
                host_usage.sequential_app(g_reply->host.p_fpops);
            }

            // skip versions that go against resource prefs
            //
            int pt = host_usage.proc_type;
            if (g_wreq->dont_use_proc_type[pt]) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] [AV#%d] Skipping %s version - user prefs say no %s\n",
                        av.id,
                        proc_type_name(pt),
                        proc_type_name(pt)
                    );
                }
                continue;
            }

            if (reliable_only && !app_version_is_reliable(av.id)) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] [AV#%d] not reliable\n", av.id
                    );
                }
                continue;
            }

            if (daily_quota_exceeded(av.id, host_usage)) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] [AV#%d] daily quota exceeded\n", av.id
                    );
                }
                continue;
            }

            // skip versions for which we're at the jobs-in-progress limit
            //
            if (config.max_jobs_in_progress.exceeded(app, host_usage.uses_gpu())) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] [AV#%d] jobs in progress limit exceeded\n",
                        av.id
                    );
                    config.max_jobs_in_progress.print_log();
                }
                continue;
            }

            // skip versions for resources we don't need
            //
            if (check_req && !need_this_resource(host_usage, &av, NULL)) {
                continue;
            }

            // skip versions which require a newer core client
            //
            if (g_request->core_client_version < av.min_core_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] [AV#%d] client version %d < min core version %d\n",
                        av.id, g_request->core_client_version, av.min_core_version
                    );
                }
                // Do not tell the user he needs to update the client
                // just because the client is too old for a particular app version
                // g_wreq->outdated_client = true;
                continue;
            }
            if (av.max_core_version && g_request->core_client_version > av.max_core_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] [AV#%d] client version %d > max core version %d\n",
                        av.id, g_request->core_client_version, av.max_core_version
                    );
                }
                continue;
            }

            // at this point we know the version is feasible,
            // so if config.prefer_primary_platform is set
            // we won't look any further.
            //
            found_feasible_version = true;

            // pick the fastest version.
            // Throw in a random factor in case the estimates are off.
            //
	    DB_HOST_APP_VERSION* havp = gavid_to_havp(av.id);
            double r = 1;
	    long n=1;
            if (havp) {
                n=std::max((long)havp->pfc.n,(long)n);
            } 
            if (config.version_select_random_factor) {
                r += config.version_select_random_factor*rand_normal()/n;
            }
            if (r*host_usage.projected_flops > bavp->host_usage.projected_flops) {
                if (config.debug_version_select && (host_usage.projected_flops <= bavp->host_usage.projected_flops)) {
                      log_messages.printf(MSG_NORMAL,
                          "[version] [AV#%d] Random factor wins.  r=%f n=%ld\n",
                          av.id, r, n
                    );
                }
                host_usage.projected_flops*=r;
                bavp->host_usage = host_usage;
                bavp->avp = &av;
                bavp->reliable = app_version_is_reliable(av.id);
                bavp->trusted = app_version_is_trusted(av.id);
            }
        }   // loop over app versions

        if (config.prefer_primary_platform && found_feasible_version) {
            break;
        }
    }   // loop over client platforms

    if (bavp->avp) {
        estimate_flops(bavp->host_usage, *bavp->avp);
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Best version of app %s is [AV#%d] (%.2f GFLOPS)\n",
                app->name, bavp->avp->id, bavp->host_usage.projected_flops/1e9
            );
        }
        bavp->present = true;
        g_wreq->best_app_versions.push_back(bavp);
    } else {
        // Here if there's no app version we can use.
        //
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] returning NULL; platforms:\n"
            );
            for (i=0; i<g_request->platforms.list.size(); i++) {
                PLATFORM* p = g_request->platforms.list[i];
                log_messages.printf(MSG_NORMAL,
                    "[version] %s\n",
                    p->name
                );
            }
        }
        g_wreq->best_app_versions.push_back(bavp);
        return NULL;
    }
    return bavp;
}

