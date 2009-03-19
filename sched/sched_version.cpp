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

#include "main.h"
#include "sched_msgs.h"
#include "sched_config.h"
#include "sched_plan.h"
#include "server_types.h"

#include "sched_version.h"

inline void dont_need_message(
    const char* p, APP_VERSION* avp, CLIENT_APP_VERSION* cavp
) {
    if (!config.debug_version_select) return;
    if (avp) {
        APP* app = ssp->lookup_app(avp->appid);
        log_messages.printf(MSG_NORMAL,
            "[version] Don't need %s jobs, skipping version %d for %s (%s)\n",
            p, avp->version_num, app->name, avp->plan_class
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
    if (g_wreq->rsc_spec_request) {
        if (host_usage.ncudas) {
            if (!g_wreq->need_cuda()) {
                dont_need_message("CUDA", avp, cavp);
                return false;
            }
        } else {
            if (!g_wreq->need_cpu()) {
                dont_need_message("CPU", avp, cavp);
                return false;;
            }
        }
    }
    return true;
}

// scan through client's anonymous apps and pick the best one
//
CLIENT_APP_VERSION* get_app_version_anonymous(APP& app) {
    unsigned int i;
    CLIENT_APP_VERSION* best = NULL;
    bool found = false;
    char message[256];

    for (i=0; i<g_request->client_app_versions.size(); i++) {
        CLIENT_APP_VERSION& cav = g_request->client_app_versions[i];
        if (strcmp(cav.app_name, app.name)) {
            continue;
        }
        if (cav.version_num < app.min_version) {
            continue;
        }
        found = true;
        if (!need_this_resource(cav.host_usage, NULL, &cav)) {
            continue;
        }
        if (best) {
            if (cav.host_usage.flops > best->host_usage.flops) {
                best = &cav;
            }
        } else {
            best = &cav;
        }
    }
    if (!best) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] Didn't find anonymous platform app for %s\n",
                app.name
            );
        }
    }
    if (!found) {
        sprintf(message,
            "Your app_info.xml file doesn't have a version of %s.",
            app.user_friendly_name
        );
        g_wreq->insert_no_work_message(USER_MESSAGE(message, "high"));
    }
    return best;
}

// return BEST_APP_VERSION for the given host, or NULL if none
//
//
BEST_APP_VERSION* get_app_version(WORKUNIT& wu, bool check_req) {
    bool found;
    unsigned int i;
    int retval, j;
    BEST_APP_VERSION* bavp;
    char message[256];

    // see if app is already in memoized array
    //
    std::vector<BEST_APP_VERSION*>::iterator bavi;
    bavi = g_wreq->best_app_versions.begin();
    while (bavi != g_wreq->best_app_versions.end()) {
        bavp = *bavi;
        if (bavp->appid == wu.appid) {
            if (!bavp->present) return NULL;

            // if we previously chose a CUDA app but don't need more CUDA work,
            // delete record, fall through, and find another version
            //
            if (check_req
                && g_wreq->rsc_spec_request
                && bavp->host_usage.ncudas > 0
                && !g_wreq->need_cuda()
            ) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] have CUDA version but no more CUDA work needed\n"
                    );
                }
                g_wreq->best_app_versions.erase(bavi);
                break;
            }

            // same, CPU
            //
            if (check_req
                && g_wreq->rsc_spec_request
                && !bavp->host_usage.ncudas
                && !g_wreq->need_cpu()
            ) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] have CPU version but no more CPU work needed\n"
                    );
                }
                g_wreq->best_app_versions.erase(bavi);
                break;
            }

            return bavp;
        }
        bavi++;
    }

    APP* app = ssp->lookup_app(wu.appid);
    if (!app) {
        log_messages.printf(MSG_CRITICAL,
            "WU refers to nonexistent app: %d\n", wu.appid
        );
        return NULL;
    }

    bavp = new BEST_APP_VERSION;
    bavp->appid = wu.appid;
    if (g_wreq->anonymous_platform) {
        CLIENT_APP_VERSION* cavp = get_app_version_anonymous(*app);
        if (!cavp) {
            bavp->present = false;
        } else {
            bavp->present = true;
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] Found anonymous platform app for %s: plan class %s\n",
                    app->name, cavp->plan_class
                );
            }
            bavp->host_usage = cavp->host_usage;

            // if client didn't tell us about the app version,
            // assume it uses 1 CPU
            //
            if (bavp->host_usage.flops == 0) {
                bavp->host_usage.flops = g_reply->host.p_fpops;
            }
            if (bavp->host_usage.avg_ncpus == 0 && bavp->host_usage.ncudas == 0) {
                bavp->host_usage.avg_ncpus = 1;
            }
            bavp->cavp = cavp;
        }
        g_wreq->best_app_versions.push_back(bavp);
        if (!bavp->present) return NULL;
        return bavp;
    }

    // Go through the client's platforms.
    // Scan the app versions for each platform.
    // Find the one with highest expected FLOPS
    //
    bavp->host_usage.flops = 0;
    bavp->avp = NULL;
    bool no_version_for_platform = true;
    int app_plan_reject = 0;
    for (i=0; i<g_request->platforms.list.size(); i++) {
        PLATFORM* p = g_request->platforms.list[i];
        for (j=0; j<ssp->napp_versions; j++) {
            HOST_USAGE host_usage;
            APP_VERSION& av = ssp->app_versions[j];
            if (av.appid != wu.appid) continue;
            if (av.platformid != p->id) continue;
            no_version_for_platform = false;
            if (g_request->core_client_version < av.min_core_version) {
                log_messages.printf(MSG_NORMAL,
                    "outdated client version %d < min core version %d\n",
                    g_request->core_client_version, av.min_core_version
                );
                g_wreq->outdated_client = true;
                continue;
            }
            if (strlen(av.plan_class)) {
                if (!g_request->client_cap_plan_class) {
                    log_messages.printf(MSG_NORMAL,
                        "client version %d lacks plan class capability\n",
                        g_request->core_client_version
                    );
                    continue;
                }
                retval = app_plan(*g_request, av.plan_class, host_usage);
                if (retval) {
                    app_plan_reject = retval;
                    continue;
                }
            } else {
                host_usage.sequential_app(g_reply->host.p_fpops);
            }

            // skip versions for resources we don't need
            //
            if (!need_this_resource(host_usage, &av, NULL)) {
                continue;
            }

            // skip versions that go against resource prefs
            //
            if (host_usage.ncudas && g_wreq->no_gpus) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] Skipping CUDA version - user prefs say no GPUS\n"
                    );
                    g_wreq->no_gpus_prefs = true;
                }
                continue;
            }
            if (!host_usage.ncudas && g_wreq->no_cpu) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] Skipping CPU version - user prefs say no CPUs\n"
                    );
                    g_wreq->no_cpu_prefs = true;
                }
                continue;
            }

            // pick the fastest version
            //
            if (host_usage.flops > bavp->host_usage.flops) {
                bavp->host_usage = host_usage;
                bavp->avp = &av;
            }
        }
    }
    g_wreq->best_app_versions.push_back(bavp);
    if (bavp->avp) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Best version of app %s is ID %d (%.2f GFLOPS)\n",
                app->name, bavp->avp->id, bavp->host_usage.flops/1e9
            );
        }
        bavp->present = true;
    } else {
        // Here if there's no app version we can use.
        // Could be because:
        // - none exists for platform
        // - one exists for platform, but host lacks processor type
        // - one exists for platform, but no work requested for processor type
        // - one exists but requires newer client
        // - one exists but plan function rejects this host
        //
        if (config.debug_version_select) {
            for (i=0; i<g_request->platforms.list.size(); i++) {
                PLATFORM* p = g_request->platforms.list[i];
                log_messages.printf(MSG_NORMAL,
                    "[version] no app version available: APP#%d (%s) PLATFORM#%d (%s) min_version %d\n",
                    app->id, app->name, p->id, p->name, app->min_version
                );
            }
        }
        if (no_version_for_platform) {
            sprintf(message,
                "%s is not available for your type of computer.",
                app->user_friendly_name
            );
            g_wreq->insert_no_work_message(USER_MESSAGE(message, "high"));
        }
        const char* p = NULL;
        switch (app_plan_reject) {
        case PLAN_REJECT_CUDA_NO_DEVICE:
            p = "Your computer has no CUDA device"; break;
        case PLAN_REJECT_CUDA_VERSION:
            p = "Your CUDA device has the wrong software version"; break;
        case PLAN_REJECT_NVIDIA_DRIVER_VERSION:
            p = "Your CUDA device has the wrong driver version"; break;
        case PLAN_REJECT_CUDA_MEM:
            p = "Your CUDA device has insufficient memory"; break;
        case PLAN_REJECT_CUDA_SPEED:
            p = "Your CUDA device is too slow"; break;
        }
        if (p) {
            sprintf(message,
                "Can't use CUDA app for %s: %s",
                app->user_friendly_name, p
            );
            g_wreq->insert_no_work_message(USER_MESSAGE(message, "high"));
        }
        return NULL;
    }
    return bavp;
}

