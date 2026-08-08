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

// Manage a (perhaps multi-processor) benchmark.
// Because of hyperthreaded CPUs we can't just benchmark 1 CPU;
// we must run parallel benchmarks
// and ensure that they run more or less concurrently.
// Here's our scheme:
// - the main program forks N benchmarks threads or processes
// - after FP_START seconds it creates a file "do_fp"
// - after FP_END seconds it deletes do_fp
// - after INT_START seconds it creates do_int
// - after INT_END seconds it deletes do_int and starts waiting for processes
// Each thread/process checks for the relevant file before
//  starting or stopping each benchmark

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <string>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <signal.h>
#if HAVE_SYS_SIGNAL_H
#endif
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "util.h"
#include "cpu_benchmark.h"

#include "client_msgs.h"
#include "log_flags.h"
#include "client_state.h"

#include <vector>

// defaults in case benchmarks fail or time out.
// better to err on the low side so hosts don't get too much work

#define DEFAULT_FPOPS   1e9
#define DEFAULT_IOPS    1e9
#define DEFAULT_MEMBW   1e9
#define DEFAULT_CACHE   1e6

#define FP_START    2
#define FP_END      12
#define INT_START   17
#define INT_END     27
#define OVERALL_END 30

#define MIN_CPU_TIME  2
    // if the CPU time accumulated during one of the 10-sec segments
    // is less than this, ignored the benchmark

#define BM_FP_INIT  0
#define BM_FP       1
#define BM_INT_INIT 2
#define BM_INT      3
#define BM_SLEEP    4
#define BM_DONE     5
static int bm_state;

static bool did_benchmarks = false;
    // true if we successfully did benchmarks.

#define BENCHMARK_PERIOD        (SECONDS_PER_DAY*30)
    // rerun CPU benchmarks this often (hardware may have been upgraded)

// represents a benchmark thread/process, in progress or completed
//
struct BENCHMARK_DESC {
    int ordinal;
    HOST_INFO host_info;
    bool done;
    bool error;
    char error_str[256];
    double int_loops;
    double int_time;
#ifdef _WIN32
    HANDLE handle;
    DWORD pid;
#else
    char filename[256];
    PROCESS_ID pid;
#endif
};

static std::vector<BENCHMARK_DESC> benchmark_descs;
static double cpu_benchmarks_start;
static int bm_ncpus;
    // user might change ncpus during benchmarks.
    // store starting value here.

const char *file_names[2] = {"do_fp", "do_int"};

static void remove_benchmark_file(int which) {
    boinc_delete_file(file_names[which]);
}

static void make_benchmark_file(int which) {
    FILE* f = boinc_fopen(file_names[which], "w");
    fclose(f);
}

void benchmark_wait_to_start(int which) {
    while (1) {
        if (boinc_file_exists(file_names[which])) {
            break;
        }
#ifndef _WIN32
        // UNIX: check if client has died.
        // Not needed on windows, where we run as thread in client process
        //
        if (getppid() == 1) {
            exit(0);
        }
#endif
        boinc_sleep(0.1);
    }
}

bool benchmark_time_to_stop(int which) {
#ifndef _WIN32
    if (getppid() == 1) {
        exit(0);
    }
#endif
    if (boinc_file_exists(file_names[which])) {
        return false;
    }
    return true;
}

// benchmark a single CPU
//
int cpu_benchmarks(BENCHMARK_DESC* bdp) {
    HOST_INFO host_info;
    int retval;
    double vax_mips, int_loops=0, int_time=0, fp_time;

    bdp->error_str[0] = '\0';

#if defined(ANDROID) && defined(__arm__)
#if defined(ARMV6)
    retval = whetstone(host_info.p_fpops, fp_time, MIN_CPU_TIME);
#else
    // check for FP accelerator: VFP, Neon, or none;
    // run the appropriate version of Whetstone
    // (separated using namespaces)
    //
    if (strstr(gstate.host_info.p_features, " neon ")) {
        // have ARM neon FP capabilities
        retval = android_neon::whetstone(host_info.p_fpops, fp_time, MIN_CPU_TIME);
    } else if (strstr(gstate.host_info.p_features, " vfp ")) {
        // have ARM vfp FP capabilities
        retval = android_vfp::whetstone(host_info.p_fpops, fp_time, MIN_CPU_TIME);
    } else { // just run normal test
        retval = whetstone(host_info.p_fpops, fp_time, MIN_CPU_TIME);
    }
#endif
#else
    retval = whetstone(host_info.p_fpops, fp_time, MIN_CPU_TIME);
#endif
    if (retval) {
        bdp->error = true;
        snprintf(bdp->error_str, sizeof(bdp->error_str), "FP benchmark ran only %f sec; ignoring", fp_time);
        return 0;
    }
#ifdef _WIN32
    // Windows: do integer benchmark only on CPU zero.
    // There's a mysterious bug/problem that gives wildly
    // differing benchmarks on multi-CPU and multi-core machines,
    // if you use all the CPUs at once.
    //
    if (bdp->ordinal == 0) {
#endif
    retval = dhrystone(vax_mips, int_loops, int_time, MIN_CPU_TIME);
    if (retval) {
        bdp->error = true;
        snprintf(bdp->error_str, sizeof(bdp->error_str), "Integer benchmark ran only %f sec; ignoring", int_time);
        return 0;
    }
    host_info.p_iops = vax_mips*1e6;
    host_info.p_membw = 1e9;
#ifdef _WIN32
    }
    bdp->host_info = host_info;
    bdp->int_loops = int_loops;
    bdp->int_time = int_time;
#else
    FILE* finfo;
    finfo = boinc_fopen(bdp->filename, "w");
    if (!finfo) return ERR_FOPEN;
    host_info.write_cpu_benchmarks(finfo);
    fclose(finfo);
#endif
    return 0;
}

#ifdef _WIN32
DWORD WINAPI win_cpu_benchmarks(LPVOID p) {
    return cpu_benchmarks((BENCHMARK_DESC*)p);
}
#endif

void CLIENT_STATE::start_cpu_benchmarks(bool force) {
    int i;

    if (benchmarks_running) {
        msg_printf(0, MSG_INFO,
            "Can't start benchmarks - they're already running"
        );
        return;
    }

    if (cc_config.skip_cpu_benchmarks && !force) {
        if (log_flags.benchmark_debug) {
            msg_printf(0, MSG_INFO,
                "[benchmark] start_cpu_benchmarks(): Skipping CPU benchmarks"
            );
        }
        cpu_benchmarks_set_defaults();
        return;
    }
    msg_printf(NULL, MSG_INFO, "Running CPU benchmarks");

    bm_state = BM_FP_INIT;
    remove_benchmark_file(BM_TYPE_FP);
    remove_benchmark_file(BM_TYPE_INT);
    cpu_benchmarks_start = dtime();

    benchmark_descs.clear();
    benchmark_descs.resize(n_usable_cpus);

    bm_ncpus = n_usable_cpus;
    benchmarks_running = true;

    for (i=0; i<bm_ncpus; i++) {
        benchmark_descs[i].ordinal = i;
        benchmark_descs[i].done = false;
        benchmark_descs[i].error = false;
#ifdef _WIN32
        benchmark_descs[i].handle = CreateThread(
            NULL, 0, win_cpu_benchmarks, &benchmark_descs[i], 0,
            &benchmark_descs[i].pid
        );
        int n = host_info.p_ncpus;
        int j = (i >= n/2)? 2*i+1-n : 2*i;
        SetThreadAffinityMask(benchmark_descs[i].handle, 1ull<<j);
        SetThreadPriority(benchmark_descs[i].handle, THREAD_PRIORITY_IDLE);
#else
        sprintf(benchmark_descs[i].filename, "%s_%d.xml", CPU_BENCHMARKS_FILE_NAME, i);
        PROCESS_ID pid = fork();
        if (pid == 0) {
#if HAVE_SETPRIORITY
            if (setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY)) {
                perror("setpriority");
            }
#endif
            int retval = cpu_benchmarks(&benchmark_descs[i]);
            fflush(NULL);
            _exit(retval);
        } else {
            benchmark_descs[i].pid = pid;
        }
#endif
    }
}

// called at startup to decide if we need to do benchmarks;
// set run_cpu_benchmarks if so.
//
void CLIENT_STATE::check_if_need_benchmarks() {
    if (run_cpu_benchmarks) return;
    // if user has changed p_calculated into the future
    // (as part of cheating, presumably) always run benchmarks
    //
    double diff = now - host_info.p_calculated;
    if (diff < 0) {
        run_cpu_benchmarks = true;
    } else if (diff > BENCHMARK_PERIOD) {
        if (host_info.p_calculated) {
            msg_printf(NULL, MSG_INFO,
                "Last CPU benchmark was %s ago", timediff_format(diff).c_str()
            );
        } else {
            msg_printf(NULL, MSG_INFO, "No CPU benchmark yet");
        }
        run_cpu_benchmarks = true;
    }
}

// Returns true if CPU benchmarks can be run
//
bool CLIENT_STATE::can_run_cpu_benchmarks() {
    if (tasks_suspended) return false;
    return true;
}

// abort a running benchmark thread/process
//
void abort_benchmark(BENCHMARK_DESC& desc) {
    if (desc.done) return;
#ifdef _WIN32
    TerminateThread(desc.handle, 0);
    CloseHandle(desc.handle);
#else
    kill(desc.pid, SIGKILL);
#endif
}

// check a running benchmark thread/process.
//
void check_benchmark(BENCHMARK_DESC& desc) {
#ifdef _WIN32
    DWORD exit_code = 0;
    GetExitCodeThread(desc.handle, &exit_code);
    if (exit_code != STILL_ACTIVE) {
        CloseHandle(desc.handle);
        desc.done = true;
    }
#else
    int retval;
    int exit_code = 0;
    retval = waitpid(desc.pid, &exit_code, WNOHANG);
    if (retval) {
        desc.done = true;
        FILE* f = fopen(desc.filename, "r");
        if (!f) {
            desc.error = true;
            return;
        }
        retval = desc.host_info.parse_cpu_benchmarks(f);
        fclose(f);
        boinc_delete_file(desc.filename);
        if (retval) {
            desc.error = true;
        }
    }
#endif
}

void CLIENT_STATE::abort_cpu_benchmarks() {
    int i;
    if (!benchmarks_running) return;
    for (i=0; i<bm_ncpus; i++) {
        abort_benchmark(benchmark_descs[i]);
    }
}

bool CLIENT_STATE::cpu_benchmarks_poll() {
    int i;
    static double last_time = 0;
    if (!benchmarks_running) return false;

    if (!clock_change && now < last_time + BENCHMARK_POLL_PERIOD) return false;
    last_time = now;

    active_tasks.send_heartbeats();

    // if active tasks don't quit after 10 sec, give up on benchmark
    //
    if (gstate.clock_change || (now >= (cpu_benchmarks_start + 10.0) && active_tasks.is_task_executing())) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Failed to stop applications; aborting CPU benchmarks"
        );
        host_info.p_calculated = now;
        abort_cpu_benchmarks();
        benchmarks_running = false;
        set_client_state_dirty("CPU benchmarks");
        cpu_benchmarks_set_defaults();
        return false;
    }

    // do transitions through benchmark states
    //
    switch (bm_state) {
    case BM_FP_INIT:
        if (now - cpu_benchmarks_start > FP_START) {
            if (log_flags.benchmark_debug) {
                msg_printf(0, MSG_INFO,
                    "[benchmark] Starting floating-point benchmark"
                );
            }
            make_benchmark_file(BM_TYPE_FP);
            bm_state = BM_FP;
        }
        return false;
    case BM_FP:
        if (now - cpu_benchmarks_start > FP_END) {
            if (log_flags.benchmark_debug) {
                msg_printf(0, MSG_INFO,
                    "[benchmark] Ended floating-point benchmark"
                );
            }
            remove_benchmark_file(BM_TYPE_FP);
            bm_state = BM_INT_INIT;
        }
        return false;
    case BM_INT_INIT:
        if (now - cpu_benchmarks_start > INT_START) {
            if (log_flags.benchmark_debug) {
                msg_printf(0, MSG_INFO,
                    "[benchmark] Starting integer benchmark"
                );
            }
            make_benchmark_file(BM_TYPE_INT);
            bm_state = BM_INT;
        }
        return false;
    case BM_INT:
        if (now - cpu_benchmarks_start > INT_END) {
            if (log_flags.benchmark_debug) {
                msg_printf(0, MSG_INFO,
                    "[benchmark] Ended integer benchmark"
                );
            }
            remove_benchmark_file(BM_TYPE_INT);
            bm_state = BM_SLEEP;
        }
        return false;
    case BM_SLEEP:
        if (now - cpu_benchmarks_start > OVERALL_END) {
            if (log_flags.benchmark_debug) {
                msg_printf(0, MSG_INFO,
                    "[benchmark] Ended benchmark"
                );
            }
            bm_state = BM_DONE;
        }
        return false;
    }

    // check for timeout
    //
    if (now > cpu_benchmarks_start + MAX_CPU_BENCHMARKS_SECONDS) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "CPU benchmarks timed out, using default values"
        );
        abort_cpu_benchmarks();
        cpu_benchmarks_set_defaults();
        benchmarks_running = false;
        set_client_state_dirty("CPU benchmarks");
    }

    int ndone = 0;
    bool had_error = false;
    for (i=0; i<bm_ncpus; i++) {
        if (!benchmark_descs[i].done) {
            check_benchmark(benchmark_descs[i]);
        }
        if (benchmark_descs[i].done) {
            if (log_flags.benchmark_debug) {
                msg_printf(0, MSG_INFO,
                    "[benchmark] CPU %d has finished", i
                );
            }
            ndone++;
            if (benchmark_descs[i].error) {
                msg_printf(0, MSG_INFO, "%s", benchmark_descs[i].error_str);
                had_error = true;
            }
        }
    }
    if (log_flags.benchmark_debug) {
        msg_printf(0, MSG_INFO,
            "[benchmark] %d out of %d CPUs done", ndone, bm_ncpus
        );
    }
    if (ndone == bm_ncpus) {
        double old_p_fpops = host_info.p_fpops;
        if (had_error) {
            cpu_benchmarks_set_defaults();
        } else {
            double p_fpops = 0;
            double p_iops = 0;
            double p_membw = 0;
            for (i=0; i<bm_ncpus; i++) {
                if (log_flags.benchmark_debug) {
                    msg_printf(0, MSG_INFO,
                        "[benchmark] CPU %d: fp %f int %f intloops %f inttime %f",
                        i, benchmark_descs[i].host_info.p_fpops,
                        benchmark_descs[i].host_info.p_iops,
                        benchmark_descs[i].int_loops,
                        benchmark_descs[i].int_time
                    );
                }
                p_fpops += benchmark_descs[i].host_info.p_fpops;
#ifdef _WIN32
                p_iops += benchmark_descs[0].host_info.p_iops;
#else
                p_iops += benchmark_descs[i].host_info.p_iops;
#endif
                p_membw += benchmark_descs[i].host_info.p_membw;
            }
            p_fpops /= bm_ncpus;
            p_iops /= bm_ncpus;
            p_membw /= bm_ncpus;
            if (p_fpops > 0) {
                host_info.p_fpops = p_fpops;
            } else {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "Benchmark: FP unexpectedly zero; ignoring");
            }
            if (p_iops > 0) {
                host_info.p_iops = p_iops;
            } else {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "Benchmark: int unexpectedly zero; ignoring");
            }
            host_info.p_membw = p_membw;
            print_benchmark_results();
            did_benchmarks = true;
        }

        // scale duration correction factor according to change in benchmarks.
        //
        if (host_info.p_calculated && old_p_fpops) {
            scale_duration_correction_factors(host_info.p_fpops/old_p_fpops);
        }
        host_info.p_calculated = now;
        benchmarks_running = false;
        set_client_state_dirty("CPU benchmarks");
    }
    return false;
}

void CLIENT_STATE::print_benchmark_results() {
    msg_printf(NULL, MSG_INFO, "Benchmark results:");
    msg_printf(NULL, MSG_INFO, "   Number of CPUs: %d", bm_ncpus);
    msg_printf(
        NULL, MSG_INFO, "   %.0f floating point MIPS (Whetstone) per CPU",
        host_info.p_fpops/1e6
    );
    msg_printf(
        NULL, MSG_INFO, "   %.0f integer MIPS (Dhrystone) per CPU",
        host_info.p_iops/1e6
    );
#if 0
    msg_printf(
        NULL, MSG_INFO, "Benchmark results: %.0f million bytes/sec memory bandwidth%s",
        host_info.p_membw/1e6, (host_info.p_membw_err?" [ERROR]":"")
    );
#endif
}

bool CLIENT_STATE::cpu_benchmarks_done() {
    return (host_info.p_calculated != 0);
}

// If a benchmark is nonzero, keep it.  Otherwise use default value
//
void CLIENT_STATE::cpu_benchmarks_set_defaults() {
    if (!host_info.p_fpops) host_info.p_fpops = DEFAULT_FPOPS;
    if (!host_info.p_iops) host_info.p_iops = DEFAULT_IOPS;
    if (!host_info.p_membw) host_info.p_membw = DEFAULT_MEMBW;
    if (!host_info.m_cache) host_info.m_cache = DEFAULT_CACHE;
    host_info.p_calculated = now;
}
