// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//


#include "cpp.h"

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

#define BENCHMARK_PERIOD        (SECONDS_PER_DAY*30)
    // rerun CPU benchmarks this often (hardware may have been upgraded)

extern void guiOnBenchmarksBegin();
extern void guiOnBenchmarksEnd();

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

// benchmark a single CPU
// This takes 10-20 seconds,
//
int cpu_benchmarks(BENCHMARK_DESC* bdp) {
    HOST_INFO host_info;
    double fpop_test_secs = 3.3;
    double iop_test_secs = 3.3;
    double mem_test_secs = 3.3;
    double x, y;

    host_info.clear_host_info();
    whetstone(3.0, host_info.p_fpops);
    dhrystone(3.0, x, y);
    host_info.p_iops = y*1e6;
    host_info.p_membw = 1e9;
    //host_info.p_fpop_err = run_double_prec_test(fpop_test_secs, host_info.p_fpops);
    //host_info.p_iop_err = run_int_test(iop_test_secs, host_info.p_iops);
    //host_info.p_membw_err = run_mem_bandwidth_test(mem_test_secs, host_info.p_membw);
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

	cpu_benchmarks_start = dtime();
	msg_printf(NULL, MSG_INFO, "Running CPU benchmarks");
    if (!benchmark_descs) {
        benchmark_descs = (BENCHMARK_DESC*)calloc(
            host_info.p_ncpus, sizeof(BENCHMARK_DESC)
        );
    }
    guiOnBenchmarksBegin();
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
// flag is set or it's been a month since we last ran
//
bool CLIENT_STATE::should_run_cpu_benchmarks() {
    // Note: if skip_cpu_benchmarks we still should "run" cpu benchmarks
    // (we'll just use default values in cpu_benchmarks())
    return (
        run_cpu_benchmarks ||
        dtime() - host_info.p_calculated > BENCHMARK_PERIOD
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

    // check for timeout
    //
    if (dtime() > cpu_benchmarks_start + MAX_CPU_BENCHMARKS_SECONDS) {
        msg_printf(NULL, MSG_ERROR, "CPU benchmarks timed out, using default values");
        abort_cpu_benchmarks();
        host_info.p_fpops = DEFAULT_FPOPS;
        host_info.p_iops = DEFAULT_IOPS;
        host_info.p_membw = DEFAULT_MEMBW;
        host_info.m_cache = DEFAULT_CACHE;
        guiOnBenchmarksEnd();
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
        msg_printf(
            NULL, MSG_INFO, "Benchmark results: %.0f double precision MIPS (Whetstone)%s",
        host_info.p_fpops/1e6, (host_info.p_fpop_err?" [ERROR]":"")
        );
        msg_printf(
            NULL, MSG_INFO, "Benchmark results: %.0f integer MIPS (Dhrystone)%s",
        host_info.p_iops/1e6,  (host_info.p_iop_err?" [ERROR]":"")
        );
#if 0
        msg_printf(
            NULL, MSG_INFO, "Benchmark results: %.0f million bytes/sec memory bandwidth%s",
        host_info.p_membw/1e6, (host_info.p_membw_err?" [ERROR]":"")
        );
#endif

        host_info.p_calculated = dtime();
        guiOnBenchmarksEnd();
        benchmarks_running = false;
        set_client_state_dirty("CPU benchmarks");
    }
    return false;
}

// return true if any CPU benchmark thread/process is running
//
bool CLIENT_STATE::are_cpu_benchmarks_running() {
    return benchmarks_running;
}
