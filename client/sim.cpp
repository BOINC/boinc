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

// BOINC client simulator.
//
// usage: sim options
//
//  [--input_file_prefix X]
//      Prefix of input filenames; default is blank.
//      Input files are:
//          client_state.xml
//          global_prefs.xml
//          cc_config.xml
//  [--output_file_prefix X]
//      Prefix of output filenames; default is blank.
//      Output files are:
//          timeline.html
//          log.txt
//          summary.xml
//          debt.dat
//          debt_overall.png
//          debt_cpu_std.png
//          debt_cpu_ltd.png
//          debt_nvidia_std.png
//          debt_nvidia_ltd.png
//          ...
//
//  Simulation params:
//  [--duration x]
//      simulation duration (default 86400)
//  [--delta x]
//      delta = simulation time step (default 10)
//
//  Policy options:
//  [--server_uses_workload]
//      simulate use of EDF sim by scheduler
//  [--cpu_sched_rr_only]
//      use only RR scheduling

#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "log_flags.h"
#include "filesys.h"
#include "client_msgs.h"
#include "client_state.h"
#include "../sched/edf_sim.h"
#include "sim.h"

#define SCHED_RETRY_DELAY_MIN    60                // 1 minute
#define SCHED_RETRY_DELAY_MAX    (60*60*4)         // 4 hours

const char* input_file_prefix = "";
const char* output_file_prefix = "";

#define TIMELINE_FNAME "timeline.html"
#define LOG_FNAME "log.txt"
#define SUMMARY_FNAME "summary.xml"
#define DEBT_FNAME "debt.dat"

bool user_active;
double duration = 86400, delta = 60;
FILE* logfile;
FILE* html_out;
FILE* debt_file;

string html_msg;
bool running;
double running_time = 0;
bool server_uses_workload = false;
bool cpu_sched_rr_only;

SIM_RESULTS sim_results;

void usage(char* prog) {
    fprintf(stderr, "usage: %s\n"
        "[--input_file_prefix F]\n"
        "[--output_file_prefix F]\n"
        "[--duration X]\n"
        "[--delta X]\n"
        "[--server_uses_workload]\n"
        "[--cpu_sched_rr_only]\n",
        prog
    );
    exit(1);
}

void print_project_results(FILE* f) {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->print_results(f, sim_results);
    }
}

APP* choose_app(vector<APP*>& apps) {
    double x = drand();
    double sum = 0;
    unsigned int i;

    for (i=0; i<apps.size(); i++) {
        sum += apps[i]->weight;
    }
    for (i=0; i<apps.size(); i++) {
        APP* app = apps[i];
        x -= app->weight/sum;
        if (x <= 0) {
            return app;
        }
    }
    return apps.back();
}

bool app_version_needs_work(APP_VERSION* avp) {
    if (avp->ncudas) {
        return (cuda_work_fetch.req_secs>0 || cuda_work_fetch.req_instances>0);
    }
    if (avp->natis) {
        return (ati_work_fetch.req_secs>0 || ati_work_fetch.req_instances>0);
    }
    return (cpu_work_fetch.req_secs>0 || cpu_work_fetch.req_instances>0);
}

bool has_app_version_needing_work(APP* app) {
    for (unsigned int i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->app != app) continue;
        if (app_version_needs_work(avp)) return true;
    }
    return false;
}

// choose a version for this app for which we need work
//
APP_VERSION* choose_app_version(APP* app) {
    APP_VERSION* best_avp = NULL;
    for (unsigned int i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->app != app) continue;
        if (!app_version_needs_work(avp)) continue;
        if (!best_avp) {
            best_avp = avp;
        } else if (avp->flops > best_avp->flops) {
            best_avp = avp;
        }
    }
    return best_avp;
}

// generate a job; pick a random app for this project,
// and pick a FLOP count from its distribution
//
void make_job(
    PROJECT* p, WORKUNIT* wup, RESULT* rp, vector<APP*>app_list
) {
    APP* app = choose_app(app_list);
    APP_VERSION* avp = choose_app_version(app);
    rp->clear();
    rp->avp = avp;
    rp->app = app;
    if (!rp->avp) {
        printf("ERROR - NO APP VERSION\n");
        exit(1);
    }
    rp->project = p;
    rp->wup = wup;
    sprintf(rp->name, "%s_%d", p->project_name, p->result_index++);
    wup->project = p;
    wup->rsc_fpops_est = app->fpops_est;
    strcpy(wup->name, rp->name);
    strcpy(wup->app_name, app->name);
    wup->app = app;
    double ops = app->fpops.sample();
    if (ops < 0) ops = 0;
    rp->final_cpu_time = ops/avp->flops;
    rp->report_deadline = gstate.now + app->latency_bound;
}

// process ready-to-report results
//
void CLIENT_STATE::handle_completed_results(PROJECT* p) {
    char buf[256];
    vector<RESULT*>::iterator result_iter;

    result_iter = results.begin();
    while (result_iter != results.end()) {
        RESULT* rp = *result_iter;
        if (rp->project == p && rp->ready_to_report) {
            sprintf(buf, "result %s reported; %s<br>",
                rp->name,
                (gstate.now > rp->report_deadline)?
                "<font color=#cc0000>MISSED DEADLINE</font>":
                "<font color=#00cc00>MADE DEADLINE</font>"
            );
            PROJECT* spp = rp->project;
            if (gstate.now > rp->report_deadline) {
                sim_results.cpu_wasted += rp->final_cpu_time;
                sim_results.nresults_missed_deadline++;
                spp->project_results.nresults_missed_deadline++;
                spp->project_results.cpu_wasted += rp->final_cpu_time;
            } else {
                sim_results.cpu_used += rp->final_cpu_time;
                sim_results.nresults_met_deadline++;
                spp->project_results.nresults_met_deadline++;
                spp->project_results.cpu_used += rp->final_cpu_time;
            }
            html_msg += buf;
            delete rp;
            result_iter = results.erase(result_iter);
        } else {
            result_iter++;
        }
    }
}

// convert results in progress to IP_RESULTs,
// and get an initial schedule for them
//
void CLIENT_STATE::get_workload(vector<IP_RESULT>& ip_results) {
    for (unsigned int i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        double x = rp->estimated_time_remaining(false);
        if (x == 0) continue;
        IP_RESULT ipr(rp->name, rp->report_deadline, x);
        ip_results.push_back(ipr);
    }
    init_ip_results(work_buf_min(), ncpus, ip_results);
}

void get_apps_needing_work(PROJECT* p, vector<APP*>& apps) {
    apps.clear();
    for (unsigned int i=0; i<gstate.apps.size(); i++) {
        APP* app = gstate.apps[i];
        if (app->project != p) continue;
        if (!has_app_version_needing_work(app)) continue;
        apps.push_back(app);
    }
}

void decrement_request_rsc(
    RSC_WORK_FETCH& rwf, double ninstances, double est_runtime
) {
    rwf.req_secs -= est_runtime * ninstances;
    rwf.req_instances -= ninstances;
}

void decrement_request(RESULT* rp) {
    APP_VERSION* avp = rp->avp;
    double est_runtime = rp->wup->rsc_fpops_est/avp->flops;
    est_runtime /= (gstate.time_stats.on_frac*gstate.time_stats.active_frac);
    decrement_request_rsc(cpu_work_fetch, avp->avg_ncpus, est_runtime);
    decrement_request_rsc(cuda_work_fetch, avp->ncudas, est_runtime);
    decrement_request_rsc(ati_work_fetch, avp->natis, est_runtime);
}

// simulate trying to do an RPC;
// return true if we actually did one
//
bool CLIENT_STATE::simulate_rpc(PROJECT* p) {
    char buf[256], buf2[256];
    static double last_time=0;
    vector<IP_RESULT> ip_results;
    int infeasible_count = 0;
    vector<RESULT*> new_results;

    double diff = now - last_time;
    if (diff && diff < connection_interval) {
        msg_printf(NULL, MSG_INFO,
            "simulate_rpc: too soon %f < %f",
            diff, connection_interval
        );
        return false;
    }
    last_time = now;

    // save request params for WORK_FETCH::handle_reply
    double save_cpu_req_secs = cpu_work_fetch.req_secs;
    host_info.coprocs.cuda.req_secs = cuda_work_fetch.req_secs;
    host_info.coprocs.ati.req_secs = ati_work_fetch.req_secs;


    work_fetch.request_string(buf2);
    sprintf(buf, "RPC to %s: %s<br>", p->project_name, buf2);
    html_msg += buf;

    msg_printf(0, MSG_INFO, buf);

    handle_completed_results(p);

    if (server_uses_workload) {
        get_workload(ip_results);
    }

    bool sent_something = false;
    while (1) {
        vector<APP*> apps;
        get_apps_needing_work(p, apps);
        if (apps.empty()) break;
        RESULT* rp = new RESULT;
        WORKUNIT* wup = new WORKUNIT;
        make_job(p, wup, rp, apps);

        if (server_uses_workload) {
            IP_RESULT c(rp->name, rp->report_deadline, rp->final_cpu_time);
            if (check_candidate(c, ncpus, ip_results)) {
                ip_results.push_back(c);
            } else {
                delete rp;
                delete wup;
                if (++infeasible_count > p->max_infeasible_count) {
                    p->min_rpc_time = now + 1;
                    break;
                }
            }
        }

        sent_something = true;
        rp->set_state(RESULT_FILES_DOWNLOADED, "simulate_rpc");
        results.push_back(rp);
        new_results.push_back(rp);
#if 0
        sprintf(buf, "got job %s: CPU time %.2f, deadline %s<br>",
            rp->name, rp->final_cpu_time, time_to_string(rp->report_deadline)
        );
        html_msg += buf;
#endif
        decrement_request(rp);
    }

    msg_printf(0, MSG_INFO, "Got %d tasks", new_results.size());
    sprintf(buf, "got %d tasks<br>", new_results.size());
    html_msg += buf;

    SCHEDULER_REPLY sr;
    cpu_work_fetch.req_secs = save_cpu_req_secs;
    work_fetch.handle_reply(p, &sr, new_results);
    p->nrpc_failures = 0;
    if (sent_something) {
        request_schedule_cpus("simulate_rpc");
        request_work_fetch("simulate_rpc");
    }
    return true;
}

SCHEDULER_REPLY::SCHEDULER_REPLY() {
    cpu_backoff = 0;
    cuda_backoff = 0;
    ati_backoff = 0;
}
SCHEDULER_REPLY::~SCHEDULER_REPLY() {}

void PROJECT::backoff() {
    nrpc_failures++;
    double backoff = calculate_exponential_backoff(
        nrpc_failures, SCHED_RETRY_DELAY_MIN, SCHED_RETRY_DELAY_MAX
    );
    min_rpc_time = gstate.now + backoff;
}

bool CLIENT_STATE::scheduler_rpc_poll() {
    PROJECT *p;
    bool action = false;
    static double last_time=0;
    static double last_work_fetch_time = 0;
    double elapsed_time;

    // check only every 5 sec
    //
    if (now - last_time < SCHEDULER_RPC_POLL_PERIOD) {
        msg_printf(NULL, MSG_INFO, "RPC poll: not time %f - %f < %f",
            now, last_time, SCHEDULER_RPC_POLL_PERIOD
        );
        return false;
    }
    last_time = now;

    msg_printf(NULL, MSG_INFO, "RPC poll start");
    while (1) {
        p = next_project_sched_rpc_pending();
        if (p) {
            work_fetch.compute_work_request(p);
            action = simulate_rpc(p);
            break;
        }
    
        p = find_project_with_overdue_results();
        if (p) {
            work_fetch.compute_work_request(p);
            action = simulate_rpc(p);
            break;
        }

        // should we check work fetch?  Do this at most once/minute

        if (must_check_work_fetch) {
            last_work_fetch_time = 0;
        }
        elapsed_time = now - last_work_fetch_time;
        if (elapsed_time < WORK_FETCH_PERIOD) {
            return false;
        }
        must_check_work_fetch = false;
        last_work_fetch_time = now;

        p = work_fetch.choose_project();
        if (p) {
            action = simulate_rpc(p);
            break;
        }
        break;
    }
    if (action) {
        msg_printf(p, MSG_INFO, "RPC poll: did an RPC");
    } else {
        msg_printf(0, MSG_INFO, "RPC poll: didn't do an RPC");
    }
    return action;
}

bool ACTIVE_TASK_SET::poll() {
    unsigned int i;
    char buf[256];
    bool action = false;
    static double last_time = 0;
    double diff = gstate.now - last_time;
    if (diff < 1.0) return false;
    last_time = gstate.now;
    PROJECT* p;

    if (!running) return false;

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        p->idle = true;
#if 0
        sprintf(buf, "%s STD: %f LTD %f<br>",
            p->project_name, p->cpu_pwf.short_term_debt,
            p->pwf.overall_debt
        );
        html_msg += buf;
#endif
    }

    double x=0;
    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        switch (atp->task_state()) {
        case PROCESS_EXECUTING:
            atp->cpu_time_left -= diff;
            atp->current_cpu_time += diff;
            RESULT* rp = atp->result;

            double cpu_time_used = rp->final_cpu_time - atp->cpu_time_left;
            atp->fraction_done = cpu_time_used/rp->final_cpu_time;
            atp->checkpoint_wall_time = gstate.now;

            if (atp->cpu_time_left <= 0) {
                atp->set_task_state(PROCESS_EXITED, "poll");
                rp->exit_status = 0;
                rp->ready_to_report = true;
                gstate.request_schedule_cpus("ATP poll");
                gstate.request_work_fetch("ATP poll");
                sprintf(buf, "result %s finished<br>", rp->name);
                html_msg += buf;
                action = true;
            }
            rp->project->idle = false;
            x += rp->avp->avg_ncpus;
        }
    }
    if (x < gstate.ncpus) {
        sim_results.cpu_idle += diff*(gstate.ncpus-x);
    }

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->idle) {
            p->idle_time += diff;
            p->idle_time_sumsq += diff*(p->idle_time*p->idle_time);
        } else {
            p->idle_time = 0;
        }
    }
    running_time += diff;

    return action;
}

// return the fraction of CPU time that was spent in violation of shares
// i.e., if a project got X and it should have got Y,
// add up |X-Y| over all projects, and divide by total CPU
//
double CLIENT_STATE::share_violation() {
    unsigned int i;

    double tot = 0, trs=0;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        tot += p->project_results.cpu_used + p->project_results.cpu_wasted;
        trs += p->resource_share;
    }
    double sum = 0;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        double t = p->project_results.cpu_used + p->project_results.cpu_wasted;
        double rs = p->resource_share/trs;
        double rt = tot*rs;
        sum += fabs(t - rt);
    }
    return sum/tot;

}

// "monotony" is defined as follows:
// for each project P, maintain R(P), the time since P last ran,
// let S(P) be the RMS of R(P).
// Let X = mean(S(P))/(sched_interval*nprojects)
//  (the *nprojects reflects the fact that in the limit of nprojects,
//   each one waits for a time to run proportional to nprojects)
//  X varies from zero (no monotony) to infinity.
//   X is one in the case of round-robin on 1 CPU.
// Let monotony = 1-1/(x+1)
//
double CLIENT_STATE::monotony() {
    double sum = 0;
    double schedint = global_prefs.cpu_scheduling_period();
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        double avg_ss = p->idle_time_sumsq/running_time;
        double s = sqrt(avg_ss);
        sum += s;
    }
    int n = (int)projects.size();
    double x = sum/(n*schedint*n);
    double m = 1-(1/(x+1));
    //printf("sum: %f; x: %f m: %f\n", sum, x, m);
    return m;
}

// the CPU totals are there; compute the other fields
//
void SIM_RESULTS::compute() {
    double total = cpu_used + cpu_wasted + cpu_idle;
    cpu_wasted_frac = cpu_wasted/total;
    cpu_idle_frac = cpu_idle/total;
    share_violation = gstate.share_violation();
    monotony = gstate.monotony();
}

// top-level results (for aggregating multiple simulations)
//
void SIM_RESULTS::print(FILE* f, const char* title) {
    if (title) {
        fprintf(f, "%s: ", title);
    }
    fprintf(f, "wasted_frac %f idle_frac %f share_violation %f monotony %f\n",
        cpu_wasted_frac, cpu_idle_frac, share_violation, monotony
    );
}

void SIM_RESULTS::parse(FILE* f) {
    fscanf(f, "wasted_frac %lf idle_frac %lf share_violation %lf monotony %lf",
        &cpu_wasted_frac, &cpu_idle_frac, &share_violation, &monotony
    );
}

void SIM_RESULTS::add(SIM_RESULTS& r) {
    cpu_wasted_frac += r.cpu_wasted_frac;
    cpu_idle_frac += r.cpu_idle_frac;
    share_violation += r.share_violation;
    monotony += r.monotony;
}

void SIM_RESULTS::divide(int n) {
    cpu_wasted_frac /= n;
    cpu_idle_frac /= n;
    share_violation /= n;
    monotony /= n;
}

void SIM_RESULTS::clear() {
    memset(this, 0, sizeof(*this));
}

void PROJECT::print_results(FILE* f, SIM_RESULTS& sr) {
    double t = project_results.cpu_used + project_results.cpu_wasted;
    double gt = sr.cpu_used + sr.cpu_wasted;
    fprintf(f, "%s: share %.2f total CPU %2f (%.2f%%)\n"
        "   used %.2f wasted %.2f\n"
        "   met %d missed %d\n",
        project_name, resource_share,
        t, (t/gt)*100,
        project_results.cpu_used,
        project_results.cpu_wasted,
        project_results.nresults_met_deadline,
        project_results.nresults_missed_deadline
    );
}

const char* colors[] = {
    "#000088",
    "#008800",
    "#880000",
    "#880088",
    "#888800",
    "#000088",
};

int njobs_in_progress(PROJECT* p, int rsc_type) {
    int n = 0;
    unsigned int i;
    for (i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != p) continue;
        if (rp->resource_type() != rsc_type) continue;
        if (rp->state() > RESULT_FILES_DOWNLOADED) continue;
        n++;
    }
    return n;
}

void show_resource(int rsc_type) {
    unsigned int i;

    fprintf(html_out, "<td>");
    bool found = false;
    for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
        RESULT* rp = atp->result;
        if (rp->resource_type() != rsc_type) continue;
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        PROJECT* p = rp->project;
        double ninst;
        if (rsc_type == RSC_TYPE_CUDA) {
            ninst = rp->avp->ncudas;
        } else if (rsc_type == RSC_TYPE_ATI) {
            ninst = rp->avp->natis;
        } else {
            ninst = rp->avp->avg_ncpus;
        }

        fprintf(html_out, "%.2f: <font color=%s>%s%s: %.2f</font><br>",
            ninst,
            colors[p->index],
            atp->result->rr_sim_misses_deadline?"*":"",
            atp->result->name,
            atp->cpu_time_left
        );
        found = true;
    }
    if (!found) fprintf(html_out, "IDLE");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        int n = njobs_in_progress(p, rsc_type);
        fprintf(html_out, "<br>%s: %d jobs in progress\n", p->project_name, n);
    }
    fprintf(html_out, "</td>");
}

int nproc_types = 1;

void html_start() {
    char buf[256];

    sprintf(buf, "%s%s", output_file_prefix, TIMELINE_FNAME);
    html_out = fopen(buf, "w");
    if (!html_out) {
        fprintf(stderr, "can't open %s for writing\n", buf);
        exit(1);
    }
    setbuf(html_out, 0);
    fprintf(html_out, "<h2>BOINC client simulator</h2>\n");
    fprintf(html_out,
        "<table border=1><tr><th>Time</th>\n"
    );
    fprintf(html_out,
        "<th>CPU<br><font size=-2>Job name and estimated time left<br>color denotes project<br>* means EDF mode</font></th>"
    );
    if (gstate.host_info.have_cuda()) {
        fprintf(html_out, "<th>NVIDIA GPU</th>");
        nproc_types++;
    }
    if (gstate.host_info.have_ati()) {
        fprintf(html_out, "<th>ATI GPU</th>");
        nproc_types++;
    }
    fprintf(html_out, "</tr>\n");
}

void html_rec() {
    if (html_msg.size()) {
        //fprintf(html_out, "<tr><td>%s</td>", time_to_string(gstate.now));
        fprintf(html_out, "<tr><td>%f</td>", gstate.now);
        fprintf(html_out,
            "<td colspan=%d><font size=-2>%s</font></td></tr>\n",
            nproc_types,
            html_msg.c_str()
        );
        html_msg = "";
    }
    //fprintf(html_out, "<tr><td>%s</td>", time_to_string(gstate.now));
    fprintf(html_out, "<tr><td>%f</td>", gstate.now);

    if (!running) {
        fprintf(html_out, "<td bgcolor=#aaaaaa>OFF</td>");
        if (gstate.host_info.have_cuda()) {
            fprintf(html_out, "<td bgcolor=#aaaaaa>OFF</td>");
        }
        if (gstate.host_info.have_ati()) {
            fprintf(html_out, "<td bgcolor=#aaaaaa>OFF</td>");
        }
    } else {
        show_resource(RSC_TYPE_CPU);
        if (gstate.host_info.have_cuda()) {
            show_resource(RSC_TYPE_CUDA);
        }
        if (gstate.host_info.have_ati()) {
            show_resource(RSC_TYPE_ATI);
        }
    }
}

void html_end() {
    fprintf(html_out, "</table>");
    fprintf(html_out, "<pre>\n");
    sim_results.compute();
    sim_results.print(html_out);
    print_project_results(html_out);
    fprintf(html_out, "</pre>\n");
    fclose(html_out);
}

// lines in the debt file have these fields:
// time
// per project:
//      overall LTD
//      CPU LTD
//      CPU STD
//      [NVIDIA LTD]
//      [NVIDIA STD]
//      [ATI LTD]
//      [ATI STD]
//
void write_debts() {
    fprintf(debt_file, "%f ", gstate.now);
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        fprintf(debt_file, "%f %f %f ",
            p->pwf.overall_debt,
            p->cpu_pwf.long_term_debt,
            p->cpu_pwf.short_term_debt
        );
        if (gstate.host_info.have_cuda()) {
            fprintf(debt_file, "%f %f ",
                p->cuda_pwf.long_term_debt,
                p->cuda_pwf.short_term_debt
            );
        }
        if (gstate.host_info.have_ati()) {
            fprintf(debt_file, "%f %f",
                p->ati_pwf.long_term_debt,
                p->ati_pwf.short_term_debt
            );
        }
    }
    fprintf(debt_file, "\n");
}

// generate a bunch of debt graphs
//

void make_graph(const char* title, const char* fname, int field, int nfields) {
    char gp_fname[256], cmd[256], png_fname[256];

    sprintf(gp_fname, "%s%s.gp", output_file_prefix, fname);
    FILE* f = fopen(gp_fname, "w");
    fprintf(f,
        "set terminal png small size 1024, 768\n"
        "set title \"%s\"\n"
        "plot ",
        title
    );
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        fprintf(f, "\"%sdebt.dat\" using 1:%d title \"%s\" with lines%s",
            output_file_prefix, 2+field+i*nfields, p->project_name,
            (i==gstate.projects.size()-1)?"\n":", \\\n"
        );
    }
    fclose(f);
    sprintf(png_fname, "%s%s.png", output_file_prefix, fname);
    sprintf(cmd, "gnuplot < %s > %s", gp_fname, png_fname);
    system(cmd);
}

void debt_graphs() {
    int nfields = 3 + (gstate.host_info.have_cuda()?2:0) + (gstate.host_info.have_ati()?2:0);
    make_graph("Overall debt", "debt_overall", 0, nfields);
    make_graph("CPU LTD", "debt_cpu_ltd", 1, nfields);
    make_graph("CPU STD", "debt_cpu_std", 2, nfields);
    if (gstate.host_info.have_cuda()) {
        make_graph("NVIDIA LTD", "debt_nvidia_ltd", 3, nfields);
        make_graph("NVIDIA STD", "debt_nvidia_std", 4, nfields);
    }
    if (gstate.host_info.have_ati()) {
        int off = gstate.host_info.have_cuda()?2:0;
        make_graph("ATI LTD", "debt_ati_ltd", 3+off, nfields);
        make_graph("ATI STD", "debt_ati_std", 4+off, nfields);
    }
}

void simulate() {
    bool action;
    double start = START_TIME;
    gstate.now = start;
    html_start();
    msg_printf(0, MSG_INFO,
        "starting simulation. delta %f duration %f", delta, duration
    );
    while (1) {
        running = gstate.available.sample(gstate.now);
        while (1) {
            action = gstate.active_tasks.poll();
            if (running) {
                action |= gstate.handle_finished_apps();
                action |= gstate.possibly_schedule_cpus();
                action |= gstate.enforce_schedule();
                action |= gstate.scheduler_rpc_poll();
            }
            msg_printf(0, MSG_INFO, action?"did action":"did no action");
            if (!action) break;
        }
        msg_printf(0, MSG_INFO, "took time step");
        for (unsigned int i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
            ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
            if (atp->task_state() == PROCESS_EXECUTING) {
                atp->elapsed_time += delta;
            }
        }
        html_rec();
        write_debts();
        gstate.now += delta;
        if (gstate.now > start + duration) break;
    }
    html_end();
}

void parse_error(char* file, int retval) {
    printf("can't parse %s: %d\n", file, retval);
    exit(1);
}

char* next_arg(int argc, char** argv, int& i) {
    if (i >= argc) {
        fprintf(stderr, "Missing command-line argument\n");
        usage(argv[0]);
    }
    return argv[i++];
}

// get application FLOPS params.
// These can be specified manually in the <app>.
// If not they are taken from a WU if one exists for the app.
// If not they default to 1 GFLOPS/hour
//
void get_app_params() {
    unsigned int i, j;
    for (i=0; i<gstate.apps.size(); i++) {
        APP* app = gstate.apps[i];
        if (!app->fpops_est) {
            for (j=0; j<gstate.workunits.size(); j++) {
                WORKUNIT* wup = gstate.workunits[j];
                if (wup->app != app) continue;
                app->fpops_est = wup->rsc_fpops_est;
                break;
            }
            if (!app->fpops_est) {
                app->fpops_est = 3600 * 1e9;
            }
        }
        if (!app->fpops.mean) {
            app->fpops.mean = app->fpops_est;
        }
        if (!app->weight) {
            app->weight = 1;
        }
        fprintf(stderr, "app %s: fpops_est %f fpops mean %f\n",
            app->name, app->fpops_est, app->fpops.mean
        );
    }
}

// fill in APP.latency_bound
//
void assign_latency_bounds() {
    unsigned int i;
    for (i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        APP* app = rp->app;
        app->latency_bound = rp->report_deadline - rp->received_time;
    }
}

// zero backoffs and debts.
//
void clear_backoff() {
    unsigned int i;
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->cpu_pwf.reset();
        p->cuda_pwf.reset();
        p->ati_pwf.reset();
        p->min_rpc_time = 0;
    }
}

// remove apps with no app versions,
// then projects with no apps
//
void cull_projects() {
    unsigned int i;

    for (i=0; i<gstate.apps.size(); i++) {
        APP* app = gstate.apps[i];
        app->has_version = false;
    }
    for (i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        avp->app->has_version = true;
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->dont_request_more_work = true;
    }
    for (i=0; i<gstate.apps.size(); i++) {
        APP* app = gstate.apps[i];
        if (app->has_version) {
            app->project->dont_request_more_work = false;
        }
    }
}

void do_client_simulation() {
    char buf[256];
    msg_printf(0, MSG_INFO, "SIMULATION START");

    sprintf(buf, "%s%s", input_file_prefix, CONFIG_FILE);
    read_config_file(true, buf);
    config.show();

    gstate.add_platform("client simulator");
    sprintf(buf, "%s%s", input_file_prefix, STATE_FILE_NAME);
    gstate.parse_state_file_aux(buf);
    sprintf(buf, "%s%s", input_file_prefix, GLOBAL_PREFS_FILE_NAME);
    gstate.read_global_prefs(buf);
    cull_projects();
    int j=0;
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        if (!gstate.projects[i]->dont_request_more_work) {
            gstate.projects[i]->index = j++;
        }
    }

    get_app_params();
    assign_latency_bounds();
    clear_backoff();

    gstate.workunits.clear();
    gstate.results.clear();

    gstate.set_ncpus();
    work_fetch.init();
    gstate.request_work_fetch("init");
    simulate();

    sim_results.compute();

    // print machine-readable first
    sim_results.print(stdout);

    // then other
    print_project_results(stdout);

    debt_graphs();
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];

    sim_results.clear();
    for (i=1; i<argc;) {
        char* opt = argv[i++];
        if (!strcmp(opt, "--input_file_prefix")) {
            input_file_prefix = argv[i++];
        } else if (!strcmp(opt, "--output_file_prefix")) {
            output_file_prefix = argv[i++];
        } else if (!strcmp(opt, "--duration")) {
            duration = atof(next_arg(argc, argv, i));
        } else if (!strcmp(opt, "--delta")) {
            delta = atof(next_arg(argc, argv, i));
        } else if (!strcmp(opt, "--server_uses_workload")) {
            server_uses_workload = true;
        } else if (!strcmp(opt, "--cpu_sched_rr_only")) {
            cpu_sched_rr_only = true;
        } else {
            usage(argv[0]);
        }
    }

    if (duration <= 0) {
        fprintf(stderr, "duration <= 0\n");
        exit(1);
    }
    if (delta <= 0) {
        fprintf(stderr, "delta <= 0\n");
        exit(1);
    }

    sprintf(buf, "%s%s", output_file_prefix, LOG_FNAME);
    logfile = fopen(buf, "w");
    if (!logfile) {
        fprintf(stderr, "Can't open %s\n", buf);
        exit(1);
    }
    setbuf(logfile, 0);

    sprintf(buf, "%s%s", output_file_prefix, DEBT_FNAME);
    debt_file = fopen(buf, "w");

    do_client_simulation();
}
