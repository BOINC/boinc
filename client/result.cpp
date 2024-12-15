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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <math.h>
#endif

#include "str_replace.h"
#include "parse.h"

#include "client_msgs.h"
#include "client_state.h"
#include "log_flags.h"

#include "result.h"

int RESULT::parse_name(XML_PARSER& xp, const char* end_tag) {
    safe_strcpy(name, "");
    while (!xp.get_tag()) {
        if (xp.match_tag(end_tag)) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] RESULT::parse_name(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

void RESULT::clear() {
    safe_strcpy(name, "");
    safe_strcpy(wu_name, "");
    received_time = 0;
    report_deadline = 0;
    version_num = 0;
    safe_strcpy(plan_class, "");
    safe_strcpy(platform, "");
    avp = NULL;
    output_files.clear();
    ready_to_report = false;
    completed_time = 0;
    got_server_ack = false;
    final_cpu_time = 0;
    final_elapsed_time = 0;
    final_peak_working_set_size = 0;
    final_peak_swap_size = 0;
    final_peak_disk_usage = 0;
    final_bytes_sent = 0;
    final_bytes_received = 0;
#ifdef SIM
    peak_flop_count = 0;
#endif
    fpops_per_cpu_sec = 0;
    fpops_cumulative = 0;
    intops_per_cpu_sec = 0;
    intops_cumulative = 0;
    _state = RESULT_NEW;
    exit_status = 0;
    stderr_out.clear();
    suspended_via_gui = false;
    report_immediately = false;
    not_started = false;
    name_md5.clear();
    index = 0;
    app = NULL;
    wup = NULL;
    project = NULL;
    rrsim_flops_left = 0;
    rrsim_finish_delay = 0;
    rrsim_flops = 0;
    rrsim_done = false;
    already_selected = false;
    rr_sim_misses_deadline = false;
    unfinished_time_slice = false;
    seqno = 0;
    edf_scheduled = false;
    safe_strcpy(resources, "");
    schedule_backoff = 0;
    safe_strcpy(schedule_backoff_reason, "");
}

// parse a <result> element from scheduling server.
//
int RESULT::parse_server(XML_PARSER& xp) {
    FILE_REF file_ref;
    int retval;

    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/result")) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("wu_name", wu_name, sizeof(wu_name))) continue;
        if (xp.parse_double("report_deadline", report_deadline)) continue;
        if (xp.parse_str("platform", platform, sizeof(platform))) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.match_tag("file_ref")) {
            retval = file_ref.parse(xp);
            if (retval) {
                msg_printf(0, MSG_INFO,
                    "can't parse file_ref in result: %s",
                    boincerror(retval)
                );
                return retval;
            }
            output_files.push_back(file_ref);
            continue;
        }
        if (xp.parse_bool("report_immediately", report_immediately)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] RESULT::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

// parse a <result> element from state file
//
int RESULT::parse_state(XML_PARSER& xp) {
    FILE_REF file_ref;
    int retval;

    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/result")) {
            // set state to something reasonable in case of bad state file
            //
            if (got_server_ack || ready_to_report) {
                switch (state()) {
                case RESULT_NEW:
                case RESULT_FILES_DOWNLOADING:
                case RESULT_FILES_DOWNLOADED:
                case RESULT_FILES_UPLOADING:
                    set_state(RESULT_FILES_UPLOADED, "RESULT::parse_state");
                    break;
                }
            }
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("wu_name", wu_name, sizeof(wu_name))) continue;
        if (xp.parse_double("received_time", received_time)) continue;
        if (xp.parse_double("report_deadline", report_deadline)) {
            continue;
        }
        if (xp.match_tag("file_ref")) {
            retval = file_ref.parse(xp);
            if (retval) {
                msg_printf(0, MSG_INFO,
                    "can't parse file_ref in result: %s",
                    boincerror(retval)
                );
                return retval;
            }
#ifndef SIM
            output_files.push_back(file_ref);
#endif
            continue;
        }
        if (xp.parse_double("final_cpu_time", final_cpu_time)) continue;
        if (xp.parse_double("final_elapsed_time", final_elapsed_time)) continue;
        if (xp.parse_double("final_peak_working_set_size", final_peak_working_set_size)) continue;
        if (xp.parse_double("final_peak_swap_size", final_peak_swap_size)) continue;
        if (xp.parse_double("final_peak_disk_usage", final_peak_disk_usage)) continue;
        if (xp.parse_double("final_bytes_sent", final_bytes_sent)) continue;
        if (xp.parse_double("final_bytes_received", final_bytes_received)) continue;
        if (xp.parse_int("exit_status", exit_status)) continue;
        if (xp.parse_bool("got_server_ack", got_server_ack)) continue;
        if (xp.parse_bool("ready_to_report", ready_to_report)) continue;
        if (xp.parse_double("completed_time", completed_time)) continue;
        if (xp.parse_bool("suspended_via_gui", suspended_via_gui)) continue;
        if (xp.parse_bool("report_immediately", report_immediately)) continue;
        if (xp.parse_int("state", _state)) continue;
        if (xp.parse_string("stderr_out", stderr_out)) continue;
        if (xp.parse_double("fpops_per_cpu_sec", fpops_per_cpu_sec)) continue;
        if (xp.parse_double("fpops_cumulative", fpops_cumulative)) continue;
        if (xp.parse_double("intops_per_cpu_sec", intops_per_cpu_sec)) continue;
        if (xp.parse_double("intops_cumulative", intops_cumulative)) continue;
        if (xp.parse_str("platform", platform, sizeof(platform))) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] RESULT::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

// write result descriptor to state file, GUI RPC reply, or sched request
//
int RESULT::write(MIOFILE& out, bool to_server) {
    unsigned int i;
    FILE_INFO* fip;
    int n, retval;

    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <final_elapsed_time>%f</final_elapsed_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n"
        "    <platform>%s</platform>\n"
        "    <version_num>%d</version_num>\n",
        name,
        final_cpu_time,
        final_elapsed_time,
        exit_status,
        state(),
        platform,
        version_num
    );
    if (strlen(plan_class)) {
        out.printf("    <plan_class>%s</plan_class>\n", plan_class);
    }
    if (fpops_per_cpu_sec) {
        out.printf("    <fpops_per_cpu_sec>%f</fpops_per_cpu_sec>\n", fpops_per_cpu_sec);
    }
    if (fpops_cumulative) {
        out.printf("    <fpops_cumulative>%f</fpops_cumulative>\n", fpops_cumulative);
    }
    if (intops_per_cpu_sec) {
        out.printf("    <intops_per_cpu_sec>%f</intops_per_cpu_sec>\n", intops_per_cpu_sec);
    }
    if (intops_cumulative) {
        out.printf("    <intops_cumulative>%f</intops_cumulative>\n", intops_cumulative);
    }
    if (final_peak_working_set_size) {
        out.printf(
            "    <final_peak_working_set_size>%.0f</final_peak_working_set_size>\n",
            final_peak_working_set_size
        );
    }
    if (final_peak_swap_size) {
        out.printf(
            "    <final_peak_swap_size>%.0f</final_peak_swap_size>\n",
            final_peak_swap_size
        );
    }
    if (final_peak_disk_usage) {
        out.printf(
            "    <final_peak_disk_usage>%.0f</final_peak_disk_usage>\n",
            final_peak_disk_usage
        );
    }
    if (final_bytes_sent) {
        out.printf(
            "    <final_bytes_sent>%.0f</final_bytes_sent>\n",
            final_bytes_sent
        );
    }
    if (final_bytes_received) {
        out.printf(
            "    <final_bytes_received>%.0f</final_bytes_received>\n",
            final_bytes_received
        );
    }
    if (to_server) {
        out.printf(
            "    <app_version_num>%d</app_version_num>\n",
            wup->version_num
        );
    }
    n = (int)stderr_out.length();
    if (n || to_server) {
        out.printf("<stderr_out>\n");

        // the following is here so that it gets recorded on server
        // (there's no core_client_version field of result table)
        //
        if (to_server) {
            out.printf(
                "<core_client_version>%d.%d.%d</core_client_version>\n",
                gstate.core_client_version.major,
                gstate.core_client_version.minor,
                gstate.core_client_version.release
            );
        }
        if (n) {
            out.printf("<![CDATA[\n");
            out.printf("%s",stderr_out.c_str());
            if (stderr_out[n-1] != '\n') {
                out.printf("\n");
            }
            out.printf("]]>\n");
        }
        out.printf("</stderr_out>\n");
    }
    if (to_server) {
        for (i=0; i<output_files.size(); i++) {
            fip = output_files[i].file_info;
            if (fip->uploaded) {
                retval = fip->write(out, true);
                if (retval) return retval;
            }
        }
    } else {
        if (got_server_ack) out.printf("    <got_server_ack/>\n");
        if (ready_to_report) out.printf("    <ready_to_report/>\n");
        if (completed_time) out.printf("    <completed_time>%f</completed_time>\n", completed_time);
        if (suspended_via_gui) out.printf("    <suspended_via_gui/>\n");
        if (report_immediately) out.printf("    <report_immediately/>\n");
        out.printf(
            "    <wu_name>%s</wu_name>\n"
            "    <report_deadline>%f</report_deadline>\n"
            "    <received_time>%f</received_time>\n",
            wu_name,
            report_deadline,
            received_time
        );
        for (i=0; i<output_files.size(); i++) {
            retval = output_files[i].write(out);
            if (retval) return retval;
        }
    }
    out.printf("</result>\n");
    return 0;
}

#ifndef SIM

static const char* cpu_string(double ncpus) {
    return (ncpus==1)?"CPU":"CPUs";
}

int RESULT::write_gui(MIOFILE& out, bool check_resources) {
    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <wu_name>%s</wu_name>\n"
        "    <platform>%s</platform>\n"
        "    <version_num>%d</version_num>\n"
        "    <plan_class>%s</plan_class>\n"
        "    <project_url>%s</project_url>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <final_elapsed_time>%f</final_elapsed_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n"
        "    <report_deadline>%f</report_deadline>\n"
        "    <received_time>%f</received_time>\n"
        "    <estimated_cpu_time_remaining>%f</estimated_cpu_time_remaining>\n",
        name,
        wu_name,
        platform,
        version_num,
        plan_class,
        project->master_url,
        final_cpu_time,
        final_elapsed_time,
        exit_status,
        state(),
        report_deadline,
        received_time,
        estimated_runtime_remaining()
    );
    if (got_server_ack) out.printf("    <got_server_ack/>\n");
    if (ready_to_report) out.printf("    <ready_to_report/>\n");
    if (completed_time) out.printf("    <completed_time>%f</completed_time>\n", completed_time);
    if (suspended_via_gui) out.printf("    <suspended_via_gui/>\n");
    if (project->suspended_via_gui) out.printf("    <project_suspended_via_gui/>\n");
    if (report_immediately) out.printf("    <report_immediately/>\n");
    if (edf_scheduled) out.printf("    <edf_scheduled/>\n");
    if (resource_usage.missing_coproc) out.printf("    <coproc_missing/>\n");
    if (schedule_backoff > gstate.now) {
        out.printf("    <scheduler_wait/>\n");
        if (strlen(schedule_backoff_reason)) {
            out.printf(
                "    <scheduler_wait_reason>%s</scheduler_wait_reason>\n",
                schedule_backoff_reason
            );
        }
    }
    if (avp->needs_network && gstate.network_suspended) out.printf("    <network_wait/>\n");
    ACTIVE_TASK* atp = gstate.active_tasks.lookup_result(this);
    if (atp) {
        atp->write_gui(out);
    }
    if (!strlen(resources) || check_resources) { // update resource string only when zero or when app_config is updated.
        if (resource_usage.rsc_type) {
            if (resource_usage.coproc_usage == 1) {
                snprintf(resources, sizeof(resources),
                    "%.3g %s + 1 %s",
                    resource_usage.avg_ncpus,
                    cpu_string(resource_usage.avg_ncpus),
                    rsc_name_long(resource_usage.rsc_type)
                );
            } else {
                snprintf(resources, sizeof(resources),
                    "%.3g %s + %.3g %ss",
                    resource_usage.avg_ncpus,
                    cpu_string(resource_usage.avg_ncpus),
                    resource_usage.coproc_usage,
                    rsc_name_long(resource_usage.rsc_type)
                );
            }
        } else if (resource_usage.missing_coproc) {
            snprintf(resources, sizeof(resources),
                "%.3g %s + %.12s GPU (missing)",
                resource_usage.avg_ncpus,
                cpu_string(resource_usage.avg_ncpus),
                resource_usage.missing_coproc_name
            );
        } else if (!project->non_cpu_intensive && (resource_usage.avg_ncpus != 1)) {
            snprintf(resources, sizeof(resources),
                "%.3g %s",
                resource_usage.avg_ncpus,
                cpu_string(resource_usage.avg_ncpus)
            );
        } else {
            safe_strcpy(resources, " ");
        }
    }
    // update app version resources
    if (strlen(resources)>1) {
        char buf[256];
        safe_strcpy(buf, "");
        if (atp && atp->scheduler_state == CPU_SCHED_SCHEDULED) {
            if (resource_usage.rsc_type) {
                COPROC& cp = coprocs.coprocs[resource_usage.rsc_type];
                if (cp.count > 1) {
                    // if there are multiple GPUs of this type,
                    // show the user which one(s) are being used
                    //
                    int n = (int)ceil(resource_usage.coproc_usage);
                    safe_strcpy(buf, n>1?" (devices ":" (device ");
                    for (int i=0; i<n; i++) {
                        char buf2[256];
                        snprintf(buf2, sizeof(buf2), "%d", cp.device_nums[coproc_indices[i]]);
                        if (i > 0) {
                            safe_strcat(buf, ", ");
                        }
                        safe_strcat(buf, buf2);
                    }
                    safe_strcat(buf, ")");
                }
            }
        }
        out.printf(
            "    <resources>%s%s</resources>\n", resources, buf
        );
    }
    out.printf("</result>\n");
    return 0;
}

#endif

// Returns true if the result's output files are all either
// successfully uploaded or have unrecoverable errors
//
bool RESULT::is_upload_done() {
    unsigned int i;
    FILE_INFO* fip;
    int retval;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->uploadable()) {
            if (fip->had_failure(retval)) continue;
            if (!fip->uploaded) {
                return false;
            }
        }
    }
    return true;
}

// resets all FILE_INFO's in result to uploaded = false
//
void RESULT::clear_uploaded_flags() {
    unsigned int i;
    FILE_INFO* fip;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        fip->uploaded = false;
    }
}

bool RESULT::is_not_started() {
    if (computing_done()) return false;
    if (gstate.active_tasks.lookup_result(this)) return false;
    return true;
}

// return true if some file needed by this result (input or application)
// is downloading and backed off
//
bool RESULT::some_download_stalled() {
#ifndef SIM
    unsigned int i;
    FILE_INFO* fip;
    PERS_FILE_XFER* pfx;
    bool some_file_missing = false;

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) some_file_missing = true;
        pfx = fip->pers_file_xfer;
        if (pfx && pfx->next_request_time > gstate.now) {
            return true;
        }
    }
    for (i=0; i<avp->app_files.size(); i++) {
        fip = avp->app_files[i].file_info;
        if (fip->status != FILE_PRESENT) some_file_missing = true;
        pfx = fip->pers_file_xfer;
        if (pfx && pfx->next_request_time > gstate.now) {
            return true;
        }
    }

    if (some_file_missing && !project->download_backoff.ok_to_transfer()) {
        return true;
    }
#endif
    return false;
}

FILE_REF* RESULT::lookup_file(FILE_INFO* fip) {
    for (unsigned int i=0; i<output_files.size(); i++) {
        FILE_REF& fr = output_files[i];
        if (fr.file_info == fip) return &fr;
    }
    return 0;
}

FILE_INFO* RESULT::lookup_file_logical(const char* lname) {
    for (unsigned int i=0; i<output_files.size(); i++) {
        FILE_REF& fr = output_files[i];
        if (!strcmp(lname, fr.open_name)) {
            return fr.file_info;
        }
    }
    return 0;
}

void RESULT::append_log_record() {
    char filename[256];
    job_log_filename(*project, filename, sizeof(filename));
    FILE* f = fopen(filename, "ab");
    if (!f) return;
    fprintf(f, "%.0f ue %f ct %f fe %.0f nm %s et %f es %d\n",
        gstate.now, estimated_runtime_uncorrected(), final_cpu_time,
        wup->rsc_fpops_est, name, final_elapsed_time,
        exit_status
    );
    fclose(f);
}

// abort a result that's not currently running
//
void RESULT::abort_inactive(int status) {
    if (state() >= RESULT_COMPUTE_ERROR) return;
    set_state(RESULT_ABORTED, "RESULT::abort_inactive");
    exit_status = status;
}

// whether this task can be run right now
//
bool RESULT::runnable() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (state() != RESULT_FILES_DOWNLOADED) return false;
    if (resource_usage.missing_coproc) return false;
    if (schedule_backoff > gstate.now) return false;
    if (avp->needs_network && gstate.file_xfers_suspended) {
        // check file_xfers_suspended rather than network_suspended;
        // the latter remains false for a period after GUI RPCs
        //
        return false;
    }
    return true;
}

// whether this task should be included in RR simulation
// Like runnable, except downloading backoff is OK
// Schedule-backoff is not OK;
// we should be able to get GPU jobs from project A
// even if project B has backed-off jobs.
//
bool RESULT::nearly_runnable() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    switch (state()) {
    case RESULT_FILES_DOWNLOADED:
    case RESULT_FILES_DOWNLOADING:
        break;
    default:
        return false;
    }
    if (resource_usage.missing_coproc) return false;
    if (schedule_backoff > gstate.now) return false;
    return true;
}

// Return true if the result is waiting for its files to download,
// and nothing prevents this from happening soon
//
bool RESULT::downloading() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (state() > RESULT_FILES_DOWNLOADING) return false;
    if (some_download_stalled()) return false;
    return true;
}

double RESULT::estimated_runtime_uncorrected() {
    return wup->rsc_fpops_est/resource_usage.flops;
}

// estimate how long a result will take on this host
//
double RESULT::estimated_runtime() {
    double x = estimated_runtime_uncorrected();
    if (!project->dont_use_dcf) {
        x *= project->duration_correction_factor;
    }
    return x;
}

double RESULT::estimated_runtime_remaining() {
    if (computing_done()) return 0;
    ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(this);
    if (non_cpu_intensive()) {
        // the following is questionable
        if (atp && atp->fraction_done>0) {
            double est_dur = atp->fraction_done_elapsed_time / atp->fraction_done;
            double x = est_dur - atp->elapsed_time;
            if (x <= 0) x = 1;
            return x;
        }
        return 0;
    }
    if (sporadic()) return 0;

    if (atp) {
#ifdef SIM
        return sim_flops_left/resource_usage.flops;
#else
        return atp->est_dur() - atp->elapsed_time;
#endif
    }
    return estimated_runtime();
}

// Results must be complete early enough to report before the report deadline.
// Not all hosts are connected all of the time.
//
double RESULT::computation_deadline() {
    return report_deadline - (
        gstate.work_buf_min()
            // Seconds that the host will not be connected to the Internet
        + DEADLINE_CUSHION
    );
}

static const char* result_state_name(int val) {
    switch (val) {
    case RESULT_NEW: return "NEW";
    case RESULT_FILES_DOWNLOADING: return "FILES_DOWNLOADING";
    case RESULT_FILES_DOWNLOADED: return "FILES_DOWNLOADED";
    case RESULT_COMPUTE_ERROR: return "COMPUTE_ERROR";
    case RESULT_FILES_UPLOADING: return "FILES_UPLOADING";
    case RESULT_FILES_UPLOADED: return "FILES_UPLOADED";
    case RESULT_ABORTED: return "ABORTED";
    }
    return "Unknown";
}

void RESULT::set_state(int val, const char* where) {
    _state = val;
    if (log_flags.task_debug) {
        msg_printf(project, MSG_INFO,
            "[task] result state=%s for %s from %s",
            result_state_name(val), name, where
        );
    }
}

void add_old_result(RESULT& r) {
    while (!old_results.empty()) {
        OLD_RESULT& ores = *old_results.begin();
        if (ores.create_time < gstate.now - 3600) {
            old_results.pop_front();
        } else {
            break;
        }
    }
    OLD_RESULT ores;
    safe_strcpy(ores.project_url, r.project->master_url);
    safe_strcpy(ores.result_name, r.name);
    safe_strcpy(ores.app_name, r.app->name);
    ores.elapsed_time = r.final_elapsed_time;
    ores.cpu_time = r.final_cpu_time;
    ores.completed_time = r.completed_time;
    ores.create_time = gstate.now;
    ores.exit_status = r.exit_status;
    old_results.push_back(ores);
}

void print_old_results(MIOFILE& mf) {
    mf.printf("<old_results>\n");
    deque<OLD_RESULT>::iterator i = old_results.begin();
    while (i != old_results.end()) {
        OLD_RESULT& ores = *i;
        mf.printf(
            "    <old_result>\n"
            "         <project_url>%s</project_url>\n"
            "         <result_name>%s</result_name>\n"
            "         <app_name>%s</app_name>\n"
            "         <exit_status>%d</exit_status>\n"
            "         <elapsed_time>%f</elapsed_time>\n"
            "         <cpu_time>%f</cpu_time>\n"
            "         <completed_time>%f</completed_time>\n"
            "         <create_time>%f</create_time>\n"
            "    </old_result>\n",
            ores.project_url,
            ores.result_name,
            ores.app_name,
            ores.exit_status,
            ores.elapsed_time,
            ores.cpu_time,
            ores.completed_time,
            ores.create_time
        );
        ++i;
    }
    mf.printf("</old_results>\n");
}

std::deque<OLD_RESULT> old_results;
