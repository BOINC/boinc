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

#include "app_config.h"

bool have_max_concurrent = false;

int APP_CONFIG::parse_gpu_versions(XML_PARSER& xp, PROJECT* p) {
    double x;
    while (!xp.get_tag()) {
        if (xp.match_tag("/gpu_versions")) return 0;
        else if (xp.parse_double("gpu_usage", x)) {
            if (x <= 0) {
                msg_printf(p, MSG_USER_ALERT,
                    "gpu_usage must be positive in app_config.xml"
                );
            } else {
                gpu_gpu_usage = x;
            }
            continue;
        }
        else if (xp.parse_double("cpu_usage", x)) {
            if (x < 0) {
                msg_printf(p, MSG_USER_ALERT,
                    "cpu_usage must be non-negative in app_config.xml"
                );
            } else {
                gpu_cpu_usage = x;
            }
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(p, MSG_INFO,
                "Unparsed line in app_config.xml: %s",
                xp.parsed_tag
            );
        }
    }
    msg_printf_notice(p, false, NULL,
        "Missing </gpu_versions> in app_config.xml"
    );
    return ERR_XML_PARSE;
}

// In these parsing functions, if there's an error you must
// - generate a notice containing the string "app_config.xml"
// - return an error code
//
int APP_CONFIG::parse(XML_PARSER& xp, PROJECT* p) {
    memset(this, 0, sizeof(APP_CONFIG));

    while (!xp.get_tag()) {
        if (xp.match_tag("/app")) return 0;
        if (xp.parse_str("name", name, 256)) continue;
        if (xp.parse_int("max_concurrent", max_concurrent)) {
            if (max_concurrent) have_max_concurrent = true;
            continue;
        }
        if (xp.match_tag("gpu_versions")) {
            int retval = parse_gpu_versions(xp, p);
            if (retval) return retval;
            continue;
        }
        if (xp.parse_bool("fraction_done_exact", fraction_done_exact)) {
            continue;
        }

        // unparsed XML not considered an error; maybe it should be?
        //
        if (log_flags.unparsed_xml) {
            msg_printf(p, MSG_INFO,
                "Unparsed line in app_config.xml: %s",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "APP_CONFIG::parse");
    }
    msg_printf_notice(p, false, NULL,
        "Missing </app> in app_config.xml"
    );
    return ERR_XML_PARSE;
}

int APP_VERSION_CONFIG::parse(XML_PARSER& xp, PROJECT* p) {
    memset(this, 0, sizeof(APP_VERSION_CONFIG));

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            msg_printf_notice(p, false, NULL,
                "unexpected text '%s' in app_config.xml", xp.parsed_tag
            );
            return ERR_XML_PARSE;
        }
        if (xp.match_tag("/app_version")) return 0;
        if (xp.parse_str("app_name", app_name, 256)) continue;
        if (xp.parse_str("plan_class", plan_class, 256)) continue;
        if (xp.parse_str("cmdline", cmdline, 256)) continue;
        if (xp.parse_double("avg_ncpus", avg_ncpus)) continue;
        if (xp.parse_double("ngpus", ngpus)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(p, MSG_INFO,
                "Unparsed line in app_config.xml: %s",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "APP_VERSION_CONFIG::parse");
    }
    msg_printf_notice(p, false, NULL,
        "Missing </app_version> in app_config.xml"
    );
    return ERR_XML_PARSE;
}

int APP_CONFIGS::parse(XML_PARSER& xp, PROJECT* p) {
    int n;
    clear();
    if (!xp.parse_start("app_config")) {
        msg_printf_notice(p, false, NULL,
            "Missing <app_config> in app_config.xml"
        );
        return ERR_XML_PARSE;
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            msg_printf_notice(p, false, NULL,
                "unexpected text '%s' in app_config.xml", xp.parsed_tag
            );
            return ERR_XML_PARSE;
        }
        if (xp.match_tag("/app_config")) return 0;
        if (xp.match_tag("app")) {
            APP_CONFIG ac;
            int retval = ac.parse(xp, p);
            if (retval) return retval;
            app_configs.push_back(ac);
            continue;
        }
        if (xp.match_tag("app_version")) {
            APP_VERSION_CONFIG avc;
            int retval = avc.parse(xp, p);
            if (retval) return retval;
            app_version_configs.push_back(avc);
            continue;
        }
        if (xp.parse_int("project_max_concurrent", n)) {
            if (n >= 0) {
                have_max_concurrent = true;
                project_max_concurrent = n;
            }
            continue;
        }
        msg_printf_notice(p, false, NULL,
            "Unknown tag in app_config.xml: %s",
            xp.parsed_tag
        );

        xp.skip_unexpected(log_flags.unparsed_xml, "APP_CONFIGS::parse");
    }
    msg_printf_notice(p, false, NULL,
        "Missing </app_config> in app_config.xml"
    );
    return ERR_XML_PARSE;
}

int APP_CONFIGS::parse_file(FILE* f, PROJECT* p) {
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    return parse(xp, p);
}

static void show_warning(PROJECT* p, char* name) {
    msg_printf(p, MSG_USER_ALERT,
        "Your app_config.xml file refers to an unknown application '%s'.  Known applications: %s",
        name, app_list_string(p).c_str()
    );
}

int APP_CONFIGS::config_app_versions(PROJECT* p, bool show_warnings) {
    unsigned int i;
    bool showed_notice = false;
    for (i=0; i<app_configs.size(); i++) {
        APP_CONFIG& ac = app_configs[i];
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

        if (!ac.gpu_gpu_usage || !ac.gpu_cpu_usage) continue;
        for (unsigned int j=0; j<gstate.app_versions.size(); j++) {
            APP_VERSION* avp = gstate.app_versions[j];
            if (avp->app != app) continue;
            if (!avp->gpu_usage.rsc_type) continue;
            avp->gpu_usage.usage = ac.gpu_gpu_usage;
            avp->avg_ncpus = ac.gpu_cpu_usage;
        }
    }
    for (i=0; i<app_version_configs.size(); i++) {
        APP_VERSION_CONFIG& avc = app_version_configs[i];
        APP* app = gstate.lookup_app(p, avc.app_name);
        if (!app) {
            if (show_warnings) {
                show_warning(p, avc.app_name);
                showed_notice = true;
            }
            continue;
        }
        bool found = false;
        for (unsigned int j=0; j<gstate.app_versions.size(); j++) {
            APP_VERSION* avp = gstate.app_versions[j];
            if (avp->app != app) continue;
            if (strcmp(avp->plan_class, avc.plan_class)) continue;
            found = true;
            if (strlen(avc.cmdline)) {
                strcpy(avp->cmdline, avc.cmdline);
            }
            if (avc.avg_ncpus) {
                avp->avg_ncpus = avc.avg_ncpus;
            }
            if (avc.ngpus) {
                avp->gpu_usage.usage = avc.ngpus;
            }
        }
        if (!found) {
            msg_printf(p, MSG_USER_ALERT,
                "Entry in app_config.xml for app '%s', plan class '%s' doesn't match any app versions",
                avc.app_name, avc.plan_class
            );
        }
    }
    if (showed_notice) return ERR_XML_PARSE;
    return 0;
}

void max_concurrent_init() {
    for (unsigned int i=0; i<gstate.apps.size(); i++) {
        gstate.apps[i]->n_concurrent = 0;
    }
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        gstate.projects[i]->n_concurrent = 0;
    }
}

// undo the effects of an app_config.xml that no longer exists
// NOTE: all we can do here is to clear APP::max_concurrent;
// we can't restore device usage info because we don't have it.
// It will be restored on next scheduler RPC.
//
static void clear_app_config(PROJECT* p) {
    p->app_configs.clear();
    for (unsigned int i=0; i<gstate.apps.size(); i++) {
        APP* app = gstate.apps[i];
        if (app->project != p) continue;
        app->max_concurrent = 0;
    }
}

// check for app_config.xml files, and parse them.
// Called at startup and on read_cc_config() RPC
//
void check_app_config() {
    char path[MAXPATHLEN];
    FILE* f;

    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        sprintf(path, "%s/%s", p->project_dir(), APP_CONFIG_FILE_NAME);
        f = boinc_fopen(path, "r");
        if (!f) {
            clear_app_config(p);
            continue;
        }
        msg_printf(p, MSG_INFO, "Found %s", APP_CONFIG_FILE_NAME);
        int retval = p->app_configs.parse_file(f, p);
        if (!retval) {
            retval = p->app_configs.config_app_versions(p, true);
            if (!retval) {
                notices.remove_notices(p, REMOVE_APP_CONFIG_MSG);
            }
        }
        fclose(f);
    }
}
