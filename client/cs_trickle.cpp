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
#include "str_util.h"
#include "sandbox.h"
#include "client_state.h"

using std::string;

// Scan project dir for file names of the form trickle_up_X_Y
// where X is a result name and Y is a timestamp.
// Convert them to XML (for sched request message)
//
int CLIENT_STATE::read_trickle_files(PROJECT* project, FILE* f) {
    char project_dir[256], *p, *q, result_name[256], fname[256];
    char* file_contents, path[256], newpath[256];
    string fn;
    time_t t;
    int retval;

    get_project_dir(project, project_dir, sizeof(project_dir));
    DirScanner ds(project_dir);

    // trickle-up filenames are of the form trickle_up_RESULTNAME_TIME[.sent]
    //
    while (ds.scan(fn)) {
        strcpy(fname, fn.c_str());
        if (strstr(fname, "trickle_up_") != fname) continue;
        q = fname + strlen("trickle_up_");
        p = strrchr(fname, '_');
        if (p <= q) continue;
        *p = 0;
        strcpy(result_name, q);
        *p = '_';
        t = atoi(p+1);

        sprintf(path, "%s/%s", project_dir, fname);
        retval = read_file_malloc(path, file_contents);
        if (retval) continue;
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
        free(file_contents);

        // append .sent to filename, so we'll know which ones to delete later
        //
        if (!ends_with(fname, ".sent")) {
            sprintf(newpath, "%s/%s.sent", project_dir, fname);
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
    char project_dir[256], path[256], fname[256];
    string fn;

    get_project_dir(project, project_dir, sizeof(project_dir));
    DirScanner ds(project_dir);

    while (ds.scan(fn)) {
        strcpy(fname, fn.c_str());
        if (!starts_with(fname, "trickle_up")) continue;
        if (!ends_with(fname, ".sent")) continue;
        sprintf(path, "%s/%s", project_dir, fname);
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
    char result_name[256], path[256];
    string body;
    int send_time=0;

    strcpy(result_name, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</trickle_down>")) {
            RESULT* rp = lookup_result(project, result_name);
            if (!rp) return ERR_NULL;
            ACTIVE_TASK* atp = lookup_active_task_by_result(rp);
            if (!atp) return ERR_NULL;
            sprintf(path, "%s/trickle_down_%d", atp->slot_dir, send_time);
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
    bool action = false;
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        for (j=0; j<p->trickle_up_ops.size(); j++) {
            TRICKLE_UP_OP *t = p->trickle_up_ops[j];
            action |= t->gui_http->poll();
        }
    }
    return action;
}

void send_replicated_trickles(PROJECT* p, string& msg) {
    for (unsigned int i=0; i<p->trickle_up_ops.size(); i++) {
        TRICKLE_UP_OP *t = p->trickle_up_ops[i];
        if (t->gui_http->is_busy()) return;
        t->do_rpc(msg);
    }
}

// A scheduler reply gave us a list of trickle handler URLs.
// If this is different than the list we currently have, replace it.
//
void update_trickle_up_urls(PROJECT* p, vector<string> &urls) {
    unsigned int i, j;
    bool lists_equal = true;
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
            lists_equal = false;
            break;
        }
    }
    if (lists_equal) {
        for (j=0; j<p->trickle_up_ops.size(); j++) {
            TRICKLE_UP_OP *t = p->trickle_up_ops[j];
            bool found = false;
            for (i=0; i<urls.size(); i++) {
                string& url = urls[i];
                if (t->url == url) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                lists_equal = false;
                break;
            }
        }
    }
    if (lists_equal) return;
    for (j=0; j<p->trickle_up_ops.size(); j++) {
        TRICKLE_UP_OP *t = p->trickle_up_ops[j];
        delete t;
    }
    p->trickle_up_ops.clear();
    for (i=0; i<urls.size(); i++) {
        string& url = urls[i];
        p->trickle_up_ops.push_back(new TRICKLE_UP_OP(url));
    }
}

int TRICKLE_UP_OP::do_rpc(string msg) {
    req_buf = strdup(msg.c_str());
    int retval = gui_http->do_rpc_post_str(
        this, const_cast<char*>(url.c_str()), req_buf
    );
    if (retval) {
        free(req_buf);
        req_buf = 0;
    }
    return retval;
}

int parse_trickle_up_urls(XML_PARSER& xp, std::vector<std::string>& urls) {
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

void trickle_up_request_message(MIOFILE& fout, PROJECT* p) {
    fout.printf(
        "<scheduler_request>\n"
        "    <authenticator>%s</authenticator>\n"
        "    <hostid>%d</hostid>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <core_client_major_version>%d</core_client_major_version>\n"
        "    <core_client_minor_version>%d</core_client_minor_version>\n"
        "    <core_client_release>%d</core_client_release>\n"
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
        gstate.core_client_version.release
    );
}
