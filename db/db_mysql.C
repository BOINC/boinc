// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mysql_util.h"

#include "db.h"

#define TYPE_PROJECT            1
#define TYPE_PLATFORM           2
#define TYPE_APP                3
#define TYPE_APP_VERSION        4
#define TYPE_USER               5
#define TYPE_TEAM               6
#define TYPE_HOST               7
#define TYPE_WORKUNIT           8
#define TYPE_RESULT             9
#define TYPE_WORKSEQ            10

char* table_name[] = {
    "",
    "project",
    "platform",
    "app",
    "app_version",
    "user",
    "team",
    "host",
    "workunit",
    "result",
    "workseq",
};

void struct_to_str(void* vp, char* q, int type) {
    PROJECT* prp;
    PLATFORM* pp;
    APP* app;
    APP_VERSION* avp;
    USER* up;
    TEAM* tp;
    HOST* hp;
    WORKUNIT* wup;
    RESULT* rp;
    WORKSEQ* wsp;

    switch(type) {
    case TYPE_PROJECT:
        prp = (PROJECT*)vp;
        sprintf(q,
            "id=%d, name='%s'",
            prp->id,
            prp->name
        );
        break;
    case TYPE_PLATFORM:
        pp = (PLATFORM*)vp;
        sprintf(q,
            "id=%d, create_time=%d, name='%s'",
            pp->id,
            pp->create_time,
            pp->name
        );
        break;
    case TYPE_APP:
        app = (APP*)vp;
        sprintf(q,
            "id=%d, create_time=%d, name='%s', "
            "min_version=%d ",
            app->id,
            app->create_time,
            app->name,
            app->min_version
        );
        break;
    case TYPE_APP_VERSION:
        avp = (APP_VERSION*)vp;
        sprintf(q,
            "id=%d, create_time=%d, appid=%d, version_num=%d, platformid=%d, "
            "xml_doc='%s', "
            "min_core_version=%d, max_core_version=%d, "
            "message='%s', deprecated=%d",
            avp->id,
            avp->create_time,
            avp->appid,
            avp->version_num,
            avp->platformid,
            avp->xml_doc,
            avp->min_core_version,
            avp->max_core_version,
            avp->message,
            avp->deprecated?1:0
        );
        break;
    case TYPE_USER:
        up = (USER*)vp;
        escape(up->email_addr);
        escape(up->name);
        escape(up->web_password);
        sprintf(q,
            "id=%d, create_time=%d, email_addr='%s', name='%s', "
            "web_password='%s', authenticator='%s', "
            "country='%s', postal_code='%s', "
            "total_credit=%.12e, expavg_credit=%.12e, expavg_time=%f, "
            "global_prefs='%s', project_prefs='%s', "
            "teamid=%d",
            up->id,
            up->create_time,
            up->email_addr,
            up->name,
            up->web_password,
            up->authenticator,
            up->country,
            up->postal_code,
            up->total_credit,
            up->expavg_credit,
            up->expavg_time,
            up->global_prefs,
            up->project_prefs,
            up->teamid
        );
        unescape(up->email_addr);
        unescape(up->name);
        unescape(up->web_password);
        break;
    case TYPE_TEAM:
        tp = (TEAM*)vp;
        sprintf(q,
            "id=%d, userid=%d, name='%s', "
            "name_lc='%s', url='%s', "
            "type=%d, name_html='%s', description='%s', nusers=%d",
            tp->id,
            tp->userid,
            tp->name,
            tp->name_lc,
            tp->url,
            tp->type,
            tp->name_html,
            tp->description,
            tp->nusers
        );
        break;
    case TYPE_HOST:
        hp = (HOST*)vp;
        sprintf(q,
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
            "n_bwup=%f, n_bwdown=%f, "
            "credit_per_cpu_sec=%f",
            hp->id, hp->create_time, hp->userid,
            hp->rpc_seqno, hp->rpc_time,
            hp->total_credit, hp->expavg_credit, hp->expavg_time,
            hp->timezone, hp->domain_name, hp->serialnum,
            hp->last_ip_addr, hp->nsame_ip_addr,
            hp->on_frac, hp->connected_frac, hp->active_frac,
            hp->p_ncpus, hp->p_vendor, hp->p_model,
            hp->p_fpops, hp->p_iops, hp->p_membw,
            hp->os_name, hp->os_version,
            hp->m_nbytes, hp->m_cache, hp->m_swap,
            hp->d_total, hp->d_free,
            hp->n_bwup, hp->n_bwdown,
            hp->credit_per_cpu_sec
        );
        break;
    case TYPE_WORKUNIT:
        wup = (WORKUNIT*)vp;
        sprintf(q,
            "id=%d, create_time=%d, appid=%d, "
            "name='%s', xml_doc='%s', batch=%d, "
            "rsc_fpops=%f, rsc_iops=%f, rsc_memory=%f, rsc_disk=%f, "
            "need_validate=%d, "
            "canonical_resultid=%d, canonical_credit=%f, "
            "retry_check_time=%f, delay_bound=%d, main_state=%d, "
            "error=%d, file_delete_state=%d, assimilate_state=%d, "
            "workseq_next=%d",
            wup->id, wup->create_time, wup->appid,
            wup->name, wup->xml_doc, wup->batch,
            wup->rsc_fpops, wup->rsc_iops, wup->rsc_memory, wup->rsc_disk, 
            wup->need_validate,
            wup->canonical_resultid, wup->canonical_credit,
            wup->retry_check_time, wup->delay_bound, wup->main_state,
            wup->error, wup->file_delete_state, wup->assimilate_state,
            wup->workseq_next
        );
        break;
    case TYPE_RESULT:
        rp = (RESULT*)vp;
        sprintf(q,
            "id=%d, create_time=%d, workunitid=%d, server_state=%d, "
            "hostid=%d, report_deadline=%d, sent_time=%d, received_time=%d, "
            "name='%s', client_state=%d, cpu_time=%f, "
            "xml_doc_in='%s', xml_doc_out='%s', stderr_out='%s', "
            "batch=%d, file_delete_state=%d, validate_state=%d, "
            "claimed_credit=%f, granted_credit=%f",
            rp->id, rp->create_time, rp->workunitid, rp->server_state,
            rp->hostid, rp->report_deadline, rp->sent_time, rp->received_time,
            rp->name, rp->client_state, rp->cpu_time,
            rp->xml_doc_in, rp->xml_doc_out, rp->stderr_out,
            rp->batch, rp->file_delete_state, rp->validate_state,
            rp->claimed_credit, rp->granted_credit
        );
        break;
    case TYPE_WORKSEQ:
        wsp = (WORKSEQ*)vp;
        sprintf(q,
            "id=%d, create_time=%d, "
            "state=%d, hostid=%d, "
            "wuid_last_done=%d, wuid_last_sent=%d, "
            "workseqid_master=%d",
            wsp->id, wsp->create_time,
            wsp->state, wsp->hostid,
            wsp->wuid_last_done, wsp->wuid_last_sent,
            wsp->workseqid_master
        );
        break;
    }
}

static void strcpy2(char* dest, char* src) {
    if (!src) *dest = 0;
    else strcpy(dest, src);
}

void row_to_struct(MYSQL_ROW& r, void* vp, int type) {
    PROJECT* prp;
    PLATFORM* pp;
    APP* app;
    APP_VERSION* avp;
    USER* up;
    TEAM* tp;
    HOST* hp;
    WORKUNIT* wup;
    RESULT* rp;
    WORKSEQ* wsp;
    int i=0;

    switch(type) {
    case TYPE_PROJECT:
        prp = (PROJECT*)vp;
        memset(prp, 0, sizeof(PROJECT));
        prp->id = atoi(r[i++]);
        strcpy2(prp->name, r[i++]);
        break;
    case TYPE_PLATFORM:
        pp = (PLATFORM*)vp;
        memset(pp, 0, sizeof(PLATFORM));
        pp->id = atoi(r[i++]);
        pp->create_time = atoi(r[i++]);
        strcpy2(pp->name, r[i++]);
        break;
    case TYPE_APP:
        app = (APP*)vp;
        memset(app, 0, sizeof(APP));
        app->id = atoi(r[i++]);
        app->create_time = atoi(r[i++]);
        strcpy2(app->name, r[i++]);
        app->min_version = atoi(r[i++]);
        break;
    case TYPE_APP_VERSION:
        avp = (APP_VERSION*)vp;
        memset(avp, 0, sizeof(APP_VERSION));
        avp->id = atoi(r[i++]);
        avp->create_time = atoi(r[i++]);
        avp->appid = atoi(r[i++]);
        avp->version_num = atoi(r[i++]);
        avp->platformid = atoi(r[i++]);
        strcpy2(avp->xml_doc, r[i++]);
        avp->min_core_version = atoi(r[i++]);
        avp->max_core_version = atoi(r[i++]);
        strcpy2(avp->message, r[i++]);
        avp->deprecated = atoi(r[i++]);
        break;
    case TYPE_USER:
        up = (USER*)vp;
        memset(up, 0, sizeof(USER));
        up->id = atoi(r[i++]);
        up->create_time = atoi(r[i++]);
        strcpy2(up->email_addr, r[i++]);
        strcpy2(up->name, r[i++]);
        strcpy2(up->web_password, r[i++]);
        strcpy2(up->authenticator, r[i++]);
        strcpy2(up->country, r[i++]);
        strcpy2(up->postal_code, r[i++]);
        up->total_credit = atof(r[i++]);
        up->expavg_credit = atof(r[i++]);
        up->expavg_time = atof(r[i++]);
        strcpy2(up->global_prefs, r[i++]);
        strcpy2(up->project_prefs, r[i++]);
        up->teamid = atoi(r[i++]);
        break;
    case TYPE_TEAM:
        tp = (TEAM*)tp;
        memset(tp, 0, sizeof(TEAM));
        tp->id = atoi(r[i++]);
        tp->userid = atoi(r[i++]);
        strcpy2(tp->name, r[i++]);
        strcpy2(tp->name_lc, r[i++]);
        strcpy2(tp->url, r[i++]);
        strcpy2(tp->name_html, r[i++]);
        strcpy2(tp->description, r[i++]);
        tp->nusers = atoi(r[i++]);
        break;
    case TYPE_HOST:
        hp = (HOST*)vp;
        memset(hp, 0, sizeof(HOST));
        hp->id = atoi(r[i++]);
        hp->create_time = atoi(r[i++]);
        hp->userid = atoi(r[i++]);
        hp->rpc_seqno = atoi(r[i++]);
        hp->rpc_time = atoi(r[i++]);
        hp->total_credit = atof(r[i++]);
        hp->expavg_credit = atof(r[i++]);
        hp->expavg_time = atof(r[i++]);
        hp->timezone = atoi(r[i++]);
        strcpy2(hp->domain_name, r[i++]);
        strcpy2(hp->serialnum, r[i++]);
        strcpy2(hp->last_ip_addr, r[i++]);
        hp->nsame_ip_addr = atoi(r[i++]);
        hp->on_frac = atof(r[i++]);
        hp->connected_frac = atof(r[i++]);
        hp->active_frac = atof(r[i++]);
        hp->p_ncpus = atoi(r[i++]);
        strcpy2(hp->p_vendor, r[i++]);
        strcpy2(hp->p_model, r[i++]);
        hp->p_fpops = atof(r[i++]);
        hp->p_iops = atof(r[i++]);
        hp->p_membw = atof(r[i++]);
        strcpy2(hp->os_name, r[i++]);
        strcpy2(hp->os_version, r[i++]);
        hp->m_nbytes = atof(r[i++]);
        hp->m_cache = atof(r[i++]);
        hp->m_swap = atof(r[i++]);
        hp->d_total = atof(r[i++]);
        hp->d_free = atof(r[i++]);
        hp->n_bwup = atof(r[i++]);
        hp->n_bwdown = atof(r[i++]);
        hp->credit_per_cpu_sec = atof(r[i++]);
        break;
    case TYPE_WORKUNIT:
        wup = (WORKUNIT*)vp;
        memset(wup, 0, sizeof(WORKUNIT));
        wup->id = atoi(r[i++]);
        wup->create_time = atoi(r[i++]);
        wup->appid = atoi(r[i++]);
        strcpy2(wup->name, r[i++]);
        strcpy2(wup->xml_doc, r[i++]);
        wup->batch = atoi(r[i++]);
        wup->rsc_fpops = atof(r[i++]);
        wup->rsc_iops = atof(r[i++]);
        wup->rsc_memory = atof(r[i++]);
        wup->rsc_disk = atof(r[i++]);
        wup->need_validate = atoi(r[i++]);
        wup->canonical_resultid = atoi(r[i++]);
        wup->canonical_credit = atof(r[i++]);
        wup->retry_check_time = atof(r[i++]);
        wup->delay_bound = atoi(r[i++]);
        wup->main_state = atoi(r[i++]);
        wup->error = atoi(r[i++]);
        wup->file_delete_state = atoi(r[i++]);
        wup->assimilate_state = atoi(r[i++]);
        wup->workseq_next = atoi(r[i++]);
        break;
    case TYPE_RESULT:
        rp = (RESULT*)vp;
        memset(rp, 0, sizeof(RESULT));
        rp->id = atoi(r[i++]);
        rp->create_time = atoi(r[i++]);
        rp->workunitid = atoi(r[i++]);
        rp->server_state = atoi(r[i++]);
        rp->hostid = atoi(r[i++]);
        rp->report_deadline = atoi(r[i++]);
        rp->sent_time = atoi(r[i++]);
        rp->received_time = atoi(r[i++]);
        strcpy2(rp->name, r[i++]);
        rp->client_state = atoi(r[i++]);
        rp->cpu_time = atof(r[i++]);
        strcpy2(rp->xml_doc_in, r[i++]);
        strcpy2(rp->xml_doc_out, r[i++]);
        strcpy2(rp->stderr_out, r[i++]);
        rp->batch = atoi(r[i++]);
        rp->file_delete_state = atoi(r[i++]);
        rp->validate_state = atoi(r[i++]);
        rp->claimed_credit = atof(r[i++]);
        rp->granted_credit = atof(r[i++]);
        break;
    case TYPE_WORKSEQ:
        wsp = (WORKSEQ*)vp;
        memset(wsp, 0, sizeof(WORKSEQ));
        wsp->id = atoi(r[i++]);
        wsp->create_time = atoi(r[i++]);
        wsp->state = atoi(r[i++]);
        wsp->hostid = atoi(r[i++]);
        wsp->wuid_last_done = atoi(r[i++]);
        wsp->wuid_last_sent = atoi(r[i++]);
        wsp->workseqid_master = atoi(r[i++]);
    }
}


////////// PROJECT /////////

int db_project_new(PROJECT& p) {
    return db_new(&p, TYPE_PROJECT);
}

int db_project_enum(PROJECT& p) {
    static ENUM e;
    return db_enum(e, &p, TYPE_PROJECT);
}

////////// PLATFORM /////////

int db_platform_new(PLATFORM& p) {
    return db_new(&p, TYPE_PLATFORM);
}

int db_platform_enum(PLATFORM& p) {
    static ENUM e;
    return db_enum(e, &p, TYPE_PLATFORM);
}

int db_platform_lookup_name(PLATFORM& p) {
    char buf[256];

    sprintf(buf, "name='%s'", p.name);
    return db_lookup(&p, TYPE_PLATFORM, buf);
}

////////// APP /////////

int db_app_new(APP& p) {
    return db_new(&p, TYPE_APP);
}

int db_app(int id, APP& p) {
    return db_lookup_id(id, &p, TYPE_APP);
}

int db_app_enum(APP& p) {
    static ENUM e;
    return db_enum(e, &p, TYPE_APP);
}

int db_app_update(APP& p) {
    return db_update(&p, TYPE_APP);
}

int db_app_lookup_name(APP& p) {
    char buf[256];

    sprintf(buf, "name='%s'", p.name);
    return db_lookup(&p, TYPE_APP, buf);
}

////////// APP_VERSION /////////

int db_app_version_new(APP_VERSION& p) {
    return db_new(&p, TYPE_APP_VERSION);
}

int db_app_version_lookup(
    int appid, int platformid, int version_num, APP_VERSION& p
) {
    char buf[256];
    assert(version_num>=0);
    sprintf(buf,
        "appid=%d and platformid=%d and version_num=%d",
        appid, platformid, version_num
    );
    return db_lookup(&p, TYPE_APP_VERSION, buf);
}

int db_app_version_enum(APP_VERSION& p) {
    static ENUM e;
    return db_enum(e, &p, TYPE_APP_VERSION);
}

////////// USER /////////

int db_user(int i, USER& p) {
    return db_lookup_id(i, &p, TYPE_USER);
}

int db_user_new(USER& p) {
    return db_new(&p, TYPE_USER);
}

int db_user_update(USER& p) {
    return db_update(&p, TYPE_USER);
}

int db_user_lookup_auth(USER& p) {
    char buf[256];

    sprintf(buf, "authenticator='%s'", p.authenticator);
    return db_lookup(&p, TYPE_USER, buf);
}

int db_user_lookup_email_addr(USER& p) {
    char buf[256];

    sprintf(buf, "email_addr='%s'", p.email_addr);
    return db_lookup(&p, TYPE_USER, buf);
}

////////// TEAM /////////

int db_team(int i, TEAM& p) {
    return db_lookup_id(i, &p, TYPE_TEAM);
}

int db_team_new(TEAM& p) {
    return db_new(&p, TYPE_TEAM);
}

int db_team_update(TEAM& p) {
    return db_update(&p, TYPE_TEAM);
}

int db_team_lookup_name(TEAM& p) {
    char buf[256];

    sprintf(buf, "name='%s'", p.name);
    return db_lookup(&p, TYPE_TEAM, buf);
}

int db_team_lookup_name_lc(TEAM& p) {
    char buf[256];

    sprintf(buf, "name_lc='%s'", p.name_lc);
    return db_lookup(&p, TYPE_TEAM, buf);
}

int db_team_enum(USER& p, int id) {
    static ENUM e;
    char buf[256];
    sprintf(buf, "where teamid=%d", id);
    return db_enum(e, &p, TYPE_USER, buf);
}

////////// HOST /////////

int db_host_new(HOST& p) {
    return db_new(&p, TYPE_HOST);
}

int db_host(int i, HOST& p) {
    return db_lookup_id(i, &p, TYPE_HOST);
}

int db_host_update(HOST& p) {
    return db_update(&p, TYPE_HOST);
}

////////// WORKUNIT /////////

int db_workunit_new(WORKUNIT& p) {
    return db_new(&p, TYPE_WORKUNIT);
}

int db_workunit(int id, WORKUNIT& p) {
    return db_lookup_id(id, &p, TYPE_WORKUNIT);
}

int db_workunit_update(WORKUNIT& p) {
    return db_update(&p, TYPE_WORKUNIT);
}

int db_workunit_lookup_name(WORKUNIT& p) {
    char buf[256];

    sprintf(buf, "name='%s'", p.name);
    return db_lookup(&p, TYPE_WORKUNIT, buf);
}

int db_workunit_enum_app_need_validate(WORKUNIT& p) {
    static ENUM e;
    char buf[256];

    if (!e.active) {
        sprintf(buf, "where appid=%d and need_validate<>0", p.appid);
    }
    return db_enum(e, &p, TYPE_WORKUNIT, buf);
}

int db_workunit_enum_file_delete_state(WORKUNIT& p) {
    static ENUM e;
    char buf[256];

    if (!e.active) {
        sprintf(buf, "where file_delete_state=%d", p.file_delete_state);
    }
    return db_enum(e, &p, TYPE_WORKUNIT, buf);
}

int db_workunit_enum_assimilate_state(WORKUNIT& p) {
    static ENUM e;
    char buf[256];

    if (!e.active) {
        sprintf(buf, "where assimilate_state=%d", p.assimilate_state);
    }
    return db_enum(e, &p, TYPE_WORKUNIT, buf);
}

int db_workunit_enum_retry_check_time(WORKUNIT& p) {
    static ENUM e;
    char buf[256];

    if (!e.active) {
        sprintf(buf,
            "where appid=%d and retry_check_time > 0 and retry_check_time < %f",
            p.appid, p.retry_check_time
        );
    }
    return db_enum(e, &p, TYPE_WORKUNIT, buf);
}


////////// RESULT /////////

int db_result_new(RESULT& p) {
    return db_new(&p, TYPE_RESULT);
}

int db_result(int i, RESULT& p) {
    return db_lookup_id(i, &p, TYPE_RESULT);
}

int db_result_update(RESULT& p) {
    return db_update(&p, TYPE_RESULT);
}

int db_result_lookup_name(RESULT& p) {
    char buf[256];

    sprintf(buf, "name='%s'", p.name);
    return db_lookup(&p, TYPE_RESULT, buf);
}

int db_result_enum_server_state(RESULT& p, int limit) {
    static ENUM e;
    char buf[256];

    if (!e.active) sprintf(buf, "where server_state=%d", p.server_state);
    return db_enum(e, &p, TYPE_RESULT, buf, limit);
}

int db_result_enum_file_delete_state(RESULT& p) {
    static ENUM e;
    char buf[256];

    if (!e.active) sprintf(buf, "where file_delete_state=%d", p.file_delete_state);
    return db_enum(e, &p, TYPE_RESULT, buf);
}

int db_result_enum_wuid(RESULT& p) {
    static ENUM e;
    char buf[256];

    if (!e.active) sprintf(buf, "where workunitid=%d", p.workunitid);
    return db_enum(e, &p, TYPE_RESULT, buf);
}

int db_result_count_server_state(int state, int& n) {
    char buf[256];

    sprintf(buf, " where server_state=%d", state);
    return db_count(&n, "*", TYPE_RESULT, buf);
}

/////////// WORKSEQ ///////////////

int db_workseq_new(WORKSEQ& p) {
    return db_new(&p, TYPE_WORKSEQ);
}
