// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstring>
#endif

#include "boinc_api.h"
#include "parse.h"
#include "error_numbers.h"
#include "util.h"
#include "filesys.h"
#include "miofile.h"

#include "app_ipc.h"

using std::string;

char* xml_graphics_modes[NGRAPHICS_MSGS] = {
    "<mode_unsupported/>",
    "<mode_hide_graphics/>",
    "<mode_window/>",
    "<mode_fullscreen/>",
    "<mode_blankscreen/>",
    "<reread_prefs/>"
};

int write_init_data_file(FILE* f, APP_INIT_DATA& ai) {
	string str1, str2;
    fprintf(f, "<app_init_data>\n<core_version>%d</core_version>\n", ai.core_version);
    if (strlen(ai.app_name)) {
        fprintf(f, "<app_name>%s</app_name>\n", ai.app_name);
    }
    if (ai.project_preferences && strlen(ai.project_preferences)) {
        fprintf(f, "<project_preferences>\n%s</project_preferences>\n", ai.project_preferences);
    }
    if (strlen(ai.team_name)) {
		str1 = ai.team_name;
		xml_escape(str1, str2);
        fprintf(f, "<team_name>%s</team_name>\n", str2.c_str());
    }
    if (strlen(ai.user_name)) {
		str1 = ai.user_name;
		xml_escape(str1, str2);
        fprintf(f, "<user_name>%s</user_name>\n", str2.c_str());
    }
    if (strlen(ai.project_dir)) {
        fprintf(f, "<project_dir>%s</project_dir>\n", ai.project_dir);
    }
    if (strlen(ai.boinc_dir)) {
        fprintf(f, "<boinc_dir>%s</boinc_dir>\n", ai.boinc_dir);
    }
    if (strlen(ai.authenticator)) {
        fprintf(f, "<authenticator>%s</authenticator>\n", ai.authenticator);
    }
    if (strlen(ai.wu_name)) {
        fprintf(f, "<wu_name>%s</wu_name>\n", ai.wu_name);
    }
#ifdef _WIN32
    if (strlen(ai.shmem_seg_name)) {
        fprintf(f, "<comm_obj_name>%s</comm_obj_name>\n", ai.shmem_seg_name);
    }
#else
    fprintf(f, "<shm_key>%d</shm_key>\n", ai.shmem_seg_name);
#endif
    fprintf(f,
        "<slot>%d</slot>\n"
        "<wu_cpu_time>%f</wu_cpu_time>\n"
        "<user_total_credit>%f</user_total_credit>\n"
        "<user_expavg_credit>%f</user_expavg_credit>\n"
        "<host_total_credit>%f</host_total_credit>\n"
        "<host_expavg_credit>%f</host_expavg_credit>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<fraction_done_update_period>%f</fraction_done_update_period>\n"
        "<fraction_done_start>%f</fraction_done_start>\n"
        "<fraction_done_end>%f</fraction_done_end>\n",
        ai.slot,
        ai.wu_cpu_time,
        ai.user_total_credit,
        ai.user_expavg_credit,
        ai.host_total_credit,
        ai.host_expavg_credit,
        ai.checkpoint_period,
        ai.fraction_done_update_period,
        ai.fraction_done_start,
        ai.fraction_done_end
    );
    MIOFILE mf;
    mf.init_file(f);
    ai.host_info.write(mf);
    ai.proxy_info.write(mf);
    ai.global_prefs.write(f);
    fprintf(f, "</app_init_data>\n");
    return 0;
}

int parse_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char buf[256];
    int retval;
    bool flag;

    memset(&ai, 0, sizeof(ai));
    ai.fraction_done_start = 0;
    ai.fraction_done_end = 1;
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<project_preferences>")) {
            retval = dup_element_contents(f, "</project_preferences>", &ai.project_preferences);
            if (retval) return retval;
            continue;
        }
        if (match_tag(buf, "<global_preferences>")) {
            retval = ai.global_prefs.parse(f, "", flag);
            if (retval) return retval;
            continue;
        }
        else if (match_tag(buf, "<host_info>")) {
            MIOFILE mf;
            mf.init_file(f);
            ai.host_info.parse(mf);
            continue;
        }
        else if (parse_int(buf, "<core_version>", ai.core_version)) continue;
        else if (parse_str(buf, "<app_name>", ai.app_name, sizeof(ai.app_name))) continue;
        else if (parse_str(buf, "<user_name>", ai.user_name, sizeof(ai.user_name))) continue;
        else if (parse_str(buf, "<team_name>", ai.team_name, sizeof(ai.team_name))) continue;
        else if (parse_str(buf, "<project_dir>", ai.project_dir, sizeof(ai.project_dir))) continue;
        else if (parse_str(buf, "<boinc_dir>", ai.boinc_dir, sizeof(ai.boinc_dir))) continue;
        else if (parse_str(buf, "<authenticator>", ai.authenticator, sizeof(ai.authenticator))) continue;
        else if (parse_str(buf, "<wu_name>", ai.wu_name, sizeof(ai.wu_name))) continue;
#ifdef _WIN32
        else if (parse_str(buf, "<comm_obj_name>", ai.shmem_seg_name, sizeof(ai.shmem_seg_name))) continue;
#else
        else if (parse_int(buf, "<shm_key>", ai.shmem_seg_name)) continue;
#endif
        else if (parse_int(buf, "<slot>", ai.slot)) continue;
        else if (parse_double(buf, "<user_total_credit>", ai.user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", ai.user_expavg_credit)) continue;
        else if (parse_double(buf, "<host_total_credit>", ai.host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", ai.host_expavg_credit)) continue;
        else if (parse_double(buf, "<wu_cpu_time>", ai.wu_cpu_time)) continue;
        else if (parse_double(buf, "<checkpoint_period>", ai.checkpoint_period)) continue;
        else if (parse_double(buf, "<fraction_done_update_period>", ai.fraction_done_update_period)) continue;
        else if (parse_double(buf, "<fraction_done_start>", ai.fraction_done_start)) continue;
        else if (parse_double(buf, "<fraction_done_end>", ai.fraction_done_end)) continue;
        //else fprintf(stderr, "parse_init_data_file: unrecognized %s", buf);
    }
    return 0;
}

APP_CLIENT_SHM::APP_CLIENT_SHM() {
    shm = 0;
}
bool MSG_CHANNEL::get_msg(char *msg) {
    if (!buf[0]) return false;
    safe_strncpy(msg, buf+1, MSG_CHANNEL_SIZE-1);
    buf[0] = 0;
    return true;
}

bool MSG_CHANNEL::send_msg(char *msg) {
    if (buf[0]) return false;
    safe_strncpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
    return true;
}

void MSG_CHANNEL::send_msg_overwrite(char* msg) {
    safe_strncpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
}

int APP_CLIENT_SHM::decode_graphics_msg(char* msg) {
    int i;
    for (i=0; i<NGRAPHICS_MSGS; i++) {
        if (match_tag(msg, xml_graphics_modes[i])) {
            return i;
        }
    }
    return 0;
}

void APP_CLIENT_SHM::reset_msgs() {
    memset(shm, 0, sizeof(SHARED_MEM));
}

int write_graphics_file(FILE* f, GRAPHICS_INFO* gi) {
    fprintf(f,
        "<graphics_info>\n"
        "    <graphics_xsize>%d</graphics_xsize>\n"
        "    <graphics_ysize>%d</graphics_ysize>\n"
        "    <graphics_refresh_period>%f</graphics_refresh_period>\n"
        "</graphics_info>\n",
        gi->xsize,
        gi->ysize,
        gi->refresh_period
    );

    return 0;
}

int parse_graphics_file(FILE* f, GRAPHICS_INFO* gi) {
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<graphics_info>")) continue;
        if (match_tag(buf, "</graphics_info>")) return 0;
        else if (parse_int(buf, "<graphics_xsize>", gi->xsize)) continue;
        else if (parse_int(buf, "<graphics_ysize>", gi->ysize)) continue;
        else if (parse_double(buf, "<graphics_refresh_period>", gi->refresh_period)) continue;
        else fprintf(stderr, "parse_graphics_file: unrecognized %s", buf);
    }
    return ERR_XML_PARSE;
}

// create a file (new_link) which contains an XML
// reference to existing file.
//
int boinc_link(const char *existing, const char *new_link) {
    FILE *fp;

    fp = fopen(new_link, "w");
    if (!fp) return ERR_FOPEN;
    fprintf(fp, "<soft_link>%s</soft_link>\n", existing);
    fclose(fp);

    return 0;
}

// resolve XML soft link
//
int boinc_resolve_filename(const char *virtual_name, char *physical_name, int len) {
    FILE *fp;
    char buf[512];

    safe_strncpy(physical_name, virtual_name, len);

    // Open the file and load the first line
    //
    fp = boinc_fopen(virtual_name, "r");
    if (!fp) return ERR_FOPEN;

    // must initialize buf since fgets() on an empty file won't do anything
    //
    buf[0] = 0;
    fgets(buf, 512, fp);
    fclose(fp);

    // If it's the <soft_link> XML tag, return its value,
    // otherwise, return the original file name
    //
    parse_str(buf, "<soft_link>", physical_name, len);
    return 0;
}


// resolve XML soft link
//
int boinc_resolve_filename_s(const char *virtual_name, string& physical_name) {

    physical_name = virtual_name;
    // Open the file and load the first line
    FILE *fp = fopen(virtual_name, "r");
    if (!fp) return ERR_FOPEN;

    char buf[512];
    fgets(buf, 512, fp);
    fclose(fp);

    // If it's the <soft_link> XML tag, return its value,
    // otherwise, return the original file name
    //
    parse_str(buf, "<soft_link>", physical_name);
    return 0;
}

