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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#include <cstring>
#include <string>
#endif

#ifdef _MSC_VER
#define strdup _strdup
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "miofile.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "url.h"

#include "app_ipc.h"

using std::string;

const char* xml_graphics_modes[NGRAPHICS_MSGS] = {
    "<mode_unsupported/>",
    "<mode_hide_graphics/>",
    "<mode_window/>",
    "<mode_fullscreen/>",
    "<mode_blankscreen/>",
    "<reread_prefs/>",
    "<mode_quit/>"
};

GRAPHICS_MSG::GRAPHICS_MSG() {
    memset(this, 0, sizeof(GRAPHICS_MSG));
}

APP_INIT_DATA::APP_INIT_DATA() : project_preferences(NULL) {
}

APP_INIT_DATA::~APP_INIT_DATA() {
    if (project_preferences) {
        free(project_preferences);
        project_preferences=0;      // paranoia
    }
}

APP_INIT_DATA::APP_INIT_DATA(const APP_INIT_DATA& a) {
    copy(a);
}

APP_INIT_DATA &APP_INIT_DATA::operator=(const APP_INIT_DATA& a) {
    if (this != &a) {
      copy(a);
    }
    return *this;
}

void APP_INIT_DATA::copy(const APP_INIT_DATA& a) {
    // memcpy the strings
    memcpy(app_name, a.app_name, 256); 
    memcpy(symstore, a.symstore, 256); 
    memcpy(acct_mgr_url, a.acct_mgr_url, 256); 
    memcpy(user_name, a.user_name, 256); 
    memcpy(team_name, a.team_name, 256); 
    memcpy(project_dir, a.project_dir, 256); 
    memcpy(boinc_dir, a.boinc_dir, 256); 
    memcpy(wu_name, a.wu_name, 256); 
    memcpy(authenticator, a.authenticator, 256); 
    memcpy(&shmem_seg_name, &a.shmem_seg_name, sizeof(SHMEM_SEG_NAME)); 
                
    // use assignment for the rest, especially the classes
    // (so that the overloaded operators are called!)
    major_version                 = a.major_version;               
    minor_version                 = a.minor_version;
    release                       = a.release;
    app_version                   = a.app_version;
    hostid                        = a.hostid;
    slot                          = a.slot;
    user_total_credit             = a.user_total_credit;
    user_expavg_credit            = a.user_expavg_credit;
    host_total_credit             = a.host_total_credit;
    host_expavg_credit            = a.host_expavg_credit;
    resource_share_fraction       = a.resource_share_fraction;
    host_info                     = a.host_info;
    proxy_info                    = a.proxy_info;
    global_prefs                  = a.global_prefs;
    starting_elapsed_time         = a.starting_elapsed_time;
    rsc_fpops_est                 = a.rsc_fpops_est;
    rsc_fpops_bound               = a.rsc_fpops_bound;
    rsc_memory_bound              = a.rsc_memory_bound;
    rsc_disk_bound                = a.rsc_disk_bound;
    computation_deadline          = a.computation_deadline;
    fraction_done_start           = a.fraction_done_start;
    fraction_done_end             = a.fraction_done_end;
    checkpoint_period             = a.checkpoint_period;
    wu_cpu_time                   = a.wu_cpu_time;
    if (a.project_preferences) {
        project_preferences = strdup(a.project_preferences);
    } else {
        project_preferences = NULL;
    }
}

int write_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char buf[2048];
    fprintf(f,
        "<app_init_data>\n"
        "<major_version>%d</major_version>\n"
        "<minor_version>%d</minor_version>\n"
        "<release>%d</release>\n"
        "<app_version>%d</app_version>\n"
        "<hostid>%d</hostid>\n",
        ai.major_version,
        ai.minor_version,
        ai.release,
        ai.app_version,
        ai.hostid
    );
    if (strlen(ai.app_name)) {
        fprintf(f, "<app_name>%s</app_name>\n", ai.app_name);
    }
    if (strlen(ai.symstore)) {
        fprintf(f, "<symstore>%s</symstore>\n", ai.symstore);
    }
    if (strlen(ai.acct_mgr_url)) {
        fprintf(f, "<acct_mgr_url>%s</acct_mgr_url>\n", ai.acct_mgr_url);
    }
    if (ai.project_preferences && strlen(ai.project_preferences)) {
        fprintf(f, "<project_preferences>\n%s</project_preferences>\n", ai.project_preferences);
    }
    if (strlen(ai.team_name)) {
        xml_escape(ai.team_name, buf, sizeof(buf));
        fprintf(f, "<team_name>%s</team_name>\n", buf);
    }
    if (strlen(ai.user_name)) {
        xml_escape(ai.user_name, buf, sizeof(buf));
        fprintf(f, "<user_name>%s</user_name>\n", buf);
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
        "<starting_elapsed_time>%f</starting_elapsed_time>\n"
        "<user_total_credit>%f</user_total_credit>\n"
        "<user_expavg_credit>%f</user_expavg_credit>\n"
        "<host_total_credit>%f</host_total_credit>\n"
        "<host_expavg_credit>%f</host_expavg_credit>\n"
        "<resource_share_fraction>%f</resource_share_fraction>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<fraction_done_start>%f</fraction_done_start>\n"
        "<fraction_done_end>%f</fraction_done_end>\n"
        "<rsc_fpops_est>%f</rsc_fpops_est>\n"
        "<rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "<rsc_memory_bound>%f</rsc_memory_bound>\n"
        "<rsc_disk_bound>%f</rsc_disk_bound>\n"
        "<computation_deadline>%f</computation_deadline>\n",
        ai.slot,
        ai.wu_cpu_time,
        ai.starting_elapsed_time,
        ai.user_total_credit,
        ai.user_expavg_credit,
        ai.host_total_credit,
        ai.host_expavg_credit,
        ai.resource_share_fraction,
        ai.checkpoint_period,
        ai.fraction_done_start,
        ai.fraction_done_end,
        ai.rsc_fpops_est,
        ai.rsc_fpops_bound,
        ai.rsc_memory_bound,
        ai.rsc_disk_bound,
        ai.computation_deadline
    );
    MIOFILE mf;
    mf.init_file(f);
    ai.host_info.write(mf, false, true);
    ai.proxy_info.write(mf);
    ai.global_prefs.write(mf);
    fprintf(f, "</app_init_data>\n");
    return 0;
}

void APP_INIT_DATA::clear() {
    major_version = 0;
    minor_version = 0;
    release = 0;
    app_version = 0;
    strcpy(app_name, "");
    strcpy(symstore, "");
    strcpy(acct_mgr_url, "");
    project_preferences = NULL;
    hostid = 0;
    strcpy(user_name, "");
    strcpy(team_name, "");
    strcpy(project_dir, "");
    strcpy(boinc_dir, "");
    strcpy(wu_name, "");
    strcpy(authenticator, "");
    slot = 0;
    user_total_credit = 0;
    user_expavg_credit = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    resource_share_fraction = 0;
    host_info.clear_host_info();
    proxy_info.clear();
    global_prefs.defaults();
    starting_elapsed_time = 0;
    rsc_fpops_est = 0;
    rsc_fpops_bound = 0;
    rsc_memory_bound = 0;
    rsc_disk_bound = 0;
    computation_deadline = 0;
    fraction_done_start = 0;
    fraction_done_end = 0;
    checkpoint_period = 0;
    memset(&shmem_seg_name, 0, sizeof(shmem_seg_name));
    wu_cpu_time = 0;
}

int parse_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char tag[1024], buf[256];
    int retval;
    bool flag, is_tag;

    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("app_init_data")) {
        fprintf(stderr, "no start tag in app init data\n");
        return ERR_XML_PARSE;
    }

    if (ai.project_preferences) {
        free(ai.project_preferences);
        ai.project_preferences = 0;
    }
    ai.clear();
    ai.fraction_done_start = 0;
    ai.fraction_done_end = 1;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "unexpected text in init_data.xml: %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/app_init_data")) return 0;
        if (!strcmp(tag, "project_preferences")) {
            retval = dup_element(f, "project_preferences", &ai.project_preferences);
            if (retval) return retval;
            continue;
        }
        if (!strcmp(tag, "global_preferences")) {
            GLOBAL_PREFS_MASK mask;
            retval = ai.global_prefs.parse(xp, "", flag, mask);
            if (retval) return retval;
            continue;
        }
        if (!strcmp(tag, "host_info")) {
            ai.host_info.parse(mf);
            continue;
        }
        if (!strcmp(tag, "proxy_info")) {
            ai.proxy_info.parse(mf);
            continue;
        }
        if (xp.parse_int(tag, "major_version", ai.major_version)) continue;
        if (xp.parse_int(tag, "minor_version", ai.minor_version)) continue;
        if (xp.parse_int(tag, "release", ai.release)) continue;
        if (xp.parse_int(tag, "app_version", ai.app_version)) continue;
        if (xp.parse_str(tag, "app_name", ai.app_name, sizeof(ai.app_name))) continue;
        if (xp.parse_str(tag, "symstore", ai.symstore, sizeof(ai.symstore))) continue;
        if (xp.parse_str(tag, "acct_mgr_url", ai.acct_mgr_url, sizeof(ai.acct_mgr_url))) continue;
        if (xp.parse_int(tag, "hostid", ai.hostid)) continue;
        if (xp.parse_str(tag, "user_name", buf, sizeof(buf))) {
            xml_unescape(buf, ai.user_name, sizeof(ai.user_name));
            continue;
        }
        if (xp.parse_str(tag, "team_name", buf, sizeof(buf))) {
            xml_unescape(buf, ai.team_name, sizeof(ai.team_name));
            continue;
        }
        if (xp.parse_str(tag, "project_dir", ai.project_dir, sizeof(ai.project_dir))) continue;
        if (xp.parse_str(tag, "boinc_dir", ai.boinc_dir, sizeof(ai.boinc_dir))) continue;
        if (xp.parse_str(tag, "authenticator", ai.authenticator, sizeof(ai.authenticator))) continue;
        if (xp.parse_str(tag, "wu_name", ai.wu_name, sizeof(ai.wu_name))) continue;
#ifdef _WIN32
        if (xp.parse_str(tag, "comm_obj_name", ai.shmem_seg_name, sizeof(ai.shmem_seg_name))) continue;
#else
        if (xp.parse_int(tag, "shm_key", ai.shmem_seg_name)) continue;
#endif
        if (xp.parse_int(tag, "slot", ai.slot)) continue;
        if (xp.parse_double(tag, "user_total_credit", ai.user_total_credit)) continue;
        if (xp.parse_double(tag, "user_expavg_credit", ai.user_expavg_credit)) continue;
        if (xp.parse_double(tag, "host_total_credit", ai.host_total_credit)) continue;
        if (xp.parse_double(tag, "host_expavg_credit", ai.host_expavg_credit)) continue;
        if (xp.parse_double(tag, "resource_share_fraction", ai.resource_share_fraction)) continue;
        if (xp.parse_double(tag, "rsc_fpops_est", ai.rsc_fpops_est)) continue;
        if (xp.parse_double(tag, "rsc_fpops_bound", ai.rsc_fpops_bound)) continue;
        if (xp.parse_double(tag, "rsc_memory_bound", ai.rsc_memory_bound)) continue;
        if (xp.parse_double(tag, "rsc_disk_bound", ai.rsc_disk_bound)) continue;
        if (xp.parse_double(tag, "computation_deadline", ai.computation_deadline)) continue;
        if (xp.parse_double(tag, "wu_cpu_time", ai.wu_cpu_time)) continue;
        if (xp.parse_double(tag, "starting_elapsed_time", ai.starting_elapsed_time)) continue;
        if (xp.parse_double(tag, "checkpoint_period", ai.checkpoint_period)) continue;
        if (xp.parse_double(tag, "fraction_done_start", ai.fraction_done_start)) continue;
        if (xp.parse_double(tag, "fraction_done_end", ai.fraction_done_end)) continue;
        xp.skip_unexpected(tag, false, "parse_init_data_file");
    }
    fprintf(stderr, "parse_init_data_file: no end tag\n");
    return ERR_XML_PARSE;
}

APP_CLIENT_SHM::APP_CLIENT_SHM() : shm(NULL) {
}

bool MSG_CHANNEL::get_msg(char *msg) {
    if (!buf[0]) return false;
    strlcpy(msg, buf+1, MSG_CHANNEL_SIZE-1);
    buf[0] = 0;
    return true;
}

bool MSG_CHANNEL::has_msg() {
    if (buf[0]) return true;
    return false;
}

bool MSG_CHANNEL::send_msg(const char *msg) {
    if (buf[0]) return false;
    strlcpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
    return true;
}

void MSG_CHANNEL::send_msg_overwrite(const char* msg) {
    strlcpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
}

int APP_CLIENT_SHM::decode_graphics_msg(char* msg, GRAPHICS_MSG& m) {
    int i;

    parse_str(msg, "<window_station>", m.window_station, sizeof(m.window_station));
    parse_str(msg, "<desktop>", m.desktop, sizeof(m.desktop));
    parse_str(msg, "<display>", m.display, sizeof(m.display));

    m.mode = 0;
    for (i=0; i<NGRAPHICS_MSGS; i++) {
        if (match_tag(msg, xml_graphics_modes[i])) {
            m.mode = i;
        }
    }
    return 0;
}

void APP_CLIENT_SHM::reset_msgs() {
    memset(shm, 0, sizeof(SHARED_MEM));
}

// Resolve virtual name (in slot dir) to physical path (in project dir).
// Cases:
// - Windows and pre-6.12 Unix:
//   virtual name refers to a "soft link" (XML file acting as symbolic link)
// - 6.12+ Unix:
//   virtual name is a symbolic link
// - Standalone: physical path is same as virtual name
//
int boinc_resolve_filename(
    const char *virtual_name, char *physical_name, int len
) {
    FILE *fp;
    char buf[512], *p;

    if (!virtual_name) return ERR_NULL;
    strlcpy(physical_name, virtual_name, len);

#ifndef _WIN32
    if (is_symlink(virtual_name)) {
        return 0;
    }
#endif

    // Open the link file and read the first line
    //
    fp = boinc_fopen(virtual_name, "r");
    if (!fp) return 0;

    // must initialize buf since fgets() on an empty file won't do anything
    //
    buf[0] = 0;
    p =fgets(buf, sizeof(buf), fp);
    fclose(fp);

    // If it's the <soft_link> XML tag, return its value,
    // otherwise, return the original file name
    //
    if (p) parse_str(buf, "<soft_link>", physical_name, len);
    return 0;
}


// same, std::string version
//
int boinc_resolve_filename_s(const char *virtual_name, string& physical_name) {
    char buf[512], *p;
    if (!virtual_name) return ERR_NULL;
    physical_name = virtual_name;
#ifndef _WIN32
    if (is_symlink(virtual_name)) {
        return 0;
    }
#endif
    FILE *fp = boinc_fopen(virtual_name, "r");
    if (!fp) return 0;
    buf[0] = 0;
    p = fgets(buf, 512, fp);
    fclose(fp);
    if (p) parse_str(buf, "<soft_link>", physical_name);
    return 0;
}

void url_to_project_dir(char* url, char* dir) {
    char buf[256];
    escape_project_url(url, buf);
    sprintf(dir, "%s/%s", PROJECT_DIR, buf);
}

