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

#include "client_types.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "parse.h"
#include "scheduler_op.h"

SCHEDULER_OP::SCHEDULER_OP(HTTP_OP_SET* h) {
    if(h==NULL) {
        fprintf(stderr, "error: SCHEDULER_OP: unexpected NULL pointer h\n");
    }
    state = SCHEDULER_OP_STATE_IDLE;
    http_op.http_op_state = HTTP_STATE_IDLE;
    http_ops = h;
}

int SCHEDULER_OP::start_rpc() {
    FILE *f;

    // TODO: try scheduler URLs other than the first one
    //
    strcpy(scheduler_url, project->scheduler_urls[0].text);
    if (log_flags.sched_ops) {
        printf("Sending request to scheduler: %s\n", scheduler_url);
    }
    if (log_flags.sched_op_debug) {
        f = fopen(SCHED_OP_REQUEST_FILE, "r");
        printf("--------- SCHEDULER REQUEST ---------\n");
        copy_stream(f, stdout);
        printf("--------- END ---------\n");
        fclose(f);
    }
    http_op.init_post(
        scheduler_url, SCHED_OP_REQUEST_FILE,
        SCHED_OP_RESULT_FILE
    );
    http_ops->insert(&http_op);
    project->rpc_seqno++;
    state = SCHEDULER_OP_STATE_RPC;
    return 0;
}

int SCHEDULER_OP::start_op(PROJECT* p) {
    if(p==NULL) {
        fprintf(stderr, "error: SCHEDULER_OP.start_op: unexpected NULL pointer p\n"); 
        return ERR_NULL;
    }
    project = p;
    if (project->scheduler_urls.size() == 0) {
        http_op.init_get(project->master_url, MASTER_FILE_NAME);
        http_ops->insert(&http_op);
        state = SCHEDULER_OP_STATE_GET_MASTER;
    } else {
        start_rpc();
    }
    return 0;
}

// parse a master file.
//
int SCHEDULER_OP::parse_master_file() {
    char buf[256];
    STRING256 str;
    FILE* f;
    
    f = fopen(MASTER_FILE_NAME, "r");
    if (!f) {
        fprintf(stderr, "Can't open master file\n");
        return ERR_FOPEN;
    }
    project->scheduler_urls.clear();
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<scheduler>", str.text)) {
            project->scheduler_urls.push_back(str);
        }
    }
    return 0;
}

int SCHEDULER_OP::poll() {
    int retval;

    switch(state) {
    case SCHEDULER_OP_STATE_GET_MASTER:
        if (http_op.http_op_state == HTTP_STATE_DONE) {
            http_ops->remove(&http_op);
            if (http_op.http_op_retval) {
                state = SCHEDULER_OP_STATE_DONE;
                scheduler_op_retval = http_op.http_op_retval;
            } else {
                if (log_flags.sched_op_debug) {
                    printf(
                        "Got master file from %s; parsing\n",
                        project->master_url
                    );
                }
                retval = parse_master_file();
                if (retval) {
                    state = SCHEDULER_OP_STATE_DONE;
                    scheduler_op_retval = retval;
                } else {
                    start_rpc();
                }
            }
        }
        break;
    case SCHEDULER_OP_STATE_RPC:
        if (http_op.http_op_state == HTTP_STATE_DONE) {
            state = SCHEDULER_OP_STATE_DONE;
            scheduler_op_retval = http_op.http_op_retval;
            http_ops->remove(&http_op);
        }
    }
    return 0;
}

SCHEDULER_REPLY::SCHEDULER_REPLY() {
    prefs_xml = 0;
    code_sign_key = 0;
    code_sign_key_signature = 0;
}

SCHEDULER_REPLY::~SCHEDULER_REPLY() {
    if (prefs_xml) free(prefs_xml);
    if (code_sign_key) free(code_sign_key);
    if (code_sign_key_signature) free(code_sign_key_signature);
}

int SCHEDULER_REPLY::parse(FILE* in) {
    char buf[256];
    int retval;
    if(in==NULL) {
        fprintf(stderr, "error: SCHEDULER_REPLY.parse: unexpected NULL pointer in\n");
        return ERR_NULL;
    }
    strcpy(message, "");
    strcpy(message_priority, "");
    request_delay = 0;
    hostid = 0;
    prefs_mod_time = 0;
    prefs_xml = 0;
    code_sign_key = 0;
    code_sign_key_signature = 0;

    fgets(buf, 256, in);
    if (!match_tag(buf, "<scheduler_reply>")) {
        fprintf(stderr, "SCHEDULER_REPLY::parse(): bad first tag %s\n", buf);
        return ERR_XML_PARSE;
    }
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</scheduler_reply>")) {
            return 0;
        } else if (parse_int(buf, "<hostid>", hostid)) {
            continue;
        } else if (parse_int(buf, "<request_delay>", request_delay)) {
            continue;
        } else if (parse_int(buf, "<prefs_mod_time>", prefs_mod_time)) {
            continue;
        } else if (match_tag(buf, "<preferences>")) {
            retval = dup_element_contents(in, "</preferences>", &prefs_xml);
            if (retval) return ERR_XML_PARSE;
        } else if (match_tag(buf, "<code_sign_key>")) {
            retval = dup_element_contents(in, "</code_sign_key>", &code_sign_key);
	    //fprintf(stderr, "code_sign_key: %s\n", code_sign_key);
            if (retval) {
		fprintf(stderr, "error: SCHEDULER_REPLY.parse: xml parsing error\n");
                return ERR_XML_PARSE;
	    }
        } else if (match_tag(buf, "<code_sign_key_signature>")) {
            retval = dup_element_contents(in, "</code_sign_key_signature>", &code_sign_key_signature);
            if (retval) return ERR_XML_PARSE;
        } else if (match_tag(buf, "<app>")) {
            APP app;
            app.parse(in);
            apps.push_back(app);
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO file_info;
            file_info.parse(in, true);
            file_infos.push_back(file_info);
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION av;
            av.parse(in);
            app_versions.push_back(av);
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT wu;
            wu.parse(in);
            workunits.push_back(wu);
        } else if (match_tag(buf, "<result>")) {
            RESULT result;      // make sure this is here so constructor
                                // gets called each time
            result.parse_server(in);
            results.push_back(result);
        } else if (match_tag(buf, "<result_ack>")) {
            RESULT result;
            result.parse_ack(in);
            result_acks.push_back(result);
        } else if (parse_str(buf, "<message", message)) {
            parse_attr(buf, "priority", message_priority);
            continue;
        } else {
            fprintf(stderr, "SCHEDULER_REPLY::parse: unrecognized %s\n", buf);
        }
    }
    fprintf(stderr, "SCHEDULER_REPLY::parse: no close tag\n");
    return ERR_XML_PARSE;
}
