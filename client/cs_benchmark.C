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

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "cpu_benchmark.h"
#include "client_state.h"

extern void guiOnBenchmarksBegin();
extern void guiOnBenchmarksEnd();

// defaults in case benchmarks fail or time out.
// better to err on the low side so hosts don't get too much work

#define DEFAULT_FPOPS   1e7
#define DEFAULT_IOPS    1e7
#define DEFAULT_MEMBW   1e8
#define DEFAULT_CACHE   1e6

#define BENCHMARK_PERIOD        (SECONDS_PER_DAY*30)
    // rerun CPU benchmarks this often (hardware may have been upgraded)

void CLIENT_STATE::fork_run_cpu_benchmarks() {
	cpu_benchmarks_start = time(0);
	msg_printf(NULL, MSG_INFO, "Running CPU benchmarks");
#ifdef _WIN32
    cpu_benchmarks_handle = CreateThread(
        NULL, 0, win_cpu_benchmarks, NULL, 0, &cpu_benchmarks_id
    );
#else
    cpu_benchmarks_id = fork();
    if (cpu_benchmarks_id == 0) {
        _exit(cpu_benchmarks());
    }
#endif
}

// Returns true if CPU benchmarks should be run:
// flag is set or it's been a month since we last ran
//
bool CLIENT_STATE::should_run_cpu_benchmarks() {
    // Note: we if skip_cpu_benchmarks we still should "run" cpu benchmarks
    // (we'll just use default values in cpu_benchmarks())
    return (
        run_cpu_benchmarks ||
        (difftime(time(0), (time_t)host_info.p_calculated) > BENCHMARK_PERIOD)
    );
}

#ifdef _WIN32
DWORD WINAPI CLIENT_STATE::win_cpu_benchmarks(LPVOID) {
    return gstate.cpu_benchmarks();
}
#endif

// gets info about the host
// NOTE: this locks up the process for 10-20 seconds,
// so it should be called very seldom
//
int CLIENT_STATE::cpu_benchmarks() {
    HOST_INFO host_info;
    FILE* finfo;
    double fpop_test_secs = 3.3;
    double iop_test_secs = 3.3;
    double mem_test_secs = 3.3;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_MEASUREMENT);
    scope_messages.printf("CLIENT_STATE::cpu_benchmarks(): Running CPU benchmarks.\n");

	guiOnBenchmarksBegin();

    clear_host_info(host_info);
    ++log_messages;
    if (skip_cpu_benchmarks) {
        scope_messages.printf("CLIENT_STATE::cpu_benchmarks(): Skipping CPU benchmarks.\n");
        host_info.p_fpops = DEFAULT_FPOPS;
        host_info.p_iops = DEFAULT_IOPS;
        host_info.p_membw = DEFAULT_MEMBW;
        host_info.m_cache = DEFAULT_CACHE;
    } else {
        scope_messages.printf(
            "CLIENT_STATE::cpu_benchmarks(): Running floating point test for about %.1f seconds.\n",
            fpop_test_secs
        );
        host_info.p_fpop_err = run_double_prec_test(fpop_test_secs, host_info.p_fpops);

        scope_messages.printf(
            "CLIENT_STATE::cpu_benchmarks(): Running integer test for about %.1f seconds.\n",
            iop_test_secs
        );
        host_info.p_iop_err = run_int_test(iop_test_secs, host_info.p_iops);

        scope_messages.printf(
            "CLIENT_STATE::cpu_benchmarks(): Running memory bandwidth test for about %.1f seconds.\n",
            mem_test_secs
        );
        host_info.p_membw_err = run_mem_bandwidth_test(mem_test_secs, host_info.p_membw);

        // need to check cache!!
        host_info.m_cache = 1e6;

        msg_printf(
            NULL, MSG_INFO, "Benchmark results: %.0f million floating point ops/sec%s",
           host_info.p_fpops/1e6, (host_info.p_fpop_err?" [ERROR]":"")
        );
        msg_printf(
            NULL, MSG_INFO, "Benchmark results: %.0f million integer ops/sec%s",
           host_info.p_iops/1e6,  (host_info.p_iop_err?" [ERROR]":"")
        );
        msg_printf(
            NULL, MSG_INFO, "Benchmark results: %.0f million bytes/sec memory bandwidth%s",
           host_info.p_membw/1e6, (host_info.p_membw_err?" [ERROR]":"")
        );
    }

    host_info.p_calculated = (double)time(0);
    finfo = boinc_fopen(CPU_BENCHMARKS_FILE_NAME, "w");
    if(!finfo) return ERR_FOPEN;
    host_info.write_cpu_benchmarks(finfo);
    fclose(finfo);
	guiOnBenchmarksEnd();
    --log_messages;
    return 0;
}

// checks if the CPU benchmarks are running
//
int CLIENT_STATE::check_cpu_benchmarks() {
    FILE* finfo;
    int retval;

    if (cpu_benchmarks_id) {
#ifdef _WIN32
        DWORD exit_code = 0;
        GetExitCodeThread(cpu_benchmarks_handle, &exit_code);
        if(exit_code == STILL_ACTIVE) {
            if(time(NULL) > cpu_benchmarks_start + MAX_CPU_BENCHMARKS_SECONDS) {
                msg_printf(NULL, MSG_ERROR, "CPU benchmarks timed out, using default values");
                TerminateThread(cpu_benchmarks_handle, 0);
                CloseHandle(cpu_benchmarks_handle);
                host_info.p_fpops = DEFAULT_FPOPS;
                host_info.p_iops = DEFAULT_IOPS;
                host_info.p_membw = DEFAULT_MEMBW;
                host_info.m_cache = DEFAULT_CACHE;
                cpu_benchmarks_id = 0;
                return CPU_BENCHMARKS_ERROR;
            }
            return CPU_BENCHMARKS_RUNNING;
        }
        CloseHandle(cpu_benchmarks_handle);
		guiOnBenchmarksEnd();
#else
        int exit_code = 0;
        retval = waitpid(cpu_benchmarks_id, &exit_code, WNOHANG);
        if(retval == 0) {
            if((unsigned int)time(NULL) > cpu_benchmarks_start + MAX_CPU_BENCHMARKS_SECONDS) {
                msg_printf(NULL, MSG_ERROR, "CPU benchmarks timed out, using default values");
                kill(cpu_benchmarks_id, SIGKILL);
                host_info.p_fpops = DEFAULT_FPOPS;
                host_info.p_iops = DEFAULT_IOPS;
                host_info.p_membw = DEFAULT_MEMBW;
                host_info.m_cache = DEFAULT_CACHE;
                cpu_benchmarks_id = 0;
                return CPU_BENCHMARKS_ERROR;
            }
            return CPU_BENCHMARKS_RUNNING;
        }
#endif
        cpu_benchmarks_id = 0;
        msg_printf(NULL, MSG_INFO, "CPU benchmarks complete");
        finfo = fopen(CPU_BENCHMARKS_FILE_NAME, "r");
        if (!finfo) {
            msg_printf(NULL, MSG_ERROR, "Can't open CPU benchmark file, using default values");
            host_info.p_fpops = DEFAULT_FPOPS;
            host_info.p_iops = DEFAULT_IOPS;
            host_info.p_membw = DEFAULT_MEMBW;
            host_info.m_cache = DEFAULT_CACHE;
            return CPU_BENCHMARKS_ERROR;
        }
        retval = host_info.parse_cpu_benchmarks(finfo);
        fclose(finfo);
        if (retval) return CPU_BENCHMARKS_ERROR;
        boinc_delete_file(CPU_BENCHMARKS_FILE_NAME);
        return CPU_BENCHMARKS_COMPLETE;
    }
    return CPU_BENCHMARKS_NOT_RUNNING;
}

