// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "speed_stats.h"
#include "error_numbers.h"

// # of iterations of each test to run for initial timing purposes
#define D_FLOP_ITERS    100
#define I_OP_ITERS      100
#define BANDWIDTH_ITERS 5

//#define RUN_TEST

#ifdef RUN_TEST

int main(void) {
    int cache_size;
    
    cache_size = check_cache_size(CACHE_MAX);
    
    run_test_suite(4);
    
    return 0;
}

void run_test_suite(double num_secs_per_test) {
    if (num_secs_per_test<0) {
        fprintf(stderr, "error: run_test_suite: negative num_seconds_per_test\n");
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
        12*sizeof(double)*run_mem_bandwidth_test(num_secs_per_test)/1000000
    );
}

#endif

int check_cache_size(int mem_size) {
    int i, n, index, stride, *memBlock, logStride, logCache;
    double **results;
    int steps, tsteps, csize, limit, temp, cind, sind;
    clock_t total_sec, sec;
    double secs, nanosecs, temp2;
    int not_found;
    if (mem_size<0) {
        fprintf(stderr, "error: check_cache_size: negative mem_size\n");
        return ERR_NEG;
    }
    logStride = (int)(log(STRIDE_MAX/STRIDE_MIN)/log(2))+1;
    logCache = (int)(log(CACHE_MAX/CACHE_MIN)/log(2))+1;
    
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
            sec = clock();
            do {                            // repeat until collect 1 second
                for (i = SAMPLE * stride; i != 0; i--)    {    // larger sample
                    for (index = 0; index < limit; index += stride) {
                        memBlock[index]++;                // cache access
                    }
                }
                steps++;                    // count while loop iterations
            } while (clock() < sec+(CLOCKS_PER_SEC*SECS_PER_RUN));    // until collect 1 second
            total_sec = clock()-sec;

            // Repeat empty loop to loop subtract overhead
            tsteps = 0;                        // used to match no. while iterations
            temp = 0;
            sec = clock();
            do {                            // repeat until same no. iterations as above
                for (i = SAMPLE * stride; i != 0; i--)    {    // larger sample
                    for (index = 0; index < limit; index += stride) {
                        temp += index;                // dummy code
                    }
                }
                tsteps++;                    // count while iterations
            } while (tsteps < steps);                    // until = no. iterations
            total_sec -= clock()-sec;
            
            secs = ((double)total_sec) / CLOCKS_PER_SEC;
            
            if (temp == 3) {
                printf("Howdy\n");
            }
            
            nanosecs = (double) secs * 1e9 / (steps * SAMPLE * stride * ((limit - 1) / stride + 1));
            results[sind][cind] = nanosecs;
            
            //if (stride==STRIDE_MIN) printf("\n");
            printf(
                "Size (bytes): %7d Stride (bytes): %4d read+write: %4.0f ns, %d %d\n",
                csize * sizeof (int), stride * sizeof(int), nanosecs, sind, cind
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
            printf("Level 2 Data Cache is %d KB.\n", csize*sizeof(int)/CACHE_MIN);
            not_found = 0;
        }
        if (not_found == 2 && results[0][i] > 1.5) {
            printf("Level 1 Data Cache is %d KB.\n", csize*sizeof(int)/CACHE_MIN);
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

// Run the test of double precision math speed for about num_secs seconds
//
double run_double_prec_test(double num_secs) {
    int df_test_time, df_iters;
    double df_secs;
    if (num_secs<0) {
        fprintf(stderr, "error: run_double_prec_test: negatvie num_secs\n");
        return ERR_NEG;
    }
    // Start by doing some quick timing tests for rough calibration
    df_test_time = (int)double_flop_test(D_FLOP_ITERS, 0);
    if (df_test_time <= 0) df_test_time = 1;
    df_secs = (double)df_test_time/CLOCKS_PER_SEC;

    // Calculate the # of iterations based on these tests
    df_iters = (int)(D_FLOP_ITERS*num_secs/df_secs);

    if (df_iters > D_FLOP_ITERS)  {    // no need to redo test if we already got enough
        df_test_time = (int)double_flop_test(df_iters, 0);
    } else {
        df_iters = D_FLOP_ITERS;
    }
        
    df_secs = (double)df_test_time/CLOCKS_PER_SEC;
    
    return 1000000*df_iters/df_secs;
}

// Run the test of integer math speed for about num_secs seconds
//
double run_int_test(double num_secs) {
    int int_test_time, int_iters;
    double int_secs;
    if (num_secs<0) {
        fprintf(stderr, "error: run_int_test: negative num_secs\n");
        return ERR_NEG;
    }
    // Start by doing some quick timing tests for rough calibration
    int_test_time = (int)int_op_test(I_OP_ITERS, 0);
    if (int_test_time <= 0) int_test_time = 1;
    int_secs = (double)int_test_time/CLOCKS_PER_SEC;

    // Calculate the # of iterations based on these tests
    int_iters = (int)(I_OP_ITERS*num_secs/int_secs);

    if (int_iters > I_OP_ITERS)  { // no need to redo test
        int_test_time = (int)int_op_test(int_iters, 0);
    } else {
        int_iters = I_OP_ITERS;
    }
    
    int_secs = (double)int_test_time/CLOCKS_PER_SEC;
    
    return 1000000*int_iters/int_secs;
}

// Run the test of memory bandwidth speed for about num_secs seconds
//
double run_mem_bandwidth_test(double num_secs) {
    int bw_test_time;
    double bw_secs;
    int bw_iters;
    if (num_secs<0) {
        fprintf(stderr, "error: run_mem_bandwidth_test: negative num_secs\n");
        return ERR_NEG;
    }
    // Start by doing some quick timing tests for rough calibration
    bw_test_time = (int)bandwidth_test(BANDWIDTH_ITERS, 0);
    if (bw_test_time <= 0) bw_test_time = 1;
    bw_secs = (double)bw_test_time/CLOCKS_PER_SEC;
    
    // Calculate the # of iterations based on these tests
    bw_iters = (int)(BANDWIDTH_ITERS*num_secs/bw_secs);
    
    if (bw_iters > BANDWIDTH_ITERS)  { // no need to redo test
        bw_test_time = (int)bandwidth_test(bw_iters, 0);
    } else {
        bw_iters = BANDWIDTH_ITERS;
    }

    bw_secs = (double)bw_test_time/CLOCKS_PER_SEC;
    return 1000000*bw_iters/bw_secs;
}

// One iteration == D_LOOP_ITERS (1,000,000) floating point operations
// If time_total is negative, there was an error in the calculation,
// meaning there is probably something wrong with the CPU

clock_t double_flop_test(int iterations, int print_debug) {
    double a[NUM_DOUBLES],t1,t2;
    double temp;
    clock_t time_start, time_total;
    int i,j,k,calc_error;
    if (iterations<0) {
        fprintf(stderr, "error: double_flop_test: negative iterations\n");
        return ERR_NEG;
    }
    // Initialize the array
    a[0] = 1;
    for (i=1;i<NUM_DOUBLES;i++)
        a[i] = a[i-1]/2.0;
    
    // Ideally, the array "a" will fit into cache, meaning this test doesn't
    // really include memory accesses
    time_start = clock();
    for (i=0;i<iterations;i++) {
        for (j=0;j<D_LOOP_ITERS;j+=((NUM_DOUBLES-1)*5)) {
            temp = 1;
            t1 = a[0];
            // These tests do a pretty good job of preventing optimization
            // since the result from all but one of the lines is required for the
            // next line.  At the end of each iteration through the for loop,
            // the array should be the same as when it started
           
            for (k=0;k<NUM_DOUBLES-1;k++) {
                t2 = a[k+1];
                t1 = t1 * t2;            // 1st FLOP
                temp = temp + temp;       // 2nd FLOP
                t1 = t1 * temp;          // 3rd FLOP
                t1 = t1 + t2;            // 4th FLOP
                t1 = t1 / 1.5;           // 5th FLOP
                a[k] = t1;
                t1 = t2;
            }
        }
    }
    
    // Stop the clock
    time_total = clock();
    
    // Accomodate for the possibility of clock wraparound
    if (time_total > time_start) {
        time_total -= time_start;
    } else {
        time_total = 0; // this is just a kludge
    }
    
    
    calc_error = 0;
    temp = 1;
    // Check to make sure all the values are the same as when we started
    for (i=0;i<NUM_DOUBLES;i++) {
        if ((float)a[i] != (float)temp) {
            calc_error = 1;
        }
        
        temp /= 2;
    }
    
    if (calc_error) {
        time_total *= -1;
    }
    
    if (print_debug) {
        for (i=0;i<NUM_DOUBLES;i++) {
            printf("%3d: %.50f\n", i, a[i]);
        }
    }
    
    return time_total;
}

// One iteration == 1,000,000 integer operations
// If time_total is negative, there was an error in the calculation,
// meaning there is probably something wrong with the CPU

clock_t int_op_test(int iterations, int print_debug) {
    int a[NUM_INTS], temp;
    clock_t time_start, time_total;
    int i,j,k,calc_error;
    if (iterations<0) {
        fprintf(stderr, "error: int_op_test: negative iterations\n");
        return ERR_NEG;
    }
    a[0] = 1;
    for (i=1;i<NUM_INTS;i++) {
        a[i] = 2*a[i-1];
    }
   
    time_start = clock();
    for (i=0;i<iterations;i++) {
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
    }

    // Stop the clock
    time_total = clock();
    
    // Accomodate for the possibility of clock wraparound
    if (time_total > time_start) {
        time_total -= time_start;
    } else {
        time_total = 0; // this is just a kludge
    }
    
    calc_error = 0;
    temp = 1;
    // Check to make sure all the values are the same as when we started
    for (i=0;i<NUM_INTS;i++) {
        if (a[i] != temp) {
            calc_error = 1;
        }
        
        temp *= 2;
    }
    
    if (calc_error) {
        time_total *= -1;
    }
    
    if (print_debug) {
        for (i=0;i<NUM_INTS;i++) {
            printf("%3d: %d\n", i, a[i]);
        }
    }
    
    return time_total;
}

// One iteration == Read of 6,000,000*sizeof(double), Write of 6,000,000*sizeof(double)
// If time_total is negative, there was an error in the copying,
// meaning there is probably something wrong with the CPU

clock_t bandwidth_test(int iterations, int print_debug) {
    // a, b, and c are arrays of doubles we will copy around to test memory bandwidth
    double *a, *b, *c;
    // aVal and bVal are the values of all elements of a and b.
    // These values use every other bit,
    // so that if there is a HW problem it will easily manifest itself

    double aVal, bVal;
    // Start and stop times for the clock
    clock_t time_start, time_total;
    int i,j,copy_error;
    if (iterations<0) {
        fprintf(stderr, "error: bandwidth_test: negative iterations\n");
        return ERR_NEG;
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
    
    // Start the clock
    time_start = clock();
    
    // 6 read, 6 write operations per iteration which will preserve a and b
    for (i=0;i<iterations*2;i++) {
        for (j=0;j<MEM_SIZE;j++) {
            c[j] = a[j];
            a[j] = b[j];
            b[j] = c[j];
        }
    }

    // Stop the clock
    time_total = clock();
    
    // Accomodate for the possibility of clock wraparound
    if (time_total > time_start) {
        time_total -= time_start;
    } else {
        time_total = 0; // this is just a kludge
    }
    
    copy_error = 0;
    for (i=0;i<MEM_SIZE;i++) {
        if (a[i] != aVal+i || b[i] != bVal+i) {
            copy_error = 1;
        }
    }
    
    if (copy_error) {
        time_total *= -1;
    }
    
    free(a);
    free(b);
    free(c);
    
    return time_total;
}
