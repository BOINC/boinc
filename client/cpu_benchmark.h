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

#define MAX_CPU_BENCHMARKS_SECONDS	300
#define CPU_BENCHMARKS_RUNNING		0
#define CPU_BENCHMARKS_COMPLETE		1
#define CPU_BENCHMARKS_NOT_RUNNING	2	
#define CPU_BENCHMARKS_ERROR		3

#define BM_TYPE_FP       0
#define BM_TYPE_INT      1

extern int check_cache_size(int mem_size);
extern int double_flop_test(
    int iterations, double &flops_per_sec, int print_debug
);
extern int int_op_test(int iterations, double &iops_per_sec, int print_debug);
extern int bandwidth_test(
    int iterations, double &bytes_per_sec, int print_debug
);
extern void run_benchmark_suite(double num_secs_per_test);
extern int run_double_prec_test(double num_secs, double &flops_per_sec);
extern int run_int_test(double num_secs, double &iops_per_sec);
extern int run_mem_bandwidth_test(double num_secs, double &bytes_per_sec);
extern int set_benchmark_timer(double num_secs);
extern int destroy_benchmark_timer();

extern void dhrystone(double& dps, double& vax_mips);
extern void whetstone(double& flops);
extern void benchmark_wait_to_start(int which);
extern bool benchmark_time_to_stop(int which);
