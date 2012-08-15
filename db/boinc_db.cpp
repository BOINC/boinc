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

#include "config.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <cmath>
#include <math.h>

// For machines with finite() defined in ieeefp.h
#if HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#include "common_defs.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "boinc_db.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

using std::string;

extern "C" {
    int isnan(double);
}

DB_CONN boinc_db;

static struct random_init {
    random_init() {
        srand48(getpid() + time(0));
    }
} random_init;

#define ESCAPE(x) escape_string(x, sizeof(x))
#define UNESCAPE(x) unescape_string(x, sizeof(x))

void PLATFORM::clear() {memset(this, 0, sizeof(*this));}
void APP::clear() {memset(this, 0, sizeof(*this));}
void APP_VERSION::clear() {memset(this, 0, sizeof(*this));}
void USER::clear() {memset(this, 0, sizeof(*this));}
void TEAM::clear() {memset(this, 0, sizeof(*this));}
void HOST::clear() {memset(this, 0, sizeof(*this));}
void RESULT::clear() {memset(this, 0, sizeof(*this));}
void WORKUNIT::clear() {memset(this, 0, sizeof(*this));}
void CREDITED_JOB::clear() {memset(this, 0, sizeof(*this));}
void MSG_FROM_HOST::clear() {memset(this, 0, sizeof(*this));}
void MSG_TO_HOST::clear() {memset(this, 0, sizeof(*this));}
void ASSIGNMENT::clear() {memset(this, 0, sizeof(*this));}
void TRANSITIONER_ITEM::clear() {memset(this, 0, sizeof(*this));}
void VALIDATOR_ITEM::clear() {memset(this, 0, sizeof(*this));}
void SCHED_RESULT_ITEM::clear() {memset(this, 0, sizeof(*this));}
void HOST_APP_VERSION::clear() {memset(this, 0, sizeof(*this));}
void STATE_COUNTS::clear() {memset(this, 0, sizeof(*this));}
void FILE_ITEM::clear() {memset(this, 0, sizeof(*this));}
void FILESET_ITEM::clear() {memset(this, 0, sizeof(*this));}
void FILESET_FILE_ITEM::clear() {memset(this, 0, sizeof(*this));}
void SCHED_TRIGGER_ITEM::clear() {
    id = 0;
    fileset_id = 0;
    need_work = false;
    work_available = false;
    no_work_available = false;
    working_set_removal = false;
}
void FILESET_SCHED_TRIGGER_ITEM::clear() {memset(this, 0, sizeof(*this));}
void VDA_FILE::clear() {memset(this, 0, sizeof(*this));}
void VDA_CHUNK_HOST::clear() {memset(this, 0, sizeof(*this));}

DB_PLATFORM::DB_PLATFORM(DB_CONN* dc) :
    DB_BASE("platform", dc?dc:&boinc_db){}
DB_APP::DB_APP(DB_CONN* dc) :
    DB_BASE("app", dc?dc:&boinc_db){}
DB_APP_VERSION::DB_APP_VERSION(DB_CONN* dc) :
    DB_BASE("app_version", dc?dc:&boinc_db){}
DB_USER::DB_USER(DB_CONN* dc) :
    DB_BASE("user", dc?dc:&boinc_db){}
DB_TEAM::DB_TEAM(DB_CONN* dc) :
    DB_BASE("team", dc?dc:&boinc_db){}
DB_HOST::DB_HOST(DB_CONN* dc) :
    DB_BASE("host", dc?dc:&boinc_db){}
DB_WORKUNIT::DB_WORKUNIT(DB_CONN* dc) :
    DB_BASE("workunit", dc?dc:&boinc_db){}
DB_CREDITED_JOB::DB_CREDITED_JOB(DB_CONN* dc) :
    DB_BASE("credited_job", dc?dc:&boinc_db){}
DB_RESULT::DB_RESULT(DB_CONN* dc) :
    DB_BASE("result", dc?dc:&boinc_db){}
DB_MSG_FROM_HOST::DB_MSG_FROM_HOST(DB_CONN* dc) :
    DB_BASE("msg_from_host", dc?dc:&boinc_db){}
DB_MSG_TO_HOST::DB_MSG_TO_HOST(DB_CONN* dc) :
    DB_BASE("msg_to_host", dc?dc:&boinc_db){}
DB_ASSIGNMENT::DB_ASSIGNMENT(DB_CONN* dc) :
    DB_BASE("assignment", dc?dc:&boinc_db){}
DB_HOST_APP_VERSION::DB_HOST_APP_VERSION(DB_CONN* dc) :
    DB_BASE("host_app_version", dc?dc:&boinc_db){}
DB_STATE_COUNTS::DB_STATE_COUNTS(DB_CONN* dc) :
    DB_BASE("state_counts", dc?dc:&boinc_db){}
DB_TRANSITIONER_ITEM_SET::DB_TRANSITIONER_ITEM_SET(DB_CONN* dc) :
    DB_BASE_SPECIAL(dc?dc:&boinc_db){}
DB_VALIDATOR_ITEM_SET::DB_VALIDATOR_ITEM_SET(DB_CONN* dc) :
    DB_BASE_SPECIAL(dc?dc:&boinc_db){}
DB_WORK_ITEM::DB_WORK_ITEM(DB_CONN* dc) :
    DB_BASE_SPECIAL(dc?dc:&boinc_db
){
    start_id = 0;
}
DB_IN_PROGRESS_RESULT::DB_IN_PROGRESS_RESULT(DB_CONN* dc) :
    DB_BASE_SPECIAL(dc?dc:&boinc_db){}
DB_SCHED_RESULT_ITEM_SET::DB_SCHED_RESULT_ITEM_SET(DB_CONN* dc) :
    DB_BASE_SPECIAL(dc?dc:&boinc_db){}
DB_FILE::DB_FILE(DB_CONN* dc) :
    DB_BASE("file", dc?dc:&boinc_db){}
DB_FILESET::DB_FILESET(DB_CONN* dc) :
    DB_BASE("fileset", dc?dc:&boinc_db){}
DB_FILESET_FILE::DB_FILESET_FILE(DB_CONN* dc) :
    DB_BASE("fileset_file", dc?dc:&boinc_db){}
DB_SCHED_TRIGGER::DB_SCHED_TRIGGER(DB_CONN* dc) :
    DB_BASE("sched_trigger", dc?dc:&boinc_db) {
    id = 0;
    fileset_id = 0;
    need_work = false;
    work_available = false;
    no_work_available = false;
    working_set_removal = false;
}
DB_FILESET_SCHED_TRIGGER_ITEM::DB_FILESET_SCHED_TRIGGER_ITEM(DB_CONN* dc) :
    DB_BASE_SPECIAL(dc?dc:&boinc_db){}
DB_FILESET_SCHED_TRIGGER_ITEM_SET::DB_FILESET_SCHED_TRIGGER_ITEM_SET(DB_CONN* dc) :
    DB_BASE_SPECIAL(dc?dc:&boinc_db){}
DB_VDA_FILE::DB_VDA_FILE(DB_CONN* dc) :
    DB_BASE("vda_file", dc?dc:&boinc_db){}
DB_VDA_CHUNK_HOST::DB_VDA_CHUNK_HOST(DB_CONN* dc) :
    DB_BASE("vda_chunk_host", dc?dc:&boinc_db){}

int DB_PLATFORM::get_id() {return id;}
int DB_APP::get_id() {return id;}
int DB_APP_VERSION::get_id() {return id;}
int DB_USER::get_id() {return id;}
int DB_TEAM::get_id() {return id;}
int DB_HOST::get_id() {return id;}
int DB_WORKUNIT::get_id() {return id;}
int DB_RESULT::get_id() {return id;}
int DB_MSG_FROM_HOST::get_id() {return id;}
int DB_MSG_TO_HOST::get_id() {return id;}
int DB_ASSIGNMENT::get_id() {return id;}
int DB_STATE_COUNTS::get_id() {return appid;}
int DB_FILE::get_id() {return id;}
int DB_FILESET::get_id() {return id;}
int DB_SCHED_TRIGGER::get_id() {return id;}
int DB_VDA_FILE::get_id() {return id;}

void DB_PLATFORM::db_print(char* buf){
    sprintf(buf,
        "create_time=%d, name='%s', user_friendly_name='%s', "
        "deprecated=%d",
        create_time, name, user_friendly_name,
        deprecated
    );
}

void DB_PLATFORM::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(user_friendly_name, r[i++]);
    deprecated = atoi(r[i++]);
}

void DB_APP::db_print(char* buf){
    sprintf(buf,
        "create_time=%d, "
        "name='%s', "
        "min_version=%d, "
        "deprecated=%d, "
        "user_friendly_name='%s', "
        "homogeneous_redundancy=%d, "
        "weight=%.15e, "
        "beta=%d, "
        "target_nresults=%d, "
        "min_avg_pfc=%.15e, "
        "host_scale_check=%d, "
        "homogeneous_app_version=%d, ",
        create_time,
        name,
        min_version,
        deprecated?1:0,
        user_friendly_name,
        homogeneous_redundancy,
        weight,
        beta?1:0,
        target_nresults,
        min_avg_pfc,
        host_scale_check?1:0,
        homogeneous_app_version?1:0
    );
}

void DB_APP::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    strcpy2(name, r[i++]);
    min_version = atoi(r[i++]);
    deprecated = atoi(r[i++]);
    strcpy2(user_friendly_name, r[i++]);
    homogeneous_redundancy = atoi(r[i++]);
    weight = atof(r[i++]);
    beta = atoi(r[i++]);
    target_nresults = atoi(r[i++]);
    min_avg_pfc = atof(r[i++]);
    host_scale_check = (atoi(r[i++]) != 0);
    homogeneous_app_version = (atoi(r[i++]) != 0);
}

void DB_APP_VERSION::db_print(char* buf){
    sprintf(buf,
        "create_time=%d, "
        "appid=%d, "
        "version_num=%d, "
        "platformid=%d, "
        "xml_doc='%s', "
        "min_core_version=%d, "
        "max_core_version=%d, "
        "deprecated=%d, "
        "plan_class='%s', "
        "pfc_n=%.15e, "
        "pfc_avg=%.15e, "
        "pfc_scale=%.15e, "
        "expavg_credit=%.15e, "
        "expavg_time=%.15e ",
        create_time,
        appid,
        version_num,
        platformid,
        xml_doc,
        min_core_version,
        max_core_version,
        deprecated,
        plan_class,
        pfc.n,
        pfc.avg,
        pfc_scale,
        expavg_credit,
        expavg_time
    );
}

void DB_APP_VERSION::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    appid = atoi(r[i++]);
    version_num = atoi(r[i++]);
    platformid = atoi(r[i++]);
    strcpy2(xml_doc, r[i++]);
    min_core_version = atoi(r[i++]);
    max_core_version = atoi(r[i++]);
    deprecated = atoi(r[i++]);
    strcpy2(plan_class, r[i++]);
    pfc.n = atof(r[i++]);
    pfc.avg = atof(r[i++]);
    pfc_scale = atof(r[i++]);
    expavg_credit = atof(r[i++]);
    expavg_time = atof(r[i++]);
}

void DB_USER::db_print(char* buf){
    ESCAPE(email_addr);
    ESCAPE(name);
    ESCAPE(country);
    ESCAPE(postal_code);
    ESCAPE(global_prefs);
    ESCAPE(project_prefs);
    ESCAPE(url);
    ESCAPE(signature);
    sprintf(buf,
        "create_time=%d, email_addr='%s', name='%s', "
        "authenticator='%s', "
        "country='%s', postal_code='%s', "
        "total_credit=%.15e, expavg_credit=%.15e, expavg_time=%.15e, "
        "global_prefs='%s', project_prefs='%s', "
        "teamid=%d, venue='%s', url='%s', send_email=%d, show_hosts=%d, "
        "posts=%d, "
        "seti_id=%d, seti_nresults=%d, seti_last_result_time=%d, "
        "seti_total_cpu=%.15e, signature='%s', has_profile=%d, "
        "cross_project_id='%s', passwd_hash='%s', "
        "email_validated=%d, donated=%d",
        create_time, email_addr, name,
        authenticator,
        country, postal_code,
        total_credit, expavg_credit, expavg_time,
        global_prefs, project_prefs,
        teamid, venue, url, send_email, show_hosts,
        posts,
        seti_id, seti_nresults, seti_last_result_time,
        seti_total_cpu, signature, has_profile,
        cross_project_id, passwd_hash,
        email_validated, donated
    );
    UNESCAPE(email_addr);
    UNESCAPE(name);
    UNESCAPE(country);
    UNESCAPE(postal_code);
    UNESCAPE(global_prefs);
    UNESCAPE(project_prefs);
    UNESCAPE(url);
    UNESCAPE(signature);
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
    posts = safe_atoi(r[i++]);
    seti_id = safe_atoi(r[i++]);
    seti_nresults = safe_atoi(r[i++]);
    seti_last_result_time = safe_atoi(r[i++]);
    seti_total_cpu = safe_atof(r[i++]);
    strcpy2(signature, r[i++]);
    has_profile = atoi(r[i++]);
    strcpy2(cross_project_id, r[i++]);
    strcpy2(passwd_hash, r[i++]);
    email_validated = atoi(r[i++]);
    donated = atoi(r[i++]);
}

void DB_TEAM::db_print(char* buf){
    ESCAPE(name);
    ESCAPE(name_lc);
    ESCAPE(url);
    ESCAPE(name_html);
    ESCAPE(description);
    sprintf(buf,
        "create_time=%d, userid=%d, name='%s', "
        "name_lc='%s', url='%s', "
        "type=%d, name_html='%s', description='%s', nusers=%d, "
        "country='%s', "
        "total_credit=%.15e, expavg_credit=%.15e, expavg_time=%.15e, "
        "seti_id=%d, ping_user=%d, ping_time=%d",
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
        expavg_credit,
        expavg_time,
        seti_id,
        ping_user,
        ping_time
    );
    UNESCAPE(name);
    UNESCAPE(name_lc);
    UNESCAPE(url);
    UNESCAPE(name_html);
    UNESCAPE(description);
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
    expavg_time = atof(r[i++]);
    seti_id = safe_atoi(r[i++]);
    ping_user = safe_atoi(r[i++]);
    ping_time = safe_atoi(r[i++]);
}

// set NaNs and infs to zeroes
//
void HOST::fix_nans() {
    if (!finite(p_fpops)) p_fpops = 0;
    if (!finite(p_iops)) p_iops = 0;
    if (!finite(p_membw)) p_membw = 0;
    if (!finite(m_nbytes)) m_nbytes = 0;
    if (!finite(m_cache)) m_cache = 0;
    if (!finite(m_swap)) m_swap = 0;
    if (!finite(d_total)) d_total = 0;
    if (!finite(d_free)) d_free = 0;
    if (!finite(d_boinc_used_total)) d_boinc_used_total = 0;
    if (!finite(d_boinc_used_project)) d_boinc_used_project = 0;
    if (!finite(d_boinc_max)) d_boinc_max = 0;
    if (!finite(n_bwup)) n_bwup = 0;
    if (!finite(n_bwdown)) n_bwdown = 0;
}

void DB_HOST::db_print(char* buf){
    ESCAPE(domain_name);
    ESCAPE(serialnum);
    ESCAPE(last_ip_addr);
    ESCAPE(host_cpid);
    ESCAPE(p_vendor);
    ESCAPE(p_model);
    ESCAPE(os_name);
    ESCAPE(os_version);
    sprintf(buf,
        "create_time=%d, userid=%d, "
        "rpc_seqno=%d, rpc_time=%d, "
        "total_credit=%.12e, expavg_credit=%.12e, expavg_time=%.15e, "
        "timezone=%d, domain_name='%s', serialnum='%s', "
        "last_ip_addr='%s', nsame_ip_addr=%d, "
        "on_frac=%.15e, connected_frac=%.15e, "
        "active_frac=%.15e, cpu_efficiency=%.15e, "
        "duration_correction_factor=%.15e, "
        "p_ncpus=%d, p_vendor='%s', p_model='%s', "
        "p_fpops=%.15e, p_iops=%.15e, p_membw=%.15e, "
        "os_name='%s', os_version='%s', "
        "m_nbytes=%.15e, m_cache=%.15e, m_swap=%.15e, "
        "d_total=%.15e, d_free=%.15e, "
        "d_boinc_used_total=%.15e, d_boinc_used_project=%.15e, d_boinc_max=%.15e, "
        "n_bwup=%.15e, n_bwdown=%.15e, "
        "credit_per_cpu_sec=%.15e, "
        "venue='%s', nresults_today=%d, "
        "avg_turnaround=%.15e, "
        "host_cpid='%s', external_ip_addr='%s', max_results_day=%d, "
        "error_rate=%.15e ",
        create_time, userid,
        rpc_seqno, rpc_time,
        total_credit, expavg_credit, expavg_time,
        timezone, domain_name, serialnum,
        last_ip_addr, nsame_ip_addr,
        on_frac, connected_frac, active_frac,
        cpu_efficiency, duration_correction_factor,
        p_ncpus, p_vendor, p_model,
        p_fpops, p_iops, p_membw,
        os_name, os_version,
        m_nbytes, m_cache, m_swap,
        d_total, d_free,
        d_boinc_used_total, d_boinc_used_project, d_boinc_max,
        n_bwup, n_bwdown,
        credit_per_cpu_sec,
        venue, nresults_today,
        avg_turnaround,
        host_cpid, external_ip_addr, _max_results_day,
        _error_rate
    );
    UNESCAPE(domain_name);
    UNESCAPE(serialnum);
    UNESCAPE(last_ip_addr);
    UNESCAPE(p_vendor);
    UNESCAPE(p_model);
    UNESCAPE(os_name);
    UNESCAPE(os_version);
    UNESCAPE(host_cpid);
}

void DB_HOST::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
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
    cpu_efficiency = atof(r[i++]);
    duration_correction_factor = atof(r[i++]);
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
    nresults_today = atoi(r[i++]);
    avg_turnaround = atof(r[i++]);
    strcpy2(host_cpid, r[i++]);
    strcpy2(external_ip_addr, r[i++]);
    _max_results_day = atoi(r[i++]);
    _error_rate = atof(r[i++]);
}

int DB_HOST::update_diff_validator(HOST& h) {
    char buf[BLOB_SIZE], updates[BLOB_SIZE], query[BLOB_SIZE];
    strcpy(updates, "");
    if (avg_turnaround != h.avg_turnaround) {
        sprintf(buf, " avg_turnaround=%.15e,", avg_turnaround);
        strcat(updates, buf);
    }
#if 0
    if (error_rate != h.error_rate) {
        sprintf(buf, " error_rate=%.15e,", error_rate);
        strcat(updates, buf);
    }
#endif
    if (total_credit != h.total_credit) {
        sprintf(buf, " total_credit=total_credit+%.15e,",
            total_credit-h.total_credit
        );
        strcat(updates, buf);
    }
    if (expavg_credit != h.expavg_credit) {
        sprintf(buf, " expavg_credit=%.15e,", expavg_credit);
        strcat(updates, buf);
    }
    if (expavg_time != h.expavg_time) {
        sprintf(buf, " expavg_time=%.15e,", expavg_time);
        strcat(updates, buf);
    }
#if 0
    if (credit_per_cpu_sec != h.credit_per_cpu_sec) {
        sprintf(buf, " credit_per_cpu_sec=%.15e,", credit_per_cpu_sec);
        strcat(updates, buf);
    }
#endif
    int n = strlen(updates);
    if (n == 0) return 0;
    updates[n-1] = 0;        // trim the final comma
    sprintf(query, "update host set %s where id=%d", updates, id);
    return db->do_query(query);
}

// Update fields that differ from the argument HOST.
// Called from scheduler (handle_request.cpp),
// so only include fields modified by the scheduler.
//
int DB_HOST::update_diff_sched(HOST& h) {
    char buf[BLOB_SIZE], updates[BLOB_SIZE], query[BLOB_SIZE];
    strcpy(updates, "");
    if (rpc_seqno != h.rpc_seqno) {
        sprintf(buf, " rpc_seqno=%d,", rpc_seqno);
        strcat(updates, buf);
    }
    if (rpc_time != h.rpc_time) {
        sprintf(buf, " rpc_time=%d,", rpc_time);
        strcat(updates, buf);
    }
    if (timezone != h.timezone) {
        sprintf(buf, " timezone=%d,", timezone);
        strcat(updates, buf);
    }
    if (strcmp(domain_name, h.domain_name)) {
        escape_string(domain_name, sizeof(domain_name));
        sprintf(buf, " domain_name='%s',", domain_name);
        unescape_string(domain_name, sizeof(domain_name));
        strcat(updates, buf);
    }
    if (strcmp(serialnum, h.serialnum)) {
        escape_string(serialnum, sizeof(serialnum));
        sprintf(buf, " serialnum='%s',", serialnum);
        unescape_string(serialnum, sizeof(serialnum));
        strcat(updates, buf);
    }
    if (strcmp(last_ip_addr, h.last_ip_addr)) {
        escape_string(last_ip_addr, sizeof(last_ip_addr));
        sprintf(buf, " last_ip_addr='%s',", last_ip_addr);
        unescape_string(last_ip_addr, sizeof(last_ip_addr));
        strcat(updates, buf);
    }
    if (nsame_ip_addr != h.nsame_ip_addr) {
        sprintf(buf, " nsame_ip_addr=%d,", nsame_ip_addr);
        strcat(updates, buf);
    }
    if (on_frac != h.on_frac) {
        sprintf(buf, " on_frac=%.15e,", on_frac);
        strcat(updates, buf);
    }
    if (connected_frac != h.connected_frac) {
        sprintf(buf, " connected_frac=%.15e,", connected_frac);
        strcat(updates, buf);
    }
    if (active_frac != h.active_frac) {
        sprintf(buf, " active_frac=%.15e,", active_frac);
        strcat(updates, buf);
    }
    if (cpu_efficiency != h.cpu_efficiency) {
        sprintf(buf, " cpu_efficiency=%.15e,", cpu_efficiency);
        strcat(updates, buf);
    }
    if (duration_correction_factor != h.duration_correction_factor) {
        sprintf(buf, " duration_correction_factor=%.15e,", duration_correction_factor);
        strcat(updates, buf);
    }
    if (p_ncpus != h.p_ncpus) {
        sprintf(buf, " p_ncpus=%d,", p_ncpus);
        strcat(updates, buf);
    }
    if (strcmp(p_vendor, h.p_vendor)) {
        escape_string(p_vendor, sizeof(p_vendor));
        sprintf(buf, " p_vendor='%s',", p_vendor);
        unescape_string(p_vendor, sizeof(p_vendor));
        strcat(updates, buf);
    }
    if (strcmp(p_model, h.p_model)) {
        escape_string(p_model, sizeof(p_model));
        sprintf(buf, " p_model='%s',", p_model);
        unescape_string(p_model, sizeof(p_model));
        strcat(updates, buf);
    }
    if (p_fpops != h.p_fpops) {
        sprintf(buf, " p_fpops=%.15e,", p_fpops);
        strcat(updates, buf);
    }
    if (p_iops != h.p_iops) {
        sprintf(buf, " p_iops=%.15e,", p_iops);
        strcat(updates, buf);
    }
    if (p_membw != h.p_membw) {
        sprintf(buf, " p_membw=%.15e,", p_membw);
        strcat(updates, buf);
    }
    if (strcmp(os_name, h.os_name)) {
        escape_string(os_name, sizeof(os_name));
        sprintf(buf, " os_name='%s',", os_name);
        unescape_string(os_name, sizeof(os_name));
        strcat(updates, buf);
    }
    if (strcmp(os_version, h.os_version)) {
        escape_string(os_version, sizeof(os_version));
        sprintf(buf, " os_version='%s',", os_version);
        unescape_string(os_version, sizeof(os_version));
        strcat(updates, buf);
    }
    if (m_nbytes != h.m_nbytes) {
        sprintf(buf, " m_nbytes=%.15e,", m_nbytes);
        strcat(updates, buf);
    }
    if (m_cache != h.m_cache) {
        sprintf(buf, " m_cache=%.15e,", m_cache);
        strcat(updates, buf);
    }
    if (m_swap != h.m_swap) {
        sprintf(buf, " m_swap=%.15e,", m_swap);
        strcat(updates, buf);
    }
    if (d_total != h.d_total) {
        sprintf(buf, " d_total=%.15e,", d_total);
        strcat(updates, buf);
    }
    if (d_free != h.d_free) {
        sprintf(buf, " d_free=%.15e,", d_free);
        strcat(updates, buf);
    }
    if (d_boinc_used_total != h.d_boinc_used_total) {
        sprintf(buf, " d_boinc_used_total=%.15e,", d_boinc_used_total);
        strcat(updates, buf);
    }
    if (d_boinc_used_project != h.d_boinc_used_project) {
        sprintf(buf, " d_boinc_used_project=%.15e,", d_boinc_used_project);
        strcat(updates, buf);
    }
    if (d_boinc_max != h.d_boinc_max) {
        sprintf(buf, " d_boinc_max=%.15e,", d_boinc_max);
        strcat(updates, buf);
    }
    if (n_bwdown != h.n_bwdown) {
        sprintf(buf, " n_bwdown=%.15e,", n_bwdown);
        strcat(updates, buf);
    }
    if (n_bwup != h.n_bwup) {
        sprintf(buf, " n_bwup=%.15e,", n_bwup);
        strcat(updates, buf);
    }
    if (strcmp(venue, h.venue)) {
        escape_string(venue, sizeof(venue));
        sprintf(buf, " venue='%s',", venue);
        unescape_string(venue, sizeof(venue));
        strcat(updates, buf);
    }
    if (nresults_today != h.nresults_today) {
        sprintf(buf, " nresults_today=%d,", nresults_today);
        strcat(updates, buf);
    }
    if (avg_turnaround != h.avg_turnaround) {
        sprintf(buf, " avg_turnaround=%.15e,", avg_turnaround);
        strcat(updates, buf);
    }
    if (strcmp(host_cpid, h.host_cpid)) {
        escape_string(host_cpid, sizeof(host_cpid));
        sprintf(buf, " host_cpid='%s',", host_cpid);
        unescape_string(host_cpid, sizeof(host_cpid));
        strcat(updates, buf);
    }
    if (strcmp(external_ip_addr, h.external_ip_addr)) {
        escape_string(external_ip_addr, sizeof(external_ip_addr));
        sprintf(buf, " external_ip_addr='%s',", external_ip_addr);
        unescape_string(external_ip_addr, sizeof(external_ip_addr));
        strcat(updates, buf);
    }
#if 0
    if (max_results_day != h.max_results_day) {
        sprintf(buf, " max_results_day=%d,", max_results_day);
        strcat(updates, buf);
    }
#endif

    int n = strlen(updates);
    if (n == 0) return 0;
    updates[n-1] = 0;        // trim the final comma
    sprintf(query, "update host set %s where id=%d", updates, id);
    return db->do_query(query);
}

int DB_HOST::fpops_percentile(double percentile, double& fpops) {
    char query[256];
    int n, retval;

    sprintf(query, "where expavg_credit>10");
    retval = count(n, query);
    if (retval) return retval;
    if (n==0) return ERR_NULL;
    int m = (int)(n*percentile/100.);
    sprintf(query,
        "select p_fpops from host where expavg_credit>10 order by p_fpops limit %d,1",
        m
    );
    return get_double(query, fpops);
}

int DB_HOST::fpops_mean(double& mean) {
    char query[256];
    sprintf(query,
        "select avg(p_fpops) from host where expavg_credit>10"
    );
    return get_double(query, mean);
}

int DB_HOST::fpops_stddev(double& stddev) {
    char query[256];
    sprintf(query,
        "select stddev(p_fpops) from host where expavg_credit>10"
    );
    return get_double(query, stddev);
}

void DB_WORKUNIT::db_print(char* buf){
    sprintf(buf,
        "create_time=%d, appid=%d, "
        "name='%s', xml_doc='%s', batch=%d, "
        "rsc_fpops_est=%.15e, rsc_fpops_bound=%.15e, "
        "rsc_memory_bound=%.15e, rsc_disk_bound=%.15e, "
        "need_validate=%d, "
        "canonical_resultid=%u, canonical_credit=%.15e, "
        "transition_time=%d, delay_bound=%d, "
        "error_mask=%d, file_delete_state=%d, assimilate_state=%d, "
        "hr_class=%d, opaque=%.15e, "
        "min_quorum=%d, target_nresults=%d, max_error_results=%d, "
        "max_total_results=%d, max_success_results=%d, "
        "result_template_file='%s', "
        "priority=%d, "
        "rsc_bandwidth_bound=%.15e, "
        "fileset_id=%d, "
        "app_version_id=%d, "
        "transitioner_flags=%d ",
        create_time, appid,
        name, xml_doc, batch,
        rsc_fpops_est, rsc_fpops_bound, rsc_memory_bound, rsc_disk_bound,
        need_validate,
        canonical_resultid, canonical_credit,
        transition_time, delay_bound,
        error_mask, file_delete_state, assimilate_state,
        hr_class, opaque,
        min_quorum,
        target_nresults,
        max_error_results,
        max_total_results,
        max_success_results,
        result_template_file,
        priority,
        rsc_bandwidth_bound,
        fileset_id,
        app_version_id,
        transitioner_flags
    );
}

void DB_WORKUNIT::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    appid = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(xml_doc, r[i++]);
    batch = atoi(r[i++]);
    rsc_fpops_est = atof(r[i++]);
    rsc_fpops_bound = atof(r[i++]);
    rsc_memory_bound = atof(r[i++]);
    rsc_disk_bound = atof(r[i++]);
    need_validate = atoi(r[i++]);
    canonical_resultid = atoi(r[i++]);
    canonical_credit = atof(r[i++]);
    transition_time = atoi(r[i++]);
    delay_bound = atoi(r[i++]);
    error_mask = atoi(r[i++]);
    file_delete_state = atoi(r[i++]);
    assimilate_state = atoi(r[i++]);
    hr_class = atoi(r[i++]);
    opaque = atof(r[i++]);
    min_quorum = atoi(r[i++]);
    target_nresults = atoi(r[i++]);
    max_error_results = atoi(r[i++]);
    max_total_results = atoi(r[i++]);
    max_success_results = atoi(r[i++]);
    strcpy2(result_template_file, r[i++]);
    priority = atoi(r[i++]);
    strcpy2(mod_time, r[i++]);
    rsc_bandwidth_bound = atof(r[i++]);
    fileset_id = atoi(r[i++]);
    app_version_id = atoi(r[i++]);
    transitioner_flags = atoi(r[i++]);
}

void DB_CREDITED_JOB::db_print(char* buf){
    sprintf(buf,
        "userid=%d, workunitid=%f",
        userid, workunitid
    );
}

void DB_CREDITED_JOB::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    userid = atoi(r[i++]);
    workunitid = atof(r[i++]);
};

void DB_RESULT::db_print(char* buf){
    ESCAPE(xml_doc_out);
    ESCAPE(stderr_out);
    sprintf(
        buf,
        "create_time=%d, workunitid=%d, "
        "server_state=%d, outcome=%d, client_state=%d, "
        "hostid=%d, userid=%d, "
        "report_deadline=%d, sent_time=%d, received_time=%d, "
        "name='%s', cpu_time=%.15e, "
        "xml_doc_in='%s', xml_doc_out='%s', stderr_out='%s', "
        "batch=%d, file_delete_state=%d, validate_state=%d, "
        "claimed_credit=%.15e, granted_credit=%.15e, opaque=%.15e, random=%d, "
        "app_version_num=%d, appid=%d, exit_status=%d, teamid=%d, "
        "priority=%d, mod_time=null, elapsed_time=%.15e, flops_estimate=%.15e, "
        "app_version_id=%d, runtime_outlier=%d",
        create_time, workunitid,
        server_state, outcome, client_state,
        hostid, userid,
        report_deadline, sent_time, received_time,
        name, cpu_time,
        xml_doc_in, xml_doc_out, stderr_out,
        batch, file_delete_state, validate_state,
        claimed_credit, granted_credit, opaque, random,
        app_version_num, appid, exit_status, teamid,
        priority, elapsed_time, flops_estimate,
        app_version_id,
        runtime_outlier?1:0
    );
    UNESCAPE(xml_doc_out);
    UNESCAPE(stderr_out);
}

// the following used for "batch insert" from transitioner
//
void DB_RESULT::db_print_values(char* buf){
    ESCAPE(xml_doc_out);
    ESCAPE(stderr_out);
    sprintf(
        buf,
        "(0, %d, %d, "
        "%d, %d, %d, "
        "%d, %d, "
        "%d, %d, %d, "
        "'%s', %.15e, "
        "'%s', '%s', '%s', "
        "%d, %d, %d, "
        "%.15e, %.15e, %.15e, %d, "
        "%d, %d, %d, %d, %d, null, 0, 0, 0, 0)",
        create_time, workunitid,
        server_state, outcome, client_state,
        hostid, userid,
        report_deadline, sent_time, received_time,
        name, cpu_time,
        xml_doc_in, xml_doc_out, stderr_out,
        batch, file_delete_state, validate_state,
        claimed_credit, granted_credit, opaque, random,
        app_version_num, appid, exit_status, teamid, priority
    );
    UNESCAPE(xml_doc_out);
    UNESCAPE(stderr_out);
}

// called from scheduler when dispatch this result.
// The "... and server_state=%d" is a safeguard against
// the case where another scheduler tries to send this result at the same time
//
int DB_RESULT::mark_as_sent(int old_server_state, int report_grace_period) {
    char query[MAX_QUERY_LEN];
    int retval;

    sprintf(query,
        "update result set server_state=%d, hostid=%d, userid=%d, sent_time=%d, report_deadline=%d, flops_estimate=%.15e, app_version_id=%d  where id=%u and server_state=%d",
        server_state,
        hostid,
        userid,
        sent_time,
        report_deadline + report_grace_period,
        flops_estimate,
        app_version_id,
        id,
        old_server_state
    );
    retval = db->do_query(query);
    if (retval) return retval;
    if (db->affected_rows() != 1) return ERR_DB_NOT_FOUND;
    return 0;
}

void DB_RESULT::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    workunitid = atoi(r[i++]);
    server_state = atoi(r[i++]);
    outcome = atoi(r[i++]);
    client_state = atoi(r[i++]);
    hostid = atoi(r[i++]);
    userid = atoi(r[i++]);
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
    opaque = atof(r[i++]);
    random = atoi(r[i++]);
    app_version_num = atoi(r[i++]);
    appid = atoi(r[i++]);
    exit_status = atoi(r[i++]);
    teamid = atoi(r[i++]);
    priority = atoi(r[i++]);
    strcpy2(mod_time, r[i++]);
    elapsed_time = atof(r[i++]);
    flops_estimate = atof(r[i++]);
    app_version_id = atoi(r[i++]);
    runtime_outlier = (atoi(r[i++]) != 0);
}

void DB_MSG_FROM_HOST::db_print(char* buf) {
    ESCAPE(xml);
    sprintf(buf,
        "create_time=%d, "
        "hostid=%d, variety='%s', "
        "handled=%d, xml='%s'",

        create_time,
        hostid, variety,
        handled, xml

    );
    UNESCAPE(xml);
}

void DB_MSG_FROM_HOST::db_parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    hostid = atoi(r[i++]);
    strcpy2(variety, r[i++]);
    handled = atoi(r[i++]);
    strcpy2(xml, r[i++]);
}

void DB_MSG_TO_HOST::db_print(char* buf) {
    ESCAPE(xml);
    sprintf(buf,
        "create_time=%d, "
        "hostid=%d, variety='%s', "
        "handled=%d, xml='%s'",
        create_time,
        hostid, variety,
        handled, xml
    );
    UNESCAPE(xml);
}

void DB_MSG_TO_HOST::db_parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    hostid = atoi(r[i++]);
    strcpy2(variety, r[i++]);
    handled = atoi(r[i++]);
    strcpy2(xml, r[i++]);
}

void DB_ASSIGNMENT::db_print(char* buf) {
    sprintf(buf,
        "create_time=%d, "
        "target_id=%d, "
        "target_type=%d, "
        "multi=%d, "
        "workunitid=%d, "
        "resultid=%u",
        create_time,
        target_id,
        target_type,
        multi,
        workunitid,
        _resultid
    );
}

void DB_ASSIGNMENT::db_parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    target_id = atoi(r[i++]);
    target_type = atoi(r[i++]);
    multi = atoi(r[i++]);
    workunitid = atoi(r[i++]);
    _resultid = atoi(r[i++]);
}

int DB_HOST_APP_VERSION::update_scheduler(DB_HOST_APP_VERSION& orig) {
    char query[1024], clause[512];

    if (consecutive_valid == orig.consecutive_valid
        && max_jobs_per_day == orig.max_jobs_per_day
        && n_jobs_today == orig.n_jobs_today
    ) {
        return 0;
    }
    sprintf(query,
        "consecutive_valid=%d, max_jobs_per_day=%d, n_jobs_today=%d",
        consecutive_valid,
        max_jobs_per_day,
        n_jobs_today
    );
    sprintf(clause, "host_id=%d and app_version_id=%d", host_id, app_version_id);
    return update_fields_noid(query, clause);
}

int DB_HOST_APP_VERSION::update_validator(DB_HOST_APP_VERSION& orig) {
    char query[8192], clause[512];

    if (pfc.n == orig.pfc.n
        && pfc.avg == orig.pfc.avg
        && et.n == orig.et.n
        && et.avg == orig.et.avg
        && et.q == orig.et.q
        && et.var == orig.et.var
        && turnaround.n == orig.turnaround.n
        && turnaround.avg == orig.turnaround.avg
        && turnaround.q == orig.turnaround.q
        && turnaround.var == orig.turnaround.var
        && consecutive_valid == orig.consecutive_valid
        && max_jobs_per_day == orig.max_jobs_per_day
    ) {
        return 0;
    }
    sprintf(query,
        "pfc_n=%.15e, "
        "pfc_avg=%.15e, "
        "et_n=%.15e, "
        "et_avg=%.15e, "
        "et_q=%.15e, "
        "et_var=%.15e, "
        "turnaround_n=%.15e, "
        "turnaround_avg=%.15e, "
        "turnaround_q=%.15e, "
        "turnaround_var=%.15e, "
        "consecutive_valid=%d, "
        "max_jobs_per_day=%d ",
        pfc.n,
        pfc.avg,
        et.n,
        et.avg,
        et.q,
        et.var,
        turnaround.n,
        turnaround.avg,
        turnaround.q,
        turnaround.var,
        consecutive_valid,
        max_jobs_per_day
    );
    sprintf(clause,
        "host_id=%d and app_version_id=%d ",
        host_id, app_version_id
    );
    return update_fields_noid(query, clause);
}

void DB_HOST_APP_VERSION::db_print(char* buf) {
    sprintf(buf,
        "host_id=%d, "
        "app_version_id=%d, "
        "pfc_n=%.15e, "
        "pfc_avg=%.15e, "
        "et_n=%.15e, "
        "et_avg=%.15e, "
        "et_var=%.15e, "
        "et_q=%.15e, "
        "max_jobs_per_day=%d, "
        "n_jobs_today=%d, "
        "turnaround_n=%.15e, "
        "turnaround_avg=%.15e, "
        "turnaround_var=%.15e, "
        "turnaround_q=%.15e, "
        "consecutive_valid=%d ",
        host_id,
        app_version_id,
        pfc.n,
        pfc.avg,
        et.n,
        et.avg,
        et.var,
        et.q,
        max_jobs_per_day,
        n_jobs_today,
        turnaround.n,
        turnaround.avg,
        turnaround.var,
        turnaround.q,
        consecutive_valid
    );
}

void DB_HOST_APP_VERSION::db_parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    host_id = atoi(r[i++]);
    app_version_id = atoi(r[i++]);
    pfc.n = atof(r[i++]);
    pfc.avg = atof(r[i++]);
    et.n = atof(r[i++]);
    et.avg = atof(r[i++]);
    et.var = atof(r[i++]);
    et.q = atof(r[i++]);
    max_jobs_per_day = atoi(r[i++]);
    n_jobs_today = atoi(r[i++]);
    turnaround.n = atof(r[i++]);
    turnaround.avg = atof(r[i++]);
    turnaround.var = atof(r[i++]);
    turnaround.q = atof(r[i++]);
    consecutive_valid = atoi(r[i++]);
}

void DB_STATE_COUNTS::db_print(char* buf) {
    sprintf(buf,
        "appid=%d, "
        "last_update_time=%d, "
        "result_server_state_2=%d, "
        "result_server_state_4=%d, "
        "result_file_delete_state_1=%d, "
        "result_file_delete_state_2=%d, "
        "result_server_state_5_and_file_delete_state_0=%d, "
        "workunit_need_validate_1=%d, "
        "workunit_assimilate_state_1=%d, "
        "workunit_file_delete_state_1=%d, "
        "workunit_file_delete_state_2=%d, ",
        appid,
        last_update_time,   
        result_server_state_2,      
        result_server_state_4,      
        result_file_delete_state_1, 
        result_file_delete_state_2, 
        result_server_state_5_and_file_delete_state_0,      
        workunit_need_validate_1,   
        workunit_assimilate_state_1,
        workunit_file_delete_state_1,       
        workunit_file_delete_state_2  
    );
}

void DB_STATE_COUNTS::db_parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    appid = atoi(r[i++]);
    last_update_time = atoi(r[i++]);
    result_server_state_2 = atoi(r[i++]);      
    result_server_state_4 = atoi(r[i++]);      
    result_file_delete_state_1 = atoi(r[i++]); 
    result_file_delete_state_2 = atoi(r[i++]); 
    result_server_state_5_and_file_delete_state_0 = atoi(r[i++]);      
    workunit_need_validate_1 = atoi(r[i++]);   
    workunit_assimilate_state_1 = atoi(r[i++]);
    workunit_file_delete_state_1 = atoi(r[i++]);       
    workunit_file_delete_state_2 = atoi(r[i++]);
}

void TRANSITIONER_ITEM::parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    strcpy2(name, r[i++]);
    appid = atoi(r[i++]);
    min_quorum = atoi(r[i++]);
    need_validate = atoi(r[i++]);
    canonical_resultid = atoi(r[i++]);
    transition_time = atoi(r[i++]);
    delay_bound = atoi(r[i++]);
    error_mask = atoi(r[i++]);
    max_error_results = atoi(r[i++]);
    max_total_results = atoi(r[i++]);
    file_delete_state = atoi(r[i++]);
    assimilate_state = atoi(r[i++]);
    target_nresults = atoi(r[i++]);
    strcpy2(result_template_file, r[i++]);
    priority = atoi(r[i++]);
    hr_class = atoi(r[i++]);
    batch = atoi(r[i++]);
    app_version_id = atoi(r[i++]);
    transitioner_flags = atoi(r[i++]);

    // use safe_atoi() from here on cuz they might not be there
    //
    res_id = safe_atoi(r[i++]);
    strcpy2(res_name, r[i++]);
    res_report_deadline = safe_atoi(r[i++]);
    res_server_state = safe_atoi(r[i++]);
    res_outcome = safe_atoi(r[i++]);
    res_validate_state = safe_atoi(r[i++]);
    res_file_delete_state = safe_atoi(r[i++]);
    res_sent_time = safe_atoi(r[i++]);
    res_hostid = safe_atoi(r[i++]);
    res_received_time = safe_atoi(r[i++]);
    res_app_version_id = safe_atoi(r[i++]);
}

int DB_TRANSITIONER_ITEM_SET::enumerate(
    int transition_time, int nresult_limit,
    int wu_id_modulus, int wu_id_remainder,
    std::vector<TRANSITIONER_ITEM>& items
) {
    int retval;
    char query[MAX_QUERY_LEN];
    char mod_clause[256];;
    MYSQL_ROW row;
    TRANSITIONER_ITEM new_item;

    if (!cursor.active) {
        if (wu_id_modulus) {
            sprintf(mod_clause,
                " and wu.id %% %d = %d ",
                wu_id_modulus, wu_id_remainder
            );
        } else {
            strcpy(mod_clause, "");
        }


        sprintf(query,
            "SELECT "
            "   wu.id, "
            "   wu.name, "
            "   wu.appid, "
            "   wu.min_quorum, "
            "   wu.need_validate, "
            "   wu.canonical_resultid, "
            "   wu.transition_time, "
            "   wu.delay_bound, "
            "   wu.error_mask, "
            "   wu.max_error_results, "
            "   wu.max_total_results, "
            "   wu.file_delete_state, "
            "   wu.assimilate_state, "
            "   wu.target_nresults, "
            "   wu.result_template_file, "
            "   wu.priority, "
            "   wu.hr_class, "
            "   wu.batch, "
            "   wu.app_version_id, "
            "   wu.transitioner_flags, "
            "   res.id, "
            "   res.name, "
            "   res.report_deadline, "
            "   res.server_state, "
            "   res.outcome, "
            "   res.validate_state, "
            "   res.file_delete_state, "
            "   res.sent_time, "
            "   res.hostid, "
            "   res.received_time, "
            "   res.app_version_id "
            "FROM "
            "   workunit AS wu "
            "       LEFT JOIN result AS res ON wu.id = res.workunitid "
            "WHERE "
            "   wu.transition_time < %d %s and transitioner_flags<>%d "
            "LIMIT "
            "   %d ",
            transition_time, mod_clause, TRANSITION_NONE, nresult_limit
        );

        retval = db->do_query(query);
        if (retval) return mysql_errno(db->mysql);

        // the following stores the entire result set in memory
        //
        cursor.rp = mysql_store_result(db->mysql);
        if (!cursor.rp) return mysql_errno(db->mysql);
        cursor.active = true;

        row = mysql_fetch_row(cursor.rp);
        if (!row) {
            mysql_free_result(cursor.rp);
            cursor.active = false;
            retval = mysql_errno(db->mysql);
            if (retval) return ERR_DB_CONN_LOST;
            return ERR_DB_NOT_FOUND;
        }
        last_item.parse(row);
        nitems_this_query = 1;
    }

    items.clear();
    while (true) {
        items.push_back(last_item);
        row = mysql_fetch_row(cursor.rp);
        if (!row) {
            mysql_free_result(cursor.rp);
            cursor.active = false;

            // if got fewer rows than requested, last group is complete
            //
            if (nitems_this_query < nresult_limit) {
                return 0;
            } else {
                return ERR_DB_NOT_FOUND;
            }
        }
        new_item.parse(row);
        nitems_this_query++;
        if (new_item.id != last_item.id) {
            last_item = new_item;
            return 0;
        }
        last_item = new_item;
    }

    return 0;
}

int DB_TRANSITIONER_ITEM_SET::update_result(TRANSITIONER_ITEM& ti) {
    char query[MAX_QUERY_LEN];

    sprintf(query,
        "update result set server_state=%d, outcome=%d, "
        "validate_state=%d, file_delete_state=%d where id=%u",
        ti.res_server_state,
        ti.res_outcome,
        ti.res_validate_state,
        ti.res_file_delete_state,
        ti.res_id
    );
    int retval = db->do_query(query);
    if (db->affected_rows() != 1) return ERR_DB_NOT_FOUND;
    return retval;
}

int DB_TRANSITIONER_ITEM_SET::update_workunit(
    TRANSITIONER_ITEM& ti, TRANSITIONER_ITEM& ti_original
) {
    char query[MAX_QUERY_LEN];
    char updates[4096], buf[256];

    strcpy(updates, "");
    if (ti.need_validate != ti_original.need_validate) {
        sprintf(buf, " need_validate=%d,", ti.need_validate);
        strcat(updates, buf);
    }
    if (ti.error_mask != ti_original.error_mask) {
        sprintf(buf, " error_mask=%d,", ti.error_mask);
        strcat(updates, buf);
    }
    if (ti.assimilate_state != ti_original.assimilate_state) {
        sprintf(buf, " assimilate_state=%d,", ti.assimilate_state);
        strcat(updates, buf);
    }
    if (ti.file_delete_state != ti_original.file_delete_state) {
        sprintf(buf, " file_delete_state=%d,", ti.file_delete_state);
        strcat(updates, buf);
    }
    if (ti.transition_time != ti_original.transition_time) {
        sprintf(buf, " transition_time=%d,", ti.transition_time);
        strcat(updates, buf);
    }
    if (ti.hr_class != ti_original.hr_class) {
        sprintf(buf, " hr_class=%d,", ti.hr_class);
        strcat(updates, buf);
    }
    if (ti.app_version_id != ti_original.app_version_id) {
        sprintf(buf, " app_version_id=%d,", ti.app_version_id);
        strcat(updates, buf);
    }
    int n = strlen(updates);
    if (n == 0) {
        return 0;
    }

    // trim the final comma
    //
    updates[n-1] = 0;

    sprintf(query, "update workunit set %s where id=%d", updates, ti.id);
    return db->do_query(query);
}

void VALIDATOR_ITEM::parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    wu.id = atoi(r[i++]);
    strcpy2(wu.name, r[i++]);
    wu.canonical_resultid = atoi(r[i++]);
    wu.canonical_credit = atof(r[i++]);
    wu.min_quorum = atoi(r[i++]);
    wu.assimilate_state = atoi(r[i++]);
    wu.transition_time = atoi(r[i++]);
    wu.opaque = atof(r[i++]);
    wu.batch = atoi(r[i++]);
    wu.target_nresults = atoi(r[i++]);
    wu.max_success_results = atoi(r[i++]);
    wu.error_mask = atoi(r[i++]);
    wu.rsc_fpops_est = atof(r[i++]);
    wu.rsc_fpops_bound = atof(r[i++]);

    res.id = atoi(r[i++]);
    strcpy2(res.name, r[i++]);
    res.validate_state = atoi(r[i++]);
    res.server_state = atoi(r[i++]);
    res.outcome = atoi(r[i++]);
    res.granted_credit = atof(r[i++]);
    strcpy2(res.xml_doc_in, r[i++]);
    strcpy2(res.xml_doc_out, r[i++]);
    strcpy2(res.stderr_out, r[i++]);
    res.cpu_time = atof(r[i++]);
    res.batch = atoi(r[i++]);
    res.opaque = atof(r[i++]);
    res.exit_status = atoi(r[i++]);
    res.hostid = atoi(r[i++]);
    res.userid = atoi(r[i++]);
    res.teamid = atoi(r[i++]);
    res.sent_time = atoi(r[i++]);
    res.received_time = atoi(r[i++]);
    res.appid = atoi(r[i++]);
    res.app_version_id = atoi(r[i++]);
    res.app_version_num = atoi(r[i++]);
    res.elapsed_time = atof(r[i++]);
    res.flops_estimate = atof(r[i++]);
    res.app_version_id = atoi(r[i++]);
    res.runtime_outlier = (atoi(r[i++]) != 0);
}

int DB_VALIDATOR_ITEM_SET::enumerate(
    int appid, int nresult_limit,
    int wu_id_modulus, int wu_id_remainder,
    std::vector<VALIDATOR_ITEM>& items
) {
    int retval;
    char query[MAX_QUERY_LEN], mod_clause[256];
    MYSQL_ROW row;
    VALIDATOR_ITEM new_item;

    if (!cursor.active) {
        if (wu_id_modulus) {
            sprintf(mod_clause,
                " and wu.id %% %d = %d ",
                wu_id_modulus, wu_id_remainder
            );
        } else {
            strcpy(mod_clause, "");
        }

        sprintf(query,
            "SELECT "
            "   wu.id, "
            "   wu.name, "
            "   wu.canonical_resultid, "
            "   wu.canonical_credit, "
            "   wu.min_quorum, "
            "   wu.assimilate_state, "
            "   wu.transition_time, "
            "   wu.opaque, "
            "   wu.batch, "
            "   wu.target_nresults, "
            "   wu.max_success_results, "
            "   wu.error_mask, "
            "   wu.rsc_fpops_est, "
            "   wu.rsc_fpops_bound, "
            "   res.id, "
            "   res.name, "
            "   res.validate_state, "
            "   res.server_state, "
            "   res.outcome, "
            "   res.granted_credit, "
            "   res.xml_doc_in, "
            "   res.xml_doc_out, "
            "   res.stderr_out, "
            "   res.cpu_time, "
            "   res.batch, "
            "   res.opaque, "
            "   res.exit_status, "
            "   res.hostid, "
            "   res.userid, "
            "   res.teamid, "
            "   res.sent_time, "
            "   res.received_time, "
            "   res.appid, "
            "   res.app_version_id, "
            "   res.app_version_num, "
            "   res.elapsed_time, "
            "   res.flops_estimate, "
            "   res.app_version_id, "
            "   res.runtime_outlier "
            "FROM "
            "   workunit AS wu, result AS res where wu.id = res.workunitid "
            "   and wu.appid = %d and wu.need_validate > 0 %s "
            "LIMIT "
            "   %d ",
            appid, mod_clause, nresult_limit
        );

        retval = db->do_query(query);
        if (retval) return mysql_errno(db->mysql);

        // the following stores the entire result set in memory
        cursor.rp = mysql_store_result(db->mysql);
        if (!cursor.rp) return mysql_errno(db->mysql);
        cursor.active = true;

        row = mysql_fetch_row(cursor.rp);
        if (!row) {
            mysql_free_result(cursor.rp);
            cursor.active = false;
            retval = mysql_errno(db->mysql);
            if (retval) return ERR_DB_CONN_LOST;
            return ERR_DB_NOT_FOUND;
        }
        last_item.parse(row);
        nitems_this_query = 1;
    }

    items.clear();
    while (true) {
        items.push_back(last_item);
        row = mysql_fetch_row(cursor.rp);
        if (!row) {
            mysql_free_result(cursor.rp);
            cursor.active = false;

            // if got fewer rows than requested, last group is complete
            //
            if (nitems_this_query < nresult_limit) {
                return 0;
            } else {
                return ERR_DB_NOT_FOUND;
            }
        }
        new_item.parse(row);
        nitems_this_query++;
        if (new_item.wu.id != last_item.wu.id) {
            last_item = new_item;
            return 0;
        }
        last_item = new_item;
    }

    return 0;
}

int DB_VALIDATOR_ITEM_SET::update_result(RESULT& res) {
    char query[MAX_QUERY_LEN];

    sprintf(query,
        "update result set validate_state=%d, granted_credit=%.15e, "
        "server_state=%d, outcome=%d, opaque=%lf, random=%d, runtime_outlier=%d "
        "where id=%u",
        res.validate_state,
        res.granted_credit,
        res.server_state,
        res.outcome,
        res.opaque,
        res.random,
        res.runtime_outlier?1:0,
        res.id
    );
    int retval = db->do_query(query);
    if (db->affected_rows() != 1) return ERR_DB_NOT_FOUND;
    return retval;
}


int DB_VALIDATOR_ITEM_SET::update_workunit(WORKUNIT& wu) {
    char query[MAX_QUERY_LEN];

    sprintf(query,
        "update workunit set need_validate=0, error_mask=%d, "
        "assimilate_state=%d, transition_time=%d, "
        "target_nresults=%d, "
        "canonical_resultid=%u, canonical_credit=%.15e "
        "where id=%d",
        wu.error_mask,
        wu.assimilate_state,
        wu.transition_time,
        wu.target_nresults,
        wu.canonical_resultid,
        wu.canonical_credit,
        wu.id
    );
    int retval = db->do_query(query);
    if (db->affected_rows() != 1) return ERR_DB_NOT_FOUND;
    return retval;
}

void WORK_ITEM::parse(MYSQL_ROW& r) {
    int i=0;
    memset(this, 0, sizeof(WORK_ITEM));
    res_id = atoi(r[i++]);
    res_priority = atoi(r[i++]);
    res_server_state = atoi(r[i++]);
    res_report_deadline = atof(r[i++]);
    wu.id = atoi(r[i++]);
    wu.create_time = atoi(r[i++]);
    wu.appid = atoi(r[i++]);
    strcpy2(wu.name, r[i++]);
    strcpy2(wu.xml_doc, r[i++]);
    wu.batch = atoi(r[i++]);
    wu.rsc_fpops_est = atof(r[i++]);
    wu.rsc_fpops_bound = atof(r[i++]);
    wu.rsc_memory_bound = atof(r[i++]);
    wu.rsc_disk_bound = atof(r[i++]);
    wu.need_validate = atoi(r[i++]);
    wu.canonical_resultid = atoi(r[i++]);
    wu.canonical_credit = atof(r[i++]);
    wu.transition_time = atoi(r[i++]);
    wu.delay_bound = atoi(r[i++]);
    wu.error_mask = atoi(r[i++]);
    wu.file_delete_state = atoi(r[i++]);
    wu.assimilate_state = atoi(r[i++]);
    wu.hr_class = atoi(r[i++]);
    wu.opaque = atof(r[i++]);
    wu.min_quorum = atoi(r[i++]);
    wu.target_nresults = atoi(r[i++]);
    wu.max_error_results = atoi(r[i++]);
    wu.max_total_results = atoi(r[i++]);
    wu.max_success_results = atoi(r[i++]);
    strcpy2(wu.result_template_file, r[i++]);
    wu.priority = atoi(r[i++]);
    strcpy2(wu.mod_time, r[i++]);
    wu.rsc_bandwidth_bound = atof(r[i++]);
    wu.fileset_id = atoi(r[i++]);
    wu.app_version_id = atoi(r[i++]);
}

int DB_WORK_ITEM::enumerate(
    int limit, const char* select_clause, const char* order_clause
) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    if (!cursor.active) {
        // use "r1" to refer to the result, since the feeder assumes that
        // (historical reasons)
        //
        sprintf(query,
            "select high_priority r1.id, r1.priority, r1.server_state, r1.report_deadline, workunit.* from result r1 force index(ind_res_st), workunit, app "
            " where r1.server_state=%d "
            " and r1.workunitid=workunit.id "
            " and workunit.appid=app.id "
            " and app.deprecated=0 "
            " and workunit.error_mask=0 "
            " and workunit.transitioner_flags=0 "
            " %s "
            " %s "
            "limit %d",
            RESULT_SERVER_STATE_UNSENT,
            select_clause,
            order_clause,
            limit
        );
        retval = db->do_query(query);
        if (retval) return mysql_errno(db->mysql);
        cursor.rp = mysql_store_result(db->mysql);
        if (!cursor.rp) return mysql_errno(db->mysql);
        cursor.active = true;
    }
    row = mysql_fetch_row(cursor.rp);
    if (!row) {
        mysql_free_result(cursor.rp);
        cursor.active = false;
        retval = mysql_errno(db->mysql);
        if (retval) return ERR_DB_CONN_LOST;
        return ERR_DB_NOT_FOUND;
    } else {
        parse(row);
    }
    return 0;
}

int DB_WORK_ITEM::enumerate_all(
    int limit, const char* select_clause
) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    if (!cursor.active) {
        // use "r1" to refer to the result, since the feeder assumes that
        // (historical reasons)
        //
        sprintf(query,
            "select high_priority r1.id, r1.priority, r1.server_state, r1.report_deadline, workunit.* from result r1 force index(ind_res_st), workunit force index(primary), app"
            " where r1.server_state=%d and r1.workunitid=workunit.id and r1.id>%d "
            " and workunit.appid=app.id and app.deprecated=0 "
            " and workunit.transitioner_flags=0 "
            " %s "
            "limit %d",
            RESULT_SERVER_STATE_UNSENT,
            start_id,
            select_clause,
            limit
        );
        retval = db->do_query(query);
        if (retval) return mysql_errno(db->mysql);
        cursor.rp = mysql_store_result(db->mysql);
        if (!cursor.rp) return mysql_errno(db->mysql);

        // if query gets no rows, start over in ID space
        //
        if (mysql_num_rows(cursor.rp) == 0) {
            mysql_free_result(cursor.rp);
            start_id = 0;
            return ERR_DB_NOT_FOUND;
        }
        cursor.active = true;
    }
    row = mysql_fetch_row(cursor.rp);
    if (!row) {
        mysql_free_result(cursor.rp);
        cursor.active = false;
        retval = mysql_errno(db->mysql);
        if (retval) return ERR_DB_CONN_LOST;
        return ERR_DB_NOT_FOUND;
    } else {
        parse(row);
        start_id = res_id;
    }
    return 0;
}


void IN_PROGRESS_RESULT::parse(MYSQL_ROW& r) {
    int i=0;
    memset(this, 0, sizeof(IN_PROGRESS_RESULT));
    strcpy2(result_name,r[i++]);
    error_mask = atoi(r[i++]);
    assimilate_state = atoi(r[i++]);
    server_state = atoi(r[i++]);
    outcome = atoi(r[i++]);
}

int DB_IN_PROGRESS_RESULT::enumerate(int hostid, const char* result_names) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    if (!cursor.active) {
        sprintf(query,
            "select high_priority result.name, workunit.error_mask, workunit.assimilate_state, result.server_state, result.outcome "
            " from result, workunit "
            " where result.hostid = %d and workunit.id = result.workunitid "
            " and (result.server_state=%d or ( result.server_state = %d and result.outcome = %d ) ) "
            " and result.name in (%s) ",
            hostid,
            RESULT_SERVER_STATE_IN_PROGRESS,
            RESULT_SERVER_STATE_OVER,
            RESULT_OUTCOME_NO_REPLY,
            result_names
        );
        retval = db->do_query(query);
        if (retval) return mysql_errno(db->mysql);
        cursor.rp = mysql_store_result(db->mysql);
        if (!cursor.rp) return mysql_errno(db->mysql);
        cursor.active = true;
    }
    row = mysql_fetch_row(cursor.rp);
    if (!row) {
        mysql_free_result(cursor.rp);
        cursor.active = false;
        retval = mysql_errno(db->mysql);
        if (retval) return ERR_DB_CONN_LOST;
        return ERR_DB_NOT_FOUND;
    } else {
        parse(row);
    }
    return 0;
}

// The items that appear here must agree with those that appear in the
// enumerate method just below!
//
void SCHED_RESULT_ITEM::parse(MYSQL_ROW& r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    strcpy2(name, r[i++]);
    workunitid = atoi(r[i++]);
    appid = atoi(r[i++]);
    server_state = atoi(r[i++]);
    hostid = atoi(r[i++]);
    userid = atoi(r[i++]);
    sent_time = atoi(r[i++]);
    received_time = atoi(r[i++]);
    validate_state = atoi(r[i++]);
    outcome = atoi(r[i++]);
    client_state = atoi(r[i++]);
    file_delete_state = atoi(r[i++]);
    app_version_id = atoi(r[i++]);
}

int DB_SCHED_RESULT_ITEM_SET::add_result(char* result_name) {
    SCHED_RESULT_ITEM result;
    result.id = 0;
    strcpy2(result.queried_name, result_name);
    results.push_back(result);
    return 0;
}

int DB_SCHED_RESULT_ITEM_SET::enumerate() {
    string query;
    int retval;
    unsigned int i;
    MYSQL_RES* rp;
    MYSQL_ROW row;
    SCHED_RESULT_ITEM ri;


    query =
        "SELECT "
        "   id, "
        "   name, "
        "   workunitid, "
        "   appid, "
        "   server_state, "
        "   hostid, "
        "   userid, "
        "   sent_time, "
        "   received_time, "
        "   validate_state, "
        "   outcome, "
        "   client_state, "
        "   file_delete_state, "
        "   app_version_id "
        "FROM "
        "   result "
        "WHERE "
        "   name IN ( "
    ;

    for (i=0; i<results.size(); i++) {
        if (i>0) query += ",";
        query += "'";
        query += results[i].queried_name;
        query += "'";
    }
    query += ")";

    retval = db->do_query(query.c_str());
    if (retval) return retval;

    // the following stores the entire result set in memory
    //
    rp = mysql_store_result(db->mysql);
    if (!rp) return mysql_errno(db->mysql);

    do {
        row = mysql_fetch_row(rp);
        if (!row) {
            mysql_free_result(rp);
        } else {
            ri.parse(row);
            for (i=0; i<results.size(); i++) {
                if (!strcmp(results[i].queried_name, ri.name)) {
                    results[i] = ri;
                }
            }
        }
    } while (row);

    return 0;
}

int DB_SCHED_RESULT_ITEM_SET::lookup_result(char* result_name, SCHED_RESULT_ITEM** rip) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (!strcmp(results[i].name, result_name)) {
            *rip = &results[i];
            return 0;
        }
    }
    return -1;
}

int DB_SCHED_RESULT_ITEM_SET::update_result(SCHED_RESULT_ITEM& ri) {
    char query[MAX_QUERY_LEN];
    int retval;

    ESCAPE(ri.xml_doc_out);
    ESCAPE(ri.stderr_out);
    sprintf(query,
        "UPDATE result SET "
        "    hostid=%d, "
        "    received_time=%d, "
        "    client_state=%d, "
        "    cpu_time=%.15e, "
        "    exit_status=%d, "
        "    app_version_num=%d, "
        "    server_state=%d, "
        "    outcome=%d, "
        "    stderr_out='%s', "
        "    xml_doc_out='%s', "
        "    validate_state=%d, "
        "    teamid=%d, "
        "    elapsed_time=%.15e "
        "WHERE "
        "    id=%u",
        ri.hostid,
        ri.received_time,
        ri.client_state,
        ri.cpu_time,
        ri.exit_status,
        ri.app_version_num,
        ri.server_state,
        ri.outcome,
        ri.stderr_out,
        ri.xml_doc_out,
        ri.validate_state,
        ri.teamid,
        ri.elapsed_time,
        ri.id
    );
    retval = db->do_query(query);
    UNESCAPE(ri.xml_doc_out);
    UNESCAPE(ri.stderr_out);
    if (db->affected_rows() != 1) return ERR_DB_NOT_FOUND;
    return retval;
}

// set transition times of workunits -
// but only those corresponding to updated results
// (i.e. those that passed "sanity checks")
//
int DB_SCHED_RESULT_ITEM_SET::update_workunits() {
    char query[MAX_QUERY_LEN], buf[256];
    unsigned int i;
    bool first= true;

    sprintf(query,
        "UPDATE workunit SET transition_time=%d WHERE id in (",
        (int)time(0)
    );
    for (i=0; i<results.size(); i++) {
        if (results[i].id == 0) continue;
            // skip non-updated results
        if (!first) strcat(query, ",");
        first = false;
        sprintf(buf, "%d", results[i].workunitid);
        strcat(query, buf);
    }
    strcat(query, ")");
    if (first) {
        return 0;
    } else {
        return db->do_query(query);
    }
}

void DB_FILE::db_print(char* buf){
    snprintf(buf, MAX_QUERY_LEN,
        "name='%s', md5sum='%s', size=%.15e",
        name, md5sum, size
    );
}

void DB_FILE::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(md5sum, r[i++]);
    size = atof(r[i++]);
}

void DB_FILESET::db_print(char* buf){
    snprintf(buf, MAX_QUERY_LEN, "name='%s'", name);
}

void DB_FILESET::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    strcpy2(name, r[i++]);
}

int DB_FILESET::select_by_name(const char* _name) {
    char where_clause[MAX_QUERY_LEN] = {0};

    // construct where clause and select single record
    snprintf(where_clause, MAX_QUERY_LEN, "WHERE name = '%s'", _name);
    return lookup(where_clause);
}

void DB_FILESET_FILE::db_print(char* buf){
    snprintf(buf, MAX_QUERY_LEN,
        "fileset_id=%d, file_id=%d",
        fileset_id, file_id
    );
}

void DB_FILESET_FILE::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    fileset_id = atoi(r[i++]);
    file_id = atoi(r[i++]);
}

void DB_SCHED_TRIGGER::db_print(char* buf){
    snprintf(buf, MAX_QUERY_LEN,
        "fileset_id=%d, need_work=%d, work_available=%d, no_work_available=%d, working_set_removal=%d",
        fileset_id, need_work?1:0, work_available?1:0, no_work_available?1:0, working_set_removal?1:0
    );
}

void DB_SCHED_TRIGGER::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    fileset_id = atoi(r[i++]);
    need_work = atoi(r[i++]);
    work_available = atoi(r[i++]);
    no_work_available = atoi(r[i++]);
    working_set_removal = atoi(r[i++]);
}

int DB_SCHED_TRIGGER::select_unique_by_fileset_name(const char* fileset_name) {
    char query[MAX_QUERY_LEN];
    int retval;
    int nrows = 0;
    MYSQL_RES* recordset;
    MYSQL_ROW row;

    //if (!cursor.active) {
        // prepare statement
        snprintf(query, MAX_QUERY_LEN,
            "SELECT"
            "  t.id,"
            "  t.fileset_id,"
            "  t.need_work,"
            "  t.work_available,"
            "  t.no_work_available,"
            "  t.working_set_removal "
            "FROM"
            "  fileset fs INNER JOIN sched_trigger t ON fs.id = t.fileset_id "
            "WHERE"
            "  fs.name = '%s'",
            fileset_name
        );

        retval = db->do_query(query);

        if (retval) return mysql_errno(db->mysql);

        recordset = mysql_store_result(db->mysql);
        if (!recordset) return mysql_errno(db->mysql);
    //}

    // determine number of records, fetch first
    nrows = mysql_num_rows(recordset);
    row = mysql_fetch_row(recordset);

    if (!row || nrows != 1) {
        // something bad happened
        if (!row) {
            // no row returned, due to an error?
            retval = mysql_errno(db->mysql);
            mysql_free_result(recordset);

            // yes, probably lost DB connection
            if (retval) return ERR_DB_CONN_LOST;

            // no, just no record available
            return ERR_DB_NOT_FOUND;
        }
        else {
            // we got more records than expected
            mysql_free_result(recordset);
            return ERR_DB_NOT_UNIQUE;
        }
    } else {
        // all fine, parse single record
        db_parse(row);
        mysql_free_result(recordset);
    }
    return 0;
}

int DB_SCHED_TRIGGER::update_single_state(const DB_SCHED_TRIGGER::STATE state, const bool value) {
    char column_clause[MAX_QUERY_LEN] = {0};
    int retval = 0;

    switch(state) {
    case DB_SCHED_TRIGGER::state_need_work:
        snprintf(column_clause, MAX_QUERY_LEN, "need_work = %d", value?1:0);
        need_work = value;
        break;
    case DB_SCHED_TRIGGER::state_work_available:
        snprintf(column_clause, MAX_QUERY_LEN, "work_available = %d", value?1:0);
        work_available = value;
        break;
    case DB_SCHED_TRIGGER::state_no_work_available:
        snprintf(column_clause, MAX_QUERY_LEN, "no_work_available = %d", value?1:0);
        no_work_available = value;
        break;
    case DB_SCHED_TRIGGER::state_working_set_removal:
        snprintf(column_clause, MAX_QUERY_LEN, "working_set_removal = %d", value?1:0);
        working_set_removal = value;
        break;
    default:
        // unknown state
        return -1;
    }

    // run actual update on current trigger (retrieved earlier)
    retval = update_field(column_clause, NULL);

    if (retval) return retval;
    return 0;
}

void DB_FILESET_SCHED_TRIGGER_ITEM::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    fileset.id = atoi(r[i++]);
    strcpy2(fileset.name, r[i++]);
    trigger.id = atoi(r[i++]);
    trigger.fileset_id = atoi(r[i++]);
    trigger.need_work = atoi(r[i++]);
    trigger.work_available = atoi(r[i++]);
    trigger.no_work_available = atoi(r[i++]);
    trigger.working_set_removal = atoi(r[i++]);
}

int DB_FILESET_SCHED_TRIGGER_ITEM_SET::select_by_name_state(
    const char* fileset_name = NULL,
    const bool use_regexp = false,
    const DB_SCHED_TRIGGER::STATE state = DB_SCHED_TRIGGER::none,
    const bool state_value = true
) {
    char where_clause[MAX_QUERY_LEN] = {0};
    char query[MAX_QUERY_LEN] = {0};
    int retval = 0;
    int count = 0;
    MYSQL_RES* recordset;
    MYSQL_ROW row;
    DB_FILESET_SCHED_TRIGGER_ITEM fileset_trigger;

    // prepare requested compare mode
    const char* comparator = use_regexp ? "REGEXP" : "=";

    // prepare optional state filter
    char state_filter[MAX_QUERY_LEN] = {0};
    switch(state) {
    case DB_SCHED_TRIGGER::state_need_work:
        snprintf(state_filter, MAX_QUERY_LEN, "need_work = %d", state_value?1:0);
        break;
    case DB_SCHED_TRIGGER::state_work_available:
        snprintf(state_filter, MAX_QUERY_LEN, "work_available = %d", state_value?1:0);
        break;
    case DB_SCHED_TRIGGER::state_no_work_available:
        snprintf(state_filter, MAX_QUERY_LEN, "no_work_available = %d", state_value?1:0);
        break;
    case DB_SCHED_TRIGGER::state_working_set_removal:
        snprintf(state_filter, MAX_QUERY_LEN, "working_set_removal = %d", state_value?1:0);
        break;
    default:
        // none or unknown state (keep empty filter)
        break;
    }

    // prepare WHERE clause
    if(fileset_name && !state) {
        snprintf(where_clause, MAX_QUERY_LEN, "WHERE fs.name %s '%s'", comparator, fileset_name);
    } else if(!fileset_name && state) {
        snprintf(where_clause, MAX_QUERY_LEN, "WHERE %s", state_filter);
    } else if(fileset_name && state) {
        snprintf(where_clause, MAX_QUERY_LEN, "WHERE fs.name %s '%s' AND %s", comparator, fileset_name, state_filter);
    }

    // prepare final statement
    snprintf(query, MAX_QUERY_LEN,
        "SELECT"
        "  fs.id,"
        "  fs.name,"
        "  t.id,"
        "  t.fileset_id,"
        "  t.need_work,"
        "  t.work_available,"
        "  t.no_work_available,"
        "  t.working_set_removal "
        "FROM"
        "  fileset fs INNER JOIN sched_trigger t ON fs.id = t.fileset_id "
        "%s",
        where_clause
    );

    retval = db->do_query(query);
    if (retval) return retval;

    recordset = mysql_store_result(db->mysql);
    if (!recordset) return mysql_errno(db->mysql);

    // check if we got at least one record
    count = mysql_num_rows(recordset);
    if(count == 0) {
        mysql_free_result(recordset);
        return ERR_DB_NOT_FOUND;
    }

    // all fine, iterate over recordset
    do {
        row = mysql_fetch_row(recordset);
        if (!row) {
            // clean up
            mysql_free_result(recordset);

            // no row returned, due to an error?
            retval = mysql_errno(db->mysql);
            // yes, probably lost DB connection
            if (retval) return ERR_DB_CONN_LOST;
        } else {
            // parse record, add to vector
            fileset_trigger.db_parse(row);
            items.push_back(fileset_trigger);
        }
    } while (row);

    return 0;
}

int DB_FILESET_SCHED_TRIGGER_ITEM_SET::contains_trigger(
    const char* fileset_name
) {
    // iterate over item vector
    for (unsigned int i=0; i<items.size(); ++i) {
        if (strcmp(items[i].fileset.name, fileset_name) == 0) {
            // return 1-indexed position for boolean tests
            return i+1;
        }
    }
    return 0;
}

void DB_VDA_FILE::db_print(char* buf){
    sprintf(buf,
        "create_time=%f, "
        "dir='%s', "
        "file_name='%s', "
        "size=%f, "
        "chunk_size=%f, "
        "need_update=%d, "
        "initialized=%d, "
        "retrieving=%d"
        "retrieved=%d",
        create_time,
        dir,
        file_name,
        size,
        chunk_size,
        need_update?1:0,
        initialized?1:0,
        retrieving?1:0,
        retrieved?1:0
    );
}

void DB_VDA_FILE::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    id = atoi(r[i++]);
    create_time = atof(r[i++]);
    strcpy(dir, r[i++]);
    strcpy(file_name, r[i++]);
    size = atof(r[i++]);
    chunk_size = atof(r[i++]);
    need_update = (atoi(r[i++]) != 0);
    initialized = (atoi(r[i++]) != 0);
    retrieving = (atoi(r[i++]) != 0);
    retrieved = (atoi(r[i++]) != 0);
}

void DB_VDA_CHUNK_HOST::db_print(char* buf) {
    sprintf(buf,
        "create_time=%f, "
        "vda_file_id=%d, "
        "host_id=%d, "
        "physical_file_name='%s', "
        "present_on_host=%d, "
        "transfer_in_progress=%d, "
        "transfer_wait=%d, "
        "transfer_request_time=%f, "
        "transfer_send_time=%f ",
        create_time,
        vda_file_id,
        host_id,
        physical_file_name,
        present_on_host,
        transfer_in_progress,
        transfer_wait,
        transfer_request_time,
        transfer_send_time
    );
}

void DB_VDA_CHUNK_HOST::db_parse(MYSQL_ROW &r) {
    int i=0;
    clear();
    create_time = atof(r[i++]);
    vda_file_id = atoi(r[i++]);
    host_id = atoi(r[i++]);
    strcpy(physical_file_name, r[i++]);
    present_on_host = (atoi(r[i++]) != 0);
    transfer_in_progress = (atoi(r[i++]) != 0);
    transfer_wait = (atoi(r[i++]) != 0);
    transfer_request_time = atof(r[i++]);
    transfer_send_time = atof(r[i++]);
}

const char *BOINC_RCSID_ac374386c8 = "$Id$";
