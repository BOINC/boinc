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

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#endif

#include "error_numbers.h"
#include "message.h"

#include "cpu_benchmark.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>    // for timing
void CALLBACK stop_benchmark(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
UINT speed_timer_id;
#else
void stop_benchmark(int a);
#endif

#define D_LOOP_ITERS		1000000
#define I_LOOP_ITERS		1000000
#define MEM_SIZE			1000000

#define NUM_DOUBLES       28
#define NUM_INTS          28

#define CACHE_MIN 1024			// smallest cache (in words)
#define CACHE_MAX 512*1024		// largest cache
#define STRIDE_MIN 1            // smallest stride (in words)
#define STRIDE_MAX 128          // largest stride
#define SAMPLE 10			    // to get a larger time sample
#define SECS_PER_RUN 0.2

// run_benchmark is volatile so the test loops will notice changes
// made by stop_test
//
static volatile bool run_benchmark;

static double cpu_time() {
    return (double)clock()/(double)CLOCKS_PER_SEC;
}

static double cpu_time_diff(double start, double end) {
    // take wraparound into account
    //
    while (end < start) {
        end += (double)(0x80000000)/(double)CLOCKS_PER_SEC;
    }
    return end-start;
}

//#define RUN_TEST

#ifdef RUN_TEST

int main(void) {
    int cache_size;
    
    cache_size = check_cache_size(CACHE_MAX);
    
    run_benchmark_suite(4);
    
    return 0;
}

void run_benchmark_suite(double num_secs_per_test) {
    if (num_secs_per_test<0) {
        msg_printf(NULL, MSG_ERROR, "error: run_benchmark_suite: negative num_seconds_per_test\n");
    }
    printf(
        "Running tests.  This will take about %.1lf seconds.\n\n",
        num_secs_per_test*3
    );
    
    printf(
        "Speed: %.5lf million flops/sec\n\n",
        run_double_prec_test(num_secs_per_test)/1000000
    );
    printf(
        "Speed: %.5lf million integer ops/sec\n\n",
        run_int_test(num_secs_per_test)/1000000
    );
    printf(
        "Speed: %.5lf MB/sec\n\n",
        run_mem_bandwidth_test(num_secs_per_test)/1000000
    );
}

#endif

int check_cache_size(int mem_size) {
    int i, n, index, stride, *memBlock, logStride, logCache;
    double **results;
    int steps, tsteps, csize, limit, temp, cind, sind;
    double start, end, elapsed;
    //clock_t total_sec, sec;
    double nanosecs, temp2;
    int not_found;
    if (mem_size<0) {
        msg_printf(NULL, MSG_ERROR, "check_cache_size: negative mem_size\n");
        return ERR_NEG;
    }
    logStride = (int)(log((double)(STRIDE_MAX/STRIDE_MIN))/log(2.0))+1;
    logCache = (int)(log((double)(CACHE_MAX/CACHE_MIN))/log(2.0))+1;
    
    printf("Test will take about %.2f seconds.\n", SECS_PER_RUN*logStride*logCache);
    results = (double **)malloc(sizeof(double *)*logStride);
    
    for (i=0;i<logStride;i++) {
        results[i] = (double *)malloc(sizeof(double)*logCache);
        for (n=0;n<logCache;n++) {
            results[i][n] = 1.0;
        }
    }
    
    printf("|");
    for (i=0;i<logCache;i++) {
        printf("-");
    }
    
    printf("|\n");
    memBlock = (int *)malloc(sizeof(int)*mem_size);
    printf(" ");
    
    for (csize=CACHE_MIN,cind=0;csize<=CACHE_MAX;csize*=2,cind++) {
        for (stride = STRIDE_MIN,sind=0; stride<=STRIDE_MAX; stride*=2,sind++) {
            limit = csize - stride + 1;                // cache size this loop

            steps = 0;
            start = cpu_time();
            do {                            // repeat until collect 1 second
                for (i = SAMPLE * stride; i != 0; i--)    {    // larger sample
                    for (index = 0; index < limit; index += stride) {
                        memBlock[index]++;                // cache access
                    }
                }
                steps++;                    // count while loop iterations
            } while (cpu_time_diff(start, cpu_time()) < SECS_PER_RUN);    // until collect 1 second
            end = cpu_time();
            elapsed = cpu_time_diff(start, end);
            //total_sec = clock()-sec;

            // Repeat empty loop to loop subtract overhead
            tsteps = 0;                        // used to match no. while iterations
            temp = 0;
            start = cpu_time();
            do {                            // repeat until same no. iterations as above
                for (i = SAMPLE * stride; i != 0; i--)    {    // larger sample
                    for (index = 0; index < limit; index += stride) {
                        temp += index;                // dummy code
                    }
                }
                tsteps++;                    // count while iterations
            } while (tsteps < steps);                    // until = no. iterations
            end = cpu_time();
            elapsed -= cpu_time_diff(start, end);
                                    
            nanosecs = elapsed * 1e9 / (steps * SAMPLE * stride * ((limit - 1) / stride + 1));
            results[sind][cind] = nanosecs;
            
            //if (stride==STRIDE_MIN) printf("\n");
            printf(
                "Size (bytes): %7d Stride (bytes): %4d read+write: %4.0f ns, %d %d\n",
                (int)(csize * sizeof (int)), (int)(stride * sizeof(int)), nanosecs, sind, cind
           );
        }
        printf(".");
        fflush(stdout);
    }
    printf("\n");
    
    for (i=0;i<logStride;i++) {
        for (n=0;n<logCache;n++) {
            printf ("%4.0f ", results[i][n]);
        }
        printf("\n");
    }
    
    for (i=0;i<logStride;i++) {
        for (n=logCache;n>0;n--) {
            results[i][n] /= results[i][n-1];
        }
    }
    
    for (i=0;i<logCache;i++) {
        temp2 = 0;
        for (n=0;n<logStride;n++) {
            temp2 += results[n][i];
        }
        results[0][i] = temp2/logStride;
    }
    
    printf("\n");
    for (i=0;i<logStride;i++) {
        for (n=1;n<logCache;n++) {
            printf ("%1.3f ", results[i][n]);
        }
        printf("\n");
    }
    
    csize=CACHE_MIN;
    i = 1;
    not_found = 2;
    while(not_found && i < logCache) {
        if (not_found == 1 && results[0][i] > 1.5) {
            printf("Level 2 Data Cache is %d KB.\n", (int)(csize*sizeof(int)/CACHE_MIN));
            not_found = 0;
        }
        if (not_found == 2 && results[0][i] > 1.5) {
            printf("Level 1 Data Cache is %d KB.\n", (int)(csize*sizeof(int)/CACHE_MIN));
            not_found = 1;
        }
        i++;
        csize *= 2;
    }
    
    free(memBlock);
    for (i=0;i<logStride;i++)
        free(results[i]);
    
    free(results);
    
    return 0;
}

// Run the test of double precision math speed for num_secs seconds
//
int run_double_prec_test(double num_secs, double &flops_per_sec) {
    int retval;
    
    if (num_secs<0) {
        msg_printf(NULL, MSG_ERROR, "run_double_prec_test: negative num_secs\n");
        return ERR_NEG;
    }
    
    // Setup a timer to interrupt the tests in num_secs
    retval = set_benchmark_timer(num_secs);
    if (retval) return retval;

    retval = (int)double_flop_test(0, flops_per_sec, 0);
        
    destroy_benchmark_timer();
    
    return retval;
}

// Run the test of integer math speed for num_secs seconds
//
int run_int_test(double num_secs, double &iops_per_sec) {
    int retval;
    
    if (num_secs<0) {
        msg_printf(NULL, MSG_ERROR, "run_int_test: negative num_secs\n");
        return ERR_NEG;
    }
    
    // Setup a timer to interrupt the tests in num_secs
    retval = set_benchmark_timer(num_secs);
    if (retval) return retval;

    retval = (int)int_op_test(0, iops_per_sec, 0);
    
    destroy_benchmark_timer();
    
    return retval;
}

// Run the test of memory bandwidth speed for num_secs seconds
//
int run_mem_bandwidth_test(double num_secs, double &bytes_per_sec) {
    int retval;
    
    if (num_secs<0) {
        msg_printf(NULL, MSG_ERROR, "run_mem_bandwidth_test: negative num_secs\n");
        return ERR_NEG;
    }
    
    // Setup a timer to interrupt the tests in num_secs
    retval = set_benchmark_timer(num_secs);
    if (retval) return retval;
    
    retval = (int)bandwidth_test(0, bytes_per_sec, 0);
    
    destroy_benchmark_timer();
    
    return retval;
}

int double_flop_test(int iterations, double &flops_per_sec, int print_debug) {
    double a[NUM_DOUBLES], b[NUM_DOUBLES], dp, temp;
    int i, n, j, error = 0;
    double actual_iters;
    double start, end, elapsed;
    
    if (iterations<0) {
        msg_printf(NULL, MSG_ERROR, "double_flop_test: negative iterations\n");
        return ERR_NEG;
    }
    
    // If iterations is 0, assume we're using the timer
    //
    if (iterations == 0) {
        run_benchmark = true;
        iterations = 200000000;
    }
    
    a[0] = b[0] = 1.0;
    
    for (i=1;i<NUM_DOUBLES;i++) {
        a[i] = a[i-1] / 2.0;
        b[i] = b[i-1] * 2.0;
    }
    
    actual_iters = 0;
    
    start = cpu_time();
    
    for (n=0; (n<iterations)&&run_benchmark; n++) {

        // do roughly 1 million FP ops
        //
        for (j=0; j<D_LOOP_ITERS; j+=((NUM_DOUBLES*4)+1)) {
            dp = 0;
            for (i=0;i<NUM_DOUBLES;i++) {    // 2*NUM_DOUBLES flops
                dp += a[i]*b[i];             // 2 flops
            }
            dp /= (float)NUM_DOUBLES;        // 1 flop
            for (i=0;i<NUM_DOUBLES;i++) {    // 2*NUM_DOUBLES flops
                a[i] *= dp;                  // 1 flop
                b[i] *= dp;                  // 1 flop
            }
        }

        actual_iters++;
    }
    
    end = cpu_time();
    elapsed = cpu_time_diff(start, end);
        
    flops_per_sec = D_LOOP_ITERS*actual_iters/elapsed;
    
    // Check to make sure all the values are the same as when we started
    //
    temp = 1;
    for (i=0;i<NUM_DOUBLES;i++) {
        if ((double)a[i] != (float)temp) error = ERR_BENCHMARK_FAILED;
        temp /= 2;
    }
    
    if (print_debug) {
        for (i=0;i<NUM_DOUBLES;i++) {
            printf("%3d: %.50f\n", i, a[i]);
        }
    }
    
    return error;
}

// One iteration == 1,000,000 integer operations
// If time_total is negative, there was an error in the calculation,
// meaning there is probably something wrong with the CPU

int int_op_test(int iterations, double &iops_per_sec, int print_debug) {
    int a[NUM_INTS], temp;
    double actual_iters;
    double start, end, elapsed;
    int i, j, k, error = 0;

    if (iterations<0) {
        msg_printf(NULL, MSG_ERROR, "int_op_test: negative iterations\n");
        return ERR_NEG;
    }
    
    // If iterations is 0, assume we're using the timer
    if (iterations == 0) {
        run_benchmark = true;
        iterations = 200000000;
    }

    a[0] = 1;
    for (i=1;i<NUM_INTS;i++) {
        a[i] = 2*a[i-1];
    }
   
    actual_iters = 0;
   
    start = cpu_time();
    for (i=0;(i<iterations) && run_benchmark;i++) {
        // The contents of the array "a" should be the same at the
        // beginning and end of each loop iteration.  Most compilers will
        // partially unroll the individual loops within this one, so
        // those integer operations (incrementing k) are not counted
        for (j=0;j<I_LOOP_ITERS/(NUM_INTS*9);j++) {
            for (k=0;k<NUM_INTS;k++) {
                a[k] *= 3;   // 1 int ops
            }
            for (k=NUM_INTS-1;k>=0;k--) {
                a[k] += 6;   // 2 int ops
            }
            for (k=0;k<NUM_INTS;k++) {
                a[k] /= 3;   // 3 int ops
            }
            for (k=NUM_INTS-1;k>=0;k--) {
                a[k] -= 2;   // 4 int ops
            }
            for (k=NUM_INTS-1;k>0;k--) {
                a[k] -= a[k-1];  // 5 int ops
            }
            for (k=1;k<NUM_INTS;k++) {
                a[k] = 2*a[k-1];   // 6 int ops
            }
            for (k=NUM_INTS-1;k>0;k--) {
                if (a[k-1] != 0)    // 7 int ops
                    a[k] /= a[k-1];    // 8 int ops
            }
            for (k=1;k<NUM_INTS;k++) {
                a[k] = 2*a[k-1];    // 9 int ops
            }
        }
        actual_iters++;
    }

    end = cpu_time();
    elapsed = cpu_time_diff(start, end);
    
    iops_per_sec = I_LOOP_ITERS*actual_iters/elapsed;
    
    // Check to make sure all the values are the same as when we started
    //
    temp = 1;
    for (i=0;i<NUM_INTS;i++) {
        if (a[i] != temp) error = ERR_BENCHMARK_FAILED;
        temp *= 2;
    }
    
    if (print_debug) {
        for (i=0;i<NUM_INTS;i++) {
            printf("%3d: %d\n", i, a[i]);
        }
    }
    
    return error;
}

// If return value is negative, there was an error in the copying,
// meaning there is probably something wrong with the CPU
//
int bandwidth_test(int iterations, double &bytes_per_sec, int print_debug) {
    // a, b, and c are arrays of doubles we will copy around to test memory bandwidth
    double *a, *b, *c;
    // aVal and bVal are the values of all elements of a and b.
    double aVal, bVal;
    double start, end, elapsed;
    int i, j, n, error = 0;
    double actual_iters;

    if (iterations<0) {
        msg_printf(NULL, MSG_ERROR, "bandwidth_test: negative iterations\n");
        return ERR_NEG;
    }
    
    // If iterations is 0, assume we're using the timer
    if (iterations == 0) {
        run_benchmark = true;
        iterations = 200000000;
    }
    
    // These are doubles in order to make full use of bus and instruction bandwidth
    a = (double *)malloc(MEM_SIZE * sizeof(double));
    b = (double *)malloc(MEM_SIZE * sizeof(double));
    c = (double *)malloc(MEM_SIZE * sizeof(double));
    
    // These values use all the bits in a floating point number (Investigate these values)
    aVal = (-2.0/3.0)*pow(2.0,-341.0);
    bVal = (1.0/3.0)*pow(2.0,342.0);
    
    // We add i to each value to prevent compiler optimizations of the copy
    for (i=0;i<MEM_SIZE;i++) {
        a[i] = aVal+i; b[i] = bVal+i; c[i] = 1.0;
    }
    
    actual_iters = 0;
    
    start = cpu_time();
    
    // One iteration == Read of 6,000,000*sizeof(double), Write of 6,000,000*sizeof(double)
    // 6 read, 6 write operations per iteration which will preserve a and b
    for (i=0;(i<iterations) && run_benchmark;i++) {
        for (n=0;n<2;n++) {
            for (j=0;j<MEM_SIZE;j++) {
                c[j] = a[j];
                a[j] = b[j];
                b[j] = c[j];
            }
        }
        actual_iters++;
    }

    end = cpu_time();
    elapsed = cpu_time_diff(start, end);
        
    bytes_per_sec = 2.0*6.0*MEM_SIZE*actual_iters*sizeof(double)/elapsed;
    
    for (i=0;i<MEM_SIZE;i++) {
        if (a[i] != aVal+i || b[i] != bVal+i) error = ERR_BENCHMARK_FAILED;
    }
    
    free(a);
    free(b);
    free(c);
    
    return error;
}

int set_benchmark_timer(double num_secs) {
    run_benchmark = true;
#ifdef _WIN32
    speed_timer_id = timeSetEvent( (int)(num_secs*1000),
        (int)(num_secs*1000), stop_benchmark, NULL, TIME_ONESHOT );
    if (speed_timer_id == NULL) return ERR_TIMER_INIT;
#else
    itimerval value;
    sighandler_t sig_retval;
    int retval;

    sig_retval = signal(SIGALRM, stop_benchmark);
    if (sig_retval == SIG_ERR) return ERR_TIMER_INIT;
    value.it_value.tv_sec = (int)num_secs;
    value.it_value.tv_usec = ((int)(num_secs*1000000))%1000000;
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_REAL, &value, NULL);
    if (retval) return ERR_TIMER_INIT;
#endif

    return 0;
}

int destroy_benchmark_timer() {
#ifdef _WIN32
    timeKillEvent(speed_timer_id);
#endif
    return 0;
}

#ifdef _WIN32
void CALLBACK stop_benchmark(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
#else
void stop_benchmark(int a) {
#endif
    run_benchmark = false;
}
