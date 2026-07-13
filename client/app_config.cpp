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

#include "filesys.h"

#include "client_msgs.h"
#include "client_state.h"
#include "client_types.h"
#include "project.h"
#include "result.h"

#include "cc_config.h"
#include "app_config.h"

static void show_warning(PROJECT* p, const char* name) {
    msg_printf(p, MSG_USER_ALERT,
        "Your app_config.xml file refers to an unknown application '%s'.  Known applications: %s",
        name, app_list_string(p).c_str()
    );
}

static void modify_usage_avc(
    const APP_VERSION_CONFIG &avc, RESOURCE_USAGE &ru
) {
    if (strlen(avc.cmdline)) {
        safe_strcpy(ru.cmdline, avc.cmdline);
    }
    if (avc.avg_ncpus) {
        ru.avg_ncpus = avc.avg_ncpus;
    }
    if (avc.ngpus) {
        ru.coproc_usage = avc.ngpus;
    }
}

static void modify_usage_ac(const APP_CONFIG &ac, RESOURCE_USAGE &ru) {
    if (!ru.rsc_type) return;
    ru.coproc_usage = ac.gpu_gpu_usage;
    ru.avg_ncpus = ac.gpu_cpu_usage;
}

// having parsed a project's app_config.xml, put the config into effect
// called:
// on startup and reread config (from check_app_config() below)
// after scheduler RPC to that project (in case got new app versions)
//
int APP_CONFIGS::config_app_versions(PROJECT* p, bool show_warnings) {
    bool showed_notice = false;
    for (const APP_CONFIG& ac: app_configs) {
        APP* app = gstate.lookup_app(p, ac.name);
        if (!app) {
            if (show_warnings) {
                show_warning(p, ac.name);
                showed_notice = true;
            }
            continue;
        }
        app->max_concurrent = ac.max_concurrent;
        app->fraction_done_exact = ac.fraction_done_exact;
        app->report_results_immediately = ac.report_results_immediately;

        if (!ac.gpu_gpu_usage || !ac.gpu_cpu_usage) continue;
        for (APP_VERSION* avp: gstate.app_versions) {
            if (avp->app != app) continue;
            modify_usage_ac(ac, avp->resource_usage);
        }

        // BUDA
        //
        for (WORKUNIT *wup: gstate.workunits) {
            if (!wup->has_resource_usage) continue;
            if (wup->project != p) continue;
            if (wup->app != app) continue;
            modify_usage_ac(ac, wup->resource_usage);
        }
    }
    for (const APP_VERSION_CONFIG& avc: app_version_configs) {
        APP* app = gstate.lookup_app(p, avc.app_name);
        if (!app) {
            if (show_warnings) {
                show_warning(p, avc.app_name);
                showed_notice = true;
            }
            continue;
        }
        bool found = false;
        for (APP_VERSION* avp: gstate.app_versions) {
            if (avp->app != app) continue;
            if (strcmp(avp->plan_class, avc.plan_class)) continue;
            found = true;

            // modify the app version's resource usage
            //
            modify_usage_avc(avc, avp->resource_usage);

            // for BUDA, modify the resource usage
            // of WUs that use this app version
            //
            // WU doesn't directly link to app version;
            // see if there's a result that links to both
            //
            for (WORKUNIT *wup: gstate.workunits) {
                wup->ref_cnt = 0;
            }
            for (RESULT *rp: gstate.results) {
                if (rp->avp == avp) {
                    rp->wup->ref_cnt = 1;
                }
            }
            for (WORKUNIT *wup: gstate.workunits) {
                if (!wup->has_resource_usage) continue;
                if (wup->ref_cnt == 0) continue;
                modify_usage_avc(avc, wup->resource_usage);
            }
            // don't break here; it's possible that multiple app versions
            // have the same app and plan class
        }
        if (!found) {
            msg_printf(p, MSG_USER_ALERT,
                "Entry in app_config.xml for app '%s', plan class '%s' doesn't match any app versions",
                avc.app_name, avc.plan_class
            );
        }
    }

    // update resource usage of this project's non-running jobs
    //
    gstate.init_result_resource_usage(p);

    if (showed_notice) return ERR_XML_PARSE;
    return 0;
}

// clear app- and project-level counters to enforce max concurrent limits
//
void max_concurrent_init() {
    for (APP *app: gstate.apps) {
        app->app_n_concurrent = 0;
    }
    for (PROJECT *p: gstate.projects) {
        p->proj_n_concurrent = 0;
    }
}

// undo the effects of an app_config.xml that no longer exists
// NOTE: all we can do here is to clear APP::max_concurrent;
// we can't restore device usage info because we don't have it.
// It will be restored on next scheduler RPC.
//
static void clear_app_config(PROJECT* p) {
    p->app_configs.clear();
    for (APP *app: gstate.apps) {
        if (app->project != p) continue;
        app->max_concurrent = 0;
        app->report_results_immediately = false;
    }
}

static void print_msgs(vector<string> msgs, PROJECT* p) {
    for (const string &msg: msgs) {
        msg_printf_notice(p, false, NULL, "%s", msg.c_str());
    }
}

// check for app_config.xml files, and parse them.
// Called at startup and on read_cc_config() RPC
//
void check_app_config(const char* prefix) {
    char path[MAXPATHLEN];
    FILE* f;

    for (PROJECT *p: gstate.projects) {
        snprintf(path, sizeof(path), "%s%s/%s",
            prefix, p->project_dir(), APP_CONFIG_FILE_NAME
        );
        f = boinc_fopen(path, "r");
        if (!f) {
            clear_app_config(p);
            continue;
        }
        msg_printf(p, MSG_INFO, "Found %s", APP_CONFIG_FILE_NAME);
        vector<string> msgs;
        int retval = p->app_configs.parse_file(f, msgs, log_flags);
        print_msgs(msgs, p);
        if (!retval) {
            p->report_results_immediately = p->app_configs.report_results_immediately;
            retval = p->app_configs.config_app_versions(p, true);
            if (!retval) {
                notices.remove_notices(p, REMOVE_APP_CONFIG_MSG);
            }
        }
        fclose(f);
    }
}

void show_app_config() {
    if (!have_max_concurrent) return;
    for (PROJECT *p: gstate.projects) {
        if (p->app_configs.project_max_concurrent) {
            msg_printf(p, MSG_INFO,
                "Max %d concurrent jobs", p->app_configs.project_max_concurrent
            );
        }
    }
    for (APP* app: gstate.apps) {
        if (app->max_concurrent) {
            msg_printf(app->project, MSG_INFO,
                "%s: Max %d concurrent jobs", app->name, app->max_concurrent
            );
        }
    }
}
