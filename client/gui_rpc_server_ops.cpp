// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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
#include <libproc.h>
#include "sandbox.h"
#include "mac_branding.h"
extern int compareOSVersionTo(int toMajor, int toMinor);
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
#include "str_replace.h"
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

const char* HTTP_HEADER_DELIM = "\r\n\r\n";
const size_t HTTP_HEADER_DELIM_LEN = strlen(HTTP_HEADER_DELIM);

static void auth_failure(MIOFILE& fout) {
    fout.printf("<unauthorized/>\n");
}

void GUI_RPC_CONN::handle_auth1(MIOFILE& fout) {
    snprintf(nonce, sizeof(nonce), "%f", dtime());
    fout.printf("<nonce>%s</nonce>\n", nonce);
}

int GUI_RPC_CONN::handle_auth2(char* buf, MIOFILE& fout) {
    char nonce_hash[256], nonce_hash_correct[256], buf2[512];
    if (!parse_str(buf, "<nonce_hash>", nonce_hash, 256)) {
        auth_failure(fout);
        return ERR_AUTHENTICATOR;
    }
    snprintf(buf2, sizeof(buf2), "%s%s", nonce, gstate.gui_rpcs.password);
    md5_block((const unsigned char*)buf2, (int)strlen(buf2), nonce_hash_correct);
    if (strcmp(nonce_hash, nonce_hash_correct)) {
        auth_failure(fout);
        return ERR_AUTHENTICATOR;
    }
    fout.printf("<authorized/>\n");
    auth_needed = false;
    return 0;
}

static void handle_exchange_versions(GUI_RPC_CONN& grc) {
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_int("major", grc.client_api.major)) continue;
        if (grc.xp.parse_int("minor", grc.client_api.minor)) continue;
        if (grc.xp.parse_int("release", grc.client_api.release)) continue;
        if (grc.xp.parse_string("name", grc.client_name)) continue;
    }
    if (log_flags.gui_rpc_debug) {
        msg_printf(NULL, MSG_INFO, "[gui_rpc] RPC client: %s; API %d.%d.%d",
            grc.client_name.size() ? grc.client_name.c_str() : "unknown",
            grc.client_api.major, grc.client_api.minor, grc.client_api.release
        );
    }

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
    double size, boinc_non_project, d_allowed;
//    double boinc_total;

    grc.mfout.printf("<disk_usage_summary>\n");
    int retval = get_filesystem_info(
        gstate.host_info.d_total, gstate.host_info.d_free
    );
    if (retval) {
        msg_printf(0, MSG_INTERNAL_ERROR,
            "get_filesystem_info(): %s", boincerror(retval)
        );
    }

    dir_size_alloc(".", boinc_non_project, false);
    dir_size_alloc("locale", size, false);
    boinc_non_project += size;
#ifdef __APPLE__
    if (gstate.launched_by_manager) {
        // If launched by Manager, get Manager's size on disk
        char path[MAXPATHLEN];
        double manager_size = 0.0;
        OSStatus err = noErr;

        retval = proc_pidpath(getppid(), path, sizeof(path));
        if (retval <= 0) {
            err = fnfErr;
        }
        if (!err) {
            dir_size_alloc(path, manager_size, true);
            boinc_non_project += manager_size;
        }
    }
#endif
//    boinc_total = boinc_non_project;
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
//        boinc_total += p->disk_usage;
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
    gstate.request_work_fetch("project work fetch resumed by user");
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
    p->detach_when_done = false;
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

// On Android, get product name, OS name, OS version, domain name (device name), and MAC addr from GUI,
//
static void handle_set_host_info(GUI_RPC_CONN& grc) {
    while (!grc.xp.get_tag()) {
        if (grc.xp.match_tag("host_info")) {
            HOST_INFO hi;
            int retval = hi.parse(grc.xp);
            if (retval) {
                grc.mfout.printf("<error>host_info parse error</error>\n");
                return;
            }
            if (strlen(hi.product_name)) {
                safe_strcpy(gstate.host_info.product_name, hi.product_name);
            }

            // this will always be "Android"
            //
            if (strlen(hi.os_name)) {
                safe_strcpy(gstate.host_info.os_name, hi.os_name);
            }

            // We already have the Linux kernel version;
            // append the Android version.
            //
            if (strlen(hi.os_version)) {
                if (!strstr(gstate.host_info.os_version, "Android")) {
                    safe_strcat(gstate.host_info.os_version, " (Android ");
                    safe_strcat(gstate.host_info.os_version, hi.os_version);
                    safe_strcat(gstate.host_info.os_version, ")");
                }
            }

            // Device name
            if (strlen(hi.domain_name)) {
                safe_strcpy(gstate.host_info.domain_name, hi.domain_name);
            }

            grc.mfout.printf("<success/>\n");
            gstate.set_client_state_dirty("set_host_info RPC");
            return;
        }
    }
    grc.mfout.printf("<error>Missing host_info</error>\n");
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
    gstate.start_cpu_benchmarks(true);
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

    safe_strcpy(result_name, "");
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
        gstate.request_work_fetch("task suspended by user");
    } else if (!strcmp(op, "resume")) {
        msg_printf(p, MSG_INFO, "task %s resumed by user", result_name);
        rp->suspended_via_gui = false;
    }
    gstate.request_schedule_cpus("task suspended, resumed or aborted by user");
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

static void handle_reset_host_info(GUI_RPC_CONN& grc) {
    gstate.host_info.get_host_info(true);
    // the amount of RAM or #CPUs may have changed
    //
    gstate.set_n_usable_cpus();
    gstate.request_schedule_cpus("reset_host_info");
    gstate.show_host_info();
    grc.mfout.printf("<success/>\n");
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
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED) {
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

    if (strlen(gstate.acct_mgr_info.login_name)
        || strlen(gstate.acct_mgr_info.authenticator)
    ) {
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
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->write_statistics(grc.mfout);
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
        "   <simple_gui_only>%d</simple_gui_only>\n"
        "   <max_event_log_lines>%d</max_event_log_lines>\n",
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
        cc_config.disallow_attach?1:0,
        cc_config.simple_gui_only?1:0,
        cc_config.max_event_log_lines
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
        if (urls_match(p->master_url, gstate.project_init.url)) {
            gstate.project_init.remove();
            break;
        }
    }

    grc.mfout.printf(
        "<get_project_init_status>\n"
        "    <url>%s</url>\n"
        "    <name>%s</name>\n"
        "    %s\n"
        "    %s\n"
        "</get_project_init_status>\n",
        gstate.project_init.url,
        gstate.project_init.name,
        strlen(gstate.project_init.account_key)?"<has_account_key/>":"",
        gstate.project_init.embedded?"<embedded/>":""
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
        const char *p = grc.get_project_config_op.reply.c_str();
        const char *q = strstr(p, "<project_config");
        if (!q) q = "<project_config/>\n";
        grc.mfout.printf("%s", q);
    }
}

void handle_lookup_account(GUI_RPC_CONN& grc) {
    ACCOUNT_IN ai;
    MIOFILE in;

    ai.parse(grc.xp);
	if ((!ai.url.size() || !ai.email_addr.size() || !ai.passwd_hash.size()) && !ai.server_assigned_cookie) {
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
        const char *p = grc.lookup_account_op.reply.c_str();
        const char *q = strstr(p, "<account_out");
        if (!q) q = strstr(p, "<error");
        if (!q) q = "<account_out/>\n";
        grc.mfout.printf("%s", q);
    }
}

void handle_create_account(GUI_RPC_CONN& grc) {
    ACCOUNT_IN ai;

    ai.parse(grc.xp);
    if (ai.consented_to_terms && !grc.client_name.size()) {
        grc.create_account_op.error_num = ERR_INVALID_STATE;
        grc.mfout.printf("<error>&lt;name&gt; must be set in &lt;exchange_versions&gt; before using &lt;consented_to_terms/&gt;</error>\n");
        return;
    }
    grc.create_account_op.do_rpc(ai, grc.client_name);
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
    string url, authenticator, project_name, email_addr;
    bool use_config_file = false;
    bool already_attached = false;
    unsigned int i;
    int retval;

    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_bool("use_config_file", use_config_file)) continue;
        if (grc.xp.parse_string("project_url", url)) continue;
        if (grc.xp.parse_string("authenticator", authenticator)) continue;
        if (grc.xp.parse_string("project_name", project_name)) continue;
        if (grc.xp.parse_string("email_addr", email_addr)) continue;
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

    canonicalize_master_url(url);

    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        string project_url = p->master_url;
        canonicalize_master_url(project_url);

        if (url == project_url) {
            already_attached = true;
            break;
        }
    }

    if (already_attached) {
        grc.mfout.printf("<error>Already attached to project</error>\n");
        return;
    }

    // clear messages from previous attach to project.
    //
    gstate.project_attach.messages.clear();
    gstate.project_attach.error_num = gstate.add_project(
        url.c_str(), authenticator.c_str(), project_name.c_str(), email_addr.c_str(), false
    );

    // if project_init.xml refers to this project,
    // delete the file, otherwise we'll just
    // reattach the next time the client starts
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

// This RPC, regrettably, serves 3 purposes
// - to join an account manager
//   pass URL of account manager and account name/passwd
// - to trigger an RPC to the current account manager
//   either
//   pass URL/name/passwd hash of current AM
//      TODO: get rid of this option;
//      the manager shouldn't have to keep track of this info
//   or pass <use_config_file/> flag: do RPC to current AM
// - to quit an account manager
//   url/name/passwd args are null
//
static void handle_acct_mgr_rpc(GUI_RPC_CONN& grc) {
    string url, name, password, authenticator;
    string password_hash, name_lc;
    bool use_config_file = false;
    bool bad_arg = false;
    bool url_found=false, name_found=false, password_found = false;
    ACCT_MGR_INFO ami;

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
    if (use_config_file) {
        // really means: use current AM
        //
        if (!gstate.acct_mgr_info.using_am()) {
            bad_arg = true;
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Not using account manager"
            );
        } else {
            ami = gstate.acct_mgr_info;
        }
    } else {
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
            safe_strcpy(ami.master_url, url.c_str());
            safe_strcpy(ami.login_name, name.c_str());
            safe_strcpy(ami.password_hash, password_hash.c_str());
            safe_strcpy(ami.authenticator, authenticator.c_str());
        }
    }

    if (bad_arg) {
        grc.mfout.printf("<error>bad arg</error>\n");
    } else if (gstate.acct_mgr_info.using_am()
        && !url.empty()
        && !gstate.acct_mgr_info.same_am(url.c_str(), name.c_str(), password_hash.c_str(), authenticator.c_str())
    ){
        grc.mfout.printf("<error>attached to a different AM - detach first</error>\n");
    } else {
        gstate.acct_mgr_op.do_rpc(ami, true);
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
    gstate.new_version_check(true);
    // this initiates an RPC to get version info.
    // Wait for it to finish.
    //
    while (gstate.get_current_version_op.gui_http->gui_http_state != HTTP_STATE_IDLE) {
        if (!gstate.poll_slow_events()) {
            gstate.do_io_or_sleep(1.0);
        }
    }
    grc.mfout.printf(
        "<newer_version>%s</newer_version>\n"
        "<download_url>%s</download_url>\n",
        gstate.newer_version.c_str(),
        nvc_config.client_download_url.c_str()
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
    if (retval) {
        grc.mfout.printf("<status>%d</status>\n", retval);
    } else {
        grc.mfout.printf("<success/>\n");
    }
}

static void read_all_projects_list_file(GUI_RPC_CONN& grc) {
    string s;
    int retval = read_file_string(ALL_PROJECTS_LIST_FILENAME, s);
    if (!retval) {
        strip_whitespace(s);
        const char *q = strstr(s.c_str(), "<projects");
        if (!q) q = "<projects/>";
        grc.mfout.printf("%s\n", q);
    }
}

static void handle_get_state(GUI_RPC_CONN& grc) {
    gstate.write_state_gui(grc.mfout);
}

static void handle_get_cc_config(GUI_RPC_CONN& grc) {
    string s;
    int retval = read_file_string(CONFIG_FILE, s);
    if (!retval) {
        strip_whitespace(s);
        grc.mfout.printf("%s\n", s.c_str());
    }
}

static void handle_get_app_config(GUI_RPC_CONN& grc) {
    string url;
    string s;
    char path[MAXPATHLEN];
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_string("url", url)) continue;
    }
    PROJECT* p = gstate.lookup_project(url.c_str());
    if (!p) {
        grc.mfout.printf("<error>no such project</error>");
        return;
    }
    snprintf(path, sizeof(path), "%s/%s", p->project_dir(), APP_CONFIG_FILE_NAME);
    int retval = read_file_string(path, s);
    if (retval) {
        grc.mfout.printf("<error>app_config.xml not found</error>\n");
    } else {
        strip_whitespace(s);
        grc.mfout.printf("%s\n", s.c_str());
    }
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
    if (retval) {
        grc.mfout.printf("<status>%d</status>\n", retval);
    } else {
        grc.mfout.printf("<success/>\n");
    }
}

static void handle_set_app_config(GUI_RPC_CONN& grc) {
    APP_CONFIGS ac;
    string url;
    MSG_VEC mv;
    LOG_FLAGS lf;
    int parse_retval = -1;
    while (!grc.xp.get_tag()) {
        if (grc.xp.match_tag("app_config")) {
            lf.init();
            parse_retval = ac.parse(grc.xp, mv, lf);
        } else if (grc.xp.parse_string("url", url)) {
            continue;
        }
    }
    if (parse_retval) {
        grc.mfout.printf("<error>XML parse failed</error>\n");
        return;
    }
    PROJECT* p = gstate.lookup_project(url.c_str());
    if (!p) {
        grc.mfout.printf("<error>no such project</error>\n");
        return;
    }
    char path[MAXPATHLEN];
    snprintf(path, sizeof(path), "%s/app_config.xml", p->project_dir());
    FILE* f = boinc_fopen(path, "w");
    if (!f) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "Can't open app config file %s", path
        );
        grc.mfout.printf("<error>can't open app_config.xml file</error>\n");
        return;

    }
    MIOFILE mf;
    mf.init_file(f);
    ac.write(mf);
    fclose(f);
    grc.mfout.printf("<success/>\n");
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

static void handle_get_old_results(GUI_RPC_CONN& grc) {
    print_old_results(grc.mfout);
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
    cc_config.show();
    log_flags.show();
    gstate.set_n_usable_cpus();
    process_gpu_exclusions();

    // also reread app_config.xml files
    //
    check_app_config();
    gstate.write_tasks_gui(grc.mfout, false, true);
    gstate.request_schedule_cpus("Core client configuration");
    gstate.request_work_fetch("Core client configuration");
    set_no_rsc_config();
}

static void handle_get_daily_xfer_history(GUI_RPC_CONN& grc) {
    daily_xfer_history.write_xml(grc.mfout);
}

#ifdef __APPLE__
static void stop_graphics_app(pid_t thePID,
    long iBrandID,
    char current_dir[],
    char switcher_path[],
    string theScreensaverLoginUser,
    GUI_RPC_CONN& grc
) {
    char* argv[16];
    int argc;
    char screensaverLoginUser[256];
    int newPID = 0;
    int retval;

    if (g_use_sandbox) {
        char pidString[10];

        snprintf(pidString, sizeof(pidString), "%d", thePID);
#if 1
        argv[0] = const_cast<char*>(SWITCHER_FILE_NAME);
        argv[1] = saverName[iBrandID];
        argv[2] = "-kill_gfx";
        argv[3] = pidString;
        argc = 4;
#else
        argv[0] = const_cast<char*>(SWITCHER_FILE_NAME);
        argv[1] = "/bin/kill";
        argv[2] = "-kill";
        argv[3] = (char *)pidString;
        argc = 4;
#endif
        // Graphics apps called by screensaver or Manager (via Show
        // Graphics button) now write files in their slot directory
        // as the logged in user, not boinc_master. This ugly hack
        // uses setprojectgrp to fix all ownerships in this slot
        // directory.
        // To fix all ownerships in the slot directory, invoke the
        // run_graphics_app RPC with operation "stop", slot number
        // for the operand and empty string for screensaverLoginUser
        // after the graphics app stops.
        if (theScreensaverLoginUser.empty()) {
            fix_slot_owners(thePID);    // Manager passes slot # instead of PID
            return;
        }

        argv[argc++] = "--ScreensaverLoginUser";
        safe_strcpy(screensaverLoginUser, theScreensaverLoginUser.c_str());
        argv[argc++] = screensaverLoginUser;
        argv[argc] = 0;

        retval = run_program(current_dir, switcher_path, argc, argv, newPID);
    } else {
        retval = kill_process(thePID);
    }
    if (retval) {
        grc.mfout.printf("<error>attempt to kill graphics app failed</error>\n");
        return;
    }
    grc.mfout.printf("<success/>\n");
    return;
}
#endif

// start, stop or get status of a graphics app on behalf of the screensaver.
// (needed for Mac OS X 10.15+; "stop & "test" are used for Mac OS X 10.13+)
//
// <slot>n</slot> { <run/> | <runfullscreen/> }
// <graphics_pid>p</graphics_pid> { <stop/> | <test/> }
//
// n is the slot number:
//   if slot = -1, start the default screensaver
// p is the process id to stop
//   test returns 0 for the pid if it has exited, else returns the child's pid
//
static void handle_run_graphics_app(GUI_RPC_CONN& grc) {
#ifndef __APPLE__
    grc.mfout.printf("<error>run_graphics_app RPC is currently available only on Mac OS</error>\n");
#else
    bool run = false;
    bool runfullscreen = false;
    bool stop = false;
    bool test = false;
    int slot = -2, retval;
    pid_t p;
    char* argv[16];
    int argc;
    int thePID = 0;
    FILE *f;
    long iBrandID;
    string theScreensaverLoginUser;
    char screensaverLoginUser[256];
    char switcher_path[MAXPATHLEN];
    char gfx_switcher_path[MAXPATHLEN];
    char *execName, *execPath;
    char current_dir[MAXPATHLEN];
    char *execDir;
    int newPID = 0;
    ACTIVE_TASK* atp = NULL;
    char cmd[256];
    bool need_to_launch_gfx_ss_bridge = false;
    static int gfx_ss_bridge_pid = 0;

    while (!grc.xp.get_tag()) {
        if (grc.xp.match_tag("/run_graphics_app")) break;
        if (grc.xp.parse_int("slot", slot)) continue;
        if (grc.xp.parse_bool("run", run)) continue;
        if (grc.xp.parse_bool("runfullscreen", runfullscreen)) continue;
        if (grc.xp.parse_bool("stop", stop)) continue;
        if (grc.xp.parse_bool("test", test)) continue;
        if (grc.xp.parse_int("graphics_pid", thePID)) continue;
        if (grc.xp.parse_string("ScreensaverLoginUser", theScreensaverLoginUser)) continue;
    }

    if (stop) {
        if (theScreensaverLoginUser.empty() ){
             if (thePID < 0) {
                 grc.mfout.printf("<error>missing or invalid slot number</error>\n");
            }
        } else {
            if (thePID < 1) {
                grc.mfout.printf("<error>missing or invalid process id</error>\n");
            }
        }
    } else if (test) {
        if (thePID < 1) {
            grc.mfout.printf("<error>missing or invalid process id</error>\n");
            return;
        }
    } else if (run || runfullscreen) {
        if (slot < -1) {
            grc.mfout.printf("<error>missing or invalid slot</error>\n");
            return;
        }
    } else {
        grc.mfout.printf("<error>missing or invalid operation</error>\n");
        return;
    }

    if (test) {
        // returns 0 for the pid if it has exited, else returns the child's pid
        p = 0;
        snprintf(cmd, sizeof(cmd), "ps -p %d -o pid", thePID);
        f = popen(cmd, "r");
        if (f) {
            fgets(cmd, sizeof(cmd), f); // Skip the header line
            fscanf(f, "%d", &p);
            pclose(f);
            grc.mfout.printf(
                "<graphics_pid>%d</graphics_pid>\n<success/>\n",
                p
            );
        }
        return;
    }

    // For branded installs, the Mac installer put a branding file in our data directory
    iBrandID = 0;   // Default value
    f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandID);
        fclose(f);
    }
    if ((iBrandID < 0) || (iBrandID > (NUMBRANDS-1))) {
        iBrandID = 0;
    }

    getcwd(current_dir, sizeof(current_dir));

    if (g_use_sandbox) {
        snprintf(switcher_path, sizeof(switcher_path),
            "%s/%s/%s",
            current_dir, SWITCHER_DIR, SWITCHER_FILE_NAME
        );
    }

    if (stop) {
        stop_graphics_app(thePID, iBrandID, current_dir, switcher_path,
                            theScreensaverLoginUser, grc);
        grc.mfout.printf("<success/>\n");
        return;
    }

    if (compareOSVersionTo(14, 0) >= 0) {
        // As of MacOS 14.0, the legacyScreenSaver sandbox prevents using
        // bootstrap_look_up, so we launch a bridging utility to relay Mach
        // communications between the graphics apps and the legacyScreenSaver.
        if (gfx_ss_bridge_pid == 0) {
            need_to_launch_gfx_ss_bridge = true;
        } else if (waitpid(gfx_ss_bridge_pid, 0, WNOHANG)) {
            gfx_ss_bridge_pid = 0;
            need_to_launch_gfx_ss_bridge = true;
        }
        if (need_to_launch_gfx_ss_bridge) {
            if (g_use_sandbox) {

                snprintf(gfx_switcher_path, sizeof(gfx_switcher_path),
                    "/Library/Screen Savers/%s.saver/Contents/Resources/gfx_switcher",
                    saverName[iBrandID]
                );
            }
            argv[0] = const_cast<char*>("gfx_switcher");
            argv[1] = "-run_bridge";
            argv[2] = gfx_switcher_path;
            argc = 3;
            if (!theScreensaverLoginUser.empty()) {
                argv[argc++] = "--ScreensaverLoginUser";
                safe_strcpy(screensaverLoginUser, theScreensaverLoginUser.c_str());
                argv[argc++] = screensaverLoginUser;
            }
            argv[argc] = 0;
            retval = run_program(current_dir, gfx_switcher_path, argc, argv, gfx_ss_bridge_pid);
        }
    }

    if (slot == -1) {
        // start boincscr
        //
        execPath = (char*)"./boincscr";
        execName = (char*)"boincscr";
        execDir = current_dir;
    } else {   // if (slot != -1)
        // start a graphics app
        //
        atp = gstate.active_tasks.lookup_slot(slot);
        if (!atp) {
            grc.mfout.printf("<error>no job in slot</error>\n");
            return;
        }
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) {
            grc.mfout.printf("<error>job not running</error>\n");
            return;
        }
        if (!strlen(atp->app_version->graphics_exec_path)) {
            grc.mfout.printf("<error>job has no graphics app</error>\n");
            return;
        }

        execPath = atp->app_version->graphics_exec_path;
        execName = atp->app_version->graphics_exec_file;
        execDir = atp->slot_path;
    }

    if (g_use_sandbox) {
        if (slot == -1) {
            argv[0] = const_cast<char*>(SWITCHER_FILE_NAME);
            argv[1] = execDir;
            argv[2] = saverName[iBrandID];
            argv[3] = "-default_gfx";
            argv[4] = "boincscr";
            argc = 5;
        } else {
            char theSlot[10];
            snprintf(theSlot, sizeof(theSlot), "%d", slot);
            argv[0] = const_cast<char*>(SWITCHER_FILE_NAME);
            argv[1] = execDir;
            argv[2] = saverName[iBrandID];
            argv[3] = "-launch_gfx";
            argv[4] = (char *)theSlot;
            argc = 5;
        }

        if (runfullscreen) {
            argv[argc++] = "--fullscreen";
        }
        if (!theScreensaverLoginUser.empty()) {
            argv[argc++] = "--ScreensaverLoginUser";
            safe_strcpy(screensaverLoginUser, theScreensaverLoginUser.c_str());
            argv[argc++] = screensaverLoginUser;
        }
        argv[argc] = 0;
        retval = run_program(execDir, switcher_path, argc, argv, newPID);
    } else {    // not g_use_sandbox
        argv[0] = execName;
        if (runfullscreen) {
            argv[1] = (char*)"--fullscreen";
            argc = 2;
        } else {
            argc = 1;
        }
        if (!theScreensaverLoginUser.empty()) {
            argv[argc++] = "--ScreensaverLoginUser";
            safe_strcpy(screensaverLoginUser, theScreensaverLoginUser.c_str());
            argv[argc++] = screensaverLoginUser;
        }
        argv[argc] = 0;
        retval = run_program(execDir, execPath, argc, argv, newPID);
    }

    if (retval) {
        grc.mfout.printf("<error>couldn't run graphics app</error>\n");
        stop_graphics_app(thePID, iBrandID, current_dir, switcher_path,
                            theScreensaverLoginUser, grc);
    } else {
        grc.mfout.printf("<success/>\n");
    }
    return;
#endif  // __APPLE__
}

// We use a different authentication scheme for HTTP because
// each request has its own connection.
// Send clients an "authentication ID".
// Each request has (in HTTP header vars) the ID,
// a sequence number, and a hash of the seq# and the password.
//
struct AUTH_INFO {
    int id;
    long seqno;
    char salt[64];
};

vector<AUTH_INFO> auth_infos;

// check HTTP authentication info
//
bool valid_auth(int id, long seqno, char* hash, char* request) {
    char buf[1024], my_hash[256];
    //printf("valid_auth: id %d seqno %ld hash %s\n", id, seqno, hash);
    for (unsigned int i=0; i<auth_infos.size(); i++) {
        AUTH_INFO& ai = auth_infos[i];
        if (ai.id != id) continue;
        if (seqno <= ai.seqno) return false;
        int n = request?(int)strlen(request):0;
        snprintf(buf, sizeof(buf), "%ld%s%s", seqno, gstate.gui_rpcs.password, ai.salt);
        md5_block((const unsigned char*)buf, (int)strlen(buf), my_hash,
            (const unsigned char*)request, n
        );
        if (strcmp(hash, my_hash)) {
            msg_printf(0, MSG_INFO, "got invalid GUI RPC request");
            return false;
        }
        ai.seqno = seqno;   // bump seqno only if valid request
        return true;
    }
    return false;
}

// create a new authentication ID
//
void handle_get_auth_id(MIOFILE& fout) {
    static int id=0;
    AUTH_INFO ai;
    ai.id = id++;
    ai.seqno = 0;
    make_secure_random_string(ai.salt);
    auth_infos.push_back(ai);
    fout.printf("<auth_id>%d</auth_id>\n<auth_salt>%s</auth_salt>\n", ai.id, ai.salt);
}

// see if the HTTP request has valid authentication info
//
static bool authenticated_request(char* buf) {
    int auth_id;
    long auth_seqno;
    char auth_hash[256];
    const char* p = strcasestr(buf, "Auth-ID: ");
    if (!p) return false;
    int n = sscanf(p+strlen("Auth-ID: "), "%d", &auth_id);
    if (n != 1) return false;
    p = strcasestr(buf, "Auth-Seqno: ");
    if (!p) return false;
    n = sscanf(p+strlen("Auth-Seqno: "), "%ld", &auth_seqno);
    if (n != 1) return false;
    p = strcasestr(buf, "Auth-Hash: ");
    if (!p) return false;
    n = sscanf(p+strlen("Auth-Hash: "), "%64s", auth_hash);
    if (n != 1) return false;
    char* request = strstr(buf, HTTP_HEADER_DELIM);
    if (request) request += HTTP_HEADER_DELIM_LEN;
    return valid_auth(auth_id, auth_seqno, auth_hash, request);
}

static void handle_set_language(GUI_RPC_CONN& grc) {
    while (!grc.xp.get_tag()) {
        if (grc.xp.parse_str("language", gstate.language, sizeof(gstate.language))) {
            gstate.set_client_state_dirty("set_language");
            grc.mfout.printf("<success/>\n");
            return;
        }
    }
    grc.mfout.printf("<error>no language found</error>\n");
}

#ifdef ANDROID
static void handle_report_device_status(GUI_RPC_CONN& grc) {
    DEVICE_STATUS d;
    while (!grc.xp.get_tag()) {
        if (grc.xp.match_tag("device_status")) {
            int retval = d.parse(grc.xp);
            if (log_flags.android_debug) {
                if (retval) {
                    msg_printf(0, MSG_INFO,
                        "report_device_status RPC parse failed: %d", retval
                    );
                } else {
                    msg_printf(0, MSG_INFO,
                        "Android device status:"
                    );
                    msg_printf(0, MSG_INFO,
                        "On AC: %s; on USB: %s; on WiFi: %s; user active: %s",
                        d.on_ac_power?"yes":"no",
                        d.on_usb_power?"yes":"no",
                        d.wifi_online?"yes":"no",
                        d.user_active?"yes":"no"
                    );
                    msg_printf(0, MSG_INFO,
                        "Battery: charge pct: %f; temp %f state %s",
                        d.battery_charge_pct,
                        d.battery_temperature_celsius,
                        battery_state_string(d.battery_state)
                    );
                }
            }
            if (!retval) {
                // if the GUI reported a device name, use it
                //
                if (strlen(d.device_name)) {
                    if (strcmp(d.device_name, gstate.host_info.domain_name)) {
                        safe_strcpy(gstate.host_info.domain_name, d.device_name);
                        gstate.set_client_state_dirty("Device name changed");
                    }
                }
                gstate.device_status = d;
                gstate.device_status_time = gstate.now;
                grc.mfout.printf("<success/>\n");
                return;
            }
        }
    }
    grc.mfout.printf("<error/>\n");
}

int DEVICE_STATUS::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/device_status")) {
            return 0;
        }
        if (xp.parse_bool("on_ac_power", on_ac_power)) continue;
        if (xp.parse_bool("on_usb_power", on_usb_power)) continue;
        if (xp.parse_double("battery_charge_pct", battery_charge_pct)) continue;
        if (xp.parse_int("battery_state", battery_state)) continue;
        if (xp.parse_double("battery_temperature_celsius", battery_temperature_celsius)) continue;
        if (xp.parse_bool("wifi_online", wifi_online)) continue;
        if (xp.parse_bool("user_active", user_active)) continue;
        if (xp.parse_str("device_name", device_name, sizeof(device_name))) continue;
    }
    return ERR_XML_PARSE;
}

#endif      // ANDROID

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
        // operations that require authentication with RPC key
    bool enable_network;
        // RPCs that should enable network communication for 5 minutes,
        // overriding other factors.
        // Things like attaching projects, etc.
    bool read_only;         // doesn't modify the client's data structs

    GUI_RPC(const char* req, GUI_RPC_HANDLER h, bool ar, bool en, bool ro) {
        req_tag = req;
        safe_strcpy(alt_req_tag, req);
        safe_strcat(alt_req_tag, "/");
        handler = h;
        auth_required = ar;
        enable_network = en;
        read_only = ro;
    }
};

                                                                    // auth required
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
    GUI_RPC("get_old_results", handle_get_old_results,              false,  false,  true),
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
    GUI_RPC("get_app_config", handle_get_app_config,                true,   false,  false),
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
#ifdef ANDROID
    GUI_RPC("report_device_status", handle_report_device_status,    true,   false,  false),
#endif
    GUI_RPC("reset_host_info", handle_reset_host_info,              true,   false,  false),
    GUI_RPC("resume_result", handle_resume_result,                  true,   false,  false),
    GUI_RPC("run_benchmarks", handle_run_benchmarks,                true,   false,  false),
    GUI_RPC("set_app_config", handle_set_app_config,                true,   false,  false),
    GUI_RPC("set_cc_config", handle_set_cc_config,                  true,   false,  false),
    GUI_RPC("set_global_prefs_override", handle_set_global_prefs_override,
                                                                    true,   false,  false),
    GUI_RPC("set_gpu_mode", handle_set_gpu_mode,                    true,   false,  false),
    GUI_RPC("set_host_info", handle_set_host_info,                  true,   false,  false),
    GUI_RPC("set_language", handle_set_language,                    true,   false,  false),
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
    GUI_RPC("run_graphics_app", handle_run_graphics_app,            false,   false,   false),
};

// return nonzero only if we need to close the connection
//
static int handle_rpc_aux(GUI_RPC_CONN& grc) {
    int retval = 0;
    grc.mfin.init_buf_read(grc.request_msg);
    if (grc.xp.get_tag()) {    // parse <boinc_gui_rpc_request>
        grc.mfout.printf("<error>missing boinc_gui_rpc_request tag</error>\n");
        return 0;
    }
    if (grc.xp.get_tag()) {    // parse the request tag
        grc.mfout.printf("<error>missing request</error>\n");
        return 0;
    }
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
        return 0;
    }
    grc.mfout.printf("<error>unrecognized op: %s</error>\n", grc.xp.parsed_tag);
    return 0;
}

// see if we got a complete HTTP POST request
//
static bool is_http_post_request(char* buf) {
    if (strstr(buf, "POST") != buf) return false;
    char* p = strstr(buf, "Content-Length: ");
    if (!p) return false;
    p += strlen("Content-Length: ");
    int n = atoi(p);
    p = strstr(p, HTTP_HEADER_DELIM);
    if (!p) return false;
    p += 4;
    if ((int)strlen(p) < n) return false;
    return true;
}

// remove HTTP header from request
//
static void strip_http_header(char* buf) {
    char* p = strstr(buf, HTTP_HEADER_DELIM);
    p += 4;
    strcpy_overlap(buf, p);
}

static bool is_http_get_request(char* buf) {
    return (strstr(buf, "GET") == buf);
}

// send HTTP error reply
//
void GUI_RPC_CONN::http_error(const char* msg) {
    send(sock, msg, (int)strlen(msg), 0);
}

// handle a GET request, returning a file from the BOINC data dir.
// This is unauthenticated so be paranoid:
// - only .html, .js, and .css filenames
// - no ..
//
void GUI_RPC_CONN::handle_get() {
    // no one is using this feature and it's a potential security risk,
    // so disable it for now.
    //
    return http_error("HTTP/1.0 403 Access denied\n\nAccess denied\n");
#if 0
    if (!cc_config.allow_gui_rpc_get) {
        return http_error("HTTP/1.0 403 Access denied\n\nAccess denied\n");
    }

    // get filename from GET /foo.html HTTP/1.1
    // and make sure it's relative
    //
    char *p, *q=0;
    p = strchr(request_msg, '/');
    if (p) {
        while (*p=='/') {
            p++;
        }
        q = strchr(p, ' ');
    }

    if (!q) {
        return http_error("HTTP/1.0 400 Bad request\n\nBad HTTP request\n");
    }

    *q = 0;
    if (strstr(p, "..") || strchr(p, ':')) {
        return http_error("HTTP/1.0 400 Bad request\n\nBad HTTP request\n");
    }
    if (!ends_with(p, ".html")
        && !ends_with(p, ".js")
        && !ends_with(p, ".css")
    ) {
        return http_error("HTTP/1.0 400 Bad request\n\nBad file type\n");
    }

    //  read the file
    //
    string file;
    if (read_file_string(p, file)) {
        return http_error("HTTP/1.0 404 Not Found\n\nFile not found\n");
    }
    int n = (int)file.size();
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "HTTP/1.1 200 OK\n"
        "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
        "Server: BOINC client\n"
        "Connection: close\n"
        "Content-Type: text/html; charset=utf-8\n"
        "Content-Length: %d\n\n",
        n
    );
    send(sock, buf, (int)strlen(buf), 0);
    send(sock, file.c_str(), n, 0);
#endif
}

// return nonzero only if we need to close the connection
//
int GUI_RPC_CONN::handle_rpc() {
    int retval=0;
    char* p;

    int left = GUI_RPC_REQ_MSG_SIZE - request_nbytes;
#ifdef _WIN32
    SSIZE_T nb = recv(sock, request_msg+request_nbytes, left, 0);
#else
    ssize_t nb = read(sock, request_msg+request_nbytes, left);
#endif
    if (nb <= 0) {
        request_nbytes = 0;
        return ERR_READ;
    }
    request_nbytes += nb;

    // buffer full?
    if (request_nbytes >= GUI_RPC_REQ_MSG_SIZE) {
        request_nbytes = 0;
        return ERR_READ;
    }
    request_msg[request_nbytes] = 0;

    if (log_flags.gui_rpc_debug) {
        msg_printf(0, MSG_INFO,
            "[gui_rpc] GUI RPC Command = '%s'\n", request_msg
        );
    }

    if (!strncmp(request_msg, "OPTIONS", 7)) {
        char buf[1024];
        snprintf(buf, sizeof(buf),
            "HTTP/1.1 200 OK\n"
            "Server: BOINC client\n"
            "Access-Control-Allow-Origin: *\n"
            "Access-Control-Allow-Methods: POST, GET, OPTIONS\n"
            "Access-Control-Allow-Headers: *\n"
            "Content-Length: 0\n"
            "Keep-Alive: timeout=2, max=100\n"
            "Connection: Keep-Alive\n"
            "Content-Type: text/plain\n\n"
        );
        send(sock, buf, (int)strlen(buf), 0);
        request_nbytes = 0;
        if (log_flags.gui_rpc_debug) {
            msg_printf(0, MSG_INFO,
                "[gui_rpc] processed OPTIONS"
            );
        }
        return 0;
    }
    if (is_http_get_request(request_msg)) {
        handle_get();
        return 1;
    }
    bool http_request;
    if (is_http_post_request(request_msg)) {
        http_request = true;
        if (authenticated_request(request_msg)) {
            got_auth1 = got_auth2 = true;
            auth_needed = false;
        }
        strip_http_header(request_msg);
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
    } else if (match_req(request_msg, "get_auth_id")) {
        handle_get_auth_id(mfout);
    } else if (auth_needed && !is_local) {
        auth_failure(mfout);
        if (sent_unauthorized) {
            retval = ERR_AUTHENTICATOR;
        }
        sent_unauthorized = true;
    } else {
        retval = handle_rpc_aux(*this);
    }

#define XML_HEADER "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"

    mfout.printf("</boinc_gui_rpc_reply>\n");
    if (!http_request) {
        mfout.printf("\003");   // delimiter for non-HTTP replies
    }
    int n;
    mout.get_buf(p, n);
    if (http_request) {
        char buf[1024];
        snprintf(buf, sizeof(buf),
            "HTTP/1.1 200 OK\n"
            "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
            "Server: BOINC client\n"
            "Access-Control-Allow-Origin: *\n"
            "Access-Control-Allow-Methods: POST, GET, OPTIONS\n"
            "Access-Control-Allow-Headers: *\n"
            "Connection: close\n"
            "Content-Type: text/xml; charset=utf-8\n"
            "Content-Length: %d\n\n"
            XML_HEADER,
            n+(int)strlen(XML_HEADER)
        );
        send(sock, buf, (int)strlen(buf), 0);
    }
    if (p) {
        send(sock, p, n, 0);
        if (log_flags.gui_rpc_debug) {
            if (!http_request) {
                p[n-1]=0;   // replace 003 with NULL
            }
            if (n > 128) p[128] = 0;
            msg_printf(0, MSG_INFO,
                "[gui_rpc] GUI RPC reply: '%s'\n", p
            );
        }
        free(p);
    }
    return retval;
}
