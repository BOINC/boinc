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

// GUI RPC server side (the actual RPCs)

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/un.h>
#include <vector>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif

#include "util.h"
#include "error_numbers.h"
#include "parse.h"
#include "network.h"
#include "filesys.h"

#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"

using std::string;
using std::vector;

static void auth_failure(MIOFILE& fout) {
    fout.printf("<unauthorized/>\n");
}

void GUI_RPC_CONN::handle_auth1(MIOFILE& fout) {
    sprintf(nonce, "%f", dtime());
    fout.printf("<nonce>%s</nonce>\n", nonce);
}

void GUI_RPC_CONN::handle_auth2(char* buf, MIOFILE& fout) {
    char nonce_hash[256], nonce_hash_correct[256], buf2[256];
    if (!parse_str(buf, "<nonce_hash>", nonce_hash, 256)) {
        auth_failure(fout);
        return;
    }
    sprintf(buf2, "%s%s", nonce, gstate.gui_rpcs.password);
    md5_block((const unsigned char*)buf2, strlen(buf2), nonce_hash_correct);
    if (strcmp(nonce_hash, nonce_hash_correct)) {
        auth_failure(fout);
        return;
    }
    fout.printf("<authorized/>\n");
    auth_needed = false;
}

static void handle_get_project_status(MIOFILE& fout) {
    unsigned int i;
    fout.printf("<projects>\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->write_state(fout, true);
    }
    fout.printf("</projects>\n");
}

static void handle_get_disk_usage(MIOFILE& fout) {
    unsigned int i;
    double size;

    fout.printf("<projects>\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        gstate.project_disk_usage(p, size);
        fout.printf(
            "<project>\n"
            "  <master_url>%s</master_url>\n"
            "  <disk_usage>%f</disk_usage>\n"
            "</project>\n",
            p->master_url, size
        );
    }
    fout.printf("</projects>\n");
}

static PROJECT* get_project(char* buf, MIOFILE& fout) {
    string url;
    if (!parse_str(buf, "<project_url>", url)) {
        fout.printf("<error>Missing project URL</error>\n");
        return 0;
    }
    PROJECT* p = gstate.lookup_project(url.c_str());
    if (!p) {
        fout.printf("<error>No such project</error>\n");
        return 0 ;
    }
    return p;
}

static void handle_result_show_graphics(char* buf, MIOFILE& fout) {
    string result_name;
    GRAPHICS_MSG gm;
    ACTIVE_TASK* atp;

    if        (match_tag(buf, "<full_screen/>")) {
        gm.mode = MODE_FULLSCREEN;
    } else if (match_tag(buf, "<hide/>")) {
        gm.mode = MODE_HIDE_GRAPHICS;
    } else {
        gm.mode = MODE_WINDOW;
    }

    parse_str(buf, "<window_station>", gm.window_station, sizeof(gm.window_station));
    parse_str(buf, "<desktop>", gm.desktop, sizeof(gm.desktop));
    parse_str(buf, "<display>", gm.display, sizeof(gm.display));

    if (parse_str(buf, "<result_name>", result_name)) {
        PROJECT* p = get_project(buf, fout);
        if (!p) {
            fout.printf("<error>No such project</error>\n");
            return;
       }
        RESULT* rp = gstate.lookup_result(p, result_name.c_str());
        if (!rp) {
            fout.printf("<error>No such result</error>\n");
            return;
        }
        atp = gstate.lookup_active_task_by_result(rp);
        if (!atp) {
            fout.printf("<error>no such result</error>\n");
            return;
        }
        atp->request_graphics_mode(gm);
    } else {
        for (unsigned int i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
            atp = gstate.active_tasks.active_tasks[i];
            if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            atp->request_graphics_mode(gm);
        }
    }
    fout.printf("<success/>\n");
}


static void handle_project_op(char* buf, MIOFILE& fout, const char* op) {
    int retval;

    PROJECT* p = get_project(buf, fout);
    if (!p) {
        fout.printf("<error>no such project</error>\n");
        return;
    }
    if (!strcmp(op, "reset")) {
        gstate.request_schedule_cpus("project reset by user");
        gstate.reset_project(p);    // writes state file
    } else if (!strcmp(op, "suspend")) {
        if (p->non_cpu_intensive) {
            msg_printf(p, MSG_ERROR, "Can't suspend non-CPU-intensive project");
        } else {
            p->suspended_via_gui = true;
            gstate.request_schedule_cpus("project suspended by user");
            gstate.set_client_state_dirty("Project suspended by user");
        }
    } else if (!strcmp(op, "resume")) {
        p->suspended_via_gui = false;
        gstate.request_schedule_cpus("project resumed by user");
        gstate.set_client_state_dirty("Project resumed by user");
    } else if (!strcmp(op, "detach")) {
        if (p->attached_via_acct_mgr) {
            msg_printf(p, MSG_ERROR,
                "This project must be detached using the account manager web site."
            );
            fout.printf("<error>must detach using account manager</error>");
            return;
        }
        gstate.detach_project(p);       // writes state file

        // if project_init.xml refers to this project,
        // delete the file, otherwise we'll just
        // reattach the next time the core client starts
        //
        if (!strcmp(p->master_url, gstate.project_init.url)) {
            retval = gstate.project_init.remove();
            if (retval) {
                msg_printf(p, MSG_ERROR,
                    "Can't delete project init file: %s", boincerror(retval)
                );
            }
        }
        gstate.request_schedule_cpus("project detached by user");
    } else if (!strcmp(op, "update")) {
        p->sched_rpc_pending = true;
        p->min_rpc_time = 0;
        gstate.set_client_state_dirty("Project updated by user");
    } else if (!strcmp(op, "nomorework")) {
        p->dont_request_more_work = true;
        gstate.set_client_state_dirty("Project modified by user");
    } else if (!strcmp(op, "allowmorework")) {
        p->dont_request_more_work = false;
        gstate.set_client_state_dirty("Project modified by user");
    }
    fout.printf("<success/>\n");
}

static void handle_set_run_mode(char* buf, MIOFILE& fout) {
    if (match_tag(buf, "<always")) {
        gstate.user_run_request = USER_RUN_REQUEST_ALWAYS;
    } else if (match_tag(buf, "<never")) {
        gstate.user_run_request = USER_RUN_REQUEST_NEVER;
    } else if (match_tag(buf, "<auto")) {
        gstate.user_run_request = USER_RUN_REQUEST_AUTO;
    } else {
        fout.printf("<error>Missing mode</error>\n");
        return;
    }
    gstate.set_client_state_dirty("Set run mode RPC");
    fout.printf("<success/>\n");
}

static void handle_get_run_mode(char* , MIOFILE& fout) {
    fout.printf("<run_mode>\n");
    switch (gstate.user_run_request) {
    case USER_RUN_REQUEST_ALWAYS:
        fout.printf("<always/>\n");
        break;
    case USER_RUN_REQUEST_NEVER:
        fout.printf("<never/>\n");
        break;
    case USER_RUN_REQUEST_AUTO:
        fout.printf("<auto/>\n");
        break;
    default:
        fout.printf("<error>Unknown run mode</error>\n");
    }
    fout.printf("</run_mode>\n");
}

static void handle_set_network_mode(char* buf, MIOFILE& fout) {
    if (match_tag(buf, "<always")) {
        gstate.user_network_request = USER_RUN_REQUEST_ALWAYS;
    } else if (match_tag(buf, "<never")) {
        gstate.user_network_request = USER_RUN_REQUEST_NEVER;
    } else if (match_tag(buf, "<auto")) {
        gstate.user_network_request = USER_RUN_REQUEST_AUTO;
    } else {
        fout.printf("<error>Missing mode</error>\n");
        return;
    }
    gstate.set_client_state_dirty("Set network mode RPC");
    fout.printf("<success/>\n");
}

static void handle_get_network_mode(char* , MIOFILE& fout) {
    fout.printf("<network_mode>\n");
    switch (gstate.user_network_request) {
    case USER_RUN_REQUEST_ALWAYS:
        fout.printf("<always/>\n");
        break;
    case USER_RUN_REQUEST_NEVER:
        fout.printf("<never/>\n");
        break;
    case USER_RUN_REQUEST_AUTO:
        fout.printf("<auto/>\n");
        break;
    default:
        fout.printf("<error>Unknown network mode</error>\n");
    }
    fout.printf("</network_mode>\n");
}

static void handle_run_benchmarks(char* , MIOFILE& fout) {
    gstate.start_cpu_benchmarks();
    fout.printf("<success/>\n");
}

static void handle_set_proxy_settings(char* buf, MIOFILE& fout) {
    MIOFILE in;
    in.init_buf(buf);
    gstate.proxy_info.parse(in);
    gstate.set_client_state_dirty("Set proxy settings RPC");
    fout.printf("<success/>\n");

    // tell running apps to reread app_info file (for F@h)
    //
    gstate.active_tasks.request_reread_app_info();
}

static void handle_get_proxy_settings(char* , MIOFILE& fout) {
    gstate.proxy_info.write(fout);
}

static void handle_get_activity_state(char* , MIOFILE& fout) {
    fout.printf("<activity_state>\n");
    if ( gstate.tasks_suspended ) {
        fout.printf("    <activities_suspended/>\n");
    }
    if ( gstate.network_suspended ) {
        fout.printf("    <network_suspended/>\n");
    }
    fout.printf("</activity_state>\n");
}

// params:
// [ <seqno>n</seqno> ]
//    return only msgs with seqno > n; if absent or zero, return all
//
static void handle_get_messages(char* buf, MIOFILE& fout) {
    int seqno=0, i, j;
    unsigned int k;
    MESSAGE_DESC* mdp;

    parse_int(buf, "<seqno>", seqno);

    // messages are stored in descreasing seqno,
    // i.e. newer ones are at the head of the vector.
    // compute j = index of first message to return
    //
    j = message_descs.size()-1;
    for (k=0; k<message_descs.size(); k++) {
        mdp = message_descs[k];
        if (mdp->seqno <= seqno) {
            j = k-1;
            break;
        }
    }

    fout.printf("<msgs>\n");
    for (i=j; i>=0; i--) {
        mdp = message_descs[i];
        fout.printf(
            "<msg>\n"
            " <project>%s</project>\n"
            " <pri>%d</pri>\n"
            " <seqno>%d</seqno>\n"
            " <body>\n%s\n</body>\n"
            " <time>%d</time>\n",
            mdp->project_name,
            mdp->priority,
            mdp->seqno,
            mdp->message.c_str(),
            mdp->timestamp
        );
        fout.printf("</msg>\n");
    }
    fout.printf("</msgs>\n");
}

// <retry_file_transfer>
//    <project_url>XXX</project_url>
//    <filename>XXX</filename>
// </retry_file_transfer>
//
static void handle_file_transfer_op(char* buf, MIOFILE& fout, const char* op) {
    string filename;

    PROJECT* p = get_project(buf, fout);
    if (!p) {
        fout.printf("<error>No such project</error>\n");
        return;
    }

    if (!parse_str(buf, "<filename>", filename)) {
        fout.printf("<error>Missing filename</error>\n");
        return;
    }
    
    FILE_INFO* f = gstate.lookup_file_info(p, filename.c_str());
    if (!f) {
        fout.printf("<error>No such file</error>\n");
        return;
    }
    
    PERS_FILE_XFER* pfx = f->pers_file_xfer;
    if (!pfx) {
        fout.printf("<error>No such transfer waiting</error>\n");
        return;
    }

    if (!strcmp(op, "retry")) {
        pfx->next_request_time = 0;
            // leave file-level backoff mode
        f->project->file_xfer_succeeded(pfx->is_upload);
            // and leave project-level backoff mode
    } else if (!strcmp(op, "abort")) {
        f->pers_file_xfer->abort();
    } else {
        fout.printf("<error>unknown op</error>\n");
        return;
    }
    gstate.set_client_state_dirty("File transfer RPC");
    fout.printf("<success/>\n");
}

static void handle_result_op(char* buf, MIOFILE& fout, const char* op) {
    RESULT* rp;
    char result_name[256];
    ACTIVE_TASK* atp;

    PROJECT* p = get_project(buf, fout);
    if (!p) {
        fout.printf("<error>No such project</error>\n");
        return;
    }

    if (!parse_str(buf, "<name>", result_name, sizeof(result_name))) {
        fout.printf("<error>Missing result name</error>\n");
        return;
    }

    rp = gstate.lookup_result(p, result_name);
    if (!rp) {
        fout.printf("<error>no such result</error>\n");
        return;
    }

    if (!strcmp(op, "abort")) {
        atp = gstate.lookup_active_task_by_result(rp);
        if (atp) {
            atp->abort_task(ERR_ABORTED_VIA_GUI, "aborted by user");
        } else {
            rp->abort_inactive(ERR_ABORTED_VIA_GUI);
        }
    } else if (!strcmp(op, "suspend")) {
        if (p->non_cpu_intensive) {
            msg_printf(p, MSG_ERROR, "Can't suspend non-CPU-intensive result");
        } else {
            rp->suspended_via_gui = true;
        }
    } else if (!strcmp(op, "resume")) {
        rp->suspended_via_gui = false;
    }
    gstate.request_schedule_cpus("result suspended, resumed or aborted by user");
    gstate.set_client_state_dirty("Result RPC");
    fout.printf("<success/>\n");
}

static void handle_get_host_info(char*, MIOFILE& fout) {
    gstate.host_info.write(fout);
}

static void handle_get_screensaver_mode(char*, MIOFILE& fout) {
    int ss_result = gstate.ss_logic.get_ss_status();
    fout.printf(
        "<screensaver_mode>\n"
        "    <status>%d</status>\n"
        "</screensaver_mode>\n",
        ss_result
    );
}

static void handle_set_screensaver_mode(char* buf, MIOFILE& fout) {
    double blank_time = 0.0;
    GRAPHICS_MSG gm;

    parse_double(buf, "<blank_time>", blank_time);
    parse_str(buf, "<desktop>", gm.desktop, sizeof(gm.desktop));
    parse_str(buf, "<window_station>", gm.window_station, sizeof(gm.window_station));
    parse_str(buf, "<display>", gm.display, sizeof(gm.display));
    if (match_tag(buf, "<enabled")) {
        gstate.ss_logic.start_ss(gm, blank_time );
    } else {
        gstate.ss_logic.stop_ss();
    }
    fout.printf("<success/>\n");
}

static void handle_quit(char*, MIOFILE& fout) {
    gstate.requested_exit = true;
    fout.printf("<success/>\n");
}

static void handle_acct_mgr_info(char*, MIOFILE& fout) {
    fout.printf(
        "<acct_mgr_info>\n"
        "   <acct_mgr_url>%s</acct_mgr_url>\n"
        "   <acct_mgr_name>%s</acct_mgr_name>\n"
        "   %s\n"
        "</acct_mgr_info>\n",
        gstate.acct_mgr_info.acct_mgr_url,
        gstate.acct_mgr_info.acct_mgr_name,
        strlen(gstate.acct_mgr_info.login_name)?"<have_credentials/>":""
    );
}

static void handle_get_statistics(char*, MIOFILE& fout) {
    fout.printf("<statistics>\n");
    for (std::vector<PROJECT*>::iterator i=gstate.projects.begin();
        i!=gstate.projects.end();++i
    ) {
        (*i)->write_statistics(fout,true);
    }
    fout.printf("</statistics>\n");
}

static void handle_network_status(char*, MIOFILE& fout) {
    fout.printf("<status>%d</status>\n", gstate.network_status());
}

static void handle_get_cc_status(MIOFILE& fout) {
    fout.printf(
        "<cc_status>\n"
        "   <network_status>%d</network_status>\n"
        "   <ams_password_error>%d</ams_password_error>\n"
        "</cc_status>\n",
        gstate.network_status(),
        gstate.acct_mgr_info.password_error?1:0
    );
}

static void handle_network_available(char*, MIOFILE&) {
    gstate.network_available();
}

static void handle_get_project_init_status(char*, MIOFILE& fout) {
    fout.printf(
        "<get_project_init_status>\n"
        "    <url>%s</url>\n"
        "    <name>%s</name>\n"
        "    %s\n"
        "</get_project_init_status>\n",
        gstate.project_init.url,
        gstate.project_init.name,
        strlen(gstate.project_init.account_key)?"<has_account_key/>":""
    );
}

static void handle_get_project_config(char* buf, MIOFILE& fout) {
    string url;

    parse_str(buf, "<url>", url);

    canonicalize_master_url(url);
    gstate.get_project_config_op.do_rpc(url);
    fout.printf("<success/>\n");
}

static void handle_get_project_config_poll(char*, MIOFILE& fout) {
    if (gstate.get_project_config_op.error_num) {
        fout.printf(
            "<project_config>\n"
            "    <error_num>%d</error_num>\n"
            "</project_config>\n",
            gstate.get_project_config_op.error_num
        );
    } else {
        fout.printf("%s", gstate.get_project_config_op.reply.c_str());
    }
}

static void handle_lookup_account(char* buf, MIOFILE& fout) {
    ACCOUNT_IN ai;

    ai.parse(buf);

    gstate.lookup_account_op.do_rpc(ai);
    fout.printf("<success/>\n");
}

static void handle_lookup_account_poll(char*, MIOFILE& fout) {
    if (gstate.lookup_account_op.error_num) {
        fout.printf(
            "<account_out>\n"
            "    <error_num>%d</error_num>\n"
            "</account_out>\n",
            gstate.lookup_account_op.error_num
        );
    } else {
        fout.printf("%s", gstate.lookup_account_op.reply.c_str());
    }
}

static void handle_create_account(char* buf, MIOFILE& fout) {
    ACCOUNT_IN ai;

    ai.parse(buf);

    gstate.create_account_op.do_rpc(ai);
    fout.printf("<success/>\n");
}

static void handle_create_account_poll(char*, MIOFILE& fout) {
    if (gstate.create_account_op.error_num) {
        fout.printf(
            "<account_out>\n"
            "    <error_num>%d</error_num>\n"
            "</account_out>\n",
            gstate.create_account_op.error_num
        );
    } else {
        fout.printf("%s", gstate.create_account_op.reply.c_str());
    }
}

static void handle_project_attach(char* buf, MIOFILE& fout) {
    string url, authenticator;
    PROJECT* p = NULL;
    bool use_config_file = false;
    bool already_attached = false;
    unsigned int i;

    if (!parse_bool(buf, "use_config_file", use_config_file)) {
        if (!parse_str(buf, "<project_url>", url)) {
            fout.printf("<error>Missing URL</error>\n");
            return;
        }

        for (i=0; i<gstate.projects.size(); i++) {
            p = gstate.projects[i];
            if (url == p->master_url) already_attached = true;
        }

        if (already_attached) {
            fout.printf("<error>Already attached to project</error>\n");
            return;
        }

        if (!parse_str(buf, "<authenticator>", authenticator)) {
            fout.printf("<error>Missing authenticator</error>\n");
            return;
        }

        if (authenticator.empty()) {
            fout.printf("<error>Missing authenticator</error>\n");
            return;
        }
    } else {
        if (!strlen(gstate.project_init.url)) {
            fout.printf("<error>Missing URL</error>\n");
            return;
        }

        if (!strlen(gstate.project_init.account_key)) {
            fout.printf("<error>Missing authenticator</error>\n");
            return;
        }

        url = gstate.project_init.url;
        authenticator = gstate.project_init.account_key;
    }

    // clear out any previous messages from any previous attach
    //   to project.
    //
    gstate.project_attach.messages.clear();
    gstate.add_project(url.c_str(), authenticator.c_str());
    gstate.project_attach.error_num = ERR_IN_PROGRESS;
    fout.printf("<success/>\n");
}

static void handle_project_attach_poll(char*, MIOFILE& fout) {
    unsigned int i;
    fout.printf(
        "<project_attach_reply>\n"
    );
    for (i=0; i<gstate.project_attach.messages.size(); i++) {
        fout.printf(
            "    <message>%s</message>\n",
            gstate.project_attach.messages[i].c_str()
        );
    }
    fout.printf(
        "    <error_num>%d</error_num>\n",
        gstate.project_attach.error_num
    );
    fout.printf(
        "</project_attach_reply>\n"
    );
}

static void handle_acct_mgr_rpc(char* buf, MIOFILE& fout) {
    std::string url, name, password;
    std::string password_hash, name_lc;
    bool use_config_file = false;
    bool bad_arg = false;
    if (!parse_bool(buf, "use_config_file", use_config_file)) {
        if (!parse_str(buf, "<url>", url)) bad_arg = true;
        if (!parse_str(buf, "<name>", name)) bad_arg = true;
        if (!parse_str(buf, "<password>", password)) bad_arg = true;
        if (!bad_arg) {
            name_lc = name;
            downcase_string(name_lc);
            password_hash = md5_string(password+name_lc);
        }
    } else {
        if (!strlen(gstate.acct_mgr_info.acct_mgr_url) || !strlen(gstate.acct_mgr_info.acct_mgr_url) || !strlen(gstate.acct_mgr_info.acct_mgr_url)) {
            bad_arg = true;
        } else {
            url = gstate.acct_mgr_info.acct_mgr_url;
            name = gstate.acct_mgr_info.login_name;
            password_hash = gstate.acct_mgr_info.password_hash;
        }
    }
    if (bad_arg) {
        fout.printf("<error>bad arg</error>\n");
    } else {
        gstate.acct_mgr_op.do_rpc(url, name, password_hash, true);
        fout.printf("<success/>\n");
    }
}

static void handle_acct_mgr_rpc_poll(char*, MIOFILE& fout) {
    fout.printf(
        "<acct_mgr_rpc_reply>\n"
    );
    if (gstate.acct_mgr_op.error_str.size()) {
        fout.printf(
            "    <message>%s</message>\n",
            gstate.acct_mgr_op.error_str.c_str()
        );
    }
    fout.printf(
        "    <error_num>%d</error_num>\n",
        gstate.acct_mgr_op.error_num
    );
    fout.printf(
        "</acct_mgr_rpc_reply>\n"
    );
}

static void handle_get_newer_version(MIOFILE& fout) {
    fout.printf("<newer_version>%s</newer_version>\n",
        gstate.newer_version.c_str()
    );
}

int GUI_RPC_CONN::handle_rpc() {
    char request_msg[4096];
    int n;
    MIOFILE mf;
    MFILE m;
    char* p;
    int major_version;
    int minor_version;
    int release;
    mf.init_mfile(&m);

    // read the request message in one read()
    // so that the core client won't hang because
    // of malformed request msgs
    //
#ifdef _WIN32
        n = recv(sock, request_msg, 4095, 0);
#else
        n = read(sock, request_msg, 4095);
#endif
    if (n <= 0) return ERR_READ;
    request_msg[n] = 0;

    if (log_flags.guirpc_debug) {
        msg_printf(0, MSG_INFO,
            "GUI RPC Command = '%s'\n", request_msg
        );
    }

    // get client version.  not used for now
    //
    parse_int(request_msg, "<major_version>", major_version);
    parse_int(request_msg, "<minor_version>", minor_version);
    parse_int(request_msg, "<release>", release);

    mf.printf(
        "<boinc_gui_rpc_reply>\n"
        "<major_version>%d</major_version>\n"
        "<minor_version>%d</minor_version>\n"
        "<release>%d</release>\n",
        BOINC_MAJOR_VERSION,
        BOINC_MINOR_VERSION,
        BOINC_RELEASE
    );
    if (match_tag(request_msg, "<auth1")) {
        handle_auth1(mf);
    } else if (match_tag(request_msg, "<auth2")) {
        handle_auth2(request_msg, mf);

    } else if (auth_needed && !is_local) {
        auth_failure(mf);

    // operations that require authentication for non-local clients start here

    } else if (match_tag(request_msg, "<get_state")) {
        gstate.write_state_gui(mf);
    } else if (match_tag(request_msg, "<get_results")) {
        gstate.write_tasks_gui(mf);
    } else if (match_tag(request_msg, "<get_screensaver_mode")) {
        handle_get_screensaver_mode(request_msg, mf);
    } else if (match_tag(request_msg, "<set_screensaver_mode")) {
        handle_set_screensaver_mode(request_msg, mf);

    // Operations that require authentication start here

    } else if (auth_needed) {
        auth_failure(mf);
    } else if (match_tag(request_msg, "<get_file_transfers")) {
        gstate.write_file_transfers_gui(mf);
    } else if (match_tag(request_msg, "<get_project_status")) {
        handle_get_project_status(mf);
    } else if (match_tag(request_msg, "<get_disk_usage")) {
        handle_get_disk_usage(mf);
    } else if (match_tag(request_msg, "<project_nomorework")) {
         handle_project_op(request_msg, mf, "nomorework");
     } else if (match_tag(request_msg, "<project_allowmorework")) {
         handle_project_op(request_msg, mf, "allowmorework");
    } else if (match_tag(request_msg, "<get_run_mode")) {
        handle_get_run_mode(request_msg, mf);
    } else if (match_tag(request_msg, "<set_network_mode")) {
        handle_set_network_mode(request_msg, mf);
    } else if (match_tag(request_msg, "<get_network_mode")) {
        handle_get_network_mode(request_msg, mf);
    } else if (match_tag(request_msg, "<run_benchmarks")) {
        handle_run_benchmarks(request_msg, mf);
    } else if (match_tag(request_msg, "<set_proxy_settings")) {
        handle_set_proxy_settings(request_msg, mf);
    } else if (match_tag(request_msg, "<get_proxy_settings")) {
        handle_get_proxy_settings(request_msg, mf);
    } else if (match_tag(request_msg, "<get_activity_state")) {
        handle_get_activity_state(request_msg, mf);
    } else if (match_tag(request_msg, "<get_messages")) {
        handle_get_messages(request_msg, mf);
    } else if (match_tag(request_msg, "<get_host_info")) {
        handle_get_host_info(request_msg, mf);
    } else if (match_tag(request_msg, "<get_statistics")) {
        handle_get_statistics(request_msg, mf);
    } else if (match_tag(request_msg, "<network_status")) {
        handle_network_status(request_msg, mf);
    } else if (match_tag(request_msg, "<network_available")) {
        handle_network_available(request_msg, mf);
    } else if (match_tag(request_msg, "<get_newer_version>")) {
        handle_get_newer_version(mf);
    } else if (match_tag(request_msg, "<abort_file_transfer")) {
        handle_file_transfer_op(request_msg, mf, "abort");
    } else if (match_tag(request_msg, "<project_detach")) {
        handle_project_op(request_msg, mf, "detach");
    } else if (match_tag(request_msg, "<abort_result")) {
        handle_result_op(request_msg, mf, "abort");
    } else if (match_tag(request_msg, "<suspend_result")) {
        handle_result_op(request_msg, mf, "suspend");
    } else if (match_tag(request_msg, "<resume_result")) {
        handle_result_op(request_msg, mf, "resume");
    } else if (match_tag(request_msg, "<result_show_graphics")) {
        handle_result_show_graphics(request_msg, mf);
    } else if (match_tag(request_msg, "<project_suspend")) {
        handle_project_op(request_msg, mf, "suspend");
    } else if (match_tag(request_msg, "<project_resume")) {
        handle_project_op(request_msg, mf, "resume");
    } else if (match_tag(request_msg, "<set_run_mode")) {
        handle_set_run_mode(request_msg, mf);
    } else if (match_tag(request_msg, "<quit")) {
        handle_quit(request_msg, mf);
    } else if (match_tag(request_msg, "<acct_mgr_info")) {
        handle_acct_mgr_info(request_msg, mf);
    } else if (match_tag(request_msg, "<read_global_prefs_override/>")) {
        gstate.read_global_prefs();
        gstate.request_schedule_cpus("Preferences override");
    } else {

        // RPCs after this point enable network communication
        // for 5 minutes, overriding other factors.
        // Things like attaching projects, etc.
        //

        gstate.gui_rpcs.last_rpc_time = gstate.now;

        if (match_tag(request_msg, "<retry_file_transfer")) {
            handle_file_transfer_op(request_msg, mf, "retry");
        } else if (match_tag(request_msg, "<project_reset")) {
            handle_project_op(request_msg, mf, "reset");
        } else if (match_tag(request_msg, "<project_update")) {
            handle_project_op(request_msg, mf, "update");
        } else if (match_tag(request_msg, "<get_project_init_status")) {
            handle_get_project_init_status(request_msg, mf);
        } else if (match_tag(request_msg, "<get_project_config>")) {
            handle_get_project_config(request_msg, mf);
        } else if (match_tag(request_msg, "<get_project_config_poll")) {
            handle_get_project_config_poll(request_msg, mf);
        } else if (match_tag(request_msg, "<lookup_account>")) {
            handle_lookup_account(request_msg, mf);
        } else if (match_tag(request_msg, "<lookup_account_poll")) {
            handle_lookup_account_poll(request_msg, mf);
        } else if (match_tag(request_msg, "<create_account>")) {
            handle_create_account(request_msg, mf);
        } else if (match_tag(request_msg, "<create_account_poll")) {
            handle_create_account_poll(request_msg, mf);
#if 0
        } else if (match_tag(request_msg, "<lookup_website>")) {
            handle_lookup_website(request_msg, mf);
        } else if (match_tag(request_msg, "<lookup_website_poll")) {
            handle_lookup_website_poll(request_msg, mf);
#endif
        } else if (match_tag(request_msg, "<project_attach>")) {
            handle_project_attach(request_msg, mf);
        } else if (match_tag(request_msg, "<project_attach_poll")) {
            handle_project_attach_poll(request_msg, mf);
        } else if (match_tag(request_msg, "<acct_mgr_rpc>")) {
            handle_acct_mgr_rpc(request_msg, mf);
        } else if (match_tag(request_msg, "<acct_mgr_rpc_poll")) {
            handle_acct_mgr_rpc_poll(request_msg, mf);
        } else if (match_tag(request_msg, "<get_cc_status")) {
            handle_get_cc_status(mf);
        } else {
            mf.printf("<error>unrecognized op</error>\n");
        }
    }

    mf.printf("</boinc_gui_rpc_reply>\n\003");
    m.get_buf(p, n);
    if (p) {
        if (log_flags.guirpc_debug) {
            msg_printf(0, MSG_INFO,
                "GUI RPC reply: '%s'\n", p
            );
        }
        send(sock, p, n, 0);
        free(p);
    }

    return 0;
}
const char *BOINC_RCSID_7bf15dcb49="$Id$";
