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

// db_dump: dump database views in XML format
//
// usage: db_dump [-dir path] [-summary_recs num_recs_per_file]
//                [-detail_recs num_recs_per_file] [-gzip] [-zip]

// files:
// NOTE: the goal is to make the full DB available (view *_id files)
// to those who want it,
// while allowing those who want to see only a particular row,
// or the highest-ranked rows, to get this info with a small download
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "boinc_db.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "parse.h"

#define LOCKFILE "db_dump.out"

#define DEFAULT_NRECS_PER_FILE_SUMMARY 1000
#define DEFAULT_NRECS_PER_FILE_DETAIL 100

int nrecs_per_file_summary;
int nrecs_per_file_detail;

bool zip_files = false;
string zip_cmd;

// class that automatically compresses on close
class ZFILE {
protected:
    FILE* f;
    string filename;
    bool zip_file;
public:
    ZFILE(bool zip_file_ = zip_files) : f(0), zip_file(zip_file_) {}
    ~ZFILE() { close(); }

    operator FILE* () { return f; }
    bool operator ! () { return !f; }

    void open(const char* filename_format, ...)
    {
        close();
        va_list ap;
        char filename_buf[256];
        va_start(ap, filename_format);
        vsprintf(filename_buf, filename_format, ap);
        va_end(ap);

        filename = filename_buf;

        f = fopen(filename.c_str(), "w");
        if (!f) {
            fprintf(stderr, "db_dump: Couldn't open %s for output\n", filename.c_str());
        }
    }
    void close()
    {
        if (f) {
            fclose(f);
            if (zip_file) {
                string cmd = zip_cmd + ' ' + filename;
                system(cmd.c_str());
            }
            f = 0;
        }
    }
};

// class that automatically opens a new file every N records
class NUMBERED_ZFILE : public ZFILE {
    const char* filename_format;
    int nrec_max;
    int nfile, nrec;
public:
    NUMBERED_ZFILE(const char* filename_format_, int nrec_max_)
        : filename_format(filename_format_), nrec_max(nrec_max_), nfile(0), nrec(0)
    {
    }

    NUMBERED_ZFILE& operator ++() {
        if (!f || nrec >= nrec_max) {
            open(filename_format, nfile);
            ++nfile;
            nrec = 0;
        }
        ++nrec;
        return *this;
    }
};

void write_host(HOST& host, FILE* f, bool detail, bool show_user) {
    fprintf(f,
        "<host>\n"
        "    <id>%d</id>\n",
        host.id
    );
    if (show_user) {
        DB_USER user;
        user.lookup_id(host.userid);
        if (user.show_hosts) {
            fprintf(f,
                "    <userid>%d</userid>\n",
                host.userid
            );
        }
    }
    fprintf(f,
        "    <total_credit>%f</total_credit>\n"
        "    <expavg_credit>%f</expavg_credit>\n"
        "    <p_vendor>%s</p_vendor>\n"
        "    <p_model>%s</p_model>\n"
        "    <os_name>%s</os_name>\n"
        "    <os_version>%s</os_version>\n",
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

void write_user(USER& user, FILE* f, bool detail, bool show_team) {
    DB_HOST host;
    char buf[256];

    fprintf(f,
        "<user>\n"
        " <id>%d</id>\n"
        " <name>%s</name>\n"
        " <url>%s</url>\n"
        " <country>%s</country>\n"
        " <create_time>%d</create_time>\n"
        " <total_credit>%f</total_credit>\n"
        " <expavg_credit>%f</expavg_credit>\n",
        user.id,
        user.name,
        user.url,
        user.country,
        user.create_time,
        user.total_credit,
        user.expavg_credit
    );
    if (show_team) {
        fprintf(f,
            " <teamid>%d</teamid>\n",
            user.teamid
        );
    }
    if (detail && user.show_hosts) {
        sprintf(buf, "where userid=%d", user.id);
        while (!host.enumerate(buf)) {
            write_host(host, f, false, false);
        }
    }
    fprintf(f,
        "</user>\n"
    );
}

void write_team(TEAM& team, FILE* f, bool detail) {
    DB_USER user;
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
        sprintf(buf, "where teamid=%d", team.id);
        while (!user.enumerate(buf)) {
            write_user(user, f, false, false);
        }
    }
    fprintf(f,
        "</team>\n"
    );
}

void team_total_credit() {
    DB_TEAM team;
    NUMBERED_ZFILE f("team_total_credit_%d", nrecs_per_file_summary);

    while (!team.enumerate("order by total_credit desc")) {
        write_team(team, ++f, false);
    }
}

void team_expavg_credit() {
    DB_TEAM team;
    NUMBERED_ZFILE f("team_expavg_credit_%d", nrecs_per_file_summary);

    while (!team.enumerate("order by expavg_credit desc")) {
        write_team(team, ++f, false);
    }
}

void team_id() {
    DB_TEAM team;
    NUMBERED_ZFILE f("team_id_%d", nrecs_per_file_detail);

    while (!team.enumerate("order by id")) {
        write_team(team, ++f, true);
    }
}

void user_total_credit() {
    DB_USER user;
    NUMBERED_ZFILE f("user_total_credit_%d", nrecs_per_file_summary);

    while (!user.enumerate("order by total_credit desc")) {
        write_user(user, ++f, false, true);
    }
}

void user_expavg_credit() {
    DB_USER user;
    NUMBERED_ZFILE f("user_expavg_credit_%d", nrecs_per_file_summary);

    while (!user.enumerate("order by expavg_credit desc")) {
        write_user(user, ++f, false, true);
    }
}

void user_id() {
    DB_USER user;
    NUMBERED_ZFILE f("user_id_%d", nrecs_per_file_detail);

    while (!user.enumerate("order by id")) {
        write_user(user, ++f, true, true);
    }
}

void host_total_credit() {
    DB_HOST host;
    NUMBERED_ZFILE f("host_total_credit_%d", nrecs_per_file_summary);

    while (!host.enumerate("order by total_credit desc")) {
        write_host(host, ++f, false, true);
    }
}

void host_expavg_credit() {
    DB_HOST host;
    NUMBERED_ZFILE f("host_expavg_credit_%d", nrecs_per_file_summary);

    while (!host.enumerate("order by expavg_credit desc")) {
        write_host(host, ++f, false, true);
    }
}

void host_id() {
    DB_HOST host;
    NUMBERED_ZFILE f("host_id_%d", nrecs_per_file_detail);

    while (!host.enumerate("order by id")) {
        write_host(host, ++f, true, true);
    }
}

void core_versions() {
    ZFILE f(false);
    f.open("core_versions.xml");
    if (!f) return;
    fprintf(f, "<core_versions>\n");

    DB_PLATFORM platform;
    while (!platform.enumerate("order by name")) {
        DB_CORE_VERSION core_version;
        char query_buf[256];
        sprintf(query_buf, "where platformid=%d order by version_num desc", platform.id);
        if (!core_version.enumerate(query_buf)) {
            char url[256] = "";
            parse_str(core_version.xml_doc, "<url>", url, sizeof(url));

            fprintf(f,
                    "   <core_version>\n"
                    "      <id>%d</id>\n"
                    "      <platform id=\"%d\" name=\"%s\">%s</platform>\n"
                    "      <version>%d</version>\n"
                    "      <create_time>%d</create_time>\n"
                    "      <url>%s</url>\n"
                    "   </core_version>\n",
                    core_version.id,
                    platform.id, platform.name, platform.user_friendly_name,
                    core_version.version_num,
                    core_version.create_time,
                    url);
        }
    }
    fprintf(f, "</core_versions>\n");
}

int tables_file() {
    int nusers, nteams, nhosts;
    int retval;
    DB_USER user;
    DB_TEAM team;
    DB_HOST host;
    ZFILE f(false);
    f.open("tables.xml");
    if (!f) return -1;
    retval = user.count(nusers);
    if (retval) return retval;
    retval = team.count(nteams);
    if (retval) return retval;
    retval = host.count(nhosts);
    if (retval) return retval;
    fprintf(f,
        "<tables>\n"
        "    <update_time>%d</update_time>\n"
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
        (int)time(0),
        nusers,
        nrecs_per_file_summary,
        nrecs_per_file_detail,
        nteams,
        nrecs_per_file_summary,
        nrecs_per_file_detail,
        nhosts,
        nrecs_per_file_summary,
        nrecs_per_file_detail
    );
    return 0;
}

int main(int argc, char** argv) {
    SCHED_CONFIG config;
    int retval, i;
    char dir[256];

    check_stop_trigger();

    nrecs_per_file_summary = DEFAULT_NRECS_PER_FILE_SUMMARY;
    nrecs_per_file_detail = DEFAULT_NRECS_PER_FILE_DETAIL;
    strcpy(dir, "");
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-dir")) {
            strcpy(dir, argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-gzip")) {
            zip_files = true;
            zip_cmd = "gzip -fq";
        } else if (!strcmp(argv[i], "-zip")) {
            zip_files = true;
            zip_cmd = "zip -q";
        } else if (!strcmp(argv[i], "-summary_recs")) {
            nrecs_per_file_summary = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-detail_recs")) {
            nrecs_per_file_detail = atoi(argv[++i]);
        }
    }

    if (nrecs_per_file_summary <= 0 || nrecs_per_file_detail <= 0) {
        log_messages.printf(SchedMessages::NORMAL, "Too few records per file.\n");
        exit(1);
    }

    if (lock_file(LOCKFILE)) {
        log_messages.printf(SchedMessages::NORMAL, "Another copy of db_dump is already running\n");
        exit(1);
    }
    log_messages.printf(SchedMessages::NORMAL, "Starting\n");

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SchedMessages::NORMAL, "Can't parse config file\n");
        exit(1);
    }
    retval = boinc_db.open(config.db_name, config.db_passwd);
    if (retval) {
        log_messages.printf(SchedMessages::NORMAL, "Can't open DB\n");
        exit(1);
    }

    if (strlen(dir)) {
        retval = chdir(dir);
        if (retval) {
            log_messages.printf(SchedMessages::NORMAL, "can't chdir to %s\n", dir);
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
    core_versions();
}
