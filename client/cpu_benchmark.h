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
