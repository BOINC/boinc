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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#include <vector>
#include <cstring>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "network.h"
#include "parse.h"
#include "str_util.h"
#include "url.h"
#include "util.h"

#include "client_state.h"
#include "client_msgs.h"
#include "client_state.h"
#include "cs_proxy.h"
#include "cs_notice.h"
#include "file_names.h"
#include "project.h"
#include "result.h"

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
static void handle_exchange_versions(GUI_RPC_CONN& grc) {
    grc.mfout.printf(
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

static void handle_get_simple_gui_info(GUI_RPC_CONN& grc) {
    unsigned int i;
    grc.mfout.printf("<simple_gui_info>\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->write_state(grc.mfout, true);
    }
    gstate.write_tasks_gui(grc.mfout, true);
    grc.mfout.printf("</simple_gui_info>\n");
}

static void handle_get_project_status(GUI_RPC_CONN& grc) {
    unsigned int i;
    grc.mfout.printf("<projects>\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->write_state(grc.mfout, true);
    }
    grc.mfout.printf("</projects>\n");
}

static void handle_get_disk_usage(GUI_RPC_CONN& grc) {
    unsigned int i;
    double size, boinc_non_project, d_allowed, boinc_total;

    grc.mfout.printf("<disk_usage_summary>\n");
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
    gstate.get_disk_usages();
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        grc.mfout.printf(
            "<project>\n"
            "  <master_url>%s</master_url>\n"
            "  <disk_usage>%f</disk_usage>\n"
            "</project>\n",
            p->master_url, p->disk_usage
        );
        boinc_total += p->disk_usage;
    }
    d_allowed = gstate.allowed_disk_usage(gstate.total_disk_usage);
    grc.mfout.printf(
        "<d_total>%f</d_total>\n"
        "<d_free>%f</d_free>\n"
        "<d_boinc>%f</d_boinc>\n"
        "<d_allowed>%f</d_allowed>\n",
        gstate.host_info.d_total,
        gstate.host_info.d_free,
        boinc_non_project,
        d_allowed
    );
    grc.mfout.printf("</disk_usage_summary>\n");
}

static PROJECT* get_project(GUI_RPC_CONN& grc, string url) {
    if (url.empty()) {
        grc.mfout.printf("<error>Missing project URL</error>\n");
        return 0;
    }
    PROJECT* p = gstate.lookup_project(url.c_str());
    if (!p) {
        grc.mfout.printf("<error>No such project</error>\n");
        return 0 ;
    }
    return p;
}

static PROJECT* get_project_parse(GUI_RPC_CONN& grc) {
    string url;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_string("project_url", url)) continue;
    }
    return get_project(grc, url);
}

static void handle_project_reset(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    gstate.request_schedule_cpus("project reset by user");
    gstate.request_work_fetch("project reset by user");
    gstate.reset_project(p, false);
    grc.mfout.printf("<success/>\n");
}

static void handle_project_suspend(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    msg_printf(p, MSG_INFO, "project suspended by user");
    p->suspend();
    grc.mfout.printf("<success/>\n");
}

static void handle_project_resume(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    msg_printf(p, MSG_INFO, "project resumed by user");
    p->resume();
    grc.mfout.printf("<success/>\n");
}

static void handle_project_detach(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    if (p->attached_via_acct_mgr) {
        msg_printf(p, MSG_INFO,
            "This project must be detached using the account manager web site."
        );
        grc.mfout.printf("<error>must detach using account manager</error>");
        return;
    }
    gstate.detach_project(p);
    gstate.request_schedule_cpus("project detached by user");
    gstate.request_work_fetch("project detached by user");
    grc.mfout.printf("<success/>\n");
}

static void handle_project_update(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    msg_printf(p, MSG_INFO, "update requested by user");
    p->sched_rpc_pending = RPC_REASON_USER_REQ;
    p->min_rpc_time = 0;
#if 1
    rss_feeds.trigger_fetch(p);
#endif
    gstate.request_work_fetch("project updated by user");
    grc.mfout.printf("<success/>\n");
}

static void handle_project_nomorework(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    msg_printf(p, MSG_INFO, "work fetch suspended by user");
    p->dont_request_more_work = true;
    grc.mfout.printf("<success/>\n");
}

static void handle_project_allowmorework(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    msg_printf(p, MSG_INFO, "work fetch resumed by user");
    p->dont_request_more_work = false;
    grc.mfout.printf("<success/>\n");
}

static void handle_project_detach_when_done(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    msg_printf(p, MSG_INFO, "detach when done set by user");
    p->detach_when_done = true;
    p->dont_request_more_work = true;
    grc.mfout.printf("<success/>\n");
}

static void handle_project_dont_detach_when_done(GUI_RPC_CONN& grc) {
    PROJECT* p = get_project_parse(grc);
    if (!p) return;
    gstate.set_client_state_dirty("Project modified by user");
    msg_printf(p, MSG_INFO, "detach when done cleared by user");
    p->detach_when_done = true;
    p->dont_request_more_work = false;
    grc.mfout.printf("<success/>\n");
}

static void handle_set_run_mode(GUI_RPC_CONN& grc) {
    double duration = 0;
    bool btemp;
    int mode=-1;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_double("duration", duration)) continue;
        if (grc.xp.parse_bool("always", btemp)) {
            mode = RUN_MODE_ALWAYS;
            continue;
        }
        if (grc.xp.parse_bool("never", btemp)) {
            mode = RUN_MODE_NEVER;
            continue;
        }
        if (grc.xp.parse_bool("auto", btemp)) {
            mode = RUN_MODE_AUTO;
            continue;
        }
        if (grc.xp.parse_bool("restore", btemp)) {
            mode = RUN_MODE_RESTORE;
            continue;
        }
    }
    if (mode < 0) {
        grc.mfout.printf("<error>Missing mode</error>\n");
        return;
    }
    gstate.cpu_run_mode.set(mode, duration);
    grc.mfout.printf("<success/>\n");
}

static void handle_set_gpu_mode(GUI_RPC_CONN& grc) {
    double duration = 0;
    bool btemp;
    int mode=-1;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_double("duration", duration)) continue;
        if (grc.xp.parse_bool("always", btemp)) {
            mode = RUN_MODE_ALWAYS;
            continue;
        }
        if (grc.xp.parse_bool("never", btemp)) {
            mode = RUN_MODE_NEVER;
            continue;
        }
        if (grc.xp.parse_bool("auto", btemp)) {
            mode = RUN_MODE_AUTO;
            continue;
        }
        if (grc.xp.parse_bool("restore", btemp)) {
            mode = RUN_MODE_RESTORE;
            continue;
        }
    }
    if (mode < 0) {
        grc.mfout.printf("<error>Missing mode</error>\n");
        return;
    }
    gstate.gpu_run_mode.set(mode, duration);
    gstate.request_schedule_cpus("GPU mode changed");
    grc.mfout.printf("<success/>\n");
}

static void handle_set_network_mode(GUI_RPC_CONN& grc) {
    double duration = 0;
    bool btemp;
    int mode=-1;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_double("duration", duration)) continue;
        if (grc.xp.parse_bool("always", btemp)) {
            mode = RUN_MODE_ALWAYS;
            continue;
        }
        if (grc.xp.parse_bool("never", btemp)) {
            mode = RUN_MODE_NEVER;
            continue;
        }
        if (grc.xp.parse_bool("auto", btemp)) {
            mode = RUN_MODE_AUTO;
            continue;
        }
        if (grc.xp.parse_bool("restore", btemp)) {
            mode = RUN_MODE_RESTORE;
            continue;
        }
    }
    if (mode < 0) {
        grc.mfout.printf("<error>Missing mode</error>\n");
        return;
    }
    // user is turning network on/off explicitly,
    // so disable the "5 minute grace period" mechanism
    //
    gstate.gui_rpcs.time_of_last_rpc_needing_network = 0;

    gstate.network_run_mode.set(mode, duration);
    grc.mfout.printf("<success/>\n");
}

static void handle_run_benchmarks(GUI_RPC_CONN& grc) {
    gstate.start_cpu_benchmarks();
    grc.mfout.printf("<success/>\n");
}

static void handle_set_proxy_settings(GUI_RPC_CONN& grc) {
    gui_proxy_info.parse(grc.xp);
    gstate.set_client_state_dirty("Set proxy settings RPC");
    grc.mfout.printf("<success/>\n");
    select_proxy_info();

    // tell running apps to reread app_info file (for F@h)
    //
    gstate.active_tasks.request_reread_app_info();
}

static void handle_get_proxy_settings(GUI_RPC_CONN& grc) {
    gui_proxy_info.write(grc.mfout);
}

// params:
// [ <seqno>n</seqno> ]
//    return only msgs with seqno > n; if absent or zero, return all
//
static void handle_get_messages(GUI_RPC_CONN& grc) {
    int seqno=0;
    bool translatable = false;

    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_int("seqno", seqno)) continue;
        if (grc.xp.parse_bool("translatable", translatable)) continue;
    }
    message_descs.write(seqno, grc.mfout, translatable);
}

static void handle_get_message_count(GUI_RPC_CONN& grc) {
    grc.mfout.printf("<seqno>%d</seqno>\n", message_descs.highest_seqno());
}

// <retry_file_transfer>
//    <project_url>XXX</project_url>
//    <filename>XXX</filename>
// </retry_file_transfer>
//

static void handle_file_transfer_op(GUI_RPC_CONN& grc, const char* op) {
    string project_url, filename;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_string("filename", filename)) continue;
        if (grc.xp.parse_string("project_url", project_url)) continue;
    }
    PROJECT* p = get_project(grc, project_url);
    if (!p) return;

    if (filename.empty()) {
        grc.mfout.printf("<error>Missing filename</error>\n");
        return;
    }
    
    FILE_INFO* f = gstate.lookup_file_info(p, filename.c_str());
    if (!f) {
        grc.mfout.printf("<error>No such file</error>\n");
        return;
    }
    
    PERS_FILE_XFER* pfx = f->pers_file_xfer;
    if (!pfx) {
        grc.mfout.printf("<error>No such transfer waiting</error>\n");
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
        grc.mfout.printf("<error>unknown op</error>\n");
        return;
    }
    gstate.set_client_state_dirty("File transfer RPC");
    grc.mfout.printf("<success/>\n");
}

static void handle_retry_file_transfer(GUI_RPC_CONN& grc) {
    handle_file_transfer_op(grc, "retry");
}

static void handle_abort_file_transfer(GUI_RPC_CONN& grc) {
    handle_file_transfer_op(grc, "abort");
}

static void handle_result_op(GUI_RPC_CONN& grc, const char* op) {
    RESULT* rp;
    char result_name[256];
    ACTIVE_TASK* atp;
    string project_url;

    strcpy(result_name, "");
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_str("name", result_name, sizeof(result_name))) continue;
        if (grc.xp.parse_string("project_url", project_url)) continue;
    }
    PROJECT* p = get_project(grc, project_url);
    if (!p) return;

    if (!strlen(result_name)) {
        grc.mfout.printf("<error>Missing result name</error>\n");
        return;
    }

    rp = gstate.lookup_result(p, result_name);
    if (!rp) {
        grc.mfout.printf("<error>no such result</error>\n");
        return;
    }

    if (!strcmp(op, "abort")) {
        msg_printf(p, MSG_INFO, "task %s aborted by user", result_name);
        atp = gstate.lookup_active_task_by_result(rp);
        if (atp) {
            atp->abort_task(EXIT_ABORTED_VIA_GUI, "aborted by user");
        } else {
            rp->abort_inactive(EXIT_ABORTED_VIA_GUI);
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
    grc.mfout.printf("<success/>\n");
}

static void handle_suspend_result(GUI_RPC_CONN& grc) {
    handle_result_op(grc, "suspend");
}

static void handle_resume_result(GUI_RPC_CONN& grc) {
    handle_result_op(grc, "resume");
}

static void handle_abort_result(GUI_RPC_CONN& grc) {
    handle_result_op(grc, "abort");
}

static void handle_get_host_info(GUI_RPC_CONN& grc) {
    gstate.host_info.write(grc.mfout, true, true);
}

static void handle_get_screensaver_tasks(GUI_RPC_CONN& grc) {
    unsigned int i;
    ACTIVE_TASK* atp;
    grc.mfout.printf(
        "<handle_get_screensaver_tasks>\n"
        "    <suspend_reason>%d</suspend_reason>\n",
        gstate.suspend_reason
    );
    for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        atp = gstate.active_tasks.active_tasks[i];
        if ((atp->task_state() == PROCESS_EXECUTING) || 
                ((atp->task_state() == PROCESS_SUSPENDED) && (gstate.suspend_reason == SUSPEND_REASON_CPU_THROTTLE))) {
            atp->result->write_gui(grc.mfout);
        }
    }
    grc.mfout.printf("</handle_get_screensaver_tasks>\n");
}

static void handle_quit(GUI_RPC_CONN& grc) {
    gstate.requested_exit = true;
    grc.mfout.printf("<success/>\n");
}

static void handle_acct_mgr_info(GUI_RPC_CONN& grc) {
    grc.mfout.printf(
        "<acct_mgr_info>\n"
        "   <acct_mgr_url>%s</acct_mgr_url>\n"
        "   <acct_mgr_name>%s</acct_mgr_name>\n",
        gstate.acct_mgr_info.master_url,
        gstate.acct_mgr_info.project_name
    );

    if (strlen(gstate.acct_mgr_info.login_name)) {
        grc.mfout.printf("   <have_credentials/>\n");
    }

    if (gstate.acct_mgr_info.cookie_required) {
        grc.mfout.printf("   <cookie_required/>\n");
        grc.mfout.printf(
            "   <cookie_failure_url>%s</cookie_failure_url>\n",
            gstate.acct_mgr_info.cookie_failure_url
        );
    }

    grc.mfout.printf("</acct_mgr_info>\n");
}

static void handle_get_statistics(GUI_RPC_CONN& grc) {
    grc.mfout.printf("<statistics>\n");
    for (std::vector<PROJECT*>::iterator i=gstate.projects.begin();
        i!=gstate.projects.end();++i
    ) {
        (*i)->write_statistics(grc.mfout,true);
    }
    grc.mfout.printf("</statistics>\n");
}

static void handle_get_cc_status(GUI_RPC_CONN& grc) {
    grc.mfout.printf(
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
        "   <simple_gui_only>%d</simple_gui_only>\n",
        net_status.network_status(),
        gstate.acct_mgr_info.password_error?1:0,
        gstate.suspend_reason,
        gstate.cpu_run_mode.get_current(),
        gstate.cpu_run_mode.get_perm(),
        gstate.cpu_run_mode.delay(),
        gpu_suspend_reason,
        gstate.gpu_run_mode.get_current(),
        gstate.gpu_run_mode.get_perm(),
        gstate.gpu_run_mode.delay(),
        gstate.network_suspend_reason,
        gstate.network_run_mode.get_current(),
        gstate.network_run_mode.get_perm(),
        gstate.network_run_mode.delay(),
        config.disallow_attach?1:0,
        config.simple_gui_only?1:0
    );
    if (grc.au_mgr_state == AU_MGR_QUIT_REQ) {
        grc.mfout.printf(
            "   <manager_must_quit>1</manager_must_quit>\n"
        );
        grc.au_mgr_state = AU_MGR_QUIT_SENT;
    }
    grc.mfout.printf(
        "</cc_status>\n"
    );
}

static void handle_network_available(GUI_RPC_CONN& grc) {
    net_status.network_available();
    grc.mfout.printf("<success/>\n");
}

static void handle_get_project_init_status(GUI_RPC_CONN& grc) {
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

    grc.mfout.printf(
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

void handle_get_project_config(GUI_RPC_CONN& grc) {
    string url;

    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_string("url", url)) continue;
    }
    if (url.empty()) {
        grc.mfout.printf("<error>no url</error>\n");
        return;
    }

    canonicalize_master_url(url);
    grc.get_project_config_op.do_rpc(url);
    grc.mfout.printf("<success/>\n");
}

void handle_get_project_config_poll(GUI_RPC_CONN& grc) {
    if (grc.get_project_config_op.error_num) {
        grc.mfout.printf(
            "<project_config>\n"
            "    <error_num>%d</error_num>\n"
            "</project_config>\n",
            grc.get_project_config_op.error_num
        );
    } else {
        grc.mfout.printf("%s", grc.get_project_config_op.reply.c_str());
    }
}

void handle_lookup_account(GUI_RPC_CONN& grc) {
    ACCOUNT_IN ai;
    MIOFILE in;

    ai.parse(grc.xp);
    if (!ai.url.size() || !ai.email_addr.size() || !ai.passwd_hash.size()) {
        grc.mfout.printf("<error>missing URL, email address, or password</error>\n");
        return;
    }

    grc.lookup_account_op.do_rpc(ai);
    grc.mfout.printf("<success/>\n");
}

void handle_lookup_account_poll(GUI_RPC_CONN& grc) {
    if (grc.lookup_account_op.error_num) {
        grc.mfout.printf(
            "<account_out>\n"
            "    <error_num>%d</error_num>\n"
            "</account_out>\n",
            grc.lookup_account_op.error_num
        );
    } else {
        grc.mfout.printf("%s", grc.lookup_account_op.reply.c_str());
    }
}

void handle_create_account(GUI_RPC_CONN& grc) {
    ACCOUNT_IN ai;

    ai.parse(grc.xp);
    grc.create_account_op.do_rpc(ai);
    grc.mfout.printf("<success/>\n");
}

void handle_create_account_poll(GUI_RPC_CONN& grc) {
    if (grc.create_account_op.error_num) {
        grc.mfout.printf(
            "<account_out>\n"
            "    <error_num>%d</error_num>\n"
            "</account_out>\n",
            grc.create_account_op.error_num
        );
    } else {
        grc.mfout.printf("%s", grc.create_account_op.reply.c_str());
    }
}

static void handle_project_attach(GUI_RPC_CONN& grc) {
    string url, authenticator, project_name;
    bool use_config_file = false;
    bool already_attached = false;
    unsigned int i;
    int retval;

    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_bool("use_config_file", use_config_file)) continue;
        if (grc.xp.parse_string("project_url", url)) continue;
        if (grc.xp.parse_string("authenticator", authenticator)) continue;
        if (grc.xp.parse_string("project_name", project_name)) continue;
    }

    // Get URL/auth from project_init.xml?
    //
    if (use_config_file) {
        if (!strlen(gstate.project_init.url)) {
            grc.mfout.printf("<error>Missing URL</error>\n");
            return;
        }

        if (!strlen(gstate.project_init.account_key)) {
            grc.mfout.printf("<error>Missing authenticator</error>\n");
            return;
        }

        url = gstate.project_init.url;
        authenticator = gstate.project_init.account_key;
    } else {
        if (url.empty()) {
            grc.mfout.printf("<error>Missing URL</error>\n");
            return;
        }
        if (authenticator.empty()) {
            grc.mfout.printf("<error>Missing authenticator</error>\n");
            return;
        }
    }

    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (url == p->master_url) already_attached = true;
    }

    if (already_attached) {
        grc.mfout.printf("<error>Already attached to project</error>\n");
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

    grc.mfout.printf("<success/>\n");
}

static void handle_project_attach_poll(GUI_RPC_CONN& grc) {
    unsigned int i;
    grc.mfout.printf(
        "<project_attach_reply>\n"
    );
    for (i=0; i<gstate.project_attach.messages.size(); i++) {
        grc.mfout.printf(
            "    <message>%s</message>\n",
            gstate.project_attach.messages[i].c_str()
        );
    }
    grc.mfout.printf(
        "    <error_num>%d</error_num>\n",
        gstate.project_attach.error_num
    );
    grc.mfout.printf(
        "</project_attach_reply>\n"
    );
}

static void handle_acct_mgr_rpc(GUI_RPC_CONN& grc) {
    string url, name, password;
    string password_hash, name_lc;
    bool use_config_file = false;
    bool bad_arg = false;
    bool url_found=false, name_found=false, password_found = false;

    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_string("url", url)) {
            url_found = true;
            continue;
        }
        if (grc.xp.parse_string("name", name)) {
            name_found = true;
            continue;
        }
        if (grc.xp.parse_string("password", password)) {
            password_found = true;
            continue;
        }
        if (grc.xp.parse_bool("use_config_file", use_config_file)) continue;
    }
    if (!use_config_file) {
        bad_arg = !url_found || !name_found || !password_found;
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
        if (!strlen(gstate.acct_mgr_info.master_url)) {
            bad_arg = true;
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Account manager info missing from config file"
            );
        } else {
            url = gstate.acct_mgr_info.master_url;
            name = gstate.acct_mgr_info.login_name;
            password_hash = gstate.acct_mgr_info.password_hash;
        }
    }
    if (bad_arg) {
        grc.mfout.printf("<error>bad arg</error>\n");
    } else {
        gstate.acct_mgr_op.do_rpc(url, name, password_hash, true);
        grc.mfout.printf("<success/>\n");
    }
}

static void handle_acct_mgr_rpc_poll(GUI_RPC_CONN& grc) {
    grc.mfout.printf(
        "<acct_mgr_rpc_reply>\n"
    );
    if (gstate.acct_mgr_op.error_str.size()) {
        grc.mfout.printf(
            "    <message>%s</message>\n",
            gstate.acct_mgr_op.error_str.c_str()
        );
    }
    grc.mfout.printf(
        "    <error_num>%d</error_num>\n",
        gstate.acct_mgr_op.error_num
    );
    grc.mfout.printf(
        "</acct_mgr_rpc_reply>\n"
    );
}

static void handle_get_newer_version(GUI_RPC_CONN& grc) {
    grc.mfout.printf(
        "<newer_version>%s</newer_version>\n"
        "<download_url>%s</download_url>\n",
        gstate.newer_version.c_str(),
        config.client_download_url.c_str()
    );
}

static void handle_get_global_prefs_file(GUI_RPC_CONN& grc) {
    GLOBAL_PREFS p;
    bool found;
    int retval = p.parse_file(
        GLOBAL_PREFS_FILE_NAME, gstate.main_host_venue, found
    );
    if (retval) {
        grc.mfout.printf("<error>%d</error>\n", retval);
        return;
    }
    p.write(grc.mfout);
}

static void handle_get_global_prefs_working(GUI_RPC_CONN& grc) {
    gstate.global_prefs.write(grc.mfout);
}

static void handle_get_global_prefs_override(GUI_RPC_CONN& grc) {
    string s;
    int retval = read_file_string(GLOBAL_PREFS_OVERRIDE_FILE, s);
    if (!retval) {
        strip_whitespace(s);
        grc.mfout.printf("%s\n", s.c_str());
    } else {
        grc.mfout.printf("<error>no prefs override file</error>\n");
    }
}

static void handle_set_global_prefs_override(GUI_RPC_CONN& grc) {
    int retval;
    char buf[65536];

    retval = grc.xp.element_contents("</set_global_prefs_override>", buf, sizeof(buf));
    if (!retval) {
        if (strlen(buf)) {
            FILE* f = boinc_fopen(GLOBAL_PREFS_OVERRIDE_FILE, "w");
            if (f) {
                fprintf(f, "%s\n", buf);
                fclose(f);
                retval = 0;
            } else {
                retval = ERR_FOPEN;
            }
        } else {
            retval = boinc_delete_file(GLOBAL_PREFS_OVERRIDE_FILE);
        }
    }
    grc.mfout.printf(
        "<set_global_prefs_override_reply>\n"
        "    <status>%d</status>\n"
        "</set_global_prefs_override_reply>\n",
        retval
    );
}

static void handle_get_cc_config(GUI_RPC_CONN& grc) {
    string s;
    int retval = read_file_string(CONFIG_FILE, s);
    if (!retval) {
        strip_whitespace(s);
        grc.mfout.printf("%s\n", s.c_str());
    }
}

static void read_all_projects_list_file(GUI_RPC_CONN& grc) {
    string s;
    int retval = read_file_string(ALL_PROJECTS_LIST_FILENAME, s);
    if (!retval) {
        strip_whitespace(s);
        grc.mfout.printf("%s\n", s.c_str());
    }
}

static void handle_get_state(GUI_RPC_CONN& grc) {
    gstate.write_state_gui(grc.mfout);
}

static void handle_set_cc_config(GUI_RPC_CONN& grc) {
    int retval;
    char buf[65536];

    retval = grc.xp.element_contents("</set_cc_config>", buf, sizeof(buf));

    if (!retval) {
        if (strlen(buf)) {
            FILE* f = boinc_fopen(CONFIG_FILE, "w");
            if (f) {
                fprintf(f, "%s\n", buf);
                fclose(f);
                retval = 0;
            } else {
                retval = ERR_FOPEN;
            }
        } else {
            retval = boinc_delete_file(CONFIG_FILE);
        }
    }
    grc.mfout.printf(
        "<set_cc_config_reply>\n"
        "    <status>%d</status>\n"
        "</set_cc_config_reply>\n",
        retval
    );
}

static void handle_get_notices(GUI_RPC_CONN& grc) {
    int seqno = 0;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_int("seqno", seqno)) continue;
    }
    notices.write(seqno, grc, false);
}

static void handle_get_notices_public(GUI_RPC_CONN& grc) {
    int seqno = 0;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_int("seqno", seqno)) continue;
    }
    notices.write(seqno, grc, true);
}

static void handle_get_results(GUI_RPC_CONN& grc) {
    bool active_only = false;
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_bool("active_only", active_only)) continue;
    }
    grc.mfout.printf("<results>\n");
    gstate.write_tasks_gui(grc.mfout, active_only);
    grc.mfout.printf("</results>\n");
}

static void handle_get_all_projects_list(GUI_RPC_CONN& grc) {
    read_all_projects_list_file(grc);
}

static void handle_get_file_transfers(GUI_RPC_CONN& grc) {
    gstate.write_file_transfers_gui(grc.mfout);
}

static void handle_read_global_prefs_override(GUI_RPC_CONN& grc) {
    grc.mfout.printf("<success/>\n");
    gstate.read_global_prefs();
    gstate.request_schedule_cpus("Preferences override");
    gstate.request_work_fetch("Preferences override");
}

static void handle_read_cc_config(GUI_RPC_CONN& grc) {
    grc.mfout.printf("<success/>\n");
    read_config_file(false);
    config.show();
    log_flags.show();
    gstate.set_ncpus();
    process_gpu_exclusions();
    gstate.request_schedule_cpus("Core client configuration");
    gstate.request_work_fetch("Core client configuration");
    set_no_rsc_config();
}

static void handle_get_daily_xfer_history(GUI_RPC_CONN& grc) {
    daily_xfer_history.write_xml(grc.mfout);
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

typedef void (*GUI_RPC_HANDLER)(GUI_RPC_CONN&);

struct GUI_RPC {
    const char* req_tag;
    char alt_req_tag[256];
    GUI_RPC_HANDLER handler;
    bool auth_required;
        // operations that require authentication only for non-local clients.
        // Use this only for information that should be available to people
        // sharing this computer (e.g. what jobs are running)
        // but not for anything sensitive (passwords etc.)
    bool enable_network;
        // RPCs that should enable network communication for 5 minutes,
        // overriding other factors.
        // Things like attaching projects, etc.
    bool read_only;         // doesn't modify the client's data structs

    GUI_RPC(const char* req, GUI_RPC_HANDLER h, bool ar, bool en, bool ro) {
        req_tag = req;
        strcpy(alt_req_tag, req);
        strcat(alt_req_tag, "/");
        handler = h;
        auth_required = ar;
        enable_network = en;
        read_only = ro;
    }
};

                                                                    // local auth required
                                                                            // enable network
                                                                                    // read-only
GUI_RPC gui_rpcs[] = {
    GUI_RPC("exchange_versions", handle_exchange_versions,          false,  false,  true),
    GUI_RPC("get_all_projects_list", handle_get_all_projects_list,  false,  false,  true),
    GUI_RPC("get_cc_status", handle_get_cc_status,                  false,  false,  true),
    GUI_RPC("get_disk_usage", handle_get_disk_usage,                false,  false,  true),
    GUI_RPC("get_daily_xfer_history", handle_get_daily_xfer_history,
                                                                    false,  false,  true),
    GUI_RPC("get_file_transfers", handle_get_file_transfers,        false,  false,  true),
    GUI_RPC("get_host_info", handle_get_host_info,                  false,  false,  true),
    GUI_RPC("get_messages", handle_get_messages,                    false,  false,  true),
    GUI_RPC("get_message_count", handle_get_message_count,          false,  false,  true),
    GUI_RPC("get_newer_version", handle_get_newer_version,          false,  false,  true),
    GUI_RPC("get_notices_public", handle_get_notices_public,        false,  false,  true),
    GUI_RPC("get_project_status", handle_get_project_status,        false,  false,  true),
    GUI_RPC("get_results", handle_get_results,                      false,  false,  true),
    GUI_RPC("get_screensaver_tasks", handle_get_screensaver_tasks,  false,  false,  true),
    GUI_RPC("get_simple_gui_info", handle_get_simple_gui_info,      false,  false,  true),
    GUI_RPC("get_state", handle_get_state,                          false,  false,  true),
    GUI_RPC("get_statistics", handle_get_statistics,                false,  false,  true),

    // ops requiring local auth start here

    GUI_RPC("abort_file_transfer", handle_abort_file_transfer,      true,   false,  false),
    GUI_RPC("abort_result", handle_abort_result,                    true,   false,  false),
    GUI_RPC("acct_mgr_info", handle_acct_mgr_info,                  true,   false,  true),
    GUI_RPC("get_cc_config", handle_get_cc_config,                  true,   false,  false),
    GUI_RPC("get_global_prefs_file", handle_get_global_prefs_file,  true,   false,  false),
    GUI_RPC("get_global_prefs_override", handle_get_global_prefs_override,
                                                                    true,   false,  false),
    GUI_RPC("get_global_prefs_working", handle_get_global_prefs_working,
                                                                    true,   false,  false),
    GUI_RPC("get_notices", handle_get_notices,                      true,   false,  true),
    GUI_RPC("get_project_init_status", handle_get_project_init_status,
                                                                    true,   false,  false),
    GUI_RPC("get_proxy_settings", handle_get_proxy_settings,        true,   false,  true),
    GUI_RPC("network_available", handle_network_available,          true,   false,  false),
    GUI_RPC("project_allowmorework", handle_project_allowmorework,  true,   false,  false),
    GUI_RPC("project_detach", handle_project_detach,                true,   false,  false),
    GUI_RPC("project_detach_when_done", handle_project_detach_when_done,
                                                                    true,   false,  false),
    GUI_RPC("project_dont_detach_when_done", handle_project_dont_detach_when_done,
                                                                    true,   false,  false),
    GUI_RPC("project_nomorework", handle_project_nomorework,        true,   false,  false),
    GUI_RPC("project_resume", handle_project_resume,                true,   false,  false),
    GUI_RPC("project_suspend", handle_project_suspend,              true,   false,  false),
    GUI_RPC("quit", handle_quit,                                    true,   false,  false),
    GUI_RPC("read_cc_config", handle_read_cc_config,                true,   false,  false),
    GUI_RPC("read_global_prefs_override", handle_read_global_prefs_override,
                                                                    true,   false,  false),
    GUI_RPC("resume_result", handle_resume_result,                  true,   false,  false),
    GUI_RPC("run_benchmarks", handle_run_benchmarks,                true,   false,  false),
    GUI_RPC("set_cc_config", handle_set_cc_config,                  true,   false,  false),
    GUI_RPC("set_global_prefs_override", handle_set_global_prefs_override,
                                                                    true,   false,  false),
    GUI_RPC("set_gpu_mode", handle_set_gpu_mode,                    true,   false,  false),
    GUI_RPC("set_network_mode", handle_set_network_mode,            true,   false,  false),
    GUI_RPC("set_proxy_settings", handle_set_proxy_settings,        true,   false,  false),
    GUI_RPC("set_run_mode", handle_set_run_mode,                    true,   false,  false),
    GUI_RPC("suspend_result", handle_suspend_result,                true,   false,  false),

    // ops requiring temporary network access start here

    GUI_RPC("acct_mgr_rpc", handle_acct_mgr_rpc,                    true,   true,   false),
    GUI_RPC("acct_mgr_rpc_poll", handle_acct_mgr_rpc_poll,          true,   true,   false),
    GUI_RPC("create_account", handle_create_account,                true,   true,   false),
    GUI_RPC("create_account_poll", handle_create_account_poll,      true,   true,   false),
    GUI_RPC("get_project_config", handle_get_project_config,        true,   true,   false),
    GUI_RPC("get_project_config_poll", handle_get_project_config_poll,
                                                                    true,   true,   false),
    GUI_RPC("lookup_account", handle_lookup_account,                true,   true,   false),
    GUI_RPC("lookup_account_poll", handle_lookup_account_poll,      true,   true,   false),
    GUI_RPC("project_attach", handle_project_attach,                true,   true,   false),
    GUI_RPC("project_attach_poll", handle_project_attach_poll,      true,   true,   false),
    GUI_RPC("project_reset", handle_project_reset,                  true,   true,   false),
    GUI_RPC("project_update", handle_project_update,                true,   true,   false),
    GUI_RPC("retry_file_transfer", handle_retry_file_transfer,      true,   true,   false),
};

// return nonzero only if we need to close the connection
//
static int handle_rpc_aux(GUI_RPC_CONN& grc) {
    int retval = 0;
    grc.mfin.init_buf_read(grc.request_msg);
    if (grc.xp.get_tag()) return ERR_XML_PARSE;   // parse <boinc_gui_rpc_request>
    if (grc.xp.get_tag()) return ERR_XML_PARSE;   // parse the request tag
    for (unsigned int i=0; i<sizeof(gui_rpcs)/sizeof(GUI_RPC); i++) {
        GUI_RPC& gr = gui_rpcs[i];
        if (!grc.xp.match_tag(gr.req_tag) && !grc.xp.match_tag(gr.alt_req_tag)) {
            continue;
        }
        if (gr.auth_required && grc.auth_needed) {
            auth_failure(grc.mfout);
            if (grc.sent_unauthorized) {
                retval = ERR_AUTHENTICATOR;
            }
            grc.sent_unauthorized = true;
            return retval;
        }
        if (gr.enable_network)  {
            gstate.gui_rpcs.time_of_last_rpc_needing_network = gstate.now;
        }
        (*gr.handler)(grc);
        return 0;;
    }
    grc.mfout.printf("<error>unrecognized op: %s</error>\n", grc.xp.parsed_tag);
    return 0;
}

// return nonzero only if we need to close the connection
//
int GUI_RPC_CONN::handle_rpc() {
    int n, retval=0;
    char* p;

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

    mfout.printf("<boinc_gui_rpc_reply>\n");
    if (match_req(request_msg, "auth1")) {
        if (got_auth1 && auth_needed) {
            retval = ERR_AUTHENTICATOR;
        } else {
            handle_auth1(mfout);
            got_auth1 = true;
        }
    } else if (match_req(request_msg, "auth2")) {
        if ((!got_auth1 || got_auth2) && auth_needed) {
            retval = ERR_AUTHENTICATOR;
        } else {
            retval = handle_auth2(request_msg, mfout);
            got_auth2 = true;
        }
    } else if (auth_needed && !is_local) {
        auth_failure(mfout);
        if (sent_unauthorized) {
            retval = ERR_AUTHENTICATOR;
        }
        sent_unauthorized = true;
    } else {
        retval = handle_rpc_aux(*this);
    }

    mfout.printf("</boinc_gui_rpc_reply>\n\003");
    mout.get_buf(p, n);
    if (http_request) {
        char buf[1024];
        sprintf(buf,
            "HTTP/1.1 200 OK\n"
            "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
            "Server: BOINC client\n"
            "Connection: close\n"
            "Content-Type: text/xml; charset=utf-8\n"
            "Content-Length: %d\n\n"
            "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n",
            n
        );
        send(sock, buf, strlen(buf), 0);
    }
    if (p) {
        send(sock, p, n, 0);
        p[n-1]=0;   // replace 003 with NULL
        if (log_flags.gui_rpc_debug) {
            if (n > 128) p[128] = 0;
            msg_printf(0, MSG_INFO,
                "[gui_rpc] GUI RPC reply: '%s'\n", p
            );
        }
        free(p);
    }
    return retval;
}
