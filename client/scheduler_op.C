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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "util.h"
#include "parse.h"
#include "error_numbers.h"

#include "client_state.h"
#include "client_types.h"
#include "file_names.h"
#include "log_flags.h"
#include "scheduler_op.h"

SCHEDULER_OP::SCHEDULER_OP(HTTP_OP_SET* h) {
    state = SCHEDULER_OP_STATE_IDLE;
    http_op.http_op_state = HTTP_STATE_IDLE;
    http_ops = h;
}

// try to get enough work to bring us up to max buffer level
//
int SCHEDULER_OP::init_get_work() {
    int retval;
    char err_msg[256];
    double ns = gstate.work_needed_secs();

    must_get_work = true;
    project = gstate.next_project(0);
    if (project) {
        retval = init_op_project(ns);
        if (retval) {
            sprintf(err_msg, "init_op_project failed, error %d\n", retval);
            backoff(project, err_msg);
            return retval;
        }
    } else {
        project = gstate.next_project_master_pending();
        if (project) {
            retval = init_master_fetch(project);
            if (retval) {
                sprintf(err_msg, "init_master_fetch failed, error %d\n", retval);
                backoff(project, err_msg);
            }
        }
    }

    return 0;
}

// report results for a particular project.
// also get work from that project if below max buffer level
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
    char err_msg[256];

    if (log_flags.sched_op_debug) {
        printf("init_op_project: starting op for %s\n", project->master_url);
    }

    // if project has no schedulers, skip everything else
    // and just get its master file.
    //
    url_index = 0;
    if (project->scheduler_urls.size() == 0) {
        retval = init_master_fetch(project);
        goto done;
    }
    retval = gstate.make_scheduler_request(project, ns);
    if (retval) {
        fprintf(stderr, "make_scheduler_request: %d\n", retval);
        goto done;
    }
    retval = start_rpc();
done:
    if (retval) {
        sprintf(err_msg,
            "scheduler init_op_project to %s failed, error %d\n",
            project->scheduler_urls[url_index].text, retval
        );
        backoff(project, err_msg);
    }
    return retval;
}

// Set a project's min RPC time to something in the future,
// based on exponential backoff
// TODO: integrate with other backoff sources
//
int SCHEDULER_OP::set_min_rpc_time(PROJECT* p) {
    double x;
    int exp_backoff;

    int n = p->nrpc_failures;
    if (n > RETRY_CAP) n = RETRY_CAP;

    // we've hit the limit on master_url fetches
    //
    if (p->master_fetch_failures >= MASTER_FETCH_RETRY_CAP) {
        if (log_flags.sched_op_debug) {
            printf("we've hit the limit on master_url fetches\n");
        }
        x = exp(drand()*p->master_fetch_failures);
        exp_backoff = (int) min((int)x,MASTER_FETCH_INTERVAL);
    } else {
        x = RETRY_BASE_PERIOD * exp(drand() * n);
        exp_backoff =  (int)max(SCHED_RETRY_DELAY_MIN,min(SCHED_RETRY_DELAY_MAX,(int) x));
    }
    p->min_rpc_time = time(0) + exp_backoff;
    if (log_flags.sched_op_debug) {
        printf(
            "setting min RPC time for %s to %d seconds from now\n",
            p->master_url, exp_backoff
        );
    }
    return 0;
}

// Back off on the scheduler and output an error msg if needed
//
void SCHEDULER_OP::backoff(PROJECT* p, char *error_msg ) {
    show_message(p, error_msg, MSG_ERROR);
    
    if (p->master_fetch_failures >= MASTER_FETCH_RETRY_CAP) {
        p->master_url_fetch_pending = true;
        set_min_rpc_time(p);
        return;
    }

    // if nrpc failures a multiple of master_fetch_period,
    // then set master_url_fetch_pending and initialize again
    //
    if (p->nrpc_failures == MASTER_FETCH_PERIOD) {
        p->master_url_fetch_pending = true;
        p->min_rpc_time = 0;
        p->nrpc_failures = 0;
        p->master_fetch_failures++;
    }
    
    p->nrpc_failures++;
    set_min_rpc_time(p);
}

// low-level routine to initiate an RPC
// If successful, creates an HTTP_OP that must be polled
//
int SCHEDULER_OP::start_rpc() {
    FILE *f;
    int retval;
	char msg_buf[256];

    safe_strcpy(scheduler_url, project->scheduler_urls[url_index].text);
    if (log_flags.sched_ops) {
        sprintf(msg_buf, "Sending request to scheduler: %s\n", scheduler_url);
        show_message(project,msg_buf,MSG_INFO);
    }
    if (log_flags.sched_op_debug) {
        f = fopen(SCHED_OP_REQUEST_FILE, "r");
        printf("--------- SCHEDULER REQUEST ---------\n");
        copy_stream(f, stdout);
        printf("--------- END ---------\n");
        fclose(f);
    }
    if (gstate.use_http_proxy) {
        http_op.use_http_proxy = true;
        safe_strcpy(http_op.proxy_server_name, gstate.proxy_server_name);
        http_op.proxy_server_port = gstate.proxy_server_port;
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
    if (gstate.use_http_proxy) {
        http_op.use_http_proxy = true;
        safe_strcpy(http_op.proxy_server_name, gstate.proxy_server_name);
        http_op.proxy_server_port = gstate.proxy_server_port;
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
    
    // couldn't find any urls in the master file?
    //
    if ((int) urls.size() == 0) {
        return -1;
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
    int retval, nresults;
    vector<STRING256> urls;
    bool changed, scheduler_op_done;
    bool action = false;
    char err_msg[256],*err_url;

    switch(state) {
    case SCHEDULER_OP_STATE_GET_MASTER:
        // here we're fetching the master file for a project
        //
        if (http_op.http_op_state == HTTP_STATE_DONE) {
            action = true;
            project->master_url_fetch_pending = false;
            gstate.set_client_state_dirty("master URL fetch done");
            http_ops->remove(&http_op);
            if (http_op.http_op_retval == 0) {
                if (log_flags.sched_op_debug) {
                    printf(
                        "Got master file from %s; parsing\n",
                        project->master_url
                    );
                }
                retval = parse_master_file(urls);
                if (retval) {
                    // master file parse failed.
                    //
                    project->master_fetch_failures++;
                    backoff(project, "Master file parse failed\n");
                    err_url = project->master_url;
                } else {
                    // everything succeeded.  Clear error counters
                    //
                    changed = update_urls(*project, urls);
                    if (changed) {
                        project->min_rpc_time = 0;
                        project->nrpc_failures = 0;
                        project->master_fetch_failures = 0;
                    }
                }
            } else {
                // master file fetch failed.
                //
                project->master_fetch_failures++;
                backoff(project, "Master file fetch failed\n");
                err_url = project->master_url;
            }

            // If don't have any schedulers for this project,
            // it may be the wrong URL.  notify the user
            //
            if (project->scheduler_urls.size() == 0) {
                sprintf(err_msg,
                    "Could not contact %s. Make sure this is the correct project URL.",
                    err_url
                );
                show_message(project, err_msg, MSG_ERROR);
                project->master_fetch_failures++;
                backoff(project, err_msg);
            }

            // See if need to read master file for another project
            //
            project = gstate.next_project_master_pending();
            if (project) {
                retval = init_master_fetch(project);
                if (retval) {
                    project->master_fetch_failures++;
                    backoff(project, "Master file fetch failed\n");
                    err_url = project->master_url;
                }
            } else {
                state = SCHEDULER_OP_STATE_IDLE;
                if (log_flags.sched_op_debug) {
                    printf("Scheduler_op: return to idle state\n");
                }
            }

        }
        break;
    case SCHEDULER_OP_STATE_RPC:

        // here we're doing a scheduler RPC
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

                // scheduler RPC failed.  Try another scheduler if one exists
                //
                url_index++;
                if (url_index < project->scheduler_urls.size()) {
                    start_rpc();
                } else {
                    backoff(project, "No schedulers responded");
                    if (must_get_work) {
                        project = gstate.next_project(project);
                        if (project) {
                            retval = init_op_project(gstate.work_needed_secs());
                        } else {
                            scheduler_op_done = true;
                        }
                    }
                    else {
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
                gstate.handle_scheduler_reply(project, scheduler_url, nresults);

                // if we asked for work and didn't get any,
                // back off this project
                //
                if (must_get_work && nresults==0) {
                    backoff(project, "No work from project\n");
                } else {
                    project->nrpc_failures = 0;
                    project->min_rpc_time = 0;
                }
                    
                // if we didn't get all the work we needed,
                // ask another project for work
                //
                if (must_get_work) {
                    double x = gstate.work_needed_secs();
                    if (x > 0) {
                        project = gstate.next_project(project);
                        if (project) {
                            retval = init_op_project(x);
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

        // If no outstanding ops, see if need a master fetch
        //
        if (scheduler_op_done) {
            project = gstate.next_project_master_pending();
            if (project) {
                retval = init_master_fetch(project);
                if (retval) {
                    if (log_flags.sched_op_debug) {
                        printf("Scheduler op: init_master_fetch failed.\n" );
                    }
                    backoff(project, "Scheduler op: init_master_fetch failed.\n" );
                }
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

    hostid = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    host_create_time = 0;
    request_delay = 0;
    strcpy(message, "");
    strcpy(message_priority, "");
    strcpy(project_name, "");
    global_prefs_xml = 0;
    project_prefs_xml = 0;
    strcpy(user_name, "");
    strcpy(team_name, "");
    user_total_credit = 0;
    user_expavg_credit = 0;
    strcpy(host_venue, "");
    user_create_time = 0;
    code_sign_key = 0;
    code_sign_key_signature = 0;

    p = fgets(buf, 256, in);
    if (!p) {
        fprintf(stderr, "SCHEDULER_REPLY::parse(): empty file\n");
        return ERR_XML_PARSE;
    }
    // First part of content should either be tag (HTTP 1.0) or
    // hex length of response (HTTP 1.1)
    if (!match_tag(buf, "<scheduler_reply>")) {
        fprintf(stderr, "SCHEDULER_REPLY::parse(): bad first tag %s\n", buf);
        return ERR_XML_PARSE;
    }
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<scheduler_reply>")) {
            // Do nothing
        } else if (match_tag(buf, "</scheduler_reply>")) return 0;
        else if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) continue;
        else if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_int(buf, "<user_create_time>", (int &)user_create_time)) continue;
        else if (parse_str(buf, "<team_name>", team_name, sizeof(team_name))) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        else if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) continue;
        else if (parse_int(buf, "<host_create_time>", (int &)host_create_time)) continue;
        else if (parse_int(buf, "<request_delay>", request_delay)) continue;
        else if (match_tag(buf, "<global_preferences>")) {
            retval = dup_element_contents(
                in,
                "</global_preferences>",
                &global_prefs_xml
            );
            if (retval) return ERR_XML_PARSE;
        } else if (match_tag(buf, "<project_preferences>")) {
            retval = dup_element_contents(
                in,
                "</project_preferences>",
                &project_prefs_xml
            );
            if (retval) return ERR_XML_PARSE;
        } else if (match_tag(buf, "<code_sign_key>")) {
            retval = dup_element_contents(
                in,
                "</code_sign_key>",
                &code_sign_key
            );
            if (retval) {
                fprintf(stderr, "error: SCHEDULER_REPLY.parse: xml parsing error\n");
                return ERR_XML_PARSE;
            }
        } else if (match_tag(buf, "<code_sign_key_signature>")) {
            retval = dup_element_contents(
                in,
                "</code_sign_key_signature>",
                &code_sign_key_signature
            );
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
