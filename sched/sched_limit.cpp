// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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


#include "sched_main.h"

#include "sched_limit.h"

int RSC_JOB_LIMIT::parse(XML_PARSER& xp, const char* end_tag) {
    char tag[1024];
    bool is_tag;
    
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, end_tag)) {
            return 0;
        }
        if (xp.parse_int(tag, "jobs", base_limit)) {
            continue;
        }
        if (xp.parse_bool(tag, "per_proc", per_proc)) {
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int JOB_LIMIT::parse(XML_PARSER& xp, const char* end_tag) {
    char tag[1024];
    bool is_tag;
    
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, end_tag)) {
            return 0;
        }
        if (xp.parse_str(tag, "app_name", app_name, sizeof(app_name))) {
            continue;
        }
        if (!strcmp(tag, "total_limit")) {
            total.parse(xp, "/total_limit");
            continue;
        }
        if (!strcmp(tag, "cpu_limit")) {
            cpu.parse(xp, "/cpu_limit");
            continue;
        }
        if (!strcmp(tag, "gpu_limit")) {
            gpu.parse(xp, "/gpu_limit");
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int JOB_LIMITS::parse(XML_PARSER& xp, const char* end_tag) {
    char tag[1024];
    bool is_tag;
    
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, end_tag)) {
            return 0;
        }
        if (!strcmp(tag, "project")) {
            project_limits.parse(xp, "/project");
            continue;
        }
        if (!strcmp(tag, "app")) {
            JOB_LIMIT jl;
            jl.parse(xp, "/app");
            if (!strlen(jl.app_name)) {
                log_messages.printf(MSG_NORMAL, "missing app name\n");
                continue;
            }
            app_limits.push_back(jl);
            continue;
        }
    }
    return ERR_XML_PARSE;
}
