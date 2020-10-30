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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#ifdef _MSC_VER
#define strdup _strdup
#endif
#else
#include "config.h"
#include <cstring>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"
#include "str_replace.h"
#include "str_util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "project.h"
#include "sandbox.h"

// Scan project dir for file names of the form trickle_up_X_Y
// where X is a result name and Y is a timestamp.
// Convert them to XML (for sched request message)
//
int CLIENT_STATE::read_trickle_files(PROJECT* project, FILE* f) {
    char *p, *q, result_name[256], fname[256];
    char* file_contents, path[MAXPATHLEN], newpath[MAXPATHLEN];
    string fn;
    time_t t;
    int retval;

    DirScanner ds(project->project_dir());

    // trickle-up filenames are of the form trickle_up_RESULTNAME_TIME[.sent]
    //
    const size_t prefix_len = strlen("trickle_up_");
    while (ds.scan(fn)) {
        safe_strcpy(fname, fn.c_str());
        if (strstr(fname, "trickle_up_") != fname) continue;
        q = fname + prefix_len;
        p = strrchr(fname, '_');
        if (p <= q) continue;
        *p = 0;
        safe_strcpy(result_name, q);
        *p = '_';
        t = atoi(p+1);

        snprintf(path, sizeof(path), "%s/%s", project->project_dir(), fname);
        retval = read_file_malloc(path, file_contents);
        if (retval) {
            if (log_flags.trickle_debug) {
                msg_printf(project, MSG_INFO,
                    "[trickle] can't read trickle file %s", path
                );
            }
            continue;
        }
        if (log_flags.trickle_debug) {
            msg_printf(project, MSG_INFO,
                "[trickle] read trickle file %s", path
            );
        }
        fprintf(f,
            "  <msg_from_host>\n"
            "      <result_name>%s</result_name>\n"
            "      <time>%d</time>\n"
            "%s\n"
            "  </msg_from_host>\n",
            result_name,
            (int)t,
            file_contents
        );
        send_replicated_trickles(project, file_contents, result_name, t);
        free(file_contents);

        // append .sent to filename, so we'll know which ones to delete later
        //
        if (!ends_with(fname, ".sent")) {
            snprintf(newpath, sizeof(newpath), "%s/%s.sent", project->project_dir(), fname);
            boinc_rename(path, newpath);
        }
    }
    return 0;
}

// Remove files when ack has been received.
// Remove only those ending with ".sent"
// (others arrived from application while RPC was happening)
//
int CLIENT_STATE::remove_trickle_files(PROJECT* project) {
    char path[MAXPATHLEN], fname[256];
    string fn;

    DirScanner ds(project->project_dir());

    while (ds.scan(fn)) {
        safe_strcpy(fname, fn.c_str());
        if (!starts_with(fname, "trickle_up")) continue;
        if (!ends_with(fname, ".sent")) continue;
        snprintf(path, sizeof(path), "%s/%s", project->project_dir(), fname);
        delete_project_owned_file(path, true);
    }
    return 0;
}

// parse a trickle-down message in a scheduler reply.
// Locate the corresponding active task,
// write a file in the slot directory,
// and notify the task
//
int CLIENT_STATE::handle_trickle_down(PROJECT* project, FILE* in) {
    char buf[256];
    char result_name[256], path[MAXPATHLEN];
    string body;
    int send_time=0;

    safe_strcpy(result_name, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</trickle_down>")) {
            RESULT* rp = lookup_result(project, result_name);
            if (!rp) return ERR_NULL;
            ACTIVE_TASK* atp = lookup_active_task_by_result(rp);
            if (!atp) return ERR_NULL;
            snprintf(path, sizeof(path), "%s/trickle_down_%d", atp->slot_dir, send_time);
            FILE* f = fopen(path, "w");
            if (!f) return ERR_FOPEN;
            fputs(body.c_str(), f);
            fclose(f);
            atp->have_trickle_down = true;
            return 0;
        } else if (parse_str(buf, "<result_name>", result_name, 256)) {
            continue;
        } else if (parse_int(buf, "<time>", send_time)) {
            continue;
        } else {
            body += buf;
        }
    }
    return ERR_XML_PARSE;
}

/////////////// STUFF RELATED TO REPLICATED TRICKLES FOLLOWS //////////////

bool trickle_up_poll() {
    unsigned int i, j;
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        for (j=0; j<p->trickle_up_ops.size(); j++) {
            TRICKLE_UP_OP *t = p->trickle_up_ops[j];
            t->gui_http->poll();
        }
    }
    return false;
}

static void trickle_up_request_message(
    PROJECT* p, const char* msg, char* result_name, int t, char* buf, size_t len
) {
    snprintf(buf, len,
        "<scheduler_request>\n"
        "    <authenticator>%s</authenticator>\n"
        "    <hostid>%d</hostid>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <core_client_major_version>%d</core_client_major_version>\n"
        "    <core_client_minor_version>%d</core_client_minor_version>\n"
        "    <core_client_release>%d</core_client_release>\n"
        "    <platform_name>%s</platform_name>\n"
        "    <msg_from_host>\n"
        "        <result_name>%s</result_name>\n"
        "        <time>%d</time>\n"
        "%s\n"
        "    </msg_from_host>\n"
        "</scheduler_request>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
        gstate.core_client_version.major,
        gstate.core_client_version.minor,
        //gstate.core_client_version.release,
        99,
        gstate.get_primary_platform(),
        result_name,
        t,
        msg
    );
}

void send_replicated_trickles(
    PROJECT* p, const char* msg, char* result_name, int now
) {
    if (!p->trickle_up_ops.size()) return;
    size_t trickle_len = strlen(msg) + 4096;
    char *buf = (char*)malloc(trickle_len);
    if (!buf) return;
    trickle_up_request_message(p, msg, result_name, now, buf, trickle_len);
    for (unsigned int i=0; i<p->trickle_up_ops.size(); i++) {
        TRICKLE_UP_OP *t = p->trickle_up_ops[i];
        if (t->gui_http->is_busy()) {
            if (log_flags.trickle_debug) {
                msg_printf(p, MSG_INFO,
                    "[trickle] Trickle channel to %s is busy", t->url.c_str()
                );
            }
            continue;
        }
        if (log_flags.trickle_debug) {
            msg_printf(p, MSG_INFO,
                "[trickle] Sending replicated trickle to %s", t->url.c_str()
            );
        }
        t->do_rpc(buf);
    }
    free(buf);
}

// A scheduler reply gave us a list of trickle handler URLs.
// Add and remove as needed.
//
void update_trickle_up_urls(PROJECT* p, vector<string> &urls) {
    unsigned int i, j;

    // add new URLs
    //
    for (i=0; i<urls.size(); i++) {
        string& url = urls[i];
        bool found = false;
        for (j=0; j<p->trickle_up_ops.size(); j++) {
            TRICKLE_UP_OP *t = p->trickle_up_ops[j];
            if (t->url == url) {
                found = true;
                break;
            }
        }
        if (!found) {
            p->trickle_up_ops.push_back(new TRICKLE_UP_OP(url));
            break;
        }
    }

    // remove old URLs
    //
    vector<TRICKLE_UP_OP*>::iterator iter = p->trickle_up_ops.begin();
    while (iter != p->trickle_up_ops.end()) {
        TRICKLE_UP_OP *t = *iter;
        bool found = false;
        for (i=0; i<urls.size(); i++) {
            string& url = urls[i];
            if (t->url == url) {
                found = true;
                break;
            }
        }
        if (!found) {
            gstate.http_ops->remove(&(t->gui_http->http_op));
            delete t;
            iter = p->trickle_up_ops.erase(iter);
        } else {
            ++iter;
        }
    }
}

int TRICKLE_UP_OP::do_rpc(const char* msg) {
    int n = (int)strlen(msg)+1;
    if (n<65536) n = 65536;     // make it big enough to handle the reply
    req_buf = (char*)malloc(n);
    strlcpy(req_buf, msg, n);
    int retval = gui_http->do_rpc_post_str(
        this, const_cast<char*>(url.c_str()), req_buf, n
    );
    if (retval) {
        msg_printf(0, MSG_INTERNAL_ERROR, "Trickle-up post failed: %d", retval);
        free(req_buf);
        req_buf = 0;
    }
    return retval;
}

int parse_trickle_up_urls(XML_PARSER& xp, vector<string>& urls) {
    string s;
    while (!xp.get_tag()) {
        if (xp.match_tag("/trickle_up_urls")) {
            return 0;
        }
        if (xp.parse_string("url", s)) {
            urls.push_back(s);
        }
    }
    return ERR_XML_PARSE;
}

void TRICKLE_UP_OP::handle_reply(int http_op_retval) {
    if (log_flags.trickle_debug) {
        msg_printf(0, MSG_INFO,
            "[trickle] Replicated trickle done; retval %d", http_op_retval
        );
    }
    if (req_buf) {
        free(req_buf);
        req_buf = 0;
    }
}
