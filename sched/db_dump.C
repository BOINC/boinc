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

// db_dump: dump database views in XML format
//
// usage: db_dump [-dir path] [-update_teams]

// files:
// NOTE: the goal is to make the full DB available (view *_id files)
// to those who want it,
// while allowing those who want to see only a particular row,
// or the highest-ranked rows, to get this info with limited transfer
//
//
// tables.xml
//    for each table (team, user, host):
//      total number of records
//      number of records per file for summary files
//      number of records per file for detailed files
// team_total_credit_N.xml
//    list of teams by decreasing total credit (summary)
// team_expavg_credit_N.xml
// team_id_N.xml
//    teams by ID (detailed, including list of user summaries)
// user_total_credit_N.xml
//    list of users by total credit (summary)
// user_expavg_credit_N.xml
// user_id_N.xml
//    users by ID (detailed, including list of host summaries)
// host_total_credit_N.xml
//    hosts by decreasing total credit (summary)
// host_expavg_credit_N.xml
// host_id_N.xml
//    hosts by ID (detailed)

// NOTE: for now we're using verbose XML tag names.
// We may change to short tag names to save bandwidth.

#include <stdio.h>
#include <unistd.h>

#include "db.h"
#include "util.h"
#include "config.h"

#define NRECS_PER_FILE_SUMMARY 1000
#define NRECS_PER_FILE_DETAIL 100

// fill in the nusers, total_credit and expavg_credit fields
// of the team table.
// This may take a while; don't do it often
//
int update_teams() {
    TEAM team;
    USER user;
    int retval;

    while (!db_team_enum(team)) {
        team.nusers = 0;
        team.total_credit = 0;
        team.expavg_credit = 0;
        user.teamid = team.id;
        while (!db_user_enum_teamid(user)) {
            team.nusers++;
            team.total_credit += user.total_credit;
            team.expavg_credit += user.expavg_credit;
        }
        retval = db_team_update(team);
        if (retval) return retval;
    }
    return 0;
}

void write_host(HOST& host, FILE* f, bool detail) {
    fprintf(f,
        "<host>\n"
        "    <id>%d</id>\n"
        "    <userid>%d</userid>\n"
        "    <total_credit>%f</total_credit>\n"
        "    <expavg_credit>%f</expavg_credit>\n"
        "    <p_vendor>%s</p_vendor>\n"
        "    <p_model>%s</p_model>\n"
        "    <os_name>%s</os_name>\n"
        "    <os_version>%s</os_version>\n",
        host.id,
        host.userid,
        host.total_credit,
        host.expavg_credit,
        host.p_vendor,
        host.p_model,
        host.os_name,
        host.os_version
    );
    if (detail) {
        fprintf(f,
            "  <create_time>%d</create_time>\n"
            "  <timezone>%d</timezone>\n"
            "  <ncpus>%d</ncpus>\n"
            "  <p_fpops>%f</p_fpops>\n"
            "  <p_iops>%f</p_iops>\n"
            "  <p_membw>%f</p_membw>\n"
            "  <m_nbytes>%f</m_nbytes>\n"
            "  <m_cache>%f</m_cache>\n"
            "  <m_swap>%f</m_swap>\n"
            "  <d_total>%f</d_total>\n"
            "  <d_free>%f</d_free>\n"
            "  <n_bwup>%f</n_bwup>\n"
            "  <n_bwdown>%f</n_bwdown>\n",
            host.create_time,
            host.timezone,
            host.p_ncpus,
            host.p_fpops,
            host.p_iops,
            host.p_membw,
            host.m_nbytes,
            host.m_cache,
            host.m_swap,
            host.d_total,
            host.d_free,
            host.n_bwup,
            host.n_bwdown
        );
    }
    fprintf(f,
        "</host>\n"
    );
}

void write_user(USER& user, FILE* f, bool detail) {
    HOST host;
    fprintf(f,
        "<user>\n"
        " <id>%d</id>\n"
        " <name>%s</name>\n"
        " <total_credit>%f</total_credit>\n"
        " <expavg_credit>%f</expavg_credit>\n"
        " <teamid>%d</teamid>\n",
        user.id,
        user.name,
        user.total_credit,
        user.expavg_credit,
        user.teamid
    );
    if (detail) {
        host.userid = user.id;
        while (!db_host_enum_userid(host)) {
            write_host(host, f, false);
        }
    }
    fprintf(f,
        "</user>\n"
    );
}

void write_team(TEAM& team, FILE* f, bool detail) {
    USER user;
    char buf[MAX_BLOB_SIZE*2];

    fprintf(f,
        "<team>\n"
        " <id>%d</id>\n"
        " <name>%s</name>\n"
        " <total_credit>%f</total_credit>\n"
        " <expavg_credit>%f</expavg_credit>\n"
        " <nusers>%d</nusers>\n",
        team.id,
        team.name,
        team.total_credit,
        team.expavg_credit,
        team.nusers
    );
    if (detail) {
        fprintf(f,
            " <create_time>%d</create_time>\n",
            team.create_time
        );
        if (strlen(team.url)) {
            fprintf(f,
                " <url>%s</url>\n",
                team.url
            );
        }
        if (strlen(team.name_html)) {
            escape_url(team.name_html, buf);
            fprintf(f,
                "<name_html>%s</name_html>\n",
                buf
            );
        }
        if (strlen(team.description)) {
            escape_url(team.description, buf);
            fprintf(f,
                "<description>%s</description>\n",
                buf
            );
        }
        fprintf(f,
            " <country>%s</country>\n",
            team.country
        );
        user.teamid = team.id;
        while (!db_user_enum_teamid(user)) {
            write_user(user, f, false);
        }
    }
    fprintf(f,
        "</team>\n"
    );
}

void team_total_credit() {
    TEAM team;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_team_enum_total_credit(team)) {
        if (!f) {
            sprintf(buf, "team_total_credit_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_team(team, f, false);
        nrec++;
        if (nrec == NRECS_PER_FILE_SUMMARY) {
            fclose(f);
            f = 0;
        }
    }
}

void team_expavg_credit() {
    TEAM team;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_team_enum_expavg_credit(team)) {
        if (!f) {
            sprintf(buf, "team_expavg_credit_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_team(team, f, false);
        nrec++;
        if (nrec == NRECS_PER_FILE_SUMMARY) {
            fclose(f);
            f = 0;
        }
    }
}

void team_id() {
    TEAM team;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_team_enum_id(team)) {
        if (!f) {
            sprintf(buf, "team_id_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_team(team, f, true);
        nrec++;
        if (nrec == NRECS_PER_FILE_DETAIL) {
            fclose(f);
            f = 0;
        }
    }
}

void user_total_credit() {
    USER user;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_user_enum_total_credit(user)) {
        if (!f) {
            sprintf(buf, "user_total_credit_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_user(user, f, false);
        nrec++;
        if (nrec == NRECS_PER_FILE_SUMMARY) {
            fclose(f);
            f = 0;
        }
    }
}

void user_expavg_credit() {
    USER user;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_user_enum_expavg_credit(user)) {
        if (!f) {
            sprintf(buf, "user_expavg_credit_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_user(user, f, false);
        nrec++;
        if (nrec == NRECS_PER_FILE_SUMMARY) {
            fclose(f);
            f = 0;
        }
    }
}

void user_id() {
    USER user;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_user_enum_id(user)) {
        if (!f) {
            sprintf(buf, "user_id_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_user(user, f, true);
        nrec++;
        if (nrec == NRECS_PER_FILE_DETAIL) {
            fclose(f);
            f = 0;
        }
    }
}

void host_total_credit() {
    HOST host;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_host_enum_total_credit(host)) {
        if (!f) {
            sprintf(buf, "host_total_credit_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_host(host, f, false);
        nrec++;
        if (nrec == NRECS_PER_FILE_SUMMARY) {
            fclose(f);
            f = 0;
        }
    }
}
void host_expavg_credit() {
    HOST host;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_host_enum_expavg_credit(host)) {
        if (!f) {
            sprintf(buf, "host_expavg_credit_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_host(host, f, false);
        nrec++;
        if (nrec == NRECS_PER_FILE_SUMMARY) {
            fclose(f);
            f = 0;
        }
    }
}

void host_id() {
    HOST host;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_host_enum_id(host)) {
        if (!f) {
            sprintf(buf, "host_id_%d", nfile);
            f = fopen(buf, "w");
            nfile++;
            nrec = 0;
        }
        write_host(host, f, true);
        nrec++;
        if (nrec == NRECS_PER_FILE_DETAIL) {
            fclose(f);
            f = 0;
        }
    }
}

int tables_file() {
    int nusers, nteams, nhosts;
    int retval;
    FILE* f;

    f = fopen("tables.xml", "w");
    if (!f) return -1;
    retval = db_user_count(nusers);
    if (retval) return retval;
    retval = db_team_count(nteams);
    if (retval) return retval;
    retval = db_host_count(nhosts);
    if (retval) return retval;
    fprintf(f,
        "<tables>\n"
        "    <nusers_total>%d</nusers_total>\n"
        "    <nusers_per_file_summary>%d</nusers_per_file_summary>\n"
        "    <nusers_per_file_detail>%d</nusers_per_file_detail>\n"
        "    <nteams_total>%d</nteams_total>\n"
        "    <nteams_per_file_summary>%d</nteams_per_file_summary>\n"
        "    <nteams_per_file_detail>%d</nteams_per_file_detail>\n"
        "    <nhosts_total>%d</nhosts_total>\n"
        "    <nhosts_per_file_summary>%d</nhosts_per_file_summary>\n"
        "    <nhosts_per_file_detail>%d</nhosts_per_file_detail>\n"
        "</tables>\n",
        nusers,
        NRECS_PER_FILE_SUMMARY,
        NRECS_PER_FILE_DETAIL,
        nteams,
        NRECS_PER_FILE_SUMMARY,
        NRECS_PER_FILE_DETAIL,
        nhosts,
        NRECS_PER_FILE_SUMMARY,
        NRECS_PER_FILE_DETAIL
    );
    fclose(f);
    return 0;
}

int main(int argc, char** argv) {
    CONFIG config;
    int retval, i;
    bool do_update_teams = false;
    char dir[256];

    strcpy(dir, "");
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-update_teams")) {
            do_update_teams = true;
        } else if (!strcmp(argv[i], "-dir")) {
            strcpy(dir, argv[++i]);
        }
    }
    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config file\n");
        exit(1);
    }
    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr, "Can't open DB\n");
        exit(1);
    }

    if (strlen(dir)) {
        retval = chdir(dir);
        if (retval) {
            fprintf(stderr, "can't chdir to %s\n", dir);
            exit(1);
        }
    }
    if (do_update_teams) {
        retval = update_teams();
        if (retval) {
            fprintf(stderr, "update_teams failed: %d\n", retval);
            exit(1);
        }
    }
    tables_file();
    team_total_credit();
    team_expavg_credit();
    team_id();
    user_total_credit();
    user_expavg_credit();
    user_id();
    host_total_credit();
    host_expavg_credit();
    host_id();
}
