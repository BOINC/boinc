// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include "windows_cpp.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parse.h"

#include "error_numbers.h"
#include "file_names.h"

#include "prefs.h"

PREFS::PREFS() {
    mod_time = 0;
    dont_run_on_batteries = false;
    dont_run_if_user_active = false;
    confirm_before_connecting = false;
    high_water_days = 3;
    low_water_days = 1;
    disk_max_used_gb = 1;
    disk_max_used_pct = 0.5;
    disk_min_free_gb = 0.1;
};

// Parse XML based prefs, usually from prefs.xml
//
int PREFS::parse(FILE* in) {
    char buf[256];
    PROJECT* project;

    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</preferences>")) return 0;
        else if (match_tag(buf, "<project>")) {
            project = new PROJECT;
            project->parse_prefs(in);
            projects.push_back(project);
        } else if (match_tag(buf, "<dont_run_on_batteries/>")) {
            dont_run_on_batteries = true;
            continue;
        } else if (match_tag(buf, "<dont_run_if_user_active/>")) {
            dont_run_if_user_active = true;
            continue;
        } else if (match_tag(buf, "<confirm_before_connecting/>")) {
            confirm_before_connecting = true;
            continue;
        } else if (parse_double(buf, "<high_water_days>", high_water_days)) {
            continue;
        } else if (parse_double(buf, "<low_water_days>", low_water_days)) {
            continue;
        } else if (parse_double(buf, "<disk_max_used_gb>", disk_max_used_gb)) {
            continue;
        } else if (parse_double(buf, "<disk_max_used_pct>", disk_max_used_pct)) {
            continue;
        } else if (parse_double(buf, "<disk_min_free_gb>", disk_min_free_gb)) {
            continue;
        }
    }
    return ERR_XML_PARSE;
}

// Parse prefs.xml for user preferences
//
int PREFS::parse_file() {
    FILE* f;
    int retval;

    f = fopen(PREFS_FILE_NAME, "r");
    if (!f) return ERR_FOPEN;
    retval = parse(f);
    fclose(f);
    return retval;
}

// Write the default preference set for a project
// TODO: should mod_time really be 1?
int write_initial_prefs(char* master_url, char* authenticator) {
    FILE* f = fopen(PREFS_FILE_NAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<preferences>\n"
        "    <mod_time>1</mod_time>\n"
        "    <high_water_days>2</high_water_days>\n"
        "    <low_water_days>1</low_water_days>\n"
        "    <project>\n"
        "        <master_url>%s</master_url>\n"
        "        <authenticator>%s</authenticator>\n"
        "        <resource_share>1</resource_share>\n"
        "    </project>\n"
        "</preferences>\n",
        master_url,
        authenticator
    );
    fclose(f);
    return 0;
}
