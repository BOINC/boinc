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
            while (!xp.get_tag()) {
                if (xp.match_tag("/gpu_versions")) break;
                if (xp.parse_double("gpu_usage", gpu_gpu_usage)) continue;
                if (xp.parse_double("cpu_usage", gpu_cpu_usage)) continue;
            }
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(p, MSG_INFO,
                "Unparsed line in app_info.xml: %s",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "APP_CONFIG::parse");
    }
    return ERR_XML_PARSE;
}

int APP_CONFIGS::parse(XML_PARSER& xp, PROJECT* p) {
    app_configs.clear();
    if (!xp.parse_start("app_config")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (xp.match_tag("/app_config")) return 0;
        if (xp.match_tag("app")) {
            APP_CONFIG ac;
            int retval = ac.parse(xp, p);
            if (!retval) {
                app_configs.push_back(ac);
            }
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(p, MSG_INFO,
                "Unparsed line in app_info.xml: %s",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "APP_CONFIGS::parse");
    }
    return ERR_XML_PARSE;
}

int APP_CONFIGS::parse_file(FILE* f, PROJECT* p) {
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    int retval = parse(xp, p);
    return retval;
}

void APP_CONFIGS::config_app_versions(PROJECT* p) {
    for (unsigned int i=0; i<app_configs.size(); i++) {
        APP_CONFIG& ac = app_configs[i];
        APP* app = gstate.lookup_app(p, ac.name);
        if (!app) {
            msg_printf(p, MSG_USER_ALERT,
                "app %s not found in app_config.xml", ac.name
            );
            continue;
        }
        app->max_concurrent = ac.max_concurrent;
        if (!ac.gpu_gpu_usage || !ac.gpu_cpu_usage) continue;
        for (unsigned int j=0; j<gstate.app_versions.size(); j++) {
            APP_VERSION* avp = gstate.app_versions[j];
            if (avp->app != app) continue;
            if (!avp->gpu_usage.rsc_type) continue;
            avp->gpu_usage.usage = ac.gpu_gpu_usage;
            avp->avg_ncpus = ac.gpu_cpu_usage;
        }
    }
}

void max_concurrent_init() {
    for (unsigned int i=0; i<gstate.apps.size(); i++) {
        gstate.apps[i]->n_concurrent = 0;
    }
}

void check_app_config() {
    char dir[256], path[MAXPATHLEN];
    FILE* f;

    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        get_project_dir(p, dir, sizeof(dir));
        sprintf(path, "%s/%s", dir, APP_CONFIG_FILE_NAME);
        f = boinc_fopen(path, "r");
        if (!f) continue;
        msg_printf(p, MSG_INFO,
            "Found %s", APP_CONFIG_FILE_NAME
        );
        int retval = p->app_configs.parse_file(f, p);
        if (!retval) {
            p->app_configs.config_app_versions(p);
        }
        fclose(f);
    }
}
