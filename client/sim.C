// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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

// BOINC client simulator.
//
// usage:
// sim [--duration x] [--delta x] [--dirs dir ...]
//  duration = simulation duration (default 86400)
//  delta = simulation time step (default 10)
//
// If no dirs are specified:
// reads input files
//    sim_projects.xml, sim_host.xml, sim_prefs.xml, cc_config.xml
// and does simulation, generating output files
//    sim_log.txt, sim_out.html
//
// If dirs are specified, chdir into each directory in sequence,
// do the above for each one, and write summary info to stdout

#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "log_flags.h"
#include "filesys.h"
#include "network.h"
#include "client_msgs.h"
#include "../sched/edf_sim.h"
#include "sim.h"

#define SCHED_RETRY_DELAY_MIN    60                // 1 minute
#define SCHED_RETRY_DELAY_MAX    (60*60*4)         // 4 hours

CLIENT_STATE gstate;
NET_STATUS net_status;
//bool g_use_sandbox;
bool user_active;
double duration = 86400, delta = 60;
FILE* logfile;
bool running;
double running_time = 0;
bool server_uses_workload = false;

SIM_RESULTS sim_results;

// generate a job; pick a random app,
// and pick a FLOP count from its distribution
//
void CLIENT_STATE::make_job(SIM_PROJECT* p, WORKUNIT* wup, RESULT* rp) {
    SIM_APP* ap1, *ap=0;
    double net_fpops = host_info.p_fpops;
    double x = drand();

    for (unsigned int i=0; i<apps.size();i++) {
        ap1 = (SIM_APP*)apps[i];
        if (ap1->project != p) continue;
        x -= ap1->weight;
        if (x <= 0) {
            ap = ap1;
            break;
        }
    }
    if (!ap) {
        printf("ERROR-NO APP\n");
        exit(1);
    }
    rp->clear();
    rp->project = p;
    rp->wup = wup;
    sprintf(rp->name, "%s_%d", p->project_name, p->result_index++);
    wup->project = p;
    wup->rsc_fpops_est = ap->fpops_est;
    double ops = ap->fpops.sample();
    if (ops < 0) ops = 0;
    rp->final_cpu_time = ops/net_fpops;
    rp->report_deadline = now + ap->latency_bound;
}

// process ready-to-report results
//
void CLIENT_STATE::handle_completed_results() {
    char buf[256];
    vector<RESULT*>::iterator result_iter;

    result_iter = results.begin();
    while (result_iter != results.end()) {
        RESULT* rp = *result_iter;
        if (rp->ready_to_report) {
            sprintf(buf, "result %s reported; %s<br>",
                rp->name,
                (gstate.now > rp->report_deadline)?
                "<font color=#cc0000>MISSED DEADLINE</font>":
                "<font color=#00cc00>MADE DEADLINE</font>"
            );
            SIM_PROJECT* spp = (SIM_PROJECT*)rp->project;
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
            gstate.html_msg += buf;
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
        double x = rp->estimated_cpu_time_remaining();
        if (x == 0) continue;
        IP_RESULT ipr(rp->name, rp->report_deadline, x);
        ip_results.push_back(ipr);
    }
    init_ip_results(work_buf_min(), ncpus, ip_results);
}

bool CLIENT_STATE::simulate_rpc(PROJECT* _p) {
    char buf[256];
    SIM_PROJECT* p = (SIM_PROJECT*) _p;
    static double last_time=0;
    vector<IP_RESULT> ip_results;

    double diff = now - last_time;
    if (diff && diff < host_info.connection_interval) {
        msg_printf(NULL, MSG_INFO, "simulate_rpc: too soon %f < %f", diff, host_info.connection_interval);
        return false;
    }
    last_time = now;

    sprintf(buf, "RPC to %s; asking for %f<br>", p->project_name, p->work_request);
    html_msg += buf;

    handle_completed_results();

    if (server_uses_workload) {
        get_workload(ip_results);
    }

    bool sent_something = false;
    double work_left = p->work_request;
    while (work_left > 0) {
        RESULT* rp = new RESULT;
        WORKUNIT* wup = new WORKUNIT;
        make_job(p, wup, rp);

        if (server_uses_workload) {
            IP_RESULT c(rp->name, rp->report_deadline, rp->final_cpu_time);
            if (check_candidate(c, ncpus, ip_results)) {
                ip_results.push_back(c);
            } else {
                delete rp;
                delete wup;
                break;
            }
        }

        sent_something = true;
        rp->set_state(RESULT_FILES_DOWNLOADED, "simulate_rpc");
        results.push_back(rp);
        sprintf(buf, "got job %s: CPU time %.2f, deadline %s<br>",
            rp->name, rp->final_cpu_time, time_to_string(rp->report_deadline)
        );
        html_msg += buf;
        printf("%s\n", buf);
        work_left -= p->duration_correction_factor*wup->rsc_fpops_est/host_info.p_fpops;
    }

    if (p->work_request > 0 && !sent_something) {
        p->backoff();
    } else {
        p->nrpc_failures = 0;
    }
    p->work_request = 0;
    request_schedule_cpus("simulate_rpc");
    request_work_fetch("simulate_rpc");
    return true;
}

void SIM_PROJECT::backoff() {
    nrpc_failures++;
    double backoff = calculate_exponential_backoff(
        nrpc_failures, SCHED_RETRY_DELAY_MIN, SCHED_RETRY_DELAY_MAX
    );
    min_rpc_time = gstate.now + backoff;
}

bool CLIENT_STATE::scheduler_rpc_poll() {
    PROJECT *p;

    msg_printf(NULL, MSG_INFO, "RPC poll start");
    p = next_project_sched_rpc_pending();
    if (p) {
        return simulate_rpc(p);
    }
    
    p = find_project_with_overdue_results();
    if (p) {
        return simulate_rpc(p);
    }
    p = next_project_need_work();
    if (p) {
        return simulate_rpc(p);
    }
    msg_printf(NULL, MSG_INFO, "RPC poll: nothing to do");
    return false;
}

bool ACTIVE_TASK_SET::poll() {
    unsigned int i;
    char buf[256];
    bool action = false;
    static double last_time = 0;
    double diff = gstate.now - last_time;
    if (diff < 1.0) return false;
    last_time = gstate.now;
    SIM_PROJECT* p;

    if (!running) return false;

    for (i=0; i<gstate.projects.size(); i++) {
        p = (SIM_PROJECT*) gstate.projects[i];
        p->idle = true;
        sprintf(buf, "%s STD: %f min RPC<br>",
            p->project_name, p->short_term_debt,
            time_to_string(p->min_rpc_time)
        );
        gstate.html_msg += buf;
    }

    int n=0;
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
                gstate.html_msg += buf;
                action = true;
            }
            ((SIM_PROJECT*)rp->project)->idle = false;
            n++;
        }
    }
    if (n < gstate.ncpus) {
        sim_results.cpu_idle += diff*(gstate.ncpus-n);
    }
    if (n > gstate.ncpus) {
        sprintf(buf, "TOO MANY JOBS RUNNING");
        gstate.html_msg += buf;
    }

    for (i=0; i<gstate.projects.size(); i++) {
        p = (SIM_PROJECT*) gstate.projects[i];
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

int SIM_APP::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    int retval;

    weight = 1;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "/app")) {
            return 0;
        }
        else if (xp.parse_double(tag, "latency_bound", latency_bound)) continue;
        else if (xp.parse_double(tag, "fpops_est", fpops_est)) continue;
        else if (xp.parse_double(tag, "weight", weight)) continue;
        else if (!strcmp(tag, "fpops")) {
            retval = fpops.parse(xp, "/fpops");
            if (retval) return retval;
        } else if (!strcmp(tag, "checkpoint_period")) {
            retval = checkpoint_period.parse(xp, "/checkpoint_period");
            if (retval) return retval;
        } else if (xp.parse_double(tag, "working_set", working_set)) continue;
        else {
            printf("unrecognized: %s\n", tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

// return the fraction of CPU time that was spent in violation of shares
// i.e., if a project got X and it should have got Y,
// add up |X-Y| over all projects, and divide by total CPU
//
double CLIENT_STATE::share_violation() {
    double x = 0;
    unsigned int i;

    double tot = 0, trs=0;
    for (i=0; i<projects.size(); i++) {
        SIM_PROJECT* p = (SIM_PROJECT*) projects[i];
        tot += p->project_results.cpu_used + p->project_results.cpu_wasted;
        trs += p->resource_share;
    }
    double sum = 0;
    for (i=0; i<projects.size(); i++) {
        SIM_PROJECT* p = (SIM_PROJECT*) projects[i];
        double t = p->project_results.cpu_used + p->project_results.cpu_wasted;
        double rs = p->resource_share/trs;
        double rt = tot*rs;
        sum += fabs(t - rt);
    }
    return sum/tot;

}

// "variety" is defined as follows:
// for each project P, maintain R(P), the time since P last ran,
// let S(P) be the RMS of R(P).
// Let variety = mean(S(P))
//
double CLIENT_STATE::variety() {
    double sum = 0;
    double schedint = global_prefs.cpu_scheduling_period_minutes*60;
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        SIM_PROJECT* p = (SIM_PROJECT*) projects[i];
        double avg_ss = p->idle_time_sumsq/running_time;
        double s = sqrt(avg_ss);
        sum += s;
    }
    return sum/(projects.size()*schedint);
}

// the CPU totals are there; compute the other fields
//
void SIM_RESULTS::compute() {
    double total = cpu_used + cpu_wasted + cpu_idle;
    cpu_wasted_frac = cpu_wasted/total;
    cpu_idle_frac = cpu_idle/total;
    share_violation = gstate.share_violation();
    variety = gstate.variety();
}

// top-level results (for aggregating multiple simulations)
//
void SIM_RESULTS::print(FILE* f, const char* title) {
    if (title) {
        fprintf(f, "%s: ", title);
    }
    fprintf(f, "wasted_frac %f idle_frac %f share_violation %f variety %f\n",
        cpu_wasted_frac, cpu_idle_frac, share_violation, variety
    );
}

void SIM_RESULTS::parse(FILE* f) {
    fscanf(f, "wasted_frac %lf idle_frac %lf share_violation %lf variety %lf",
        &cpu_wasted_frac, &cpu_idle_frac, &share_violation, &variety
    );
}

void SIM_RESULTS::add(SIM_RESULTS& r) {
    cpu_wasted_frac += r.cpu_wasted_frac;
    cpu_idle_frac += r.cpu_idle_frac;
    share_violation += r.share_violation;
    variety += r.variety;
}

void SIM_RESULTS::divide(int n) {
    cpu_wasted_frac /= n;
    cpu_idle_frac /= n;
    share_violation /= n;
    variety /= n;
}

void SIM_RESULTS::clear() {
    memset(this, 0, sizeof(*this));
}

void SIM_PROJECT::print_results(FILE* f, SIM_RESULTS& sr) {
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

char* colors[] = {
    "#ffffdd",
    "#ffddff",
    "#ddffff",
    "#ddffdd",
    "#ddddff",
    "#ffdddd",
};

void CLIENT_STATE::html_start() {
    html_out = fopen("sim_out.html", "w");
    setbuf(html_out, 0);
    fprintf(html_out, "<h2>Simulator output</h2>"
        "<a href=sim_log.txt>message log</a><p>"
        "<table border=1>\n"
    );
}

void CLIENT_STATE::html_rec() {
    fprintf(html_out, "<tr><td>%s</td>", time_to_string(now));

    if (!running) {
        for (int j=0; j<ncpus; j++) {
            fprintf(html_out, "<td bgcolor=#aaaaaa>OFF</td>");
        }
    } else {
        int n=0;
        for (unsigned int i=0; i<active_tasks.active_tasks.size(); i++) {
            ACTIVE_TASK* atp = active_tasks.active_tasks[i];
            if (atp->task_state() == PROCESS_EXECUTING) {
                SIM_PROJECT* p = (SIM_PROJECT*)atp->result->project;
                fprintf(html_out, "<td bgcolor=%s>%s%s: %.2f</td>",
                    colors[p->index],
                    atp->result->rr_sim_misses_deadline?"*":"",
                    atp->result->name, atp->cpu_time_left
                );
                n++;
            }
        }
        while (n<ncpus) {
            fprintf(html_out, "<td>IDLE</td>");
            n++;
        }
    }
    fprintf(html_out, "<td><font size=-2>%s</font></td></tr>\n", html_msg.c_str());
    html_msg = "";
}

void CLIENT_STATE::html_end() {
    double cpu_total=sim_results.cpu_used + sim_results.cpu_wasted + sim_results.cpu_idle;
    fprintf(html_out, "</table><pre>\n");
    sim_results.compute();
    sim_results.print(html_out);
    print_project_results(html_out);
    fprintf(html_out, "</pre>\n");
    fclose(html_out);
}

void CLIENT_STATE::simulate() {
    bool action;
    now = 0;
    html_start();
    while (1) {
        running = host_info.available.sample(now);
        while (1) {
            action = active_tasks.poll();
            if (running) {
                action |= handle_finished_apps();
                action |= possibly_schedule_cpus();
                action |= enforce_schedule();
                action |= compute_work_requests();
                action |= scheduler_rpc_poll();
            }
            if (!action) break;
        }
        now += delta;
        html_rec();
        if (now > duration) break;
    }
    html_end();
}

void parse_error(char* file, int retval) {
    printf("can't parse %s: %d\n", file, retval);
    exit(1);
}

void help(char* prog) {
    fprintf(stderr, "usage: %s [--duration X] [--delta X] [--dirs ...]\n", prog);
    exit(1);
}

char* next_arg(int argc, char** argv, int& i) {
    if (i >= argc) {
        fprintf(stderr, "Missing command-line argument\n");
        help(argv[0]);
    }
    return argv[i++];
}

#define PROJECTS_FILE "sim_projects.xml"
#define HOST_FILE "sim_host.xml"
#define PREFS_FILE "sim_prefs.xml"
#define SUMMARY_FILE "sim_summary.txt"
#define LOG_FILE "sim_log.txt"

int main(int argc, char** argv) {
    int i, retval;
    vector<std::string> dirs;

    logfile = fopen("sim_log.txt", "w");

    sim_results.clear();
    for (i=1; i<argc;) {
        char* opt = argv[i++];
        if (!strcmp(opt, "--duration")) {
            duration = atof(next_arg(argc, argv, i));
        } else if (!strcmp(opt, "--delta")) {
            delta = atof(next_arg(argc, argv, i));
        } else if (!strcmp(opt, "--dirs")) {
            while (i<argc) {
                dirs.push_back(argv[i++]);
            }
        } else if (!strcmp(opt, "--server_uses_workload")) {
            server_uses_workload = true;
        } else {
            help(argv[0]);
        }
    }

    if (duration <= 0) {
        printf("non-pos duration\n");
        exit(1);
    }
    if (delta <= 0) {
        printf("non-pos delta\n");
        exit(1);
    }

    if (dirs.size()) {
        // If we need to do several simulations,
        // use system() to do each one in a separate process,
        // because there are lots of static variables and we need to ensure
        // that they start off with the right initial values
        //
        unsigned int i;
        SIM_RESULTS total_results;
        total_results.clear();
        for (i=0; i<dirs.size(); i++) {
            std::string dir = dirs[i];
            retval = chdir(dir.c_str());
            if (retval) {
                fprintf(stderr, "can't chdir into %s: ", dir.c_str());
                perror("chdir");
                continue;
            }
            char buf[256];
            sprintf(
                buf, "../sim --duration %f --delta %f > %s",
                duration, delta, SUMMARY_FILE
            );
            retval = system(buf);
            if (retval) {
                printf("simulation in %s failed\n", dir.c_str());
                exit(1);
            }
            FILE* f = fopen(SUMMARY_FILE, "r");
            sim_results.parse(f);
            fclose(f);
            sim_results.print(stdout, dir.c_str());
            total_results.add(sim_results);
            chdir("..");
        }
        total_results.divide(dirs.size());
        total_results.print(stdout, "Total");
    } else {
        read_config_file();
        int retval;
        bool flag;

        retval = gstate.parse_projects(PROJECTS_FILE);
        if (retval) parse_error(PROJECTS_FILE, retval);
        retval = gstate.parse_host(HOST_FILE);
        if (retval) parse_error(HOST_FILE, retval);
        retval = gstate.global_prefs.parse_file(PREFS_FILE, "", flag);
        if (retval) parse_error(PREFS_FILE, retval);

        gstate.set_ncpus();
        gstate.request_work_fetch("init");
        gstate.simulate();

        sim_results.compute();

        // print machine-readable first
        sim_results.print(stdout);

        // then other
        gstate.print_project_results(stdout);
    }
}
