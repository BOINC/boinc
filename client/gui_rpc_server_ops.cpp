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

// GUI RPC server side (the actual RPCs)

#include "cpp.h"

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#include <vector>
#include <cstring>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif

#include "str_util.h"
#include "url.h"
#include "client_state.h"
#include "util.h"
#include "error_numbers.h"
#include "parse.h"
#include "network.h"
#include "filesys.h"

#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"
#include "cs_proxy.h"
#include "cs_notice.h"

using std::string;
using std::vector;

static void auth_failure(MIOFILE& fout) {
    fout.printf("<unauthorized/>\n");
}

void GUI_RPC_CONN::handle_auth1(MIOFILE& fout) {
    sprintf(nonce, "%f", dtime());
    fout.printf("<nonce>%s</nonce>\n", nonce);
}

int GUI_RPC_CONN::handle_auth2(char* buf, MIOFILE& fout) {
    char nonce_hash[256], nonce_hash_correct[256], buf2[256];
    if (!parse_str(buf, "<nonce_hash>", nonce_hash, 256)) {
        auth_failure(fout);
        return ERR_AUTHENTICATOR;
    }
    sprintf(buf2, "%s%s", nonce, gstate.gui_rpcs.password);
    md5_block((const unsigned char*)buf2, (int)strlen(buf2), nonce_hash_correct);
    if (strcmp(nonce_hash, nonce_hash_correct)) {
        auth_failure(fout);
        return ERR_AUTHENTICATOR;
    }
    fout.printf("<authorized/>\n");
    auth_needed = false;
    return 0;
}

// client passes its version, but ignore it for now
//
static void handle_exchange_versions(MIOFILE& fout) {
    fout.printf(
        "<server_version>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <release>%d</release>\n"
        "</server_version>\n",
        BOINC_MAJOR_VERSION,
        BOINC_MINOR_VERSION,
        BOINC_RELEASE
    );
}

static void handle_get_simple_gui_info(MIOFILE& fout) {
    unsigned int i;
    fout.printf("<simple_gui_info>\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->write_state(fout, true);
    }
    gstate.write_tasks_gui(fout, false);
    fout.printf("</simple_gui_info>\n");
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
    double size, boinc_non_project, d_allowed, boinc_total;

    fout.printf("<disk_usage_summary>\n");
    get_filesystem_info(gstate.host_info.d_total, gstate.host_info.d_free);
    dir_size(".", boinc_non_project, false);
    dir_size("locale", size, false);
    boinc_non_project += size;
#ifdef __APPLE__
    if (gstate.launched_by_manager) {
        // If launched by Manager, get Manager's size on disk
        ProcessSerialNumber managerPSN;
        FSRef ourFSRef;
        char path[1024];
        double manager_size = 0.0;
        OSStatus err;
        err = GetProcessForPID(getppid(), &managerPSN);
        if (! err) err = GetProcessBundleLocation(&managerPSN, &ourFSRef);
        if (! err) err = FSRefMakePath (&ourFSRef, (UInt8*)path, sizeof(path));
        if (! err) dir_size(path, manager_size, true);
        if (! err) boinc_non_project += manager_size;
    }
#endif
    boinc_total = boinc_non_project;
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
        boinc_total += size;
    }
    d_allowed = gstate.allowed_disk_usage(boinc_total);
    fout.printf(
        "<d_total>%f</d_total>\n"
        "<d_free>%f</d_free>\n"
        "<d_boinc>%f</d_boinc>\n"
        "<d_allowed>%f</d_allowed>\n",
        gstate.host_info.d_total,
        gstate.host_info.d_free,
        boinc_non_project,
        d_allowed
    );
    fout.printf("</disk_usage_summary>\n");
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

    if (match_tag(buf, "<full_screen/>")) {
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
    PROJECT* p = get_project(buf, fout);
    if (!p) {
        fout.printf("<error>no such project</error>\n");
        return;
    }
    gstate.set_client_state_dirty("Project modified by user");
    if (!strcmp(op, "reset")) {
        gstate.request_schedule_cpus("project reset by user");
        gstate.request_work_fetch("project reset by user");
        gstate.reset_project(p, false);
    } else if (!strcmp(op, "suspend")) {
        msg_printf(p, MSG_INFO, "suspended by user");
        p->suspended_via_gui = true;
        gstate.request_schedule_cpus("project suspended by user");
        gstate.request_work_fetch("project suspended by user");
    } else if (!strcmp(op, "resume")) {
        msg_printf(p, MSG_INFO, "resumed by user");
        p->suspended_via_gui = false;
        gstate.request_schedule_cpus("project resumed by user");
        gstate.request_work_fetch("project resumed by user");
    } else if (!strcmp(op, "detach")) {
        if (p->attached_via_acct_mgr) {
            msg_printf(p, MSG_USER_ALERT,
                "This project must be detached using the account manager web site."
            );
            fout.printf("<error>must detach using account manager</error>");
            return;
        }
        gstate.detach_project(p);
        gstate.request_schedule_cpus("project detached by user");
        gstate.request_work_fetch("project detached by user");
    } else if (!strcmp(op, "update")) {
        msg_printf(p, MSG_INFO, "update requested by user");
        p->sched_rpc_pending = RPC_REASON_USER_REQ;
        p->min_rpc_time = 0;
        gstate.request_work_fetch("project updated by user");
    } else if (!strcmp(op, "nomorework")) {
        msg_printf(p, MSG_INFO, "work fetch suspended by user");
        p->dont_request_more_work = true;
    } else if (!strcmp(op, "allowmorework")) {
        msg_printf(p, MSG_INFO, "work fetch resumed by user");
        p->dont_request_more_work = false;
        gstate.request_work_fetch("project allowed to fetch work by user");
    } else if (!strcmp(op, "detach_when_done")) {
        msg_printf(p, MSG_INFO, "detach when done set by user");
        p->detach_when_done = true;
        p->dont_request_more_work = true;
    } else if (!strcmp(op, "dont_detach_when_done")) {
        msg_printf(p, MSG_INFO, "detach when done cleared by user");
        p->detach_when_done = false;
        p->dont_request_more_work = false;
    }
    fout.printf("<success/>\n");
}

static void handle_set_run_mode(char* buf, MIOFILE& fout) {
    double duration = 0;
    int mode;
    parse_double(buf, "<duration>", duration);
    if (match_tag(buf, "<always")) {
        mode = RUN_MODE_ALWAYS;
    } else if (match_tag(buf, "<never")) {
        mode = RUN_MODE_NEVER;
    } else if (match_tag(buf, "<auto")) {
        mode = RUN_MODE_AUTO;
    } else if (match_tag(buf, "<restore")) {
        mode = RUN_MODE_RESTORE;
    } else {
        fout.printf("<error>Missing mode</error>\n");
        return;
    }
    gstate.run_mode.set(mode, duration);
    fout.printf("<success/>\n");
}

static void handle_set_gpu_mode(char* buf, MIOFILE& fout) {
    double duration = 0;
    int mode;
    parse_double(buf, "<duration>", duration);
    if (match_tag(buf, "<always")) {
        mode = RUN_MODE_ALWAYS;
    } else if (match_tag(buf, "<never")) {
        mode = RUN_MODE_NEVER;
    } else if (match_tag(buf, "<auto")) {
        mode = RUN_MODE_AUTO;
    } else if (match_tag(buf, "<restore")) {
        mode = RUN_MODE_RESTORE;
    } else {
        fout.printf("<error>Missing mode</error>\n");
        return;
    }
    gstate.gpu_mode.set(mode, duration);
    gstate.request_schedule_cpus("GPU mode changed");
    fout.printf("<success/>\n");
}

static void handle_set_network_mode(char* buf, MIOFILE& fout) {
    double duration = 0;
    int mode;
    parse_double(buf, "<duration>", duration);
    if (match_tag(buf, "<always")) {
        mode = RUN_MODE_ALWAYS;
    } else if (match_tag(buf, "<never")) {
        mode = RUN_MODE_NEVER;
    } else if (match_tag(buf, "<auto")) {
        mode = RUN_MODE_AUTO;
    } else if (match_tag(buf, "<restore")) {
        mode = RUN_MODE_RESTORE;
    } else {
        fout.printf("<error>Missing mode</error>\n");
        return;
    }
    // user is turning network on/off explicitly,
    // so disable the "5 minute grace period" mechanism
    //
    gstate.gui_rpcs.time_of_last_rpc_needing_network = 0;

    gstate.network_mode.set(mode, duration);
    fout.printf("<success/>\n");
}

static void handle_run_benchmarks(char* , MIOFILE& fout) {
    gstate.start_cpu_benchmarks();
    fout.printf("<success/>\n");
}

static void handle_set_proxy_settings(char* buf, MIOFILE& fout) {
    MIOFILE in;
    in.init_buf_read(buf);
    gui_proxy_info.parse(in);
    gstate.set_client_state_dirty("Set proxy settings RPC");
    fout.printf("<success/>\n");
    select_proxy_info();

    // tell running apps to reread app_info file (for F@h)
    //
    gstate.active_tasks.request_reread_app_info();
}

static void handle_get_proxy_settings(char* , MIOFILE& fout) {
    gui_proxy_info.write(fout);
}

// params:
// [ <seqno>n</seqno> ]
//    return only msgs with seqno > n; if absent or zero, return all
//
static void handle_get_messages(char* buf, MIOFILE& fout) {
    int seqno=0;

    parse_int(buf, "<seqno>", seqno);
    message_descs.write(seqno, fout);
}

static void handle_get_message_count(char*, MIOFILE& fout) {
    fout.printf("<seqno>%d</seqno>\n", message_descs.highest_seqno());
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
        f->project->file_xfer_backoff(pfx->is_upload).file_xfer_succeeded();
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
        msg_printf(p, MSG_INFO, "task %s aborted by user", result_name);
        atp = gstate.lookup_active_task_by_result(rp);
        if (atp) {
            atp->abort_task(ERR_ABORTED_VIA_GUI, "aborted by user");
        } else {
            rp->abort_inactive(ERR_ABORTED_VIA_GUI);
        }
        gstate.request_work_fetch("result aborted by user");
    } else if (!strcmp(op, "suspend")) {
        msg_printf(p, MSG_INFO, "task %s suspended by user", result_name);
        rp->suspended_via_gui = true;
        gstate.request_work_fetch("result suspended by user");
    } else if (!strcmp(op, "resume")) {
        msg_printf(p, MSG_INFO, "task %s resumed by user", result_name);
        rp->suspended_via_gui = false;
    }
    gstate.request_schedule_cpus("result suspended, resumed or aborted by user");
    gstate.set_client_state_dirty("Result RPC");
    fout.printf("<success/>\n");
}

static void handle_get_host_info(char*, MIOFILE& fout) {
    gstate.host_info.write(fout, false, true);
}

static void handle_get_screensaver_tasks(MIOFILE& fout) {
    unsigned int i;
    ACTIVE_TASK* atp;
    fout.printf(
        "<handle_get_screensaver_tasks>\n"
        "    <suspend_reason>%d</suspend_reason>\n",
        gstate.suspend_reason
    );
    for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        atp = gstate.active_tasks.active_tasks[i];
        if ((atp->task_state() == PROCESS_EXECUTING) || 
                ((atp->task_state() == PROCESS_SUSPENDED) && (gstate.suspend_reason == SUSPEND_REASON_CPU_THROTTLE))) {
            atp->result->write_gui(fout);
        }
    }
    fout.printf("</handle_get_screensaver_tasks>\n");
}

static void handle_quit(char*, MIOFILE& fout) {
    gstate.requested_exit = true;
    fout.printf("<success/>\n");
}

static void handle_acct_mgr_info(char*, MIOFILE& fout) {
    fout.printf(
        "<acct_mgr_info>\n"
        "   <acct_mgr_url>%s</acct_mgr_url>\n"
        "   <acct_mgr_name>%s</acct_mgr_name>\n",
        gstate.acct_mgr_info.acct_mgr_url,
        gstate.acct_mgr_info.acct_mgr_name
    );

    if (strlen(gstate.acct_mgr_info.login_name)) {
        fout.printf("   <have_credentials/>\n");
    }

    if (gstate.acct_mgr_info.cookie_required) {
        fout.printf("   <cookie_required/>\n");
        fout.printf(
            "   <cookie_failure_url>%s</cookie_failure_url>\n",
            gstate.acct_mgr_info.cookie_failure_url
        );
    }

    fout.printf("</acct_mgr_info>\n");
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

static void handle_get_cc_status(GUI_RPC_CONN* gr, MIOFILE& fout) {
    fout.printf(
        "<cc_status>\n"
        "   <network_status>%d</network_status>\n"
        "   <ams_password_error>%d</ams_password_error>\n"
        "   <task_suspend_reason>%d</task_suspend_reason>\n"
        "   <task_mode>%d</task_mode>\n"
        "   <task_mode_perm>%d</task_mode_perm>\n"
        "   <task_mode_delay>%f</task_mode_delay>\n"
        "   <gpu_suspend_reason>%d</gpu_suspend_reason>\n"
        "   <gpu_mode>%d</gpu_mode>\n"
        "   <gpu_mode_perm>%d</gpu_mode_perm>\n"
        "   <gpu_mode_delay>%f</gpu_mode_delay>\n"
        "   <network_suspend_reason>%d</network_suspend_reason>\n"
        "   <network_mode>%d</network_mode>\n"
        "   <network_mode_perm>%d</network_mode_perm>\n"
        "   <network_mode_delay>%f</network_mode_delay>\n"
        "   <disallow_attach>%d</disallow_attach>\n"
        "   <simple_gui_only>%ds</simple_gui_only>\n",
        net_status.network_status(),
        gstate.acct_mgr_info.password_error?1:0,
        gstate.suspend_reason,
        gstate.run_mode.get_current(),
        gstate.run_mode.get_perm(),
        gstate.run_mode.delay(),
        gpu_suspend_reason,
        gstate.gpu_mode.get_current(),
        gstate.gpu_mode.get_perm(),
        gstate.gpu_mode.delay(),
        gstate.network_suspend_reason,
        gstate.network_mode.get_current(),
        gstate.network_mode.get_perm(),
        gstate.network_mode.delay(),
        config.disallow_attach?1:0,
        config.simple_gui_only?1:0
    );
    if (gr->au_mgr_state == AU_MGR_QUIT_REQ) {
        fout.printf(
            "   <manager_must_quit>1</manager_must_quit>\n"
        );
        gr->au_mgr_state = AU_MGR_QUIT_SENT;
    }
    fout.printf(
        "</cc_status>\n"
    );
}

static void handle_network_available(char*, MIOFILE& fout) {
    net_status.network_available();
    fout.printf("<success/>\n");
}

static void handle_get_project_init_status(char*, MIOFILE& fout) {
    // If we're already attached to the project specified in the
    // project init file, delete the file.
    //
    for (unsigned i=0; i<gstate.projects.size(); i++) { 
        PROJECT* p = gstate.projects[i]; 
        if (!strcmp(p->master_url, gstate.project_init.url)) { 
            gstate.project_init.remove(); 
            break; 
        } 
    }

    fout.printf(
        "<get_project_init_status>\n"
        "    <url>%s</url>\n"
        "    <name>%s</name>\n"
        "    <team_name>%s</team_name>\n"
        "    %s\n"
        "</get_project_init_status>\n",
        gstate.project_init.url,
        gstate.project_init.name,
        gstate.project_init.team_name,
        strlen(gstate.project_init.account_key)?"<has_account_key/>":""
    );
}

void GUI_RPC_CONN::handle_get_project_config(char* buf, MIOFILE& fout) {
    string url;

    parse_str(buf, "<url>", url);

    canonicalize_master_url(url);
    get_project_config_op.do_rpc(url);
    fout.printf("<success/>\n");
}

void GUI_RPC_CONN::handle_get_project_config_poll(char*, MIOFILE& fout) {
    if (get_project_config_op.error_num) {
        fout.printf(
            "<project_config>\n"
            "    <error_num>%d</error_num>\n"
            "</project_config>\n",
            get_project_config_op.error_num
        );
    } else {
        fout.printf("%s", get_project_config_op.reply.c_str());
    }
}

void GUI_RPC_CONN::handle_lookup_account(char* buf, MIOFILE& fout) {
    ACCOUNT_IN ai;

    ai.parse(buf);
    if (!ai.url.size() || !ai.email_addr.size() || !ai.passwd_hash.size()) {
        fout.printf("<error>missing URL, email address, or password</error>\n");
        return;
    }

    lookup_account_op.do_rpc(ai);
    fout.printf("<success/>\n");
}

void GUI_RPC_CONN::handle_lookup_account_poll(char*, MIOFILE& fout) {
    if (lookup_account_op.error_num) {
        fout.printf(
            "<account_out>\n"
            "    <error_num>%d</error_num>\n"
            "</account_out>\n",
            lookup_account_op.error_num
        );
    } else {
        fout.printf("%s", lookup_account_op.reply.c_str());
    }
}

void GUI_RPC_CONN::handle_create_account(char* buf, MIOFILE& fout) {
    ACCOUNT_IN ai;

    ai.parse(buf);

    create_account_op.do_rpc(ai);
    fout.printf("<success/>\n");
}

void GUI_RPC_CONN::handle_create_account_poll(char*, MIOFILE& fout) {
    if (create_account_op.error_num) {
        fout.printf(
            "<account_out>\n"
            "    <error_num>%d</error_num>\n"
            "</account_out>\n",
            create_account_op.error_num
        );
    } else {
        fout.printf("%s", create_account_op.reply.c_str());
    }
}

static void handle_project_attach(char* buf, MIOFILE& fout) {
    string url, authenticator, project_name;
    bool use_config_file = false;
    bool already_attached = false;
    unsigned int i;
    int retval;

    // Get URL/auth from project_init.xml?
    //
    if (parse_bool(buf, "use_config_file", use_config_file)) {
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
    } else {
        if (!parse_str(buf, "<project_url>", url)) {
            fout.printf("<error>Missing URL</error>\n");
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
        parse_str(buf, "<project_name>", project_name);
    }

    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (url == p->master_url) already_attached = true;
    }

    if (already_attached) {
        fout.printf("<error>Already attached to project</error>\n");
        return;
    }

    // clear messages from previous attach to project.
    //
    gstate.project_attach.messages.clear();
    gstate.project_attach.error_num = gstate.add_project(
        url.c_str(), authenticator.c_str(), project_name.c_str(), false
    );

    // if project_init.xml refers to this project,
    // delete the file, otherwise we'll just
    // reattach the next time the core client starts
    //
    if (!strcmp(url.c_str(), gstate.project_init.url)) {
        retval = gstate.project_init.remove();
        if (retval) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Can't delete project init file: %s", boincerror(retval)
            );
        }
    }

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
            if (!starts_with(password, "hash:")) {
                password_hash = md5_string(password+name_lc);
            } else {
                // Remove 'hash:'
                password_hash = password.substr(5);
            }
        }
    } else {
        if (!strlen(gstate.acct_mgr_info.acct_mgr_url)) {
            bad_arg = true;
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Account manager info missing from config file"
            );
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
    fout.printf(
        "<newer_version>%s</newer_version>\n"
        "<download_url>%s</download_url>\n",
        gstate.newer_version.c_str(),
        config.client_download_url.c_str()
    );
}

static void handle_get_global_prefs_file(MIOFILE& fout) {
    GLOBAL_PREFS p;
    bool found;
    int retval = p.parse_file(
        GLOBAL_PREFS_FILE_NAME, gstate.main_host_venue, found
    );
    if (retval) {
        fout.printf("<error>%d</error>\n", retval);
        return;
    }
    p.write(fout);
}

static void handle_get_global_prefs_working(MIOFILE& fout) {
    gstate.global_prefs.write(fout);
}

static void handle_get_global_prefs_override(MIOFILE& fout) {
    string s;
    int retval = read_file_string(GLOBAL_PREFS_OVERRIDE_FILE, s);
    if (!retval) {
        strip_whitespace(s);
        fout.printf("%s\n", s.c_str());
    } else {
        fout.printf("<error>no prefs override file</error>\n");
    }
}

static void handle_set_global_prefs_override(char* buf, MIOFILE& fout) {
    char *p, *q=0;
    int retval = ERR_XML_PARSE;

    // strip off outer tags
    //
    p = strstr(buf, "<set_global_prefs_override>\n");
    if (p) {
        p += strlen("<set_global_prefs_override>\n");
        q = strstr(p, "</set_global_prefs_override");
    }
    if (q) {
        *q = 0;
        strip_whitespace(p);
        if (strlen(p)) {
            FILE* f = boinc_fopen(GLOBAL_PREFS_OVERRIDE_FILE, "w");
            if (f) {
                fprintf(f, "%s\n", p);
                fclose(f);
                retval = 0;
            } else {
                retval = ERR_FOPEN;
            }
        } else {
            retval = boinc_delete_file(GLOBAL_PREFS_OVERRIDE_FILE);
        }
    }
    fout.printf(
        "<set_global_prefs_override_reply>\n"
        "    <status>%d</status>\n"
        "</set_global_prefs_override_reply>\n",
        retval
    );
}

static void handle_get_cc_config(MIOFILE& fout) {
    string s;
    int retval = read_file_string(CONFIG_FILE, s);
    if (!retval) {
        strip_whitespace(s);
        fout.printf("%s\n", s.c_str());
    }
}

static void read_all_projects_list_file(MIOFILE& fout) {
    string s;
    int retval = read_file_string(ALL_PROJECTS_LIST_FILENAME, s);
    if (!retval) {
        strip_whitespace(s);
        fout.printf("%s\n", s.c_str());
    }
}

static int set_debt(XML_PARSER& xp) {
    bool is_tag;
    char tag[256], url[256];
    double short_term_debt = 0, long_term_debt = 0, cuda_debt=0, ati_debt=0;
    bool got_std=false, got_ltd=false, got_cuda_debt=false, got_ati_debt=false;
    strcpy(url, "");
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!strcmp(tag, "/project")) {
            if (!strlen(url)) return ERR_XML_PARSE;
            canonicalize_master_url(url);
            PROJECT* p = gstate.lookup_project(url);
            if (!p) return ERR_NOT_FOUND;
            if (got_std) {
                p->cpu_pwf.short_term_debt = short_term_debt;
                p->cuda_pwf.short_term_debt = short_term_debt;
                p->ati_pwf.short_term_debt = short_term_debt;
            }
            if (got_ltd) p->cpu_pwf.long_term_debt = long_term_debt;
            if (got_cuda_debt) p->cuda_pwf.long_term_debt = cuda_debt;
            if (got_ati_debt) p->ati_pwf.long_term_debt = ati_debt;
            return 0;
        }
        if (xp.parse_str(tag, "master_url", url, sizeof(url))) continue;
        if (xp.parse_double(tag, "short_term_debt", short_term_debt)) {
            got_std = true;
            continue;
        }
        if (xp.parse_double(tag, "long_term_debt", long_term_debt)) {
            got_ltd = true;
            continue;
        }
        if (xp.parse_double(tag, "cuda_debt", cuda_debt)) {
            got_cuda_debt = true;
            continue;
        }
        if (xp.parse_double(tag, "ati_debt", ati_debt)) {
            got_ati_debt = true;
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] set_debt: unrecognized %s", tag
            );
        }
        xp.skip_unexpected(tag, log_flags.unparsed_xml, "set_debt");
    }
    return 0;
}

static void handle_set_debts(char* buf, MIOFILE& fout) {
    MIOFILE in;
    XML_PARSER xp(&in);
    bool is_tag;
    char tag[256];
    int retval;

    in.init_buf_read(buf);
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "boinc_gui_rpc_request")) continue;
        if (!strcmp(tag, "set_debts")) continue;
        if (!strcmp(tag, "/set_debts")) {
            fout.printf("<success/>\n");
            gstate.set_client_state_dirty("set_debt RPC");
            return;
        }
        if (!strcmp(tag, "project")) {
            retval = set_debt(xp);
            if (retval) {
                fout.printf("<error>%d</error>\n", retval);
                return;
            }
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] handle_set_debts: unrecognized %s", tag
            );
        }
        xp.skip_unexpected(tag, log_flags.unparsed_xml, "handle_set_debts");
    }
    fout.printf("<error>No end tag</error>\n");
}

static void handle_set_cc_config(char* buf, MIOFILE& fout) {
    char *p, *q=0;
    int retval = ERR_XML_PARSE;

    // strip off outer tags
    //
    p = strstr(buf, "<set_cc_config>\n");
    if (p) {
        p += strlen("<set_cc_config>\n");
        q = strstr(p, "</set_cc_config");
    }
    if (q) {
        *q = 0;
        strip_whitespace(p);
        if (strlen(p)) {
            FILE* f = boinc_fopen(CONFIG_FILE, "w");
            if (f) {
                fprintf(f, "%s\n", p);
                fclose(f);
                retval = 0;
            } else {
                retval = ERR_FOPEN;
            }
        } else {
            retval = boinc_delete_file(CONFIG_FILE);
        }
    }
    fout.printf(
        "<set_cc_config_reply>\n"
        "    <status>%d</status>\n"
        "</set_cc_config_reply>\n",
        retval
    );
}

static void handle_get_notices(char* buf, MIOFILE& fout, bool notice_refresh) {
    int seqno = 0;
    parse_int(buf, "<seqno>", seqno);
    notices.write(seqno, fout, false, notice_refresh);
}

static void handle_get_notices_public(char* buf, MIOFILE& fout, bool notice_refresh) {
    int seqno = 0;
    parse_int(buf, "<seqno>", seqno);
    notices.write(seqno, fout, true, notice_refresh);
}

static bool complete_post_request(char* buf) {
    if (strncmp(buf, "POST", 4)) return false;
    char* p = strstr(buf, "Content-Length: ");
    if (!p) return false;
    p += strlen("Content-Length: ");
    int n = atoi(p);
    p = strstr(p, "\r\n\r\n");
    if (!p) return false;
    p += 4;
    if ((int)strlen(p) < n) return false;
    return true;
}

// Some of the RPCs have empty-element request messages.
// We accept both <foo/> and <foo></foo>
//
#define match_req(buf, tag) (match_tag(buf, "<" tag ">") || match_tag(buf, "<" tag "/>"))

int GUI_RPC_CONN::handle_rpc() {
    int n, retval=0;
    MIOFILE mf;
    MFILE m;
    char* p;
    mf.init_mfile(&m);

    int left = GUI_RPC_REQ_MSG_SIZE - request_nbytes;
#ifdef _WIN32
        n = recv(sock, request_msg+request_nbytes, left, 0);
#else
        n = read(sock, request_msg+request_nbytes, left);
#endif
    if (n <= 0) {
        request_nbytes = 0;
        return ERR_READ;
    }
    request_nbytes += n;

    // buffer full?
    if (request_nbytes >= GUI_RPC_REQ_MSG_SIZE) {
        request_nbytes = 0;
        return ERR_READ;
    }
    request_msg[request_nbytes] = 0;
    if (!strncmp(request_msg, "OPTIONS", 7)) {
        char buf[1024];
        sprintf(buf, "HTTP/1.1 200 OK\n"
            "Server: BOINC client\n"
            "Access-Control-Allow-Origin: *\n"
            "Access-Control-Allow-Methods: POST, GET, OPTIONS\n"
            "Content-Length: 0\n"
            "Keep-Alive: timeout=2, max=100\n"
            "Connection: Keep-Alive\n"
            "Content-Type: text/plain\n\n"
        );
        send(sock, buf, strlen(buf), 0);
        request_nbytes = 0;
        if (log_flags.gui_rpc_debug) {
            msg_printf(0, MSG_INFO,
                "[gui_rpc] processed OPTIONS"
            );
        }
        return 0;
    }
    bool http_request;
    if (complete_post_request(request_msg)) {
        http_request = true;
    } else {
        p = strchr(request_msg, 3);
        if (p) {
            *p = 0;
            http_request = false;
        } else {
            if (log_flags.gui_rpc_debug) {
                msg_printf(0, MSG_INFO,
                    "[gui_rpc] partial GUI RPC Command = '%s'\n", request_msg
                );
            }
            return 0;
        }
    }
    request_nbytes = 0;

    if (log_flags.gui_rpc_debug) {
        msg_printf(0, MSG_INFO,
            "[gui_rpc] GUI RPC Command = '%s'\n", request_msg
        );
    }

    // Policy:
    // - the first auth failure gets an error message; after that, disconnect
    // - if we get an unexpected auth1 or auth2, disconnect

    mf.printf("<boinc_gui_rpc_reply>\n");
    if (match_req(request_msg, "auth1")) {
        if (got_auth1 && auth_needed) {
            retval = ERR_AUTHENTICATOR;
        } else {
            handle_auth1(mf);
            got_auth1 = true;
        }
    } else if (match_req(request_msg, "auth2")) {
        if ((!got_auth1 || got_auth2) && auth_needed) {
            retval = ERR_AUTHENTICATOR;
        } else {
            retval = handle_auth2(request_msg, mf);
            got_auth2 = true;
        }
    } else if (auth_needed && !is_local) {
        auth_failure(mf);
        if (sent_unauthorized) {
            retval = ERR_AUTHENTICATOR;
        }
        sent_unauthorized = true;

    // operations that require authentication only for non-local clients start here.
    // Use this only for information that should be available to people
    // sharing this computer (e.g. what jobs are running)
    // but not for anything sensitive (passwords etc.)

    } else if (match_req(request_msg, "exchange_versions")) {
        handle_exchange_versions(mf);
    } else if (match_req(request_msg, "get_state")) {
        gstate.write_state_gui(mf);
    } else if (match_req(request_msg, "get_results")) {
        bool active_only = false;
        parse_bool(request_msg, "active_only", active_only);
        mf.printf("<results>\n");
        gstate.write_tasks_gui(mf, active_only);
        mf.printf("</results>\n");
    } else if (match_req(request_msg, "get_screensaver_tasks")) {
        handle_get_screensaver_tasks(mf);
    } else if (match_req(request_msg, "result_show_graphics")) {
        handle_result_show_graphics(request_msg, mf);
    } else if (match_req(request_msg, "get_file_transfers")) {
        gstate.write_file_transfers_gui(mf);
    } else if (match_req(request_msg, "get_simple_gui_info")) {
        handle_get_simple_gui_info(mf);
    } else if (match_req(request_msg, "get_project_status")) {
        handle_get_project_status(mf);
    } else if (match_req(request_msg, "get_disk_usage")) {
        handle_get_disk_usage(mf);
    } else if (match_req(request_msg, "get_messages")) {
        handle_get_messages(request_msg, mf);
    } else if (match_req(request_msg, "get_message_count")) {
        handle_get_message_count(request_msg, mf);
    } else if (match_req(request_msg, "get_host_info")) {
        handle_get_host_info(request_msg, mf);
    } else if (match_req(request_msg, "get_statistics")) {
        handle_get_statistics(request_msg, mf);
    } else if (match_req(request_msg, "get_newer_version")) {
        handle_get_newer_version(mf);
    } else if (match_req(request_msg, "get_cc_status")) {
        handle_get_cc_status(this, mf);
    } else if (match_req(request_msg, "get_all_projects_list")) {
        read_all_projects_list_file(mf);
    } else if (match_req(request_msg, "get_notices_public")) {
        handle_get_notices_public(request_msg, mf, notice_refresh);
        notice_refresh = false;

    // Operations that require authentication start here

    } else if (auth_needed) {
        auth_failure(mf);
        if (sent_unauthorized) {
            retval = ERR_AUTHENTICATOR;
        }
        sent_unauthorized = true;
    } else if (match_req(request_msg, "project_nomorework")) {
         handle_project_op(request_msg, mf, "nomorework");
     } else if (match_req(request_msg, "project_allowmorework")) {
         handle_project_op(request_msg, mf, "allowmorework");
    } else if (match_req(request_msg, "project_detach_when_done")) {
         handle_project_op(request_msg, mf, "detach_when_done");
    } else if (match_req(request_msg, "project_dont_detach_when_done")) {
         handle_project_op(request_msg, mf, "dont_detach_when_done");
    } else if (match_req(request_msg, "set_network_mode")) {
        handle_set_network_mode(request_msg, mf);
    } else if (match_req(request_msg, "run_benchmarks")) {
        handle_run_benchmarks(request_msg, mf);
    } else if (match_req(request_msg, "get_proxy_settings")) {
        handle_get_proxy_settings(request_msg, mf);
    } else if (match_req(request_msg, "set_proxy_settings")) {
        handle_set_proxy_settings(request_msg, mf);
    } else if (match_req(request_msg, "network_available")) {
        handle_network_available(request_msg, mf);
    } else if (match_req(request_msg, "abort_file_transfer")) {
        handle_file_transfer_op(request_msg, mf, "abort");
    } else if (match_req(request_msg, "project_detach")) {
        handle_project_op(request_msg, mf, "detach");
    } else if (match_req(request_msg, "abort_result")) {
        handle_result_op(request_msg, mf, "abort");
    } else if (match_req(request_msg, "suspend_result")) {
        handle_result_op(request_msg, mf, "suspend");
    } else if (match_req(request_msg, "resume_result")) {
        handle_result_op(request_msg, mf, "resume");
    } else if (match_req(request_msg, "project_suspend")) {
        handle_project_op(request_msg, mf, "suspend");
    } else if (match_req(request_msg, "project_resume")) {
        handle_project_op(request_msg, mf, "resume");
    } else if (match_req(request_msg, "set_run_mode")) {
        handle_set_run_mode(request_msg, mf);
    } else if (match_req(request_msg, "set_gpu_mode")) {
        handle_set_gpu_mode(request_msg, mf);
    } else if (match_req(request_msg, "quit")) {
        handle_quit(request_msg, mf);
    } else if (match_req(request_msg, "acct_mgr_info")) {
        handle_acct_mgr_info(request_msg, mf);
    } else if (match_req(request_msg, "read_global_prefs_override")) {
        mf.printf("<success/>\n");
        gstate.read_global_prefs();
        gstate.request_schedule_cpus("Preferences override");
        gstate.request_work_fetch("Preferences override");
    } else if (match_req(request_msg, "get_project_init_status")) {
        handle_get_project_init_status(request_msg, mf);
    } else if (match_req(request_msg, "get_global_prefs_file")) {
        handle_get_global_prefs_file(mf);
    } else if (match_req(request_msg, "get_global_prefs_working")) {
        handle_get_global_prefs_working(mf);
    } else if (match_req(request_msg, "get_global_prefs_override")) {
        handle_get_global_prefs_override(mf);
    } else if (match_req(request_msg, "set_global_prefs_override")) {
        handle_set_global_prefs_override(request_msg, mf);
    } else if (match_req(request_msg, "get_cc_config")) {
        handle_get_cc_config(mf);
    } else if (match_req(request_msg, "set_cc_config")) {
        handle_set_cc_config(request_msg, mf);
    } else if (match_req(request_msg, "read_cc_config")) {
        mf.printf("<success/>\n");
        read_config_file(false);
        msg_printf(0, MSG_INFO, "Re-read config file");
        config.show();
        log_flags.show();
        gstate.set_ncpus();
        gstate.request_schedule_cpus("Core client configuration");
        gstate.request_work_fetch("Core client configuration");
    } else if (match_req(request_msg, "set_debts")) {
        handle_set_debts(request_msg, mf);
    } else if (match_req(request_msg, "get_notices")) {
        handle_get_notices(request_msg, mf, notice_refresh);
        notice_refresh = false;
    } else {

        // RPCs after this point require authentication,
        // and enable network communication for 5 minutes,
        // overriding other factors.
        // Things like attaching projects, etc.
        //

        double saved_time = gstate.gui_rpcs.time_of_last_rpc_needing_network;
        gstate.gui_rpcs.time_of_last_rpc_needing_network = gstate.now;

        if (match_req(request_msg, "retry_file_transfer")) {
            handle_file_transfer_op(request_msg, mf, "retry");
        } else if (match_req(request_msg, "project_reset")) {
            handle_project_op(request_msg, mf, "reset");
        } else if (match_req(request_msg, "project_update")) {
            handle_project_op(request_msg, mf, "update");
        } else if (match_req(request_msg, "get_project_config")) {
            handle_get_project_config(request_msg, mf);
        } else if (match_req(request_msg, "get_project_config_poll")) {
            handle_get_project_config_poll(request_msg, mf);
        } else if (match_req(request_msg, "lookup_account")) {
            handle_lookup_account(request_msg, mf);
        } else if (match_req(request_msg, "lookup_account_poll")) {
            handle_lookup_account_poll(request_msg, mf);
        } else if (match_req(request_msg, "create_account")) {
            handle_create_account(request_msg, mf);
        } else if (match_req(request_msg, "create_account_poll")) {
            handle_create_account_poll(request_msg, mf);
        } else if (match_req(request_msg, "project_attach")) {
            handle_project_attach(request_msg, mf);
        } else if (match_req(request_msg, "project_attach_poll")) {
            handle_project_attach_poll(request_msg, mf);
        } else if (match_req(request_msg, "acct_mgr_rpc")) {
            handle_acct_mgr_rpc(request_msg, mf);
        } else if (match_req(request_msg, "acct_mgr_rpc_poll")) {
            handle_acct_mgr_rpc_poll(request_msg, mf);

        // DON'T JUST ADD NEW RPCS HERE - THINK ABOUT THEIR
        // AUTHENTICATION AND NETWORK REQUIREMENTS FIRST

        } else {
            mf.printf("<error>unrecognized op: %s</error>\n", request_msg);
            gstate.gui_rpcs.time_of_last_rpc_needing_network = saved_time;
        }
    }

    mf.printf("</boinc_gui_rpc_reply>\n\003");
    m.get_buf(p, n);
    if (http_request) {
        char buf[1024];
        sprintf(buf,
            "HTTP/1.1 200 OK\n"
            "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
            "Server: BOINC client\n"
            "Connection: close\n"
            "Content-Type: text/xml; charset=utf-8\n"
            "Content-Length: %d\n\n",
            n
        );
        send(sock, buf, strlen(buf), 0);
    }
    if (p) {
        send(sock, p, n, 0);
        p[n-1]=0;   // replace 003 with NULL
        if (log_flags.gui_rpc_debug) {
            if (n > 50) p[50] = 0;
            msg_printf(0, MSG_INFO,
                "[gui_rpc] GUI RPC reply: '%s'\n", p
            );
        }
        free(p);
    }
    return retval;
}

