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

// Manage a (perhaps multi-processor) benchmark.
// Because of hyperthreaded CPUs we can't just benchmark 1 CPU;
// we have to run parallel benchmarks,
// and we have to ensure that they run more or less concurrently.
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
#endif

#ifndef _WIN32
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <csignal>
#if HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

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
#include "client_state.h"

// defaults in case benchmarks fail or time out.
// better to err on the low side so hosts don't get too much work

#define DEFAULT_FPOPS   1e7
#define DEFAULT_IOPS    1e7
#define DEFAULT_MEMBW   1e8
#define DEFAULT_CACHE   1e6

#define FP_START    2
#define FP_END      22
#define INT_START   37
#define INT_END     57

#define BM_FP_INIT  0
#define BM_FP       1
#define BM_INT_INIT 2
#define BM_INT      3
#define BM_SLEEP    4
#define BM_DONE     5
static int bm_state;

#define BENCHMARK_PERIOD        (SECONDS_PER_DAY*5)
    // rerun CPU benchmarks this often (hardware may have been upgraded)

// represents a benchmark thread/process, in progress or completed
//
struct BENCHMARK_DESC {
    int ordinal;
    HOST_INFO host_info;
    bool done;
    bool error;
#ifdef _WIN32
    HANDLE handle;
    DWORD pid;
#else
    char filename[256];
    PROCESS_ID pid;
#endif
};

static BENCHMARK_DESC* benchmark_descs=0;
static bool benchmarks_running=false;    // at least 1 benchmark thread running
static double cpu_benchmarks_start;

char *file_names[2] = {"do_fp", "do_int"};

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
        boinc_sleep(0.1);
    }
}

bool benchmark_time_to_stop(int which) {
    if (boinc_file_exists(file_names[which])) {
        return false;
    }
    return true;
}

// benchmark a single CPU
// This takes 60-120 seconds,
//
int cpu_benchmarks(BENCHMARK_DESC* bdp) {
    HOST_INFO host_info;
    double x, y;

    host_info.clear_host_info();
    whetstone(host_info.p_fpops);
    dhrystone(x, y);
    host_info.p_iops = y*1e6;
    host_info.p_membw = 1e9;
    host_info.m_cache = 1e6;    // TODO: measure the cache

#ifdef _WIN32
    bdp->host_info = host_info;
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

void CLIENT_STATE::start_cpu_benchmarks() {
    int i;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_MEASUREMENT);

    if (skip_cpu_benchmarks) {
        scope_messages.printf("CLIENT_STATE::cpu_benchmarks(): Skipping CPU benchmarks.\n");
        host_info.p_fpops = DEFAULT_FPOPS;
        host_info.p_iops = DEFAULT_IOPS;
        host_info.p_membw = DEFAULT_MEMBW;
        host_info.m_cache = DEFAULT_CACHE;
        return;
    }

    bm_state = BM_FP_INIT;
    remove_benchmark_file(BM_TYPE_FP);
    remove_benchmark_file(BM_TYPE_INT);
	cpu_benchmarks_start = dtime();

	msg_printf(NULL, MSG_INFO, "Running CPU benchmarks");
    if (!benchmark_descs) {
        benchmark_descs = (BENCHMARK_DESC*)calloc(
            host_info.p_ncpus, sizeof(BENCHMARK_DESC)
        );
    }
    benchmarks_running = true;

    for (i=0; i<host_info.p_ncpus; i++) {
        benchmark_descs[i].ordinal = i;
        benchmark_descs[i].done = false;
        benchmark_descs[i].error = false;
#ifdef _WIN32
        benchmark_descs[i].handle = CreateThread(
            NULL, 0, win_cpu_benchmarks, benchmark_descs+i, 0,
            &benchmark_descs[i].pid
        );
#else
        sprintf(benchmark_descs[i].filename, "%s_%d.xml", CPU_BENCHMARKS_FILE_NAME, i);
        PROCESS_ID pid = fork();
        if (pid == 0) {
            _exit(cpu_benchmarks(benchmark_descs+i));
        } else {
            benchmark_descs[i].pid = pid;
        }
#endif
    }
}

// Returns true if CPU benchmarks should be run:
// flag is set or it's been 5 days since we last ran
//
bool CLIENT_STATE::should_run_cpu_benchmarks() {
    // Note: if skip_cpu_benchmarks we still should "run" cpu benchmarks
    // (we'll just use default values in cpu_benchmarks())
    //
    if (activities_suspended) return false;
    return (
        (run_cpu_benchmarks ||
        dtime() - host_info.p_calculated > BENCHMARK_PERIOD)
    );
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
    for (i=0; i<host_info.p_ncpus; i++) {
        abort_benchmark(benchmark_descs[i]);
    }
}

// benchmark poll routine.  Called every second
//
bool CLIENT_STATE::cpu_benchmarks_poll() {
    int i;
    if (!benchmarks_running) return false;

    double now = dtime();

    // Send heartbeat to all active tasks so they know we are alive
    // and well.
    active_tasks.send_heartbeats();

    // do transitions through benchmark states
    //
    switch (bm_state) {
    case BM_FP_INIT:
        if (now - cpu_benchmarks_start > FP_START) {
            make_benchmark_file(BM_TYPE_FP);
            bm_state = BM_FP;
        }
        return false;
    case BM_FP:
        if (now - cpu_benchmarks_start > FP_END) {
            remove_benchmark_file(BM_TYPE_FP);
            bm_state = BM_INT_INIT;
        }
        return false;
    case BM_INT_INIT:
        if (now - cpu_benchmarks_start > INT_START) {
            make_benchmark_file(BM_TYPE_INT);
            bm_state = BM_INT;
        }
        return false;
    case BM_INT:
        if (now - cpu_benchmarks_start > INT_END) {
            remove_benchmark_file(BM_TYPE_INT);
            bm_state = BM_SLEEP;
        }
        return false;
    case BM_SLEEP:
        boinc_sleep(2.0);
        bm_state = BM_DONE;
        return false;
    }

    // check for timeout
    //
    if (now > cpu_benchmarks_start + MAX_CPU_BENCHMARKS_SECONDS) {
        msg_printf(NULL, MSG_ERROR, "CPU benchmarks timed out, using default values");
        abort_cpu_benchmarks();
        host_info.p_fpops = DEFAULT_FPOPS;
        host_info.p_iops = DEFAULT_IOPS;
        host_info.p_membw = DEFAULT_MEMBW;
        host_info.m_cache = DEFAULT_CACHE;
        benchmarks_running = false;
        set_client_state_dirty("CPU benchmarks");
    }
    int ndone = 0;
    bool had_error = false;
    for (i=0; i<host_info.p_ncpus; i++) {
        if (!benchmark_descs[i].done) {
            check_benchmark(benchmark_descs[i]);
        }
        if (benchmark_descs[i].done) {
            ndone ++;
            if (benchmark_descs[i].error) had_error = true;
        }
    }
    if (ndone == host_info.p_ncpus) {
        if (had_error) {
            msg_printf(NULL, MSG_ERROR, "CPU benchmarks error");
            host_info.p_fpops = DEFAULT_FPOPS;
            host_info.p_iops = DEFAULT_IOPS;
            host_info.p_membw = DEFAULT_MEMBW;
            host_info.m_cache = DEFAULT_CACHE;
        } else {
            host_info.p_fpops = 0;
            host_info.p_iops = 0;
            host_info.p_membw = 0;
            host_info.m_cache = 0;
            for (i=0; i<host_info.p_ncpus; i++) {
                host_info.p_fpops += benchmark_descs[i].host_info.p_fpops;
                host_info.p_iops += benchmark_descs[i].host_info.p_iops;
                host_info.p_membw += benchmark_descs[i].host_info.p_membw;
                host_info.m_cache += benchmark_descs[i].host_info.m_cache;
            }
            host_info.p_fpops /= host_info.p_ncpus;
            host_info.p_iops /= host_info.p_ncpus;
            host_info.p_membw /= host_info.p_ncpus;
            host_info.m_cache /= host_info.p_ncpus;
        }
        msg_printf(NULL, MSG_INFO, "Benchmark results:");
        msg_printf(NULL, MSG_INFO, "   Number of CPUs: %d", host_info.p_ncpus);
        msg_printf(
            NULL, MSG_INFO, "   %.0f double precision MIPS (Whetstone) per CPU",
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

        host_info.p_calculated = now;
        benchmarks_running = false;
	    msg_printf(NULL, MSG_INFO, "Finished CPU benchmarks");
        set_client_state_dirty("CPU benchmarks");
    }
    return false;
}

// return true if any CPU benchmark thread/process is running
//
bool CLIENT_STATE::are_cpu_benchmarks_running() {
    return benchmarks_running;
}

const char *BOINC_RCSID_97ee090db0 = "$Id$";
