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
#include <time.h>

#include "client_state.h"
#include "client_types.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "parse.h"
#include "scheduler_op.h"

SCHEDULER_OP::SCHEDULER_OP(HTTP_OP_SET* h) {
    state = SCHEDULER_OP_STATE_IDLE;
    http_op.http_op_state = HTTP_STATE_IDLE;
    http_ops = h;
}

// try to get enough work to bring us up to high-water mark
//
int SCHEDULER_OP::init_get_work() {
    int retval;
    double ns = gstate.work_needed_secs();

    must_get_work = true;
    project = gstate.next_project(0);
    if (project) {
        if( (retval=init_op_project(ns)) ) {
            project->nrpc_failures++;
            set_min_rpc_time(project);
            if (log_flags.sched_op_debug) {
                printf("init_get_work failed, error %d\n", retval);
            }
            return retval;
        }
    }
    return 0;
}

// report results for a particular project.
// also get work from that project if below high-water mark
//
int SCHEDULER_OP::init_return_results(PROJECT* p, double ns) {
    must_get_work = false;
    project = p;
    return init_op_project(ns);
}

// try to initiate an RPC to the current project.
// If there are multiple schedulers, start with the first one
//
int SCHEDULER_OP::init_op_project(double ns) {
    int retval;

    if (log_flags.sched_op_debug) {
        printf("init_op_project: starting op for %s\n", project->master_url);
    }

    // if project has no schedulers, skip everything else
    // and just get its master file.
    //
    if (project->scheduler_urls.size() == 0) {
        init_master_fetch(project);
        return 0;
    }
    url_index = 0;
    retval = gstate.make_scheduler_request(project, ns);
    if (retval) {
        fprintf(stderr, "make_scheduler_request: %d\n", retval);
        return retval;
    }
    return start_rpc();
}

// Set a project's min RPC time to something in the future,
// based on exponential backoff
// TODO: integrate with other backoff sources
//
int SCHEDULER_OP::set_min_rpc_time(PROJECT* p) {
    int x = RETRY_BASE_PERIOD;
    int i;

    int n = p->nrpc_failures;
    if (n > RETRY_CAP) n = RETRY_CAP;
    for (i=0; i<n; i++) x *= 2;
    p->min_rpc_time = time(0) + x;
    if (log_flags.sched_op_debug) {
        printf(
            "setting min RPC time for %s to %d seconds from now\n",
            p->master_url, x
        );
    }
    return 0;
}

// low-level routine to initiate an RPC
//
int SCHEDULER_OP::start_rpc() {
    FILE *f;
    int retval;

    strcpy(scheduler_url, project->scheduler_urls[url_index].text);
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
    retval = http_op.init_post(
        scheduler_url, SCHED_OP_REQUEST_FILE,
        SCHED_OP_RESULT_FILE
    );
    if (retval) return retval;
    retval = http_ops->insert(&http_op);
    if (retval) return retval;
    project->rpc_seqno++;
    state = SCHEDULER_OP_STATE_RPC;
    return 0;
}

// initiate a fetch of a project's master URL file
//
int SCHEDULER_OP::init_master_fetch(PROJECT* p) {
    int retval;

    project = p;
    if (log_flags.sched_op_debug) {
        printf("Fetching master file for %s\n", project->master_url);
    }
    retval = http_op.init_get(project->master_url, MASTER_FILE_NAME, true);
    if (retval) return retval;
    retval = http_ops->insert(&http_op);
    if (retval) return retval;
    state = SCHEDULER_OP_STATE_GET_MASTER;
    return 0;
}

// parse a master file.
//
int SCHEDULER_OP::parse_master_file(vector<STRING256> &urls) {
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
        if (parse_str(buf, "<scheduler>", str.text, sizeof(str.text))) {
            urls.push_back(str);
        }
    }
    fclose(f);
    if (log_flags.sched_op_debug) {
        printf("Parsed master file; got %d scheduler URLs\n", (int)urls.size());
    }
    return 0;
}

// A master file has just been read.
// transfer scheduler urls to project.
// Return true if any of them is new
//
bool SCHEDULER_OP::update_urls(PROJECT& project, vector<STRING256> &urls) {
    unsigned int i, j;
    bool found, any_new;

    any_new = false;
    for (i=0; i<urls.size(); i++) {
        found = false;
        for (j=0; j<project.scheduler_urls.size(); j++) {
            if (!strcmp(urls[i].text, project.scheduler_urls[i].text)) {
                found = true;
                break;
            }
        }
        if (!found) any_new = true;
    }

    project.scheduler_urls.clear();
    for (i=0; i<urls.size(); i++) {
        project.scheduler_urls.push_back(urls[i]);
    }

    return any_new;
}

// poll routine.  If an operation is in progress, check for completion
//
bool SCHEDULER_OP::poll() {
    int retval;
    vector<STRING256> urls;
    bool changed, scheduler_op_done;
    bool action = false;

    switch(state) {
    case SCHEDULER_OP_STATE_GET_MASTER:
        // here we're fetching the master file for a project
        //
        if (http_op.http_op_state == HTTP_STATE_DONE) {
            action = true;
            project->master_url_fetch_pending = false;
            http_ops->remove(&http_op);
            if (http_op.http_op_retval == 0) {
                if (log_flags.sched_op_debug) {
                    printf(
                        "Got master file from %s; parsing\n",
                        project->master_url
                    );
                }
                retval = parse_master_file(urls);
                if (retval == 0) {
                    changed = update_urls(*project, urls);
                    if (changed) {
                        project->min_rpc_time = 0;
                        project->nrpc_failures = 0;
                    }
                } else {
                    // master file parse failed.  treat like RPC error
                    //
                    project->nrpc_failures++;
                    set_min_rpc_time(project);
                    if (log_flags.sched_op_debug) {
                        printf("Master file parse failed\n");
                    }
                }
            } else {
                // fetch of master file failed.  Treat like RPC error
                //
                project->nrpc_failures++;
                set_min_rpc_time(project);
                if (log_flags.sched_op_debug) {
                    printf("Master file fetch failed\n");
                }
            }
            project = gstate.next_project_master_pending();
            if (project) {
                init_master_fetch(project);
            } else {
                state = SCHEDULER_OP_STATE_IDLE;
                if (log_flags.sched_op_debug) {
                    printf("Scheduler_op: return to idle state\n");
                }
            }
        }
        break;
    case SCHEDULER_OP_STATE_RPC:
        // here we're doing a scheduler RPC to some project
        //
        scheduler_op_done = false;
        if (http_op.http_op_state == HTTP_STATE_DONE) {
            action = true;
            http_ops->remove(&http_op);
            if (http_op.http_op_retval) {
                if (log_flags.sched_op_debug) {
                    printf(
                        "scheduler RPC to %s failed\n",
                        project->scheduler_urls[url_index].text
                    );
                }
                url_index++;
                if (url_index < project->scheduler_urls.size()) {
                    start_rpc();
                } else {
                    project->nrpc_failures++;
                    if ((project->nrpc_failures % MASTER_FETCH_PERIOD) == 0) {
                        project->master_url_fetch_pending = true;
                    }
                    set_min_rpc_time(project);
                    if (must_get_work) {
                        project = gstate.next_project(project);
                        if (project) {
                            if( (retval=init_op_project(gstate.work_needed_secs())) ) {
                                project->nrpc_failures++;
                                set_min_rpc_time(project);
                                if (log_flags.sched_op_debug) {
                                    printf(
                                        "scheduler init_op_project to %s failed, error %d\n",
                                        project->scheduler_urls[url_index].text, retval
                                    );
                                }
                            }
                        } else {
                            scheduler_op_done = true;
                        }
                    } else {
                        scheduler_op_done = true;
                    }
                }
            } else {
                if (log_flags.sched_op_debug) {
                    printf(
                        "scheduler RPC to %s succeeded\n",
                        project->scheduler_urls[url_index].text
                    );
                }
                project->nrpc_failures = 0;
                gstate.handle_scheduler_reply(project, scheduler_url);
                if (must_get_work) {
                    double x = gstate.work_needed_secs();
                    if (x > 0) {
                        project = gstate.next_project(project);
                        if (project) {
                            if( (retval=init_op_project(x)) ) {
                                project->nrpc_failures++;
                                set_min_rpc_time(project);
                                if (log_flags.sched_op_debug) {
                                    printf(
                                        "scheduler init_op_project to %s failed, error %d\n",
                                        project->scheduler_urls[url_index].text, retval
                                    );
                                }
                            }
                        } else {
                            scheduler_op_done = true;
                        }
                    } else {
                        scheduler_op_done = true;
                    }
                } else {
		    scheduler_op_done = true;
		}
            }
        }
        if (scheduler_op_done) {
            project = gstate.next_project_master_pending();
            if (project) {
                init_master_fetch(project);
            } else {
                state = SCHEDULER_OP_STATE_IDLE;
                if (log_flags.sched_op_debug) {
                    printf("Scheduler_op: return to idle state\n");
                }
            }
        }
        break;
    default:
        break;
    }
    return action;
}

SCHEDULER_REPLY::SCHEDULER_REPLY() {
    global_prefs_xml = 0;
    project_prefs_xml = 0;
    code_sign_key = 0;
    code_sign_key_signature = 0;
}

SCHEDULER_REPLY::~SCHEDULER_REPLY() {
    if (global_prefs_xml) free(global_prefs_xml);
    if (project_prefs_xml) free(project_prefs_xml);
    if (code_sign_key) free(code_sign_key);
    if (code_sign_key_signature) free(code_sign_key_signature);
}

int SCHEDULER_REPLY::parse(FILE* in) {
    char buf[256], *p;
    int retval;

    strcpy(message, "");
    strcpy(message_priority, "");
    request_delay = 0;
    hostid = 0;
    global_prefs_xml = 0;
    project_prefs_xml = 0;
    code_sign_key = 0;
    code_sign_key_signature = 0;

    p = fgets(buf, 256, in);
    // First part of content should either be tag (HTTP 1.0) or 
    // hex length of response (HTTP 1.1)
    if (!match_tag(buf, "<scheduler_reply>")) {
        fprintf(stderr, "SCHEDULER_REPLY::parse(): bad first tag %s\n", buf);
        return ERR_XML_PARSE;
    }
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<scheduler_reply>")) {
            // Do nothing
        } else if (match_tag(buf, "</scheduler_reply>")) {
            return 0;
        } else if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) {
            continue;
        } else if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) {
            continue;
        } else if (parse_double(buf, "<total_credit>", total_credit)) {
            continue;
        } else if (parse_double(buf, "<expavg_credit>", expavg_credit)) {
            continue;
        } else if (parse_int(buf, "<hostid>", hostid)) {
            continue;
        } else if (parse_int(buf, "<request_delay>", request_delay)) {
            continue;
        } else if (match_tag(buf, "<global_preferences>")) {
            retval = dup_element_contents(in, "</global_preferences>", &global_prefs_xml);
            if (retval) return ERR_XML_PARSE;
        } else if (match_tag(buf, "<project_preferences>")) {
            retval = dup_element_contents(in, "</project_preferences>", &project_prefs_xml);
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
        } else if (parse_str(buf, "<message", message, sizeof(message))) {
            parse_attr(buf, "priority", message_priority, sizeof(message_priority));
            continue;
        } else {
            fprintf(stderr, "SCHEDULER_REPLY::parse: unrecognized %s\n", buf);
        }
    }
    fprintf(stderr, "SCHEDULER_REPLY::parse: no close tag\n");
    return ERR_XML_PARSE;
}
