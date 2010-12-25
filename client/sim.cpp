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
//  [--infile_prefix X]
//      Prefix of input filenames; default is blank.
//      Input files are:
//          client_state.xml
//          global_prefs.xml
//          cc_config.xml
//  [--outfile_prefix X]
//      Prefix of output filenames; default is blank.
//      Output files are:
//          index.html (lists other files)
//          timeline.html
//          log.txt
//          results.dat (simulation results in XML)
//          summary.txt (summary of inputs; detailed outputs)
//          if using REC:
//              rec.png
//          if not using REC:
//              debt.dat
//              debt_overall.png
//              debt_cpu_std.png
//              debt_cpu_ltd.png
//              debt_nvidia_std.png
//              debt_nvidia_ltd.png
//              ...
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

const char* infile_prefix = "";
const char* outfile_prefix = "";

#define TIMELINE_FNAME "timeline.html"
#define LOG_FNAME "log.txt"
#define RESULTS_FNAME "results.dat"
#define SUMMARY_FNAME "summary.txt"
#define DEBT_FNAME "debt.dat"

bool user_active;
double duration = 86400, delta = 60;
FILE* logfile;
FILE* html_out;
FILE* debt_file;
FILE* index_file;
FILE* summary_file;
char log_filename[256];

string html_msg;
double active_time = 0;
double gpu_active_time = 0;
bool server_uses_workload = false;
bool cpu_sched_rr_only;

RANDOM_PROCESS on_proc;
RANDOM_PROCESS active_proc;
RANDOM_PROCESS gpu_active_proc;
RANDOM_PROCESS connected_proc;
bool on;
bool active;
bool gpu_active;
bool connected;

SIM_RESULTS sim_results;
int njobs;

void usage(char* prog) {
    fprintf(stderr, "usage: %s\n"
        "[--infile_prefix F]\n"
        "[--outfile_prefix F]\n"
        "[--duration X]\n"
        "[--delta X]\n"
        "[--server_uses_workload]\n"
        "[--cpu_sched_rr_only]\n",
        prog
    );
    exit(1);
}

// peak flops of an app version
//
double app_peak_flops(APP_VERSION* avp, double cpu_scale) {
    double x = avp->avg_ncpus*cpu_scale;
    if (avp->ncudas) {
        x += avp->ncudas * cuda_work_fetch.relative_speed;
    }
    if (avp->natis) {
        x += avp->natis * ati_work_fetch.relative_speed;
    }
    x *= gstate.host_info.p_fpops;
    return x;
}

double gpu_peak_flops() {
    double cuda = gstate.host_info.coprocs.cuda.count * cuda_work_fetch.relative_speed * gstate.host_info.p_fpops;
    double ati = gstate.host_info.coprocs.ati.count * ati_work_fetch.relative_speed * gstate.host_info.p_fpops;
    return cuda + ati;
}

double cpu_peak_flops() {
    return gstate.ncpus * gstate.host_info.p_fpops;
}

double total_peak_flops() {
    return gpu_peak_flops() + cpu_peak_flops();
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
    if (avp->dont_use) return false;
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
        fprintf(stderr, "ERROR - NO APP VERSION\n");
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
    wup->rsc_fpops_est = ops;
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
                sim_results.flops_wasted += rp->peak_flop_count;
                sim_results.nresults_missed_deadline++;
                spp->project_results.nresults_missed_deadline++;
                spp->project_results.flops_wasted += rp->peak_flop_count;
            } else {
                sim_results.nresults_met_deadline++;
                spp->project_results.nresults_met_deadline++;
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
        double x = rp->estimated_time_remaining();
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
        if (app->ignore) continue;
        if (!has_app_version_needing_work(app)) continue;
        apps.push_back(app);
    }
}

void decrement_request_rsc(
    RSC_WORK_FETCH& rwf, double ninstances, double est_runtime
) {
    rwf.req_secs -= est_runtime * ninstances;
    rwf.req_instances -= ninstances;
    rwf.estimated_delay += est_runtime*(ninstances/rwf.ninstances);
}

void decrement_request(RESULT* rp) {
    APP_VERSION* avp = rp->avp;
    double est_runtime = rp->wup->rsc_fpops_est/avp->flops;
    est_runtime /= (gstate.time_stats.on_frac*gstate.time_stats.active_frac);
    decrement_request_rsc(cpu_work_fetch, avp->avg_ncpus, est_runtime);
    decrement_request_rsc(cuda_work_fetch, avp->ncudas, est_runtime);
    decrement_request_rsc(ati_work_fetch, avp->natis, est_runtime);
}

double get_estimated_delay(RESULT* rp) {
    if (rp->avp->ncudas) {
        return cuda_work_fetch.estimated_delay;
    } else if (rp->avp->natis) {
        return ati_work_fetch.estimated_delay;
    } else {
        return cpu_work_fetch.estimated_delay;
    }
}

// simulate trying to do an RPC;
// return true if we actually did one
//
bool CLIENT_STATE::simulate_rpc(PROJECT* p) {
    char buf[256], buf2[256];
    vector<IP_RESULT> ip_results;
    int infeasible_count = 0;
    vector<RESULT*> new_results;

    // save request params for WORK_FETCH::handle_reply
    //
    double save_cpu_req_secs = cpu_work_fetch.req_secs;
    host_info.coprocs.cuda.req_secs = cuda_work_fetch.req_secs;
    host_info.coprocs.ati.req_secs = ati_work_fetch.req_secs;

    if (!server_uses_workload) {
        cpu_work_fetch.estimated_delay = cpu_work_fetch.busy_time_estimator.get_busy_time();
        cuda_work_fetch.estimated_delay = cuda_work_fetch.busy_time_estimator.get_busy_time();
        ati_work_fetch.estimated_delay = ati_work_fetch.busy_time_estimator.get_busy_time();
    }

    for (unsigned int i=0; i<app_versions.size(); i++) {
        app_versions[i]->dont_use = false;
    }

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
                //printf("%d: %s misses deadline\n", (int)gstate.now, p->project_name);
                APP_VERSION* avp = rp->avp;
                delete rp;
                delete wup;
                avp->dont_use = true;
                continue;
            }
        } else {
            double et = wup->rsc_fpops_est / rp->avp->flops;
            if (get_estimated_delay(rp) + et > wup->app->latency_bound) {
                //printf("%d: %s misses deadline\n", (int)gstate.now, p->project_name);
                APP_VERSION* avp = rp->avp;
                delete rp;
                delete wup;
                avp->dont_use = true;
                continue;
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

    njobs += new_results.size();
    msg_printf(0, MSG_INFO, "Got %d tasks", new_results.size());
    sprintf(buf, "got %d tasks<br>", new_results.size());
    html_msg += buf;

    SCHEDULER_REPLY sr;
    cpu_work_fetch.req_secs = save_cpu_req_secs;
    work_fetch.handle_reply(p, &sr, new_results);
    p->nrpc_failures = 0;
    p->sched_rpc_pending = false;
    if (sent_something) {
        request_schedule_cpus("simulate_rpc");
        request_work_fetch("simulate_rpc");
    }
    sim_results.nrpcs++;
    return true;
}

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
#if 0
        msg_printf(NULL, MSG_INFO, "RPC poll: not time %f - %f < %f",
            now, last_time, SCHEDULER_RPC_POLL_PERIOD
        );
#endif
        return false;
    }
    last_time = now;

    //msg_printf(NULL, MSG_INFO, "RPC poll start");
    while (1) {
#if 0
        p = next_project_sched_rpc_pending();
        if (p) {
            work_fetch.compute_work_request(p);
            action = simulate_rpc(p);
            break;
        }
#endif
    
        p = find_project_with_overdue_results();
        if (p) {
            //printf("doing RPC to %s to report results\n", p->project_name);
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
#if 0
    if (action) {
        msg_printf(p, MSG_INFO, "RPC poll: did an RPC");
    } else {
        msg_printf(0, MSG_INFO, "RPC poll: didn't do an RPC");
    }
#endif
    return action;
}

bool ACTIVE_TASK_SET::poll() {
    if (!active) return false;

    unsigned int i;
    char buf[256];
    bool action = false;
    static double last_time = START_TIME;
    double diff = gstate.now - last_time;
    if (diff < 1.0) return false;
    last_time = gstate.now;
    if (diff > delta) {
        diff = 0;
    }
    PROJECT* p;

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        p->idle = true;
    }

    // we do two kinds of FLOPs accounting:
    // 1) actual FLOPS (for job completion)
    // 2) peak FLOPS (for total and per-project resource usage)
    //
    // CPU may be overcommitted, in which case we compute
    //  a "cpu_scale" factor that is < 1.
    // GPUs are never overcommitted.
    //
    // actual FLOPS is based on app_version.flops, scaled by cpu_scale for CPU jobs
    // peak FLOPS is based on device peak FLOPS,
    //  with CPU component scaled by cpu_scale for all jobs

    // get CPU usage by GPU and CPU jobs
    //
    double cpu_usage_cpu=0;
    double cpu_usage_gpu=0;
    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        RESULT* rp = atp->result;
        if (rp->uses_coprocs()) {
            if (gpu_active) {
                cpu_usage_gpu += rp->avp->avg_ncpus;
            }
        } else {
            cpu_usage_cpu += rp->avp->avg_ncpus;
        }
    }
    double cpu_usage = cpu_usage_cpu + cpu_usage_gpu;

    // if CPU is overcommitted, compute cpu_scale
    //
    double cpu_scale = 1;
    if (cpu_usage > gstate.ncpus) {
        cpu_scale = (gstate.ncpus - cpu_usage_gpu) / (cpu_usage - cpu_usage_gpu);
    }

    double used = 0;
    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        RESULT* rp = atp->result;
        if (!gpu_active && rp->uses_coprocs()) {
            continue;
        }
        atp->elapsed_time += diff;
        double flops = rp->avp->flops;
        if (!rp->uses_coprocs()) {
            flops *= cpu_scale;
        }

        atp->flops_left -= diff*flops;

        atp->fraction_done = 1 - (atp->flops_left / rp->wup->rsc_fpops_est);
        atp->checkpoint_wall_time = gstate.now;

        if (atp->flops_left <= 0) {
            atp->set_task_state(PROCESS_EXITED, "poll");
            rp->exit_status = 0;
            rp->ready_to_report = true;
            gstate.request_schedule_cpus("ATP poll");
            gstate.request_work_fetch("ATP poll");
            sprintf(buf, "result %s finished<br>", rp->name);
            html_msg += buf;
            action = true;
        }
        double pf = diff * app_peak_flops(rp->avp, cpu_scale);
        rp->project->project_results.flops_used += pf;
        rp->peak_flop_count += pf;
        sim_results.flops_used += pf;
        used += pf;
        rp->project->idle = false;
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
    active_time += diff;
    if (gpu_active) {
        gpu_active_time += diff;
    }

    return action;
}

// return the fraction of flops that was spent in violation of shares
// i.e., if a project got X and it should have got Y,
// add up |X-Y| over all projects, and divide by total flops
//
double CLIENT_STATE::share_violation() {
    unsigned int i;

    double tot = 0, trs=0;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        tot += p->project_results.flops_used;
        trs += p->resource_share;
    }
    double sum = 0;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        double t = p->project_results.flops_used;
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
        double avg_ss = p->idle_time_sumsq/active_time;
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
    double flops_total = cpu_peak_flops()*active_time
        + gpu_peak_flops()*gpu_active_time;
    fprintf(summary_file, "total %fG\n", flops_total/1e9);
    double flops_idle = flops_total - flops_used;
    fprintf(summary_file, "used: %fG wasted: %fG idle: %fG\n",
        flops_used/1e9, flops_wasted/1e9, flops_idle/1e9
    );
    wasted_frac = flops_wasted/flops_total;
    idle_frac = flops_idle/flops_total;
    share_violation = gstate.share_violation();
    monotony = gstate.monotony();
}

void SIM_RESULTS::print(FILE* f) {
    double r = ((double)nrpcs)/(njobs*2);
    fprintf(f, "wf %f if %f sv %f m %f r %f\n",
        wasted_frac, idle_frac, share_violation, monotony, r
    );
}

void SIM_RESULTS::parse(FILE* f) {
    fscanf(f, "wasted_frac %lf idle_frac %lf share_violation %lf monotony %lf",
        &wasted_frac, &idle_frac, &share_violation, &monotony
    );
}

void SIM_RESULTS::add(SIM_RESULTS& r) {
    wasted_frac += r.wasted_frac;
    idle_frac += r.idle_frac;
    share_violation += r.share_violation;
    monotony += r.monotony;
}

void SIM_RESULTS::divide(int n) {
    wasted_frac /= n;
    idle_frac /= n;
    share_violation /= n;
    monotony /= n;
}

void SIM_RESULTS::clear() {
    memset(this, 0, sizeof(*this));
}

void PROJECT::print_results(FILE* f, SIM_RESULTS& sr) {
    double t = project_results.flops_used;
    double gt = sr.flops_used;
    fprintf(f, "%s: share %.2f total flops %.2fG (%.2f%%)\n"
        "   used %.2fG wasted %.2fG\n"
        "   met %d missed %d\n",
        project_name, resource_share,
        t/1e9, (t/gt)*100,
        project_results.flops_used/1e9,
        project_results.flops_wasted/1e9,
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
    "#008888",
    "#0000aa",
    "#00aa00",
    "#aa0000",
    "#aa00aa",
    "#aaaa00",
    "#00aaaa",
    "#0000cc",
    "#00cc00",
    "#cc0000",
    "#cc00cc",
    "#cccc00",
    "#00cccc",
};

#define NCOLORS 12
#define WIDTH1  100
#define WIDTH2  400


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

    fprintf(html_out, "<td width=%d valign=top>", WIDTH2);
    bool found = false;
    for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
        RESULT* rp = atp->result;
        if (rsc_type!=RSC_TYPE_CPU && rp->resource_type() != rsc_type) continue;
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

        fprintf(html_out, "%.2f: <font color=%s>%s%s: %.2fG</font><br>",
            ninst,
            colors[p->index%NCOLORS],
            atp->result->rr_sim_misses_deadline?"*":"",
            atp->result->name,
            atp->flops_left/1e9
        );
        found = true;
    }
    if (!found) fprintf(html_out, "IDLE");
    fprintf(html_out, "<br>Jobs in progress\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        int n = njobs_in_progress(p, rsc_type);
        if (n) {
            fprintf(html_out, "<br>%s: %d\n", p->project_name, n);
        }
    }
    fprintf(html_out, "</td>");
}

int nproc_types = 1;

void html_start() {
    char buf[256];

    sprintf(buf, "%s%s", outfile_prefix, TIMELINE_FNAME);
    html_out = fopen(buf, "w");
    if (!html_out) {
        fprintf(stderr, "can't open %s for writing\n", buf);
        exit(1);
    }
    setbuf(html_out, 0);
    fprintf(index_file, "<br><a href=%s>Timeline</a>\n", buf);
    fprintf(html_out, "<h2>BOINC client simulator</h2>\n");
    fprintf(html_out,
        "<table border=1 cellpadding=4><tr><th width=%d>Time</th>\n", WIDTH1
    );
    fprintf(html_out,
        "<th width=%d>CPU<br><font size=-2>Job name and estimated time left<br>color denotes project<br>* means EDF mode</font></th>", WIDTH2
    );
    if (gstate.host_info.have_cuda()) {
        fprintf(html_out, "<th width=%d>NVIDIA GPU</th>", WIDTH2);
        nproc_types++;
    }
    if (gstate.host_info.have_ati()) {
        fprintf(html_out, "<th width=%d>ATI GPU</th>", WIDTH2);
        nproc_types++;
    }
    fprintf(html_out, "</tr></table>\n");
}

void html_rec() {
    if (html_msg.size()) {
        fprintf(html_out,
            "<table border=1><tr><td width=%d valign=top>%.0f</td>",
            WIDTH1, gstate.now
        );
        fprintf(html_out,
            "<td width=%d valign=top><font size=-2>%s</font></td></tr></table>\n",
            nproc_types*WIDTH2,
            html_msg.c_str()
        );
        html_msg = "";
    }
    fprintf(html_out, "<table border=1><tr><td width=%d valign=top>%.0f</td>", WIDTH1, gstate.now);

    if (active) {
        show_resource(RSC_TYPE_CPU);
        if (gpu_active) {
            if (gstate.host_info.have_cuda()) {
                show_resource(RSC_TYPE_CUDA);
            }
            if (gstate.host_info.have_ati()) {
                show_resource(RSC_TYPE_ATI);
            }
        } else {
            if (gstate.host_info.have_cuda()) {
                fprintf(html_out, "<td width=%d valign=top bgcolor=#aaaaaa>OFF</td>", WIDTH2);
            }
            if (gstate.host_info.have_ati()) {
                fprintf(html_out, "<td width=%d valign=top bgcolor=#aaaaaa>OFF</td>", WIDTH2);
            }
        }
    } else {
        fprintf(html_out, "<td width=%d valign=top bgcolor=#aaaaaa>OFF</td>", WIDTH2);
        if (gstate.host_info.have_cuda()) {
            fprintf(html_out, "<td width=%d valign=top bgcolor=#aaaaaa>OFF</td>", WIDTH2);
        }
        if (gstate.host_info.have_ati()) {
            fprintf(html_out, "<td width=%d valign=top bgcolor=#aaaaaa>OFF</td>", WIDTH2);
        }
    }

    fprintf(html_out, "</tr></table>\n");
}

void html_end() {
    fprintf(html_out, "<pre>\n");
    sim_results.compute();
    sim_results.print(html_out);
    print_project_results(html_out);
    fprintf(html_out, "</pre>\n");
    fclose(html_out);
}

//#ifdef USE_REC

void set_initial_rec() {
    unsigned int i;
    double sum=0;
    double x = total_peak_flops();
    for (i=0; i<gstate.projects.size(); i++) {
        sum += gstate.projects[i]->resource_share;
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.rec = 86400*x*(p->resource_share/sum)/1e9;
    }
}

void write_recs() {
    fprintf(debt_file, "%f ", gstate.now);
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        fprintf(debt_file, "%f ", p->pwf.rec);
    }
    fprintf(debt_file, "\n");
}

void make_graph(const char* title, const char* fname, int field) {
    char gp_fname[256], cmd[256], png_fname[256];

    sprintf(gp_fname, "%s%s.gp", outfile_prefix, fname);
    FILE* f = fopen(gp_fname, "w");
    fprintf(f,
        "set terminal png small size 1024, 768\n"
        "set title \"%s\"\n"
        "set yrange[0:]\n"
        "plot ",
        title
    );
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        fprintf(f, "\"%sdebt.dat\" using 1:%d title \"%s\" with lines%s",
            outfile_prefix, 2+i+field, p->project_name,
            (i==gstate.projects.size()-1)?"\n":", \\\n"
        );
    }
    fclose(f);
    sprintf(png_fname, "%s%s.png", outfile_prefix, fname);
    sprintf(cmd, "gnuplot < %s > %s", gp_fname, png_fname);
    fprintf(index_file, "<br><a href=%s>Graph of %s</a>\n", png_fname, title);
    system(cmd);
}

//#else

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

    sprintf(gp_fname, "%s%s.gp", outfile_prefix, fname);
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
            outfile_prefix, 2+field+i*nfields, p->project_name,
            (i==gstate.projects.size()-1)?"\n":", \\\n"
        );
    }
    fclose(f);
    sprintf(png_fname, "%s%s.png", outfile_prefix, fname);
    sprintf(cmd, "gnuplot < %s > %s", gp_fname, png_fname);
    fprintf(index_file, "<br><a href=%s>Graph of %s</a>\n", png_fname, title);
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

//#endif

void simulate() {
    bool action;
    double start = START_TIME;
    gstate.now = start;
    html_start();
    fprintf(summary_file,
        "starting simulation. delta %f duration %f\n", delta, duration
    );
    while (1) {
        on = on_proc.sample(delta);
        if (on) {
            active = active_proc.sample(delta);
            if (active) {
                gpu_active = gpu_active_proc.sample(delta);
            } else {
                gpu_active = false;
            }
            connected = connected_proc.sample(delta);
        } else {
            active = gpu_active = connected = false;
        }
        if (on) {
            while (1) {
                action = gstate.active_tasks.poll();
                action |= gstate.handle_finished_apps();
                gpu_suspend_reason = gpu_active?0:1;
                action |= gstate.possibly_schedule_cpus();
                if (connected) {
                    action |= gstate.scheduler_rpc_poll();
                }
                //msg_printf(0, MSG_INFO, action?"did action":"did no action");
                if (!action) break;
            }
        }
        //msg_printf(0, MSG_INFO, "took time step");
        for (unsigned int i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
            ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
            if (atp->task_state() == PROCESS_EXECUTING) {
                atp->elapsed_time += delta;
            }
        }
        html_rec();
if (use_rec) {
        write_recs();
} else {
        write_debts();
}
        gstate.now += delta;
        if (gstate.now > start + duration) break;
    }
    html_end();
}

void show_app(APP* app) {
    fprintf(summary_file,
        "%s: app %s fpops_est %.0fG fpops mean %.0fG std_dev %.0fG latency %.2f weight %.2f\n",
        app->project->project_name,
        app->name, app->fpops_est/1e9,
        app->fpops.mean/1e9, app->fpops.std_dev/1e9,
        app->latency_bound,
        app->weight
    );
    for (unsigned int i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->app != app) continue;
        fprintf(summary_file,
            "   app version %s %d (%s): ncpus %.2f ncuda %.2f nati %.2f flops %.0fG\n",
            avp->app_name, avp->version_num, avp->plan_class,
            avp->avg_ncpus, avp->ncudas, avp->natis,
            avp->flops/1e9
        );
    }
}

// get application params,
// and set "ignore" for apps that have no versions or no params.
//
// App params can be specified in 2 ways:
// - the presence of a WU and result for that app
// - app.latency_bound and app.fpops_est are populated
//
void get_app_params() {
    APP* app;
    unsigned int i, j;

    for (i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        app = rp->app;
        if (!app->latency_bound) {
            app->latency_bound = rp->report_deadline - rp->received_time;
        }
    }
    for (i=0; i<gstate.workunits.size(); i++) {
        WORKUNIT* wup = gstate.workunits[i];
        app = wup->app;
        if (!app->fpops_est) {
            app->fpops_est = wup->rsc_fpops_est;
        }
    }
    for (i=0; i<gstate.apps.size(); i++) {
        app = gstate.apps[i];
        app->ignore = true;
    }
    for (i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        avp->app->ignore = false;
    }
    for (i=0; i<gstate.apps.size(); i++) {
        app = gstate.apps[i];
        if (!app->fpops_est || !app->latency_bound) {
            app->ignore = true;
            fprintf(summary_file,
                "%s: ignoring app %s - no fpops_est or latency bound\n",
                app->project->project_name,
                app->name
            );
        } else if (app->ignore) {
            fprintf(summary_file,
                "%s: ignoring app %s - no app versions\n",
                app->project->project_name,
                app->name
            );
        } else {
            if (!app->fpops.mean) {
                app->fpops.mean = app->fpops_est;
            }
            if (!app->weight) {
                app->weight = 1;
            }
            show_app(app);
        }
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
    PROJECT* p;

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        p->dont_request_more_work = true;
        p->no_cpu_apps = true;
        p->no_cuda_apps = true;
        p->no_ati_apps = true;
    }
    for (i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->app->ignore) continue;
        if (avp->ncudas) {
            avp->project->no_cuda_apps = false;
        } else if (avp->natis) {
            avp->project->no_ati_apps = false;
        } else {
            avp->project->no_cpu_apps = false;
        }
    }
    for (i=0; i<gstate.apps.size(); i++) {
        APP* app = gstate.apps[i];
        if (!app->ignore) {
            app->project->dont_request_more_work = false;
        }
    }
    vector<PROJECT*>::iterator iter;
    iter = gstate.projects.begin();
    while (iter != gstate.projects.end()) {
        p = *iter;
        if (p->dont_request_more_work) {
            fprintf(summary_file,
                "%s: Removing from simulation - no jobs in client_state.xml\n",
                p->project_name
            );
            iter = gstate.projects.erase(iter);
        } else {
            iter++;
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        fprintf(summary_file, "%s: %s%s%s\n",
            p->project_name,
            p->no_cpu_apps?" no CPU apps":"",
            p->no_cuda_apps?" no nvidia apps":"",
            p->no_ati_apps?" no ATI apps":""
        );
    }
}

void do_client_simulation() {
    char buf[256], buf2[256];

    sprintf(buf, "%s%s", infile_prefix, CONFIG_FILE);
    read_config_file(true, buf);
    fprintf(index_file,
        "<br><a href=%s>Configuration file (cc_config.xml)</a>\n",
        buf
    );
    config.show();

    gstate.add_platform("client simulator");
    sprintf(buf, "%s%s", infile_prefix, STATE_FILE_NAME);
    gstate.parse_state_file_aux(buf);
    fprintf(index_file,
        "<br><a href=%s>State file (client_state.xml)</a>\n",
        buf
    );
    sprintf(buf, "%s%s", infile_prefix, GLOBAL_PREFS_FILE_NAME);
    sprintf(buf2, "%s%s", infile_prefix, GLOBAL_PREFS_OVERRIDE_FILE);
    gstate.read_global_prefs(buf, buf2);
    fprintf(index_file,
        "<br><a href=%s>Preferences file (global_prefs.xml)</a>\n"
        "<br><a href=%s>Preferences override file (global_prefs_override.xml)</a>\n"
        "<h2>Output files</h2>\n"
        "<a href=%s>Log file</a>\n",
        buf, buf2, log_filename
    );

    get_app_params();
    cull_projects();

    int j=0;
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        gstate.projects[i]->index = j++;
    }

    clear_backoff();
    gstate.workunits.clear();
    gstate.results.clear();

    gstate.set_ncpus();
    work_fetch.init();

if (use_rec) {
    set_initial_rec();
}

    gstate.request_work_fetch("init");
    simulate();

    sim_results.compute();

    sprintf(buf, "%s%s", outfile_prefix, RESULTS_FNAME);
    FILE* f = fopen(buf, "w");
    sim_results.print(f);
    fclose(f);

    fprintf(summary_file, "Peak FLOPS: CPU %.2fG GPU %.2fG\n",
        cpu_peak_flops()/1e9,
        gpu_peak_flops()/1e9
    );
    sim_results.print(summary_file);

    print_project_results(summary_file);

if (use_rec) {
    make_graph("REC", "rec", 0);
} else {
    debt_graphs();
}
}

char* next_arg(int argc, char** argv, int& i) {
    if (i >= argc) {
        fprintf(stderr, "Missing command-line argument\n");
        usage(argv[0]);
    }
    return argv[i++];
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];

    sim_results.clear();
    for (i=1; i<argc;) {
        char* opt = argv[i++];
        if (!strcmp(opt, "--infile_prefix")) {
            infile_prefix = argv[i++];
        } else if (!strcmp(opt, "--outfile_prefix")) {
            outfile_prefix = argv[i++];
        } else if (!strcmp(opt, "--duration")) {
            duration = atof(next_arg(argc, argv, i));
        } else if (!strcmp(opt, "--delta")) {
            delta = atof(next_arg(argc, argv, i));
        } else if (!strcmp(opt, "--server_uses_workload")) {
            server_uses_workload = true;
        } else if (!strcmp(opt, "--cpu_sched_rr_only")) {
            cpu_sched_rr_only = true;
        } else if (!strcmp(opt, "--use_rec")) {
            use_rec = true;
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

    sprintf(buf, "%s%s", outfile_prefix, "index.html");
    index_file = fopen(buf, "w");
    fprintf(index_file,
        "<h2>Input files</h2>\n"
    );

    sprintf(log_filename, "%s%s", outfile_prefix, LOG_FNAME);
    logfile = fopen(log_filename, "w");
    if (!logfile) {
        fprintf(stderr, "Can't open %s\n", buf);
        exit(1);
    }
    setbuf(logfile, 0);

    sprintf(buf, "%s%s", outfile_prefix, DEBT_FNAME);
    debt_file = fopen(buf, "w");

    sprintf(buf, "%s%s", outfile_prefix, SUMMARY_FNAME);
    summary_file = fopen(buf, "w");

    do_client_simulation();
}
