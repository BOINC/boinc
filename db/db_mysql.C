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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mysql_util.h"

#include "db.h"

#define TYPE_PLATFORM	        1
#define TYPE_APP	        2
#define TYPE_APP_VERSION	3
#define TYPE_USER	        4
#define TYPE_PREFS              5
#define TYPE_HOST	        6
#define TYPE_WORKUNIT	        7
#define TYPE_RESULT	        8

char* table_name[] = {
    "",
    "platform",
    "app",
    "app_version",
    "user",
    "prefs",
    "host",
    "workunit",
    "result",
};

void struct_to_str(void* vp, char* q, int type) {
    PLATFORM* pp;
    APP* app;
    APP_VERSION* avp;
    USER* up;
    PREFS* prp;
    HOST* hp;
    WORKUNIT* wup;
    RESULT* rp;

    switch(type) {
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
            "alpha_vers=%d, beta_vers=%d, prod_vers=%d, "
            "result_xml_template='%s'",
            app->id,
            app->create_time,
            app->name,
            app->alpha_vers,
            app->beta_vers,
            app->prod_vers,
            app->result_xml_template
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
            "web_password='%s', authenticator='%s', default_prefsid=%d, "
            "country='%s', postal_code='%s', "
            "total_credit=%f, expavg_credit=%f, expavg_time=%d",
	    up->id,
	    up->create_time,
	    up->email_addr,
	    up->name,
	    up->web_password,
	    up->authenticator,
	    up->default_prefsid,
	    up->country,
	    up->postal_code,
	    up->total_credit,
	    up->expavg_credit,
            up->expavg_time
	);
        unescape(up->email_addr);
        unescape(up->name);
        unescape(up->web_password);
	break;
    case TYPE_PREFS:
        prp = (PREFS*)vp;
        sprintf(q,
            "id=%d, create_time=%d, modified_time=%d, userid=%d, "
            "name='%s', xml_doc='%s'",
            prp->id,
            prp->create_time,
            prp->modified_time,
            prp->userid,
            prp->name,
            prp->xml_doc
        );
	break;
    case TYPE_HOST:
        hp = (HOST*)vp;
        sprintf(q,
            "id=%d, create_time=%d, userid=%d, prefsid=%d, "
            "rpc_seqno=%d, rpc_time=%d, "
            "timezone=%d, domain_name='%s', serialnum='%s', "
            "last_ip_addr='%s', nsame_ip_addr=%d, "
            "on_frac=%f, connected_frac=%f, active_frac=%f, "
            "p_ncpus=%d, p_vendor='%s', p_model='%s', "
            "p_fpops=%f, p_iops=%f, p_membw=%f, "
            "os_name='%s', os_version='%s', "
            "m_nbytes=%f, m_cache=%f, m_swap=%f, "
            "d_total=%f, d_free=%f, "
            "n_bwup=%f, n_bwdown=%f",
            hp->id, hp->create_time, hp->userid, hp->prefsid,
            hp->rpc_seqno, hp->rpc_time,
            hp->timezone, hp->domain_name, hp->serialnum,
            hp->last_ip_addr, hp->nsame_ip_addr,
            hp->on_frac, hp->connected_frac, hp->active_frac,
            hp->p_ncpus, hp->p_vendor, hp->p_model,
            hp->p_fpops, hp->p_iops, hp->p_membw,
            hp->os_name, hp->os_version,
            hp->m_nbytes, hp->m_cache, hp->m_swap,
            hp->d_total, hp->d_free,
            hp->n_bwup, hp->n_bwdown
        );
	break;
    case TYPE_WORKUNIT:
        wup = (WORKUNIT*)vp;
        sprintf(q,
            "id=%d, create_time=%d, appid=%d, previous_wuid=%d, "
            "has_successor=%d, name='%s', xml_doc='%s', batch=%d, "
            "rsc_fpops=%f, rsc_iops=%f, rsc_memory=%f, rsc_disk=%f, "
            "dynamic_results=%d, max_results=%d, "
            "nresults=%d, nresults_unsent=%d, "
            "nresults_done=%d, nresults_fail=%d",
            wup->id, wup->create_time, wup->appid, wup->previous_wuid,
            wup->has_successor?1:0, wup->name, wup->xml_doc, wup->batch,
            wup->rsc_fpops, wup->rsc_iops, wup->rsc_memory, wup->rsc_disk, 
            wup->dynamic_results?1:0, wup->max_results,
            wup->nresults, wup->nresults_unsent,
            wup->nresults_done, wup->nresults_fail
        );
	break;
    case TYPE_RESULT:
        rp = (RESULT*)vp;
        sprintf(q,
            "id=%d, create_time=%d, workunitid=%d, state=%d, "
            "hostid=%d, sent_time=%d, received_time=%d, "
            "name='%s', exit_status=%d, cpu_time=%f, "
            "xml_doc_in='%s', xml_doc_out='%s', stderr_out='%s', "
            "batch=%d, project_state=%d, validated=%d",
            rp->id, rp->create_time, rp->workunitid, rp->state,
            rp->hostid, rp->sent_time, rp->received_time,
            rp->name, rp->exit_status, rp->cpu_time,
            rp->xml_doc_in, rp->xml_doc_out, rp->stderr_out,
            rp->batch, rp->project_state, rp->validated
        );
	break;
    }
}

void row_to_struct(MYSQL_ROW& r, void* vp, int type) {
    PLATFORM* pp;
    APP* app;
    APP_VERSION* avp;
    USER* up;
    PREFS* prp;
    HOST* hp;
    WORKUNIT* wup;
    RESULT* rp;

    int i=0;

    switch(type) {
    case TYPE_PLATFORM:
        pp = (PLATFORM*)vp;
	memset(pp, 0, sizeof(PLATFORM));
	pp->id = atoi(r[i++]);
	pp->create_time = atoi(r[i++]);
        strcpy(pp->name, r[i++]);
	break;
    case TYPE_APP:
        app = (APP*)vp;
	memset(app, 0, sizeof(APP));
	app->id = atoi(r[i++]);
	app->create_time = atoi(r[i++]);
        strcpy(app->name, r[i++]);
	app->alpha_vers = atoi(r[i++]);
	app->beta_vers = atoi(r[i++]);
	app->prod_vers = atoi(r[i++]);
        strcpy(app->result_xml_template, r[i++]);
	break;
    case TYPE_APP_VERSION:
        avp = (APP_VERSION*)vp;
	memset(avp, 0, sizeof(APP_VERSION));
	avp->id = atoi(r[i++]);
	avp->create_time = atoi(r[i++]);
	avp->appid = atoi(r[i++]);
	avp->version_num = atoi(r[i++]);
	avp->platformid = atoi(r[i++]);
        strcpy(avp->xml_doc, r[i++]);
	avp->min_core_version = atoi(r[i++]);
	avp->max_core_version = atoi(r[i++]);
        strcpy(avp->message, r[i++]);
	avp->deprecated = atoi(r[i++]);
	break;
    case TYPE_USER:
	up = (USER*)vp;
	memset(up, 0, sizeof(USER));
	up->id = atoi(r[i++]);
	up->create_time = atoi(r[i++]);
	strcpy(up->email_addr, r[i++]);
	strcpy(up->name, r[i++]);
	strcpy(up->web_password, r[i++]);
	strcpy(up->authenticator, r[i++]);
	up->default_prefsid = atoi(r[i++]);
	strcpy(up->country, r[i++]);
	strcpy(up->postal_code, r[i++]);
	up->total_credit = atof(r[i++]);
	up->expavg_credit = atof(r[i++]);
	up->expavg_time = atoi(r[i++]);
	break;
    case TYPE_PREFS:
	prp = (PREFS*)vp;
	memset(prp, 0, sizeof(PREFS));
	prp->id = atoi(r[i++]);
	prp->create_time = atoi(r[i++]);
	prp->modified_time = atoi(r[i++]);
	prp->userid = atoi(r[i++]);
        strcpy(prp->name, r[i++]);
        strcpy(prp->xml_doc, r[i++]);
	break;
    case TYPE_HOST:
	hp = (HOST*)vp;
	memset(hp, 0, sizeof(HOST));
	hp->id = atoi(r[i++]);
	hp->create_time = atoi(r[i++]);
	hp->userid = atoi(r[i++]);
	hp->prefsid = atoi(r[i++]);
	hp->rpc_seqno = atoi(r[i++]);
	hp->rpc_time = atoi(r[i++]);
	hp->timezone = atoi(r[i++]);
	strcpy(hp->domain_name, r[i++]);
	strcpy(hp->serialnum, r[i++]);
	strcpy(hp->last_ip_addr, r[i++]);
	hp->nsame_ip_addr = atoi(r[i++]);
	hp->on_frac = atof(r[i++]);
	hp->connected_frac = atof(r[i++]);
	hp->active_frac = atof(r[i++]);
	hp->p_ncpus = atoi(r[i++]);
	strcpy(hp->p_vendor, r[i++]);
	strcpy(hp->p_model, r[i++]);
	hp->p_fpops = atof(r[i++]);
	hp->p_iops = atof(r[i++]);
	hp->p_membw = atof(r[i++]);
	strcpy(hp->os_name, r[i++]);
	strcpy(hp->os_version, r[i++]);
	hp->m_nbytes = atof(r[i++]);
	hp->m_cache = atof(r[i++]);
	hp->m_swap = atof(r[i++]);
	hp->d_total = atof(r[i++]);
	hp->d_free = atof(r[i++]);
	hp->n_bwup = atof(r[i++]);
	hp->n_bwdown = atof(r[i++]);
	break;
    case TYPE_WORKUNIT:
	wup = (WORKUNIT*)vp;
	memset(wup, 0, sizeof(WORKUNIT));
	wup->id = atoi(r[i++]);
	wup->create_time = atoi(r[i++]);
	wup->appid = atoi(r[i++]);
	wup->previous_wuid = atoi(r[i++]);
	wup->has_successor = (atoi(r[i++])!=0);
        strcpy(wup->name, r[i++]);
        strcpy(wup->xml_doc, r[i++]);
	wup->batch = atoi(r[i++]);
        wup->rsc_fpops = atof(r[i++]);
        wup->rsc_iops = atof(r[i++]);
        wup->rsc_memory = atof(r[i++]);
        wup->rsc_disk = atof(r[i++]);
	wup->dynamic_results = (atoi(r[i++])!=0);
	wup->max_results = atoi(r[i++]);
	wup->nresults = atoi(r[i++]);
	wup->nresults_unsent = atoi(r[i++]);
	wup->nresults_done = atoi(r[i++]);
	wup->nresults_fail = atoi(r[i++]);
	break;
    case TYPE_RESULT:
	rp = (RESULT*)vp;
	memset(rp, 0, sizeof(RESULT));
	rp->id = atoi(r[i++]);
	rp->create_time = atoi(r[i++]);
	rp->workunitid = atoi(r[i++]);
	rp->state = atoi(r[i++]);
	rp->hostid = atoi(r[i++]);
	rp->sent_time = atoi(r[i++]);
	rp->received_time = atoi(r[i++]);
        strcpy(rp->name, r[i++]);
	rp->exit_status = atoi(r[i++]);
	rp->cpu_time = atof(r[i++]);
        strcpy(rp->xml_doc_in, r[i++]);
        strcpy(rp->xml_doc_out, r[i++]);
        strcpy(rp->stderr_out, r[i++]);
	rp->batch = atoi(r[i++]);
	rp->project_state = atoi(r[i++]);
	rp->validated = atoi(r[i++]);
	break;
    }
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

////////// PREFS /////////

int db_prefs_new(PREFS& p) {
    return db_new(&p, TYPE_PREFS);
}

int db_prefs(int i, PREFS& p) {
    return db_lookup_id(i, &p, TYPE_PREFS);
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

int db_workunit_enum_dynamic_to_send(WORKUNIT& p, int limit) {
    static ENUM e;
    char buf[256];

    if (!e.active) {
        sprintf(buf, "where dynamic_results>0 and nresults<max_results");
    }
    return db_enum(e, &p, TYPE_WORKUNIT, buf, limit);
}

////////// RESULT /////////

int db_result_new(RESULT& p) {
    return db_new(&p, TYPE_RESULT);
}

int db_result_update(RESULT& p) {
    return db_update(&p, TYPE_RESULT);
}

int db_result_lookup_name(RESULT& p) {
    char buf[256];

    sprintf(buf, "name='%s'", p.name);
    return db_lookup(&p, TYPE_RESULT, buf);
}

int db_result_enum_to_send(RESULT& p, int limit) {
    static ENUM e;
    char buf[256];

    if (!e.active) sprintf(buf, "where state=%d", RESULT_STATE_UNSENT);
    return db_enum(e, &p, TYPE_RESULT, buf, limit);
}
