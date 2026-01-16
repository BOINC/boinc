// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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
// see https://github.com/BOINC/boinc/wiki/DbDump

// Note:
// 1) this program is way more configurable than it needs to be.
//    All projects export stats in the same format,
//    as described in the default db_dump_spec.xml that is created for you.
// 2) should scrap this and replace it with a 100 line PHP script.
//    I'll get to this someday.

#include "config.h"
#include <zlib.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

#define NUM_TABLES          5

#define TABLE_USER          0
#define TABLE_TEAM          1
#define TABLE_HOST          2
#define TABLE_USER_DELETED  3
#define TABLE_HOST_DELETED  4

#define CONSENT_TO_STATISTICS_EXPORT "STATSEXPORT"

// must match the above
const char* table_name[NUM_TABLES] = {"user", "team", "host", "user_deleted", "host_deleted"};
const char* tag_name[NUM_TABLES] = {"users", "teams", "hosts", "users_deleted", "hosts_deleted"};

int nusers, nhosts, nteams, nusers_deleted, nhosts_deleted;
double total_credit;
bool have_badges = false;

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
            for (i=0; i<NUM_TABLES; i++) {
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

//
// File streams
//
class OUTPUT_STREAM {
public:
    virtual ~OUTPUT_STREAM() {}
    virtual bool is_open() const = 0;
    virtual bool open(const char* filename) = 0;
    virtual void close() = 0;
    virtual void write(const void* buf, int size) = 0;
};

class UNCOMPRESSED_FILE : public OUTPUT_STREAM
{
private:
    FILE* f;

public:
    UNCOMPRESSED_FILE()
        : f(0) {}

    bool is_open() const{
        return f != 0;
    }

    bool open(const char* filename) {
        f = fopen(filename, "w");
        return f != 0;
    }

    void close() {
        fclose(f);
        f = 0;
    }

    void write(const void* buf, int size) {
        fwrite(buf, size, 1, f);
    }
};

class ZIP_FILE : public OUTPUT_STREAM
{
private:
    UNCOMPRESSED_FILE f;
    char current_path[MAXPATHLEN];

public:
    bool is_open() const {
        return f.is_open();
    }

    bool open(const char* filename) {
        if(!f.open(filename))
            return false;

        safe_strcpy(current_path, filename);
        return true;
    }

    void close() {
        f.close();

        // Do zip
        char buf[256];
        sprintf(buf, "zip -q %s", current_path);
        int retval = system(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "%s failed: %s\n", buf, boincerror(retval)
            );
            exit(retval);
        }
    }

    void write(const void* buf, int size) {
        f.write(buf, size);
    }
};

class GZIP_FILE : public OUTPUT_STREAM
{
private:
    gzFile gz;

public:
    GZIP_FILE()
        : gz(0) {}

    bool is_open() const {
        return gz != 0;
    }

    bool open(const char* filename) {
        char buf[MAXPATHLEN];
        snprintf(buf, sizeof(buf), "%s.gz", filename);
        gz = gzopen(buf, "wb");
        return gz != 0;
    }

    void close() {
        gzclose(gz);
        gz = 0;
    }

    void write(const void* buf, int size) {
        gzwrite(gz, buf, size);
    }
};

// class that automatically compresses on close
//
class ZFILE {
protected:
    string tag;     // enclosing XML tag
    OUTPUT_STREAM* stream;
public:
    ZFILE(string tag_, int comp): tag(tag_) {
        switch(comp) {
        case COMPRESSION_ZIP:
            stream = new ZIP_FILE;
            break;
        case COMPRESSION_GZIP:
            stream = new GZIP_FILE;
            break;
        case COMPRESSION_NONE:
        default:
            stream = new UNCOMPRESSED_FILE;
            break;
        }
    }

    ~ZFILE() {
        close();
        delete stream;
    }

    bool is_open() const {
        return stream->is_open();
    }

    void open(const char* filename) {
        close();

        if (!stream->open(filename)) {
            log_messages.printf(MSG_CRITICAL,
                "Couldn't open %s for output\n", filename
            );
            exit(ERR_FOPEN);
        }

        write(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<%s>\n", tag.c_str()
        );
    }

    void open_num(const char* filename, int filenum) {
        char buf[256];
        sprintf(buf, "%s_%d", filename, filenum);
        open(buf);
    }

    void close() {
        if(!is_open())
            return;

        write("</%s>\n", tag.c_str());
        stream->close();
    }

    void write(const char* fmt, ...) {
        if(!is_open())
            return;

        va_list args;
        va_start(args, fmt);
        char* ptr;
        int size = vasprintf(&ptr, fmt, args);
        va_end(args);

        if(size < 0) {
            log_messages.printf(MSG_CRITICAL,
                "Error allocating memory buffer\n"
            );
            exit(ERR_MALLOC);
        }

        stream->write(ptr, size);
        free(ptr);
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
    if (!is_open() || (filenum != last_filenum)) {
        open_num(filename_base, filenum);
        last_filenum = filenum;
    }
}

void write_host_deleted(HOST_DELETED& host_deleted, ZFILE* f) {
    f->write(
        "<host>\n"
        "    <id>%lu</id>\n"
        "    <host_cpid>%s</host_cpid>\n"
        "</host>\n",
        host_deleted.hostid,
        host_deleted.public_cross_project_id
    );
}

void write_host(HOST& host, ZFILE* f, bool detail) {
    int retval;
    char p_vendor[2048], p_model[2048], os_name[2048], os_version[2048];

    xml_escape(host.p_vendor, p_vendor, sizeof(p_vendor));
    xml_escape(host.p_model, p_model, sizeof(p_model));
    xml_escape(host.os_name, os_name, sizeof(os_name));
    xml_escape(host.os_version, os_version, sizeof(os_version));
    f->write(
        "<host>\n"
        "    <id>%lu</id>\n",
        host.id
    );
    if (detail) {
        DB_USER user;
        retval = user.lookup_id(host.userid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "user lookup of user %lu for host %lu: %s\n",
                host.userid, host.id, boincerror(retval)
            );
        } else {
            if (user.show_hosts) {
                f->write(
                    "    <userid>%lu</userid>\n",
                    host.userid
                );
            }
        }
    }
    f->write(
        "    <total_credit>%f</total_credit>\n"
        "    <expavg_credit>%f</expavg_credit>\n"
        "    <expavg_time>%f</expavg_time>\n"
        "    <p_vendor>%s</p_vendor>\n"
        "    <p_model>%s</p_model>\n"
        "    <os_name>%s</os_name>\n"
        "    <os_version>%s</os_version>\n",
        "    <misc><![CDATA[%s]]></misc>\n",
        host.total_credit,
        host.expavg_credit,
        host.expavg_time,
        p_vendor,
        p_model,
        os_name,
        os_version,
        host.misc
    );

    if (detail) {
        f->write(
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
    f->write(
        "</host>\n"
    );
}

void write_user_deleted(USER_DELETED& user_deleted, ZFILE* f) {
    f->write(
        "<user>\n"
        "    <id>%lu</id>\n"
        "    <cpid>%s</cpid>\n"
        "</user>\n",
        user_deleted.userid,
        user_deleted.public_cross_project_id
    );
}

void write_user(USER& user, ZFILE* f, bool /*detail*/) {
    char buf[1024];
    char cpid[MD5_LEN];

    char name[2048], url[2048];
    xml_escape(user.name, name, sizeof(name));
    xml_escape(user.url, url, sizeof(url));

    safe_strcpy(buf, user.cross_project_id);
    safe_strcat(buf, user.email_addr);
    md5_block((unsigned char*)buf, strlen(buf), cpid);

    f->write(
        "<user>\n"
        " <id>%lu</id>\n"
        " <name>%s</name>\n"
        " <create_time>%d</create_time>\n"
        " <total_credit>%f</total_credit>\n"
        " <expavg_credit>%f</expavg_credit>\n"
        " <expavg_time>%f</expavg_time>\n"
        " <cpid>%s</cpid>\n",
        user.id,
        name,
        user.create_time,
        user.total_credit,
        user.expavg_credit,
        user.expavg_time,
        cpid
    );
    if (config.user_country && strlen(user.country)) {
        f->write(
            " <country>%s</country>\n",
            user.country
        );
    }
    if (config.user_url && strlen(user.url)) {
        f->write(
            " <url>%s</url>\n",
            url
        );
    }
    if (user.teamid) {
        f->write(
            " <teamid>%lu</teamid>\n",
            user.teamid
        );
    }
    if (user.has_profile) {
        f->write(
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
    f->write(
        "</user>\n"
    );
}

void write_badge_user(char* output_dir) {
    DB_BADGE_USER bu;
    char path[MAXPATHLEN];
    ZFILE zf("badge_users", COMPRESSION_GZIP);
    sprintf(path, "%s/badge_user", output_dir);
    zf.open(path);
    while (!bu.enumerate("")) {
        zf.write(
            " <badge_user>\n"
            "    <user_id>%lu</user_id>\n"
            "    <badge_id>%lu</badge_id>\n"
            "    <create_time>%.0f</create_time>\n"
            " </badge_user>\n",
            bu.user_id,
            bu.badge_id,
            bu.create_time
        );
    }
    zf.close();
}

void write_badge_team(char* output_dir) {
    DB_BADGE_TEAM bt;
    char path[MAXPATHLEN];
    ZFILE zf("badge_teams", COMPRESSION_GZIP);
    sprintf(path, "%s/badge_team", output_dir);
    zf.open(path);
    while (!bt.enumerate("")) {
        zf.write(
            " <badge_team>\n"
            "    <team_id>%lu</team_id>\n"
            "    <badge_id>%lu</badge_id>\n"
            "    <create_time>%.0f</create_time>\n"
            " </badge_team>\n",
            bt.team_id,
            bt.badge_id,
            bt.create_time
        );
    }
    zf.close();
}

void write_team(TEAM& team, ZFILE* f, bool detail) {
    DB_USER user;
    char buf[256];
    char name[2048];
    char url[2048], name_html[2048];
    int retval;
    char description[BLOB_SIZE];

    xml_escape(team.name, name, sizeof(name));

    f->write(
        "<team>\n"
        " <id>%lu</id>\n"
        " <type>%d</type>\n"
        " <name>%s</name>\n"
        " <userid>%lu</userid>\n"
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
        f->write(
            "  <founder_name>%s</founder_name>\n",
            fname
        );
    }

    f->write(
        " <create_time>%d</create_time>\n",
        team.create_time
    );
    if (strlen(team.url)) {
        xml_escape(team.url, url, sizeof(url));
        f->write(
            " <url>%s</url>\n",
            url
        );
    }
    if (strlen(team.name_html)) {
        xml_escape(team.name_html, name_html, sizeof(name_html));
        f->write(
            "<name_html>%s</name_html>\n",
            name_html
        );
    }

    if (strlen(team.description)) {
        xml_escape(team.description, description, sizeof(description));
        f->write(
            "<description>%s</description>\n",
            description
        );
    }

    f->write(
        " <country>%s</country>\n",
        team.country
    );
    if (detail) {
        sprintf(buf, "where teamid=%lu", team.id);
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
    f->write(
        "</team>\n"
    );
}

int print_app(ZFILE* f, APP& app) {
    f->write( "        <application>\n");
    f->write( "            <name>%s</name>\n", app.user_friendly_name);

#if 0
    DB_RESULT result;
    char buf[256];
    int n, retval;
    // can't do this stuff because MySQL/InnoDB can't do counts efficiently
    //
    sprintf(buf, "where appid=%d and server_state=%d", app.id, RESULT_SERVER_STATE_UNSENT);
    retval = result.count(n, buf);
    if (!retval) {
        f->write( "            <results_unsent>%d</results_unsent>\n", n);
    }

    sprintf(buf, "where appid=%d and server_state=%d", app.id, RESULT_SERVER_STATE_IN_PROGRESS);
    retval = result.count(n, buf);
    if (!retval) {
        f->write( "            <results_in_progress>%d</results_in_progress>\n", n);
    }

    sprintf(buf, "where appid=%d and server_state=%d", app.id, RESULT_SERVER_STATE_OVER);
    retval = result.count(n, buf);
    if (!retval) {
        f->write( "            <results_over>%d</results_over>\n", n);
    }
#endif

    f->write( "        </application>\n");
    return 0;
}

int print_apps(ZFILE* f) {
    DB_APP app;
    f->write( "    <applications>\n");
    while (!app.enumerate()) {
        print_app(f, app);
    }
    f->write( "    </applications>\n");
    return 0;
}

void print_badges(ZFILE* f) {
    DB_BADGE badge;
    f->write( "    <badges>\n");
    while (!badge.enumerate()) {
        have_badges = true;
        f->write(
            "       <badge>\n"
            "           <id>%lu</id>\n"
            "           <name>%s</name>\n"
            "           <title>%s</title>\n"
            "           <image_url>%s</image_url>\n"
            "       </badge>\n",
            badge.id,
            badge.name,
            badge.title,
            badge.image_url
        );
    }
    f->write( "    </badges>\n");
}

int tables_file(char* dir) {
    char buf[256];

    ZFILE zf("tables", false);
    sprintf(buf, "%s/tables.xml", dir);
    zf.open(buf);
    zf.write(
        "    <update_time>%d</update_time>\n",
        (int)time(0)
    );
    if (config.credit_by_app) {
        zf.write("    <credit_by_app/>\n");
    }
    if (nusers) zf.write("    <nusers_total>%d</nusers_total>\n", nusers);
    if (nteams) zf.write("    <nteams_total>%d</nteams_total>\n", nteams);
    if (nhosts) zf.write("    <nhosts_total>%d</nhosts_total>\n", nhosts);
    if (nusers_deleted) zf.write("    <nusers_deleted_total>%d</nusers_deleted_total>\n", nusers_deleted);
    if (nhosts_deleted) zf.write("    <nhosts_deleted_total>%d</nhosts_deleted_total>\n", nhosts_deleted);
    if (total_credit) zf.write("    <total_credit>%lf</total_credit>\n", total_credit);
    print_apps(&zf);
    print_badges(&zf);
    zf.close();
    return 0;
}

int ENUMERATION::make_it_happen(char* output_dir) {
    unsigned int i;
    int n, retval;
    DB_USER user;
    DB_USER_DELETED user_deleted;
    DB_TEAM team;
    DB_HOST host;
    DB_HOST_DELETED host_deleted;
    DB_RESULT result;
    DB_CONSENT_TYPE consent_type;
    char clause[512];
    char lookupclause[256];
    char userclause[256];
    char hostclause[256];
    char teamclause[256];
    char joinclause[512];
    char orderclause[256];
    char path[MAXPATHLEN];
    long ncount;
    double sumtotalcredit;

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

    // Generate the SQL necessary for retrieving data
    // host, user, and team where clauses
    safe_strcpy(userclause, "WHERE total_credit > 0 AND authenticator NOT LIKE 'deleted%'");
    safe_strcpy(hostclause, "WHERE total_credit > 0 AND domain_name != 'deleted' AND host.userid != 0");
    safe_strcpy(teamclause, "WHERE total_credit > 0");

    // set order clause based on sort type
    switch(sort) {
    case SORT_NONE:
        safe_strcpy(orderclause, "");
        break;
    case SORT_ID:
        safe_strcpy(orderclause, "ORDER BY id");
        break;
    case SORT_TOTAL_CREDIT:
        safe_strcpy(orderclause, "ORDER BY total_credit DESC");
        break;
    case SORT_EXPAVG_CREDIT:
        safe_strcpy(orderclause, "ORDER BY expavg_credit DESC");
        break;
    }

    switch(table) {
    case TABLE_USER:
	// Count number of users, this needs to be independent of
	// CONSENT_TO_STATISTICS_EXPORT.

        // SQL clause to ignore deleted users.
        safe_strcpy(clause, userclause);
	safe_strcat(clause, " ");
	safe_strcat(clause, orderclause);

	retval = user.count(ncount, clause);
	if (!retval) nusers = ncount;

	retval = user.sum(sumtotalcredit, "total_credit", clause);
	if (!retval) total_credit = sumtotalcredit;

	// lookup consent_type
	sprintf(lookupclause, "where shortname = '%s'", CONSENT_TO_STATISTICS_EXPORT);
	retval = consent_type.lookup(lookupclause);
	// If retval is 0: lookup is successful, and consent_type
	// enabled flag is true, then edit the SQL clause to use the
	// JOIN statements to extract only the users who have
	// consented to statistics exports.
	if ( (!retval) && (consent_type.enabled) ) {
	    // This INNER JOIN clause does the following. It joins the
	    // user table with the latest_consent View table, see
	    // schema.sql for this view's definition. The
	    // latest_consent represents the latest consent status for
	    // all users and consent_types. Effectively returning users
	    // who have consented to statistics exports.
	    sprintf(joinclause, "INNER JOIN (\
            SELECT userid\
              FROM latest_consent\
             WHERE consent_type_id=%ld\
               AND consent_flag=1) AS lc\
            ON user.id = lc.userid", consent_type.id);
	    safe_strcat(joinclause, " ");
	    safe_strcat(joinclause, clause);
	    safe_strcpy(clause, joinclause);
	}

        n = 0;
        while (1) {
            retval = user.enumerate(clause, true);
            if (retval) break;

            if (!strncmp("deleted", user.authenticator, 7)) continue;
            for (i=0; i<outputs.size(); i++) {
                OUTPUT& out = outputs[i];
                if (sort == SORT_ID && out.recs_per_file) {
                    out.nzfile->set_id(n++);
                }
                if (out.zfile) {
                    write_user(user, out.zfile, out.detail);
                } else {
                    write_user(user, out.nzfile, out.detail);
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
    case TABLE_USER_DELETED:
        n = 0;
        while (1) {
            retval = user_deleted.enumerate("order by userid");
            if (retval) break;
            nusers_deleted++;
            for (i=0; i<outputs.size(); i++) {
                OUTPUT& out = outputs[i];
                if (sort == SORT_ID && out.recs_per_file) {
                    out.nzfile->set_id(n++);
                }
                if (out.zfile) {
                    write_user_deleted(user_deleted, out.zfile);
                } else {
                    write_user_deleted(user_deleted, out.nzfile);
                }
            }
        }
        if (retval != ERR_DB_NOT_FOUND) {
            log_messages.printf(MSG_CRITICAL,
                "user_deleted enum: %s", boincerror(retval)
            );
            exit(retval);
        }
        break;
    case TABLE_HOST:
	// Count number of hosts, this needs to be independent of
	// CONSENT_TO_STATISTICS_EXPORT.

        // SQL clause to ignore deleted hosts.
        safe_strcpy(clause, hostclause);
	safe_strcat(clause, " ");
	safe_strcat(clause, orderclause);

	retval = host.count(ncount, clause);
	if (!retval) nhosts = ncount;

	// lookup consent_type
	sprintf(lookupclause, "where shortname = '%s'", CONSENT_TO_STATISTICS_EXPORT);
	retval = consent_type.lookup(lookupclause);
	// If retval is 0: lookup is successful, and consent_type
	// enabled flag is true, then edit the SQL clause to use the
	// JOIN statements to extract only the users who have
	// consented to statistics exports.
	if ( (!retval) && (consent_type.enabled) ) {
	    // This INNER JOIN clause does the following. It joins the
	    // host table with the latest_consent View table, see
	    // schema.sql for this view's definition. The
	    // latest_consent represents the latest consent status for
	    // all users and consent_types. Effectively returning
	    // hosts of users who have consented to statistics
	    // exports.
	    sprintf(joinclause, "INNER JOIN (\
            SELECT userid\
              FROM latest_consent\
             WHERE consent_type_id=%ld\
               AND consent_flag=1) AS lc\
            ON host.userid = lc.userid", consent_type.id);
	    safe_strcat(joinclause, " ");
	    safe_strcat(joinclause, clause);
	    safe_strcpy(clause, joinclause);
	}

        n = 0;
        while(1) {
            retval = host.enumerate(clause);
            if (retval) break;
            if (!host.userid) continue;
            if (!strncmp("deleted", host.domain_name, 8)) continue;
            for (i=0; i<outputs.size(); i++) {
                OUTPUT& out = outputs[i];
                if (sort == SORT_ID && out.recs_per_file) {
                    out.nzfile->set_id(n++);
                }
                if (out.zfile) {
                    write_host(host, out.zfile, out.detail);
                } else {
                    write_host(host, out.nzfile, out.detail);
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
    case TABLE_HOST_DELETED:
        n = 0;
        while(1) {
            retval = host_deleted.enumerate("order by hostid");
            if (retval) break;
            nhosts_deleted++;
            for (i=0; i<outputs.size(); i++) {
                OUTPUT& out = outputs[i];
                if (sort == SORT_ID && out.recs_per_file) {
                    out.nzfile->set_id(n++);
                }
                if (out.zfile) {
                    write_host_deleted(host_deleted, out.zfile);
                } else {
                    write_host_deleted(host_deleted, out.nzfile);
                }
            }
        }
        if (retval != ERR_DB_NOT_FOUND) {
            log_messages.printf(MSG_CRITICAL,
                "host_deleted enum: %s", boincerror(retval)
            );
            exit(retval);
        }
        break;
    case TABLE_TEAM:
        // SQL clause for teams.
        safe_strcpy(clause, teamclause);
	safe_strcat(clause, " ");
	safe_strcat(clause, orderclause);

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
                    write_team(team, out.zfile, out.detail);
                } else {
                    write_team(team, out.nzfile, out.detail);
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
        if (out.zfile) {
          out.zfile->close();
          delete out.zfile;
        }
        if (out.nzfile) {
          out.nzfile->close();
          delete out.nzfile;
        }
    }
    return 0;
}

void usage(char* name) {
    fprintf(stderr,
        "This program generates XML files containing project statistics.\n"
        "It should be run once a day as a periodic task in config.xml.\n"
        "For more info, see https://github.com/BOINC/boinc/wiki/DbDump\n\n"
        "Usage: %s [options]\n"
        "Options:\n"
        "    --dump_spec filename          Use the given config file (use ../db_dump_spec.xml)\n"
        "    [-d N | --debug_level]        Set verbosity level (1 to 4)\n"
        "    [--db_host H]                 Use the DB server on host H\n"
        "    [--retry_period H]            When can't connect to DB, retry after N sec instead of terminating\n"
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
    int retry_period = 0;

    check_stop_daemons();
    setbuf(stderr, 0);

    retval = system("cd ../html/ops; echo 2");
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
        } else if (is_arg(argv[i], "retry_period")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            retry_period = atoi(argv[i]);
            if (retry_period < 0) retry_period = 0;
            if (retry_period > 1000000) retry_period = 1000000;
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

    retval = boinc_mkdir(spec.output_dir);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_mkdir(%s): %s; %s\n",
            spec.output_dir, boincerror(retval), boinc_db.error_string()
        );
        exit(1);
    }

    while ((retval = boinc_db.open(
        config.replica_db_name,
        db_host?db_host:config.replica_db_host,
        config.replica_db_user,
        config.replica_db_passwd
    ))) {
        log_messages.printf(MSG_CRITICAL, "Can't open DB: %s\n",
            boinc_db.error_string()
        );
        if (retry_period == 0) exit(1);
        boinc_sleep(retry_period);
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

    if (config.credit_by_app) {
        retval = system("cd ../html/ops ; ./export_credit_by_app.php ../stats_tmp");
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "export_credit_by_app.php failed: %d\n", WEXITSTATUS(retval)
            );
        }
    }

    // this must follow the above loop, to get tables counts
    //
    tables_file(spec.output_dir);

    if (have_badges) {
        write_badge_user(spec.output_dir);
        write_badge_team(spec.output_dir);
    }

    snprintf(buf, sizeof(buf), "cp %s %s/db_dump.xml", spec_filename, spec.output_dir);
    retval = system(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "%s failed: %s\n", buf, boincerror(retval)
        );
        boinc_db.close();
        exit(retval);
    }

    // rename the old stats dir to a name that includes the date

    if (boinc_file_exists(spec.final_output_dir)) {
        struct tm* tmp;
        time_t now = time(0);
        tmp = localtime(&now);
        char base[256];
        if (strlen(spec.archive_dir)) {
            safe_strcpy(base, spec.archive_dir);
            strcat(base, "/stats");
        } else {
            safe_strcpy(base, spec.final_output_dir);
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
            boinc_db.close();
            exit(1);
        }
    }
    sprintf(buf, "mv %s %s", spec.output_dir, spec.final_output_dir);
    retval = system(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't rename new stats\n");
        boinc_db.close();
        exit(1);
    }
    log_messages.printf(MSG_NORMAL, "db_dump finished\n");
    boinc_db.close();
}
