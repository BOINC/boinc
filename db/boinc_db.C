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

#include <stdlib.h>
#include <string.h>

#include "boinc_db.h"

#define MAX_QUERY_LEN 8192

MYSQL* mysql;

DB_BASE::DB_BASE(char *tn) : table_name(tn) {
    cursor.active = false;
}

int DB_BASE::get_id() { return 0;}
void DB_BASE::db_print(char*) {}

void DB_BASE::db_parse(MYSQL_ROW&) {}

int DB_BASE::insert() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "insert into %s set %s", table_name, vals);
    return mysql_query(mysql, query);
}

int DB_BASE::update() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "update %s set %s where id=%d", table_name, vals, get_id());
    return mysql_query(mysql, query);
}

int DB_BASE::lookup(char* clause) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s %s", table_name, clause);
    retval = mysql_query(mysql, query);
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) db_parse(row);
    mysql_free_result(rp);
    return (row == 0);
}

int DB_BASE::lookup_id(int id) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s where id=%d", table_name, id);
    retval = mysql_query(mysql, query);
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) db_parse(row);
    mysql_free_result(rp);
    return (row == 0);
}

int DB_BASE::enumerate(char* clause) {
    int x;
    char query[MAX_QUERY_LEN];
    MYSQL_ROW row;

    if (!cursor.active) {
        cursor.active = 1;
        sprintf(query, "select * from %s %s", table_name, clause);
        x = mysql_query(mysql, query);
        if (x) return mysql_errno(mysql);
        cursor.rp = mysql_store_result(mysql);
        if (!cursor.rp) return mysql_errno(mysql);
    }
    row = mysql_fetch_row(cursor.rp);
    if (!row) {
        mysql_free_result(cursor.rp);
        cursor.active = 0;
        return 1;
    } else {
        db_parse(row);
    }
    return 0;
}

int DB_BASE::get_integer(char* query, int& n) {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* resp;

    retval = mysql_query(mysql, query);
    if (retval) return retval;
    resp = mysql_store_result(mysql);
    if (!resp) return -1;
    row = mysql_fetch_row(resp);
    if (!row) return -1;
    if (!row[0]) return -1;
    n = atoi(row[0]);
    mysql_free_result(resp);
    return 0;
}

int DB_BASE::get_double(char* query, double& x) {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* resp;

    retval = mysql_query(mysql, query);
    if (retval) return retval;
    resp = mysql_store_result(mysql);
    if (!resp) return -1;
    row = mysql_fetch_row(resp);
    if (!row) return -1;
    if (!row[0]) return -1;
    x = atof(row[0]);
    mysql_free_result(resp);
    return 0;
}

int DB_BASE::count(int& n, char* clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "select count(*) from %s %s", table_name, clause);
    return get_integer(query, n);
}

int DB_BASE::sum(double& x, char* field, char* clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "select sum(%s) from %s %s", field, table_name, clause);
    return get_double(query, x);
}

static void strcpy2(char* dest, char* src) {
    if (!src) *dest = 0;
    else strcpy(dest, src);
}

// convert ' to \' in place
static void escape_single_quotes(char* field) {
    char buf[MAX_QUERY_LEN];
    char* q = buf, *p = field;
    while (*p) {
        if (*p == '\'') {
            *q++ = '\\';
            *q++ = '\'';
        } else {
            *q++ = *p;
        }
        p++;
    }
    *q = 0;
    strcpy(field, buf);
}

static void unescape_single_quotes(char* p) {
    char* q;
    while (1) {
        q = strstr(p, "\\'");
        if (!q) break;
        strcpy(q, q+1);
    }
}

int boinc_db_open(char* dbname, char* password) {
    mysql = mysql_init(0);
    if (!mysql) return -1;
    mysql = mysql_real_connect(mysql, 0, 0, password, dbname, 0, 0, 0);
    if (!mysql) return -1;
    return 0;
}

int boinc_db_close() {
    mysql_close(mysql);
    return 0;
}

int boinc_db_insert_id() {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    retval = mysql_query(mysql, "select LAST_INSERT_ID()");
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    row = mysql_fetch_row(rp);
    return atoi(row[0]);
}

void boinc_db_print_error(char* p) {
    if (mysql) {
        fprintf(stderr, "%s: Database error: %s\n", p, mysql_error(mysql));
    }
}

const char* boinc_db_error_string() {
    return mysql?mysql_error(mysql):"Not connected";
}

void PROJECT::clear() {memset(this, 0, sizeof(*this));}
void PLATFORM::clear() {memset(this, 0, sizeof(*this));}
void CORE_VERSION::clear() {memset(this, 0, sizeof(*this));}
void APP::clear() {memset(this, 0, sizeof(*this));}
void APP_VERSION::clear() {memset(this, 0, sizeof(*this));}
void USER::clear() {memset(this, 0, sizeof(*this));}
void TEAM::clear() {memset(this, 0, sizeof(*this));}
void HOST::clear() {memset(this, 0, sizeof(*this));}
void RESULT::clear() {memset(this, 0, sizeof(*this));}
void WORKUNIT::clear() {memset(this, 0, sizeof(*this));}
void WORKSEQ::clear() {memset(this, 0, sizeof(*this));}

DB_PROJECT::DB_PROJECT() : DB_BASE("project"){}
DB_PLATFORM::DB_PLATFORM() : DB_BASE("platform"){}
DB_CORE_VERSION::DB_CORE_VERSION() : DB_BASE("core_version"){}
DB_APP::DB_APP() : DB_BASE("app"){}
DB_APP_VERSION::DB_APP_VERSION() : DB_BASE("app_version"){}
DB_USER::DB_USER() : DB_BASE("user"){}
DB_TEAM::DB_TEAM() : DB_BASE("team"){}
DB_HOST::DB_HOST() : DB_BASE("host"){}
DB_WORKUNIT::DB_WORKUNIT() : DB_BASE("workunit"){}
DB_RESULT::DB_RESULT() : DB_BASE("result"){}
DB_WORKSEQ::DB_WORKSEQ() : DB_BASE("workseq"){}

int DB_PROJECT::get_id() {return id;}
int DB_PLATFORM::get_id() {return id;}
int DB_CORE_VERSION::get_id() {return id;}
int DB_APP::get_id() {return id;}
int DB_APP_VERSION::get_id() {return id;}
int DB_USER::get_id() {return id;}
int DB_TEAM::get_id() {return id;}
int DB_HOST::get_id() {return id;}
int DB_WORKUNIT::get_id() {return id;}
int DB_RESULT::get_id() {return id;}
int DB_WORKSEQ::get_id() {return id;}

void DB_PROJECT::db_print(char* buf){
    sprintf(buf,
        "id=%d, short_name='%s', long_name='%s'",
        id, short_name, long_name
    );
}

void DB_PROJECT::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    strcpy2(short_name, r[i++]);
    strcpy2(long_name, r[i++]);
}


void DB_PLATFORM::db_print(char* buf){
    sprintf(buf,
        "id=%d, create_time=%d, name='%s', user_friendly_name='%s'",
        id, create_time, name, user_friendly_name
    );
}

void DB_PLATFORM::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time=atol(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(user_friendly_name, r[i++]);
}

void DB_CORE_VERSION::db_print(char* buf) {
    sprintf(buf,
        "id=%d, create_time=%d, version_num=%d, platformid=%d, "
        "xml_doc='%s', message='%s', deprecated=%d",
        id, create_time, version_num, platformid,
        xml_doc, message, deprecated?1:0
    );
}

void DB_CORE_VERSION::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time = atoi(r[i++]);
    version_num = atoi(r[i++]);
    platformid = atoi(r[i++]);
    strcpy2(xml_doc, r[i++]);
    strcpy2(message, r[i++]);
    deprecated = atoi(r[i++]);
}

void DB_APP::db_print(char* buf){
    sprintf(buf,
        "id=%d, create_time=%d, name='%s', min_version=%d ",
        id, create_time, name, min_version
    );
}

void DB_APP::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time = atoi(r[i++]);
    strcpy2(name, r[i++]);
    min_version = atoi(r[i++]);
}

void DB_APP_VERSION::db_print(char* buf){
    sprintf(buf,
        "id=%d, create_time=%d, appid=%d, version_num=%d, platformid=%d, "
        "xml_doc='%s', "
        "min_core_version=%d, max_core_version=%d",
        id,
        create_time,
        appid,
        version_num,
        platformid,
        xml_doc,
        min_core_version,
        max_core_version
    );
}

void DB_APP_VERSION::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time = atoi(r[i++]);
    appid = atoi(r[i++]);
    version_num = atoi(r[i++]);
    platformid = atoi(r[i++]);
    strcpy2(xml_doc, r[i++]);
    min_core_version = atoi(r[i++]);
    max_core_version = atoi(r[i++]);
}

void DB_USER::db_print(char* buf){
    escape_single_quotes(email_addr);
    escape_single_quotes(name);
    escape_single_quotes(country);
    escape_single_quotes(postal_code);
    escape_single_quotes(global_prefs);
    escape_single_quotes(project_prefs);
    escape_single_quotes(url);
    sprintf(buf,
        "id=%d, create_time=%d, email_addr='%s', name='%s', "
        "authenticator='%s', "
        "country='%s', postal_code='%s', "
        "total_credit=%.12e, expavg_credit=%.12e, expavg_time=%f, "
        "global_prefs='%s', project_prefs='%s', "
        "teamid=%d, venue='%s', url='%s', send_email=%d, show_hosts=%d",
        id,
        create_time,
        email_addr,
        name,
        authenticator,
        country,
        postal_code,
        total_credit,
        expavg_credit,
        expavg_time,
        global_prefs,
        project_prefs,
        teamid,
        venue,
        url,
        send_email,
        show_hosts
    );
    unescape_single_quotes(email_addr);
    unescape_single_quotes(name);
    unescape_single_quotes(country);
    unescape_single_quotes(postal_code);
    unescape_single_quotes(global_prefs);
    unescape_single_quotes(project_prefs);
    unescape_single_quotes(url);
}

void DB_USER::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    strcpy2(email_addr, r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(authenticator, r[i++]);
    strcpy2(country, r[i++]);
    strcpy2(postal_code, r[i++]);
    total_credit = atof(r[i++]);
    expavg_credit = atof(r[i++]);
    expavg_time = atof(r[i++]);
    strcpy2(global_prefs, r[i++]);
    strcpy2(project_prefs, r[i++]);
    teamid = atoi(r[i++]);
    strcpy2(venue, r[i++]);
    strcpy2(url, r[i++]);
    send_email = atoi(r[i++]);
    show_hosts = atoi(r[i++]);
}

void DB_TEAM::db_print(char* buf){
    escape_single_quotes(name);
    escape_single_quotes(name_lc);
    escape_single_quotes(url);
    escape_single_quotes(name_html);
    escape_single_quotes(description);
    sprintf(buf,
        "id=%d, create_time=%d, userid=%d, name='%s', "
        "name_lc='%s', url='%s', "
        "type=%d, name_html='%s', description='%s', nusers=%d, "
        "country='%s', total_credit=%f, expavg_credit=%f",
        id,
        create_time,
        userid,
        name,
        name_lc,
        url,
        type,
        name_html,
        description,
        nusers,
        country,
        total_credit,
        expavg_credit
    );
    unescape_single_quotes(name);
    unescape_single_quotes(name_lc);
    unescape_single_quotes(url);
    unescape_single_quotes(name_html);
    unescape_single_quotes(description);
}

void DB_TEAM::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    userid = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(name_lc, r[i++]);
    strcpy2(url, r[i++]);
    type = atoi(r[i++]);
    strcpy2(name_html, r[i++]);
    strcpy2(description, r[i++]);
    nusers = atoi(r[i++]);
    strcpy2(country, r[i++]);
    total_credit = atof(r[i++]);
    expavg_credit = atof(r[i++]);
}

void DB_HOST::db_print(char* buf){
    escape_single_quotes(domain_name);
    escape_single_quotes(serialnum);
    escape_single_quotes(last_ip_addr);
    escape_single_quotes(p_vendor);
    escape_single_quotes(p_model);
    escape_single_quotes(os_name);
    escape_single_quotes(os_version);
    sprintf(buf,
        "id=%d, create_time=%d, userid=%d, "
        "rpc_seqno=%d, rpc_time=%d, "
        "total_credit=%.12e, expavg_credit=%.12e, expavg_time=%f, "
        "timezone=%d, domain_name='%s', serialnum='%s', "
        "last_ip_addr='%s', nsame_ip_addr=%d, "
        "on_frac=%f, connected_frac=%f, active_frac=%f, "
        "p_ncpus=%d, p_vendor='%s', p_model='%s', "
        "p_fpops=%f, p_iops=%f, p_membw=%f, "
        "os_name='%s', os_version='%s', "
        "m_nbytes=%f, m_cache=%f, m_swap=%f, "
        "d_total=%f, d_free=%f, "
        "d_boinc_used_total=%f, d_boinc_used_project=%f, d_boinc_max=%f, "
        "n_bwup=%f, n_bwdown=%f, "
        "credit_per_cpu_sec=%f, "
        "venue='%s', projects='%s'",
        id, create_time, userid,
        rpc_seqno, rpc_time,
        total_credit, expavg_credit, expavg_time,
        timezone, domain_name, serialnum,
        last_ip_addr, nsame_ip_addr,
        on_frac, connected_frac, active_frac,
        p_ncpus, p_vendor, p_model,
        p_fpops, p_iops, p_membw,
        os_name, os_version,
        m_nbytes, m_cache, m_swap,
        d_total, d_free,
        d_boinc_used_total, d_boinc_used_project, d_boinc_max,
        n_bwup, n_bwdown,
        credit_per_cpu_sec,
        venue, projects
    );
    unescape_single_quotes(domain_name);
    unescape_single_quotes(serialnum);
    unescape_single_quotes(last_ip_addr);
    unescape_single_quotes(p_vendor);
    unescape_single_quotes(p_model);
    unescape_single_quotes(os_name);
    unescape_single_quotes(os_version);
}

void DB_HOST::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time = atoi(r[i++]);
    userid = atoi(r[i++]);
    rpc_seqno = atoi(r[i++]);
    rpc_time = atoi(r[i++]);
    total_credit = atof(r[i++]);
    expavg_credit = atof(r[i++]);
    expavg_time = atof(r[i++]);
    timezone = atoi(r[i++]);
    strcpy2(domain_name, r[i++]);
    strcpy2(serialnum, r[i++]);
    strcpy2(last_ip_addr, r[i++]);
    nsame_ip_addr = atoi(r[i++]);
    on_frac = atof(r[i++]);
    connected_frac = atof(r[i++]);
    active_frac = atof(r[i++]);
    p_ncpus = atoi(r[i++]);
    strcpy2(p_vendor, r[i++]);
    strcpy2(p_model, r[i++]);
    p_fpops = atof(r[i++]);
    p_iops = atof(r[i++]);
    p_membw = atof(r[i++]);
    strcpy2(os_name, r[i++]);
    strcpy2(os_version, r[i++]);
    m_nbytes = atof(r[i++]);
    m_cache = atof(r[i++]);
    m_swap = atof(r[i++]);
    d_total = atof(r[i++]);
    d_free = atof(r[i++]);
    d_boinc_used_total = atof(r[i++]);
    d_boinc_used_project = atof(r[i++]);
    d_boinc_max = atof(r[i++]);
    n_bwup = atof(r[i++]);
    n_bwdown = atof(r[i++]);
    credit_per_cpu_sec = atof(r[i++]);
    strcpy2(venue, r[i++]);
    strcpy2(projects, r[i++]);
}

void DB_WORKUNIT::db_print(char* buf){
    sprintf(buf,
        "id=%d, create_time=%d, appid=%d, "
        "name='%s', xml_doc='%s', batch=%d, "
        "rsc_fpops=%f, rsc_iops=%f, rsc_memory=%f, rsc_disk=%f, "
        "need_validate=%d, "
        "canonical_resultid=%d, canonical_credit=%f, "
        "timeout_check_time=%d, delay_bound=%d, "
        "error_mask=%d, file_delete_state=%d, assimilate_state=%d, "
        "workseq_next=%d, opaque=%d",
        id, create_time, appid,
        name, xml_doc, batch,
        rsc_fpops, rsc_iops, rsc_memory, rsc_disk, 
        need_validate,
        canonical_resultid, canonical_credit,
        timeout_check_time, delay_bound,
        error_mask, file_delete_state, assimilate_state,
        workseq_next, opaque
    );
}

void DB_WORKUNIT::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time = atoi(r[i++]);
    appid = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(xml_doc, r[i++]);
    batch = atoi(r[i++]);
    rsc_fpops = atof(r[i++]);
    rsc_iops = atof(r[i++]);
    rsc_memory = atof(r[i++]);
    rsc_disk = atof(r[i++]);
    need_validate = atoi(r[i++]);
    canonical_resultid = atoi(r[i++]);
    canonical_credit = atof(r[i++]);
    timeout_check_time = atoi(r[i++]);
    delay_bound = atoi(r[i++]);
    error_mask = atoi(r[i++]);
    file_delete_state = atoi(r[i++]);
    assimilate_state = atoi(r[i++]);
    workseq_next = atoi(r[i++]);
    opaque = atoi(r[i++]);
}

void DB_RESULT::db_print(char* buf){
    escape_single_quotes(xml_doc_out);
    escape_single_quotes(stderr_out);
    sprintf(buf,
        "id=%d, create_time=%d, workunitid=%d, "
        "server_state=%d, outcome=%d, client_state=%d, "
        "hostid=%d, report_deadline=%d, sent_time=%d, received_time=%d, "
        "name='%s', cpu_time=%f, "
        "xml_doc_in='%s', xml_doc_out='%s', stderr_out='%s', "
        "batch=%d, file_delete_state=%d, validate_state=%d, "
        "claimed_credit=%f, granted_credit=%f, opaque=%d, random=%d",
        id, create_time, workunitid,
        server_state, outcome, client_state,
        hostid, report_deadline, sent_time, received_time,
        name, cpu_time,
        xml_doc_in, xml_doc_out, stderr_out,
        batch, file_delete_state, validate_state,
        claimed_credit, granted_credit, opaque, random
    );
    unescape_single_quotes(xml_doc_out);
    unescape_single_quotes(stderr_out);
}

void DB_RESULT::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time = atoi(r[i++]);
    workunitid = atoi(r[i++]);
    server_state = atoi(r[i++]);
    outcome = atoi(r[i++]);
    client_state = atoi(r[i++]);
    hostid = atoi(r[i++]);
    report_deadline = atoi(r[i++]);
    sent_time = atoi(r[i++]);
    received_time = atoi(r[i++]);
    strcpy2(name, r[i++]);
    cpu_time = atof(r[i++]);
    strcpy2(xml_doc_in, r[i++]);
    strcpy2(xml_doc_out, r[i++]);
    strcpy2(stderr_out, r[i++]);
    batch = atoi(r[i++]);
    file_delete_state = atoi(r[i++]);
    validate_state = atoi(r[i++]);
    claimed_credit = atof(r[i++]);
    granted_credit = atof(r[i++]);
    opaque = atoi(r[i++]);
    random = atoi(r[i++]);
}

void DB_WORKSEQ::db_print(char* buf){
    sprintf(buf,
        "id=%d, create_time=%d, "
        "state=%d, hostid=%d, "
        "wuid_last_done=%d, wuid_last_sent=%d, "
        "workseqid_master=%d",
        id, create_time,
        state, hostid,
        wuid_last_done, wuid_last_sent,
        workseqid_master
    );
}

void DB_WORKSEQ::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id=atol(r[i++]);
    create_time = atoi(r[i++]);
    state = atoi(r[i++]);
    hostid = atoi(r[i++]);
    wuid_last_done = atoi(r[i++]);
    wuid_last_sent = atoi(r[i++]);
    workseqid_master = atoi(r[i++]);
}
