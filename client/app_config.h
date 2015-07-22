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

#ifndef _APP_CONFIG_
#define _APP_CONFIG_

#include <vector>

#include "parse.h"

#include "client_types.h"

struct PROJECT;
struct RESULT;

struct APP_CONFIG {
    char name[256];
    int max_concurrent;
    double gpu_gpu_usage;
    double gpu_cpu_usage;
    bool fraction_done_exact;

    int parse(XML_PARSER&, PROJECT*);
    int parse_gpu_versions(XML_PARSER&, PROJECT*);
};

struct APP_VERSION_CONFIG {
    char app_name[256];
    char plan_class[256];
    char cmdline[256];
    double avg_ncpus;
    double ngpus;

    int parse(XML_PARSER&, PROJECT*);
};

struct APP_CONFIGS {
    std::vector<APP_CONFIG> app_configs;
    std::vector<APP_VERSION_CONFIG> app_version_configs;
    int project_max_concurrent;

    int parse(XML_PARSER&, PROJECT*);
    int parse_file(FILE*, PROJECT*);
    int config_app_versions(PROJECT*, bool show_warnings);
    void clear() {
        app_configs.clear();
        app_version_configs.clear();
        project_max_concurrent = 0;
    }
};

extern bool have_max_concurrent;

extern void max_concurrent_init();

extern void check_app_config();

#endif
