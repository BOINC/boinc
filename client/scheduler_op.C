// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#endif

#include "util.h"
#include "parse.h"
#include "error_numbers.h"

#include "client_state.h"
#include "client_types.h"
#include "client_msgs.h"
#include "file_names.h"
#include "log_flags.h"
#include "scheduler_op.h"

using std::vector;

extern void project_add_failed(PROJECT*);

SCHEDULER_OP::SCHEDULER_OP(HTTP_OP_SET* h) {
    state = SCHEDULER_OP_STATE_IDLE;
    http_op.http_op_state = HTTP_STATE_IDLE;
    http_ops = h;
}

// see if there's a pending master file fetch.  start it if so.
//
bool SCHEDULER_OP::check_master_fetch_start() {
    int retval;

    project = gstate.next_project_master_pending();
    if (project) {
        retval = init_master_fetch(project);
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "Couldn't read master page for %s: error %d",
                project->get_project_name(), retval
            );
            if (project->tentative) {
                msg_printf(project, MSG_ERROR, "Detaching from project - check for URL error");
                project_add_failed(project);
            } else {
                project->master_fetch_failures++;
                backoff(project, "Master file fetch failed\n");
            }
        }
        return true;
    }
    return false;
}

// try to get work, from any project from which we need it
// PRECONDITION: compute_work_requests() has been called
// to fill in PROJECT::work_request
//
int SCHEDULER_OP::init_get_work() {
    int retval;
    char err_msg[256];
    double ns;

    must_get_work = true;
    project = gstate.next_project_need_work(0);
    if (project) {
        ns = project->work_request;
        msg_printf(project, MSG_INFO, "Requesting %.2f seconds of work", ns);
        retval = init_op_project(ns);
        if (retval) {
            sprintf(err_msg, "init_op_project failed, error %d\n", retval);
            backoff(project, err_msg);
            return retval;
        }
    } else {
        check_master_fetch_start();
    }

    return 0;
}

// report results for a particular project.
// PRECONDITION: compute_work_requests() has been called
// to fill in PROJECT::work_request
//
int SCHEDULER_OP::init_return_results(PROJECT* p) {
    must_get_work = false;
    project = p;
    return init_op_project(p->work_request);
}

// try to initiate an RPC to the current project.
// If there are multiple schedulers, start with the first one
//
int SCHEDULER_OP::init_op_project(double ns) {
    int retval;
    char err_msg[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    scope_messages.printf("SCHEDULER_OP::init_op_project(): starting op for %s\n", project->master_url);

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
        msg_printf(project, MSG_ERROR, "make_scheduler_request: %d\n", retval);
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
//
int SCHEDULER_OP::set_min_rpc_time(PROJECT* p) {
    double exp_backoff;

    int n = p->nrpc_failures;
    if (n > gstate.retry_cap) n = gstate.retry_cap;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    // we've hit the limit on master_url fetches
    //
    if (p->master_fetch_failures >= gstate.master_fetch_retry_cap) {
        scope_messages.printf("SCHEDULER_OP::set_min_rpc_time(): we've hit the limit on master_url fetches\n");
        exp_backoff = calculate_exponential_backoff("scheduler_op/master_url",
            p->master_fetch_failures, gstate.sched_retry_delay_min,
            gstate.master_fetch_interval
        );
    } else {
        exp_backoff = calculate_exponential_backoff("scheduler_op",
            n, gstate.sched_retry_delay_min, gstate.sched_retry_delay_max,
            gstate.retry_base_period
        );
    }
    p->set_min_rpc_time(dtime() + exp_backoff);
    // note: we don't need to print a message now, it will be printed the
    // next time p->waiting_until_min_rpc_time() is called.
    return 0;
}

// Back off contacting scheduler and output an error msg if needed
//
void SCHEDULER_OP::backoff(PROJECT* p, char *error_msg ) {
    msg_printf(p, MSG_ERROR, error_msg);

    if (p->master_fetch_failures >= gstate.master_fetch_retry_cap) {
        p->master_url_fetch_pending = true;
    } else {
        // if nrpc failures is a multiple of master_fetch_period,
        // then set master_url_fetch_pending and initialize again
        //
        if (p->nrpc_failures == gstate.master_fetch_period) {
            p->master_url_fetch_pending = true;
            p->min_rpc_time = 0;
            p->nrpc_failures = 0;
            p->master_fetch_failures++;
        }

        p->nrpc_failures++;
    }
    set_min_rpc_time(p);
}

// low-level routine to initiate an RPC
// If successful, creates an HTTP_OP that must be polled
// PRECONDITION: the request file has been created
//
int SCHEDULER_OP::start_rpc() {
    int retval;
    char request_file[1024], reply_file[1024];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    safe_strcpy(scheduler_url, project->scheduler_urls[url_index].text);
    if (log_flags.sched_ops) {
        msg_printf(
            project, MSG_INFO,
            "Sending request to scheduler: %s\n", scheduler_url
        );
    }

    get_sched_request_filename(*project, request_file);
    get_sched_reply_filename(*project, reply_file);

    scope_messages.printf_file(request_file, "req:");

    http_op.set_proxy(&gstate.proxy_info);
    retval = http_op.init_post(
        scheduler_url, request_file, reply_file
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

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    project = p;
    scope_messages.printf("SCHEDULER_OP::init_master_fetch(): Fetching master file for %s\n", project->master_url);
    http_op.set_proxy(&gstate.proxy_info);
    retval = http_op.init_get(project->master_url, MASTER_FILE_NAME, true);
    if (retval) return retval;
    retval = http_ops->insert(&http_op);
    if (retval) return retval;
    state = SCHEDULER_OP_STATE_GET_MASTER;
    return 0;
}

static void trim(STRING256& str) {
	char* last_char = str.text + strlen(str.text);
	while (isspace(*(last_char-1)) && last_char > str.text) {
		--last_char;
	}
	*last_char = '\0';
	char *first_char = str.text;
	if (isspace(*first_char)) {
		while (isspace(*first_char)) {
			++first_char;
        }
		char* dest = str.text;
		while (*first_char) {
			*dest++ = *first_char++;
		}
		*dest = '\0';
	}
}

// parse a master file.
//
int SCHEDULER_OP::parse_master_file(vector<STRING256> &urls) {
    char buf[256];
    STRING256 str;
    FILE* f;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    f = fopen(MASTER_FILE_NAME, "r");
    if (!f) {
        msg_printf(project, MSG_ERROR, "Can't open master file\n");
        return ERR_FOPEN;
    }
    project->scheduler_urls.clear();
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<scheduler>", str.text, sizeof(str.text))) {
			trim(str);
            urls.push_back(str);
        }
    }
    fclose(f);
    scope_messages.printf("SCHEDULER_OP::parse_master_file(): got %d scheduler URLs\n", (int)urls.size());

    // couldn't find any urls in the master file?
    //
    if ((int) urls.size() == 0) {
        return ERR_XML_PARSE;
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
    bool action = false, err = false;
    char err_msg[256], *err_url=NULL;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

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
                scope_messages.printf(
                    "SCHEDULER_OP::poll(): Got master file from %s; parsing\n",
                     project->master_url
                );
                retval = parse_master_file(urls);
                if (retval) {
                    // master file parse failed.
                    //
                    if (project->tentative) {
                        PROJECT* project_temp = project;
                        project = 0;        // keep detach(0) from removing HTTP OP
                        state = SCHEDULER_OP_STATE_IDLE;    // avoid double remove
                        project_add_failed(project_temp);
                        err = true;
                    } else {
                        project->master_fetch_failures++;
                        backoff(project, "Master file parse failed\n");
                        err_url = project->master_url;
                    }
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

            // Done with master file fetch.
            // If tentative project and don't have any schedulers,
            // it may be the wrong URL.  notify the user
            //
            if (!err && project->scheduler_urls.size() == 0) {
                if (project->tentative) {
                    state = SCHEDULER_OP_STATE_IDLE;    // avoid double remove
                    project_add_failed(project);
                } else {
                    sprintf(err_msg,
                        "Could not contact any schedulers for %s.",
                        err_url
                    );
                    msg_printf(project, MSG_ERROR, err_msg);
                    project->master_fetch_failures++;
                    backoff(project, err_msg);
                }
            }

            // See if need to read master file for another project;
            // if not, we're done for now
            //
            if (!check_master_fetch_start()) {
                state = SCHEDULER_OP_STATE_IDLE;
                scope_messages.printf("SCHEDULER_OP::poll(): return to idle state\n");
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
                if (log_flags.sched_ops) {
                    msg_printf(project, MSG_ERROR,
                        "Scheduler RPC to %s failed\n",
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
                        int urgency = gstate.compute_work_requests();
                        project = gstate.next_project_need_work(project);
                        if (project && urgency != DONT_NEED_WORK) {
                            retval = init_op_project(project->work_request);
                        } else {
                            scheduler_op_done = true;
                        }
                    } else {
                        scheduler_op_done = true;
                    }
                }
            } else {
                if (log_flags.sched_ops) {
                    msg_printf(
                        project, MSG_INFO,
                        "Scheduler RPC to %s succeeded\n",
                        project->scheduler_urls[url_index].text
                    );
                }
                retval = gstate.handle_scheduler_reply(project, scheduler_url, nresults);

                // if this was a tentative project and we didn't get user name,
                // the account ID must be bad.  Tell the user.
                //
                if (project->tentative) {
                    if (retval || strlen(project->user_name)==0) {
                        state = SCHEDULER_OP_STATE_IDLE;    // avoid double remove
                        project_add_failed(project);
                    } else {
                        project->tentative = false;
                        retval = project->write_account_file();
                        if (retval) {
                            project_add_failed(project);
                        }
#if 0
                        gstate.calc_all_proj_size();
#endif
                    }
                } else {
                    switch (retval) {
                    case 0:
                        // if we asked for work and didn't get any,
                        // back off this project
                        //
                        if (must_get_work && nresults==0) {
                            backoff(project, "No work from project\n");
                        } else {
                            project->nrpc_failures = 0;
                            project->min_rpc_time = 0;
                        }
                        break;
                    case ERR_SERVER_REQ_DELAY:
                        // The server has requested a backoff of a specified
                        // amount, so lets honor it by not resetting the
                        // project->min_rpc_time value.
                        project->nrpc_failures = 0;
                        break;
                    case ERR_PROJECT_DOWN:
                        backoff(project, "Project is down");
                        break;
                    default:
                        backoff(project, "Can't parse scheduler reply");
                        break;
                    }
                }

                // if we didn't get all the work we needed,
                // ask another project for work
                //
                if (must_get_work) {
                    int urgency = gstate.compute_work_requests();
                    if (urgency != DONT_NEED_WORK) {
                        project = gstate.next_project_need_work(project);
                        if (project) {
                            retval = init_op_project(project->work_request);
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
                    scope_messages.printf("SCHEDULER_OP::poll(): init_master_fetch failed.\n" );
                    backoff(project, "Scheduler op: init_master_fetch failed.\n" );
                }
            } else {
                state = SCHEDULER_OP_STATE_IDLE;
                scope_messages.printf("SCHEDULER_OP::poll(): return to idle state\n");
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

// parse a scheduler reply.
// Some of the items go into the SCHEDULER_REPLY object.
// Others are copied straight to the PROJECT
//
int SCHEDULER_REPLY::parse(FILE* in, PROJECT* project) {
    char buf[256];
    int retval, x;
    MIOFILE mf;
    char delete_file_name[256];
    mf.init_file(in);
    bool found_start_tag = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    hostid = 0;
    request_delay = 0;
    strcpy(message, "");
    strcpy(message_priority, "");
    global_prefs_xml = 0;
    project_prefs_xml = 0;
    strcpy(host_venue, "");
    code_sign_key = 0;
    code_sign_key_signature = 0;
    message_ack = false;
    project_is_down = false;
    send_file_list = false;

    // First line should either be tag (HTTP 1.0) or
    // hex length of response (HTTP 1.1)
    //
    while (fgets(buf, 256, in)) {
        if (!found_start_tag) {
            if (match_tag(buf, "<scheduler_reply")) {
                found_start_tag = true;
            }
            continue;
        }
        if (match_tag(buf, "</scheduler_reply>")) return 0;
        else if (parse_str(buf, "<project_name>", project->project_name, sizeof(project->project_name))) continue;
		else if (parse_str(buf, "<user_name>", project->user_name, sizeof(project->user_name))) continue;
        else if (parse_double(buf, "<user_total_credit>", project->user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", project->user_expavg_credit)) continue;
        else if (parse_int(buf, "<user_create_time>", (int&)project->user_create_time)) continue;
		else if (parse_str(buf, "<team_name>", project->team_name, sizeof(project->team_name))) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_double(buf, "<host_total_credit>", project->host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", project->host_expavg_credit)) continue;
        else if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) continue;
        else if (parse_int(buf, "<host_create_time>", (int&)project->host_create_time)) continue;
        else if (parse_double(buf, "<request_delay>", request_delay)) continue;
        else if (match_tag(buf, "<global_preferences>")) {
            retval = dup_element_contents(
                in,
                "</global_preferences>",
                &global_prefs_xml
            );
            if (retval) return ERR_XML_PARSE;
            msg_printf(project, MSG_INFO,
                "General preferences have been updated\n"
            );
        } else if (match_tag(buf, "<project_preferences>")) {
            retval = dup_element_contents(
                in,
                "</project_preferences>",
                &project_prefs_xml
            );
            if (retval) return ERR_XML_PARSE;
        } else if (match_tag(buf, "<gui_urls>")) {
            std::string foo;
            retval = copy_element_contents(in, "</gui_urls>", foo);
            if (!retval) {
                project->gui_urls = "<gui_urls>\n"+foo+"</gui_urls>\n";
            }
            continue;
#if 0
        } else if (match_tag(buf, "<deletion_policy_priority/>")) {
            project->deletion_policy_priority = true;
            continue;
        } else if (match_tag(buf, "<deletion_policy_expire>")) {
            project->deletion_policy_expire = true;
            continue;
#endif
        } else if (match_tag(buf, "<code_sign_key>")) {
            retval = dup_element_contents(
                in,
                "</code_sign_key>",
                &code_sign_key
            );
            if (retval) {
                msg_printf(project, MSG_ERROR, "SCHEDULER_REPLY.parse(): xml parsing error\n");
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
            retval = app.parse(mf);
            if (retval) {
                msg_printf(project, MSG_ERROR, "Can't parse app in scheduler reply");
            } else {
                apps.push_back(app);
            }
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO file_info;
            retval = file_info.parse(mf, true);
            if (retval) {
                msg_printf(project, MSG_ERROR, "Can't parse file info in scheduler reply");
            } else {
                file_infos.push_back(file_info);
            }
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION av;
            retval = av.parse(mf);
            if (retval) {
                msg_printf(project, MSG_ERROR, "Can't parse app version in scheduler reply");
            } else {
                app_versions.push_back(av);
            }
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT wu;
            retval = wu.parse(mf);
            if (retval) {
                msg_printf(project, MSG_ERROR, "Can't parse work unit in scheduler reply");
            } else {
                workunits.push_back(wu);
            }
        } else if (match_tag(buf, "<result>")) {
            RESULT result;      // make sure this is here so constructor
                                // gets called each time
            retval = result.parse_server(mf);
            if (retval) {
                msg_printf(project, MSG_ERROR, "Can't parse result in scheduler reply");
            } else {
                results.push_back(result);
            }
        } else if (match_tag(buf, "<result_ack>")) {
            RESULT result;
            retval = result.parse_ack(in);
            if (retval) {
                msg_printf(project, MSG_ERROR, "Can't parse result ack in scheduler reply");
            } else {
                result_acks.push_back(result);
            }
        } else if (parse_str(buf, "<delete_file_info>", delete_file_name, sizeof(delete_file_name))) {
            STRING256 delete_file;
            strcpy(delete_file.text, delete_file_name);
            file_deletes.push_back(delete_file);
        } else if (parse_str(buf, "<message", message, sizeof(message))) {
            parse_attr(buf, "priority", message_priority, sizeof(message_priority));
            continue;
        } else if (match_tag(buf, "<message_ack/>")) {
            message_ack = true;
        } else if (match_tag(buf, "<project_is_down/>")) {
            project_is_down = true;
        } else if (parse_str(buf, "<email_hash>", project->email_hash, sizeof(project->email_hash))) {
            continue;
        } else if (parse_str(buf, "<cross_project_id>", project->cross_project_id, sizeof(project->cross_project_id))) {
            continue;
        } else if (match_tag(buf, "<trickle_down>")) {
            retval = gstate.handle_trickle_down(project, in);
            if (retval) {
                msg_printf(project, MSG_ERROR, "handle_trickle_down failed: %d\n", retval);
            }
            continue;
        } else if (match_tag(buf, "<non_cpu_intensive/>")) {
            project->non_cpu_intensive = true;
            continue;
        } else if (parse_int(buf, "<non_cpu_intensive>", x)) {
            project->non_cpu_intensive = (x != 0);
            continue;
        } else if (match_tag(buf, "<request_file_list/>")) {
            send_file_list = true;
        } else if (strlen(buf)>1){
            scope_messages.printf("SCHEDULER_REPLY::parse(): unrecognized %s\n", buf);
        }
    }
    if (found_start_tag) {
        msg_printf(project, MSG_ERROR, "No close tag in scheduler reply\n");
    } else {
        msg_printf(project, MSG_ERROR, "No start tag in scheduler reply\n");
    }
    return ERR_XML_PARSE;
}

const char *BOINC_RCSID_11c806525b = "$Id$";
