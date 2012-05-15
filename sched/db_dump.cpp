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

/// db_dump: dump database views in XML format
// see http://boinc.berkeley.edu/trac/wiki/DbDump

// Note: this program is way more configurable than it needs to be.
// All projects export stats in the same format,
// as described in the default db_dump_spec.xml that is created for you.

#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>

#include "boinc_db.h"
#include "filesys.h"
#include "util.h"
#include "str_util.h"
#include "str_replace.h"
#include "error_numbers.h"
#include "md5_file.h"
#include "parse.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

using std::string;
using std::vector;

#define LOCKFILE "db_dump.out"

#define COMPRESSION_NONE    0
#define COMPRESSION_GZIP    1
#define COMPRESSION_ZIP     2

#define SORT_NONE           0
#define SORT_ID             1
#define SORT_TOTAL_CREDIT   2
#define SORT_EXPAVG_CREDIT  3

#define TABLE_USER  0
#define TABLE_TEAM  1
#define TABLE_HOST  2

// must match the above
const char* table_name[3] = {"user", "team", "host"};
const char* tag_name[3] = {"users", "teams", "hosts"};

int nusers, nhosts, nteams;
double total_credit;

struct OUTPUT {
    int recs_per_file;
    bool detail;
    int compression;
    class ZFILE* zfile;
    class NUMBERED_ZFILE* nzfile;
    int parse(FILE*);
};

struct ENUMERATION {
    int table;
    int sort;
    char filename[256];
    vector<OUTPUT> outputs;
    int parse(FILE*);
    int make_it_happen(char*);
};

struct DUMP_SPEC {
    char output_dir[256];
    char final_output_dir[256];
    char archive_dir[256];
    vector<ENUMERATION> enumerations;
    int parse(FILE*);
};

int OUTPUT::parse(FILE* in) {
    char buf[256], buf2[256];

    recs_per_file = 0;
    detail = false;
    compression = COMPRESSION_NONE;
    zfile = 0;
    nzfile = 0;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</output>")) return 0;
        if (parse_int(buf, "<recs_per_file>", recs_per_file)) continue;
        if (match_tag(buf, "<detail/>")) {
            detail = true;
            continue;
        }
        if (parse_str(buf, "<compression>", buf2, sizeof(buf2))) {
            if (!strcmp(buf2, "gzip")) {
                compression = COMPRESSION_GZIP;
            } else if (!strcmp(buf2, "zip")) {
                compression = COMPRESSION_ZIP;
            } else {
                log_messages.printf(MSG_CRITICAL,
                    "unrecognized compression type: %s", buf
                );
            }
            continue;
        }
        log_messages.printf(MSG_CRITICAL,
            "OUTPUT::parse: unrecognized: %s", buf
        );
    }
    return ERR_XML_PARSE;
}

int ENUMERATION::parse(FILE* in) {
    char buf[256], buf2[256];
    int retval, i;

    table = -1;
    sort = SORT_NONE;
    strcpy(filename, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</enumeration>")) {
            if (table == -1) return ERR_XML_PARSE;
            if (sort == -1) return ERR_XML_PARSE;
            if (!strlen(filename)) return ERR_XML_PARSE;
            return 0;
        }
        if (match_tag(buf, "<output>")) {
            OUTPUT output;
            retval = output.parse(in);
            if (!retval) outputs.push_back(output);
        }
        if (parse_str(buf, "<filename>", filename, sizeof(filename))) {
            continue;
        }
        if (parse_str(buf, "<table>", buf2, sizeof(buf2))) {
            for (i=0; i<3; i++) {
                if (!strcmp(buf2, table_name[i])) {
                    table = i;
                    break;
                }
            }
        }
        if (parse_str(buf, "<sort>", buf2, sizeof(buf2))) {
            if (!strcmp(buf2, "id")) sort = SORT_ID;
            if (!strcmp(buf2, "total_credit")) sort = SORT_TOTAL_CREDIT;
            if (!strcmp(buf2, "expavg_credit")) sort = SORT_EXPAVG_CREDIT;
        }
    }
    return ERR_XML_PARSE;
}

int DUMP_SPEC::parse(FILE* in) {
    char buf[256];
    int retval;

    strcpy(output_dir, "");
    strcpy(final_output_dir, "");
    strcpy(archive_dir, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</boinc_db_dump_spec>")) {
            if (!strlen(output_dir)) return ERR_XML_PARSE;
            if (!strlen(final_output_dir)) return ERR_XML_PARSE;
            return 0;
        }
        if (match_tag(buf, "<enumeration>")) {
            ENUMERATION e;
            retval = e.parse(in);
            if (!retval) enumerations.push_back(e);
        }
        if (parse_str(buf, "<output_dir", output_dir, sizeof(output_dir))) {
            continue;
        }
        if (parse_str(buf, "<final_output_dir", final_output_dir, sizeof(final_output_dir))) {
            continue;
        }
        if (parse_str(buf, "<archive_dir", archive_dir, sizeof(archive_dir))) {
            continue;
        }
    }
    return ERR_XML_PARSE;
}

// class that automatically compresses on close
//
class ZFILE {
protected:
    string tag;     // enclosing XML tag
    char current_path[MAXPATHLEN];
    int compression;
public:
    FILE* f;
    ZFILE(string tag_, int comp): tag(tag_), compression(comp), f(0) {}
    ~ZFILE() { close(); }

    void open(const char* filename) {
        close();

        f = fopen(filename, "w");
        if (!f) {
            log_messages.printf(MSG_CRITICAL,
                "Couldn't open %s for output\n", filename
            );
        }
        fprintf(f,
            "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<%s>\n", tag.c_str()
        );
        safe_strcpy(current_path, filename);
    }

    void open_num(const char* filename, int filenum) {
        char buf[256];
        sprintf(buf, "%s_%d", filename, filenum);
        open(buf);
    }

    void close() {
        char buf[256];
        int retval;
        if (f) {
            fprintf(f, "</%s>\n", tag.c_str());
            fclose(f);
            switch(compression) {
            case COMPRESSION_ZIP:
                sprintf(buf, "zip -q %s", current_path);
                retval = system(buf);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "%s failed: %s\n", buf, boincerror(retval)
                    );
                    exit(retval);
                }
                break;
            case COMPRESSION_GZIP:
                sprintf(buf, "gzip -fq %s", current_path);
                retval = system(buf);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "%s failed: %s\n", buf, boincerror(retval)
                    );
                    exit(retval);
                }
                break;
            }
            f = 0;
        }
    }
};

// class that automatically opens a new file every N IDs
//
class NUMBERED_ZFILE : public ZFILE {
    const char* filename_base;
    int nids_per_file;
    int last_filenum;
public:
    NUMBERED_ZFILE(string tag_, int comp, const char* fb, int nids_per_file_)
        :   ZFILE(tag_, comp),
            filename_base(fb),
            nids_per_file(nids_per_file_),
            last_filenum(-1)
    {}

    void set_id(int);

};

void NUMBERED_ZFILE::set_id(int id) {
    int filenum = id/nids_per_file;
    if (!f || (filenum != last_filenum)) {
        open_num(filename_base, filenum);
        last_filenum = filenum;
    }
}

void write_host(HOST& host, FILE* f, bool detail) {
    int retval;
    char p_vendor[2048], p_model[2048], os_name[2048], os_version[2048];

    xml_escape(host.p_vendor, p_vendor, sizeof(p_vendor));
    xml_escape(host.p_model, p_model, sizeof(p_model));
    xml_escape(host.os_name, os_name, sizeof(os_name));
    xml_escape(host.os_version, os_version, sizeof(os_version));
    fprintf(f,
        "<host>\n"
        "    <id>%d</id>\n",
        host.id
    );
    if (detail) {
        DB_USER user;
        retval = user.lookup_id(host.userid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "user lookup of user %d for host %d: %s\n",
                host.userid, host.id, boincerror(retval)
            );
        } else {
            if (user.show_hosts) {
                fprintf(f,
                    "    <userid>%d</userid>\n",
                    host.userid
                );
            }
        }
    }
    fprintf(f,
        "    <total_credit>%f</total_credit>\n"
        "    <expavg_credit>%f</expavg_credit>\n"
        "    <expavg_time>%f</expavg_time>\n"
        "    <p_vendor>%s</p_vendor>\n"
        "    <p_model>%s</p_model>\n"
        "    <os_name>%s</os_name>\n"
        "    <os_version>%s</os_version>\n",
        host.total_credit,
        host.expavg_credit,
        host.expavg_time,
        p_vendor,
        p_model,
        os_name,
        os_version
    );

    // host.serialnum stores coprocessor description
    //
    if (strlen(host.serialnum)) {
        char serialnum[1024];
        xml_escape(host.serialnum, serialnum, sizeof(serialnum));
        fprintf(f,
            "    <coprocs>%s</coprocs>\n", serialnum
        );
    }
    if (detail) {
        fprintf(f,
            "  <create_time>%d</create_time>\n"
            "  <rpc_time>%d</rpc_time>\n"
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
            "  <n_bwdown>%f</n_bwdown>\n"
            "  <avg_turnaround>%f</avg_turnaround>\n"
            "  <credit_per_cpu_sec>%f</credit_per_cpu_sec>\n"
            "  <host_cpid>%s</host_cpid>\n",
            host.create_time,
            host.rpc_time,
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
            host.n_bwdown,
            host.avg_turnaround,
            host.credit_per_cpu_sec,
            host.host_cpid
        );
    }
    fprintf(f,
        "</host>\n"
    );
}

void write_user(USER& user, FILE* f, bool /*detail*/) {
    char buf[1024];
    char cpid[MD5_LEN];

    char name[2048], url[2048];
    xml_escape(user.name, name, sizeof(name));
    xml_escape(user.url, url, sizeof(url));

    safe_strcpy(buf, user.cross_project_id);
    safe_strcat(buf, user.email_addr);
    md5_block((unsigned char*)buf, strlen(buf), cpid);

    fprintf(f,
        "<user>\n"
        " <id>%d</id>\n"
        " <name>%s</name>\n"
        " <country>%s</country>\n"
        " <create_time>%d</create_time>\n"
        " <total_credit>%f</total_credit>\n"
        " <expavg_credit>%f</expavg_credit>\n"
        " <expavg_time>%f</expavg_time>\n"
        " <cpid>%s</cpid>\n",
        user.id,
        name,
        user.country,
        user.create_time,
        user.total_credit,
        user.expavg_credit,
        user.expavg_time,
        cpid
    );
    if (strlen(user.url)) {
        fprintf(f,
            " <url>%s</url>\n",
            url
        );
    }
    if (user.teamid) {
        fprintf(f,
            " <teamid>%d</teamid>\n",
            user.teamid
        );
    }
    if (user.has_profile) {
        fprintf(f,
            " <has_profile/>\n"
        );
    }
#if 0
    if (detail && user.show_hosts) {
        DB_HOST host;
        sprintf(buf, "where userid=%d", user.id);
        while (1) {
            retval = host.enumerate(buf)
            if (retval) break;
            if (host.total_credit > 0) {
                write_host(host, f, false);
            }
        }
        if (retval != ERR_DB_NOT_FOUND) {
            boinc_db.print_error("host enum: %s", boincerror(retval));
            exit(retval);
        }
    }
#endif
    fprintf(f,
        "</user>\n"
    );
}

void write_team(TEAM& team, FILE* f, bool detail) {
    DB_USER user;
    char buf[256];
    char name[2048];
    char url[2048], name_html[2048];
    int retval;
    char description[BLOB_SIZE];

    xml_escape(team.name, name, sizeof(name));

    fprintf(f,
        "<team>\n"
        " <id>%d</id>\n"
        " <type>%d</type>\n"
        " <name>%s</name>\n"
        " <userid>%d</userid>\n"
        " <total_credit>%f</total_credit>\n"
        " <expavg_credit>%f</expavg_credit>\n"
        " <expavg_time>%f</expavg_time>\n",
        team.id,
        team.type,
        name,
        team.userid,
        team.total_credit,
        team.expavg_credit,
        team.expavg_time
    );

    // show founder name since that user might not be active
    //
    retval = user.lookup_id(team.userid);
    if (!retval) {
        char fname[2048];
        xml_escape(user.name, fname, sizeof(fname));
        fprintf(f,
            "  <founder_name>%s</founder_name>\n",
            fname
        );
    }

    fprintf(f,
        " <create_time>%d</create_time>\n",
        team.create_time
    );
    if (strlen(team.url)) {
        xml_escape(team.url, url, sizeof(url));
        fprintf(f,
            " <url>%s</url>\n",
            url
        );
    }
    if (strlen(team.name_html)) {
        xml_escape(team.name_html, name_html, sizeof(name_html));
        fprintf(f,
            "<name_html>%s</name_html>\n",
            name_html
        );
    }

    if (strlen(team.description)) {
        xml_escape(team.description, description, sizeof(description));
        fprintf(f,
            "<description>%s</description>\n",
            description
        );
    }

    fprintf(f,
        " <country>%s</country>\n",
        team.country
    );
    if (detail) {
        sprintf(buf, "where teamid=%d", team.id);
        while (1) {
            retval = user.enumerate(buf);
            if (retval) break;
            write_user(user, f, false);
        }
        if (retval != ERR_DB_NOT_FOUND) {
            log_messages.printf(MSG_CRITICAL,
                "user enum: %s", boincerror(retval)
            );
            exit(retval);
        }
    }
    fprintf(f,
        "</team>\n"
    );
}

int print_app(FILE* f, APP& app) {
    fprintf(f, "        <application>\n");
    fprintf(f, "            <name>%s</name>\n", app.user_friendly_name);

#if 0
    DB_RESULT result;
    char buf[256];
    int n, retval;
    // can't do this stuff because MySQL/InnoDB can't do counts efficiently
    //
    sprintf(buf, "where appid=%d and server_state=%d", app.id, RESULT_SERVER_STATE_UNSENT);
    retval = result.count(n, buf);
    if (!retval) {
        fprintf(f, "            <results_unsent>%d</results_unsent>\n", n);
    }

    sprintf(buf, "where appid=%d and server_state=%d", app.id, RESULT_SERVER_STATE_IN_PROGRESS);
    retval = result.count(n, buf);
    if (!retval) {
        fprintf(f, "            <results_in_progress>%d</results_in_progress>\n", n);
    }

    sprintf(buf, "where appid=%d and server_state=%d", app.id, RESULT_SERVER_STATE_OVER);
    retval = result.count(n, buf);
    if (!retval) {
        fprintf(f, "            <results_over>%d</results_over>\n", n);
    }
#endif

    fprintf(f, "        </application>\n");
    return 0;
}

int print_apps(FILE* f) {
    DB_APP app;
    fprintf(f, "    <applications>\n");
    while (!app.enumerate()) {
        print_app(f, app);
    }
    fprintf(f, "    </applications>\n");
    return 0;
}

int tables_file(char* dir) {
    char buf[256];

    ZFILE f("tables", false);
    sprintf(buf, "%s/tables.xml", dir);
    f.open(buf);
    fprintf(f.f,
        "    <update_time>%d</update_time>\n",
        (int)time(0)
    );
    if (nusers) fprintf(f.f, "    <nusers_total>%d</nusers_total>\n", nusers);
    if (nteams) fprintf(f.f, "    <nteams_total>%d</nteams_total>\n", nteams);
    if (nhosts) fprintf(f.f, "    <nhosts_total>%d</nhosts_total>\n", nhosts);
    if (total_credit) fprintf(f.f, "    <total_credit>%lf</total_credit>\n", total_credit);
    print_apps(f.f);
    f.close();
    return 0;
}

int ENUMERATION::make_it_happen(char* output_dir) {
    unsigned int i;
    int n, retval;
    DB_USER user;
    DB_TEAM team;
    DB_HOST host;
    char clause[256];
    char path[MAXPATHLEN];

    sprintf(path, "%s/%s", output_dir, filename);

    for (i=0; i<outputs.size(); i++) {
        OUTPUT& out = outputs[i];
        if (out.recs_per_file) {
            out.nzfile = new NUMBERED_ZFILE(
                tag_name[table], out.compression, path, out.recs_per_file
            );
        } else {
            out.zfile = new ZFILE(tag_name[table], out.compression);
            out.zfile->open(path);
        }
    }
    switch(sort) {
    case SORT_NONE:
        strcpy(clause, "where total_credit > 0");
        break;
    case SORT_ID:
        strcpy(clause, "where total_credit > 0 order by id");
        break;
    case SORT_TOTAL_CREDIT:
        strcpy(clause, "where total_credit > 0 order by total_credit desc");
        break;
    case SORT_EXPAVG_CREDIT:
        strcpy(clause, "where total_credit > 0 order by expavg_credit desc");
        break;
    }
    switch(table) {
    case TABLE_USER:
        n = 0;
        while (1) {
            retval = user.enumerate(clause, true);
            if (retval) break;
            nusers++;
            total_credit += user.total_credit;
            for (i=0; i<outputs.size(); i++) {
                OUTPUT& out = outputs[i];
                if (sort == SORT_ID && out.recs_per_file) {
                    out.nzfile->set_id(n++);
                }
                if (out.zfile) {
                    write_user(user, out.zfile->f, out.detail);
                } else {
                    write_user(user, out.nzfile->f, out.detail);
                }
            }
        }
        if (retval != ERR_DB_NOT_FOUND) {
            log_messages.printf(MSG_CRITICAL,
                "user enum: %s", boincerror(retval)
            );
            exit(retval);
        }
        break;
    case TABLE_HOST:
        n = 0;
        while(1) {
            retval = host.enumerate(clause);
            if (retval) break;
            if (!host.userid) continue;
            nhosts++;
            for (i=0; i<outputs.size(); i++) {
                OUTPUT& out = outputs[i];
                if (sort == SORT_ID && out.recs_per_file) {
                    out.nzfile->set_id(n++);
                }
                if (out.zfile) {
                    write_host(host, out.zfile->f, out.detail);
                } else {
                    write_host(host, out.nzfile->f, out.detail);
                }
            }
        }
        if (retval != ERR_DB_NOT_FOUND) {
            log_messages.printf(MSG_CRITICAL,
                "host enum: %s", boincerror(retval)
            );
            exit(retval);
        }
        break;
    case TABLE_TEAM:
        n = 0;
        while(1) {
            retval = team.enumerate(clause);
            if (retval) break;
            nteams++;
            for (i=0; i<outputs.size(); i++) {
                OUTPUT& out = outputs[i];
                if (sort == SORT_ID && out.recs_per_file) {
                    out.nzfile->set_id(n++);
                }
                if (out.zfile) {
                    write_team(team, out.zfile->f, out.detail);
                } else {
                    write_team(team, out.nzfile->f, out.detail);
                }
            }
        }
        if (retval != ERR_DB_NOT_FOUND) {
            log_messages.printf(MSG_CRITICAL,
                "team enum: %s", boincerror(retval)
            );
            exit(retval);
        }
        break;
    }
    for (i=0; i<outputs.size(); i++) {
        OUTPUT& out = outputs[i];
        if (out.zfile) out.zfile->close();
        if (out.nzfile) out.nzfile->close();
    }
    return 0;
}

void usage(char* name) {
    fprintf(stderr,
        "This program generates XML files containing project statistics.\n"
        "It should be run once a day as a periodic task in config.xml.\n"
        "For more info, see http://boinc.berkeley.edu/trac/wiki/DbDump\n\n"
        "Usage: %s [options]\n"
        "Options:\n"
        "    --dump_spec filename          Use the given config file (use ../db_dump_spec.xml)\n"
        "    [-d N | --debug_level]        Set verbosity level (1 to 4)\n"
        "    [--db_host H]                 Use the DB server on host H\n"
        "    [-h | --help]                 Show this\n"
        "    [-v | --version]              Show version information\n",
        name
    );
}

int main(int argc, char** argv) {
    int retval, i;
    DUMP_SPEC spec;
    char* db_host = 0;
    char spec_filename[256], buf[256];
    FILE_LOCK file_lock;

    check_stop_daemons();
    setbuf(stderr, 0);

    log_messages.printf(MSG_NORMAL, "db_dump starting\n");
    strcpy(spec_filename, "");
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "dump_spec")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            safe_strcpy(spec_filename, argv[i]);
        } else if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "db_host")) {
            if(!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            db_host = argv[i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "unknown command line argument: %s\n\n", argv[i]
            );
            usage(argv[0]);
            exit(1);
        }
    }

    if (!strlen(spec_filename)) {
        log_messages.printf(MSG_CRITICAL, "no spec file given\n");
        usage(argv[0]);
        exit(1);
    }

    FILE* f = fopen(spec_filename, "r");
    if (!f) {
        log_messages.printf(MSG_CRITICAL, "spec file missing\n");
        exit(1);
    }

    retval = spec.parse(f);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't parse spec file\n");
        exit(1);
    }

    fclose(f);

    if (file_lock.lock(LOCKFILE)) {
        log_messages.printf(MSG_CRITICAL, "Another copy of db_dump is already running\n");
        exit(1);
    }
    log_messages.printf(MSG_NORMAL, "Starting\n");

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }
    retval = boinc_db.open(
        config.replica_db_name,
        db_host?db_host:config.replica_db_host,
        config.replica_db_user,
        config.replica_db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't open DB\n");
        exit(1);
    }
    retval = boinc_db.set_isolation_level(READ_UNCOMMITTED);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %s; %s\n",
            boincerror(retval), boinc_db.error_string()
        );
    }

    boinc_mkdir(spec.output_dir);

    unsigned int j;
    for (j=0; j<spec.enumerations.size(); j++) {
        ENUMERATION& e = spec.enumerations[j];
        e.make_it_happen(spec.output_dir);
    }

    tables_file(spec.output_dir);

    sprintf(buf, "cp %s %s/db_dump.xml", spec_filename, spec.output_dir);
    retval = system(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "%s failed: %s\n", buf, boincerror(retval)
        );
        exit(retval);
    }

    // rename the old stats dir to a name that includes the date

    if (boinc_file_exists(spec.final_output_dir)) {
        struct tm* tmp;
        time_t now = time(0);
        tmp = localtime(&now);
        char base[256];
        if (strlen(spec.archive_dir)) {
            strcpy(base, spec.archive_dir);
            strcat(base, "/stats");
        } else {
            strcpy(base, spec.final_output_dir);
        }
        sprintf(buf, "mv %s %s_%d_%d_%d_%d_%d_%d",
            spec.final_output_dir,
            base,
            1900+tmp->tm_year,
            tmp->tm_mon+1,
            tmp->tm_mday,
            tmp->tm_hour,
            tmp->tm_min,
            tmp->tm_sec
        );
        retval = system(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "Can't rename old stats\n");
            exit(1);
        }
    }
    sprintf(buf, "mv %s %s", spec.output_dir, spec.final_output_dir);
    retval = system(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't rename new stats\n");
        exit(1);
    }
    log_messages.printf(MSG_NORMAL, "db_dump finished\n");
}

const char *BOINC_RCSID_500089bde6 = "$Id$";
