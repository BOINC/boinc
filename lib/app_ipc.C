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
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <string.h>
#endif

#include "parse.h"
#include "error_numbers.h"
#include "util.h"

#include "app_ipc.h"

char* xml_graphics_modes[NGRAPHICS_MODES] = {
    "<mode_unsupported/>",
    "<mode_hide_graphics/>",
    "<mode_window/>",
    "<mode_fullscreen/>",
    "<mode_blankscreen/>"
};

#define REREAD_PREFS_MSG "<reread_prefs/>"

int write_init_data_file(FILE* f, APP_INIT_DATA& ai) {
	string str1, str2;
    if (strlen(ai.app_name)) {
        fprintf(f, "<app_name>%s</app_name>\n", ai.app_name);
    }
    if (strlen(ai.project_preferences)) {
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
    if (strlen(ai.comm_obj_name)) {
        fprintf(f, "<comm_obj_name>%s</comm_obj_name>\n", ai.comm_obj_name);
    }
#else
    fprintf(f, "<shm_key>%d</shm_key>\n", ai.shm_key);
#endif
    fprintf(f,
        "<slot>%d</slot>\n"
        "<wu_cpu_time>%f</wu_cpu_time>\n"
        "<user_total_credit>%f</user_total_credit>\n"
        "<user_expavg_credit>%f</user_expavg_credit>\n"
        "<host_total_credit>%f</host_total_credit>\n"
        "<host_expavg_credit>%f</host_expavg_credit>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<fraction_done_update_period>%f</fraction_done_update_period>\n",
        ai.slot,
        ai.wu_cpu_time,
        ai.user_total_credit,
        ai.user_expavg_credit,
        ai.host_total_credit,
        ai.host_expavg_credit,
        ai.checkpoint_period,
        ai.fraction_done_update_period
    );
    return 0;
}

int parse_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char buf[256];
    memset(&ai, 0, sizeof(ai));
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<project_preferences>")) {
            safe_strncpy(ai.project_preferences, "", sizeof(ai.project_preferences));
            while (fgets(buf, 256, f)) {
                if (match_tag(buf, "</project_preferences>")) break;
                safe_strcat(ai.project_preferences, buf);
            }
            continue;
        }
        else if (parse_str(buf, "<app_name>", ai.app_name, sizeof(ai.app_name))) continue;
        else if (parse_str(buf, "<user_name>", ai.user_name, sizeof(ai.user_name))) continue;
        else if (parse_str(buf, "<team_name>", ai.team_name, sizeof(ai.team_name))) continue;
        else if (parse_str(buf, "<project_dir>", ai.project_dir, sizeof(ai.project_dir))) continue;
        else if (parse_str(buf, "<boinc_dir>", ai.boinc_dir, sizeof(ai.boinc_dir))) continue;
        else if (parse_str(buf, "<authenticator>", ai.authenticator, sizeof(ai.authenticator))) continue;
        else if (parse_str(buf, "<wu_name>", ai.wu_name, sizeof(ai.wu_name))) continue;
#ifdef _WIN32
        else if (parse_str(buf, "<comm_obj_name>", ai.comm_obj_name, sizeof(ai.comm_obj_name))) continue;
#else
        else if (parse_int(buf, "<shm_key>", ai.shm_key)) continue;
#endif
        else if (parse_int(buf, "<slot>", ai.slot)) continue;
        else if (parse_double(buf, "<user_total_credit>", ai.user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", ai.user_expavg_credit)) continue;
        else if (parse_double(buf, "<host_total_credit>", ai.host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", ai.host_expavg_credit)) continue;
        else if (parse_double(buf, "<wu_cpu_time>", ai.wu_cpu_time)) continue;
        else if (parse_double(buf, "<checkpoint_period>", ai.checkpoint_period)) continue;
        else if (parse_double(buf, "<fraction_done_update_period>", ai.fraction_done_update_period)) continue;
        //else fprintf(stderr, "parse_init_data_file: unrecognized %s", buf);
    }
    return 0;
}

// TODO: this should handle arbitrarily many fd/filename pairs.
// Also, give the tags better names
//
int write_fd_init_file(FILE* f, char *file_name, int fdesc, bool input_file) {
    if (input_file) {
        fprintf(f, "<fdesc_dup_infile>%s</fdesc_dup_infile>\n", file_name);
        fprintf(f, "<fdesc_dup_innum>%d</fdesc_dup_innum>\n", fdesc);
    } else {
        fprintf(f, "<fdesc_dup_outfile>%s</fdesc_dup_outfile>\n", file_name);
        fprintf(f, "<fdesc_dup_outnum>%d</fdesc_dup_outnum>\n", fdesc);
    }
    return 0;
}

// TODO: this should handle arbitrarily many fd/filename pairs.
// Also, this shouldn't be doing the actual duping!
//
int parse_fd_init_file(FILE* f) {
    char buf[256],filename[256];
    int filedesc;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<fdesc_dup_infile>", filename, sizeof(filename))) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_innum>", filedesc)) {
                    freopen(filename, "r", stdin);
                    fprintf(stderr, "opened input file %s\n", filename);
                }
            }
        } else if (parse_str(buf, "<fdesc_dup_outfile>", filename, sizeof(filename))) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_outnum>", filedesc)) {
                    freopen(filename, "w", stdout);
                    fprintf(stderr, "opened output file %s\n", filename);
                }
            }
        } else fprintf(stderr, "parse_fd_init_file: unrecognized %s", buf);
    }
    return 0;
}

APP_CLIENT_SHM::APP_CLIENT_SHM() {
    shm = 0;
}

bool APP_CLIENT_SHM::pending_msg(int seg_num) {
    if (seg_num < 0 || seg_num >= NUM_SEGS || shm == NULL) return false;
    return (shm[seg_num*SHM_SEG_SIZE]?true:false);
}

bool APP_CLIENT_SHM::get_msg(char *msg, int seg_num) {
    if (seg_num < 0 || seg_num >= NUM_SEGS || shm == NULL) return false;

    // Check if there's an available message
    //
    if (!shm[seg_num*SHM_SEG_SIZE]) return false;

    // Copy the message from shared memory
    //
    safe_strncpy(msg, &shm[(seg_num*SHM_SEG_SIZE)+1], SHM_SEG_SIZE-1);

    // Reset the message status flag
    //
    shm[seg_num*SHM_SEG_SIZE] = 0;
    return true;
}

bool APP_CLIENT_SHM::send_msg(char *msg,int seg_num) {
    if (seg_num < 0 || seg_num >= NUM_SEGS || shm == NULL) return false;

    // Check if there's already a message
    //
    //if (shm[seg_num*SHM_SEG_SIZE]) return false;

    // Copy the message into shared memory
    //
    safe_strncpy(&shm[(seg_num*SHM_SEG_SIZE)+1], msg, SHM_SEG_SIZE-1);

    // Set the message status flag
    //
    shm[seg_num*SHM_SEG_SIZE] = 1;
    return true;
}

void APP_CLIENT_SHM::reset_msgs(void) {
    if (shm == NULL) return;
    memset(shm, 0, sizeof(char)*NUM_SEGS*SHM_SEG_SIZE);
}

void APP_CLIENT_SHM::reset_msg(int seg_num) {
    if (seg_num < 0 || seg_num >= NUM_SEGS || shm == NULL) return;
    memset(&shm[seg_num*SHM_SEG_SIZE], 0, sizeof(char)*SHM_SEG_SIZE);
}

bool APP_CLIENT_SHM::send_graphics_msg(int seg, int msg, int mode) {
    switch (msg) {
    case GRAPHICS_MSG_SET_MODE:
        return send_msg(xml_graphics_modes[mode], seg);
        break;
    case GRAPHICS_MSG_REREAD_PREFS:
        return send_msg(REREAD_PREFS_MSG, seg);
        break;
    }
    return false;
}

bool APP_CLIENT_SHM::get_graphics_msg(int seg, int& msg, int& mode) {
    char buf[SHM_SEG_SIZE];
    int i;

    if (!get_msg(buf, seg)) return false;
    for (i=0; i<NGRAPHICS_MODES; i++) {
        if (match_tag(buf, xml_graphics_modes[i])) {
            msg = GRAPHICS_MSG_SET_MODE;
            mode = i;
            return true;
        }
    }
    if (match_tag(buf, REREAD_PREFS_MSG)) {
        msg = GRAPHICS_MSG_REREAD_PREFS;
        return true;
    }
    return false;
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
    fp = fopen(virtual_name, "r");
    if (!fp) return ERR_FOPEN;

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
int boinc_resolve_filename(const char *virtual_name, string& physical_name) {

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

