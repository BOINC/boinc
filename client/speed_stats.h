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

//#define RUN_TEST

#define THOUSAND    1000
#define MILLION     THOUSAND*THOUSAND

#define D_LOOP_ITERS			1*MILLION
#define I_LOOP_ITERS			1*MILLION
#define MEM_SIZE			1*MILLION

#define NUM_DOUBLES       28
#define NUM_INTS          28

#define CACHE_MIN 1024			// smallest cache (in words)
#define CACHE_MAX 512*1024		// largest cache
#define STRIDE_MIN 1   // smallest stride (in words)
#define STRIDE_MAX 128 // largest stride
#define SAMPLE 10			// to get a larger time sample
#define SECS_PER_RUN 0.2

#define MAX_CPU_BENCHMARKS_SECONDS	60
#define CPU_BENCHMARKS_RUNNING		0
#define CPU_BENCHMARKS_COMPLETE		1
#define CPU_BENCHMARKS_NOT_RUNNING	2	
#define CPU_BENCHMARKS_ERROR		3

int check_cache_size( int mem_size );
int double_flop_test( int iterations, double &flops_per_sec, int print_debug );
int int_op_test( int iterations, double &iops_per_sec, int print_debug );
int bandwidth_test( int iterations, double &bytes_per_sec, int print_debug );
void run_test_suite( double num_secs_per_test );
int run_double_prec_test( double num_secs, double &flops_per_sec );
int run_int_test( double num_secs, double &iops_per_sec );
int run_mem_bandwidth_test( double num_secs, double &bytes_per_sec );
int set_test_timer(double num_secs);
int destroy_test_timer();


