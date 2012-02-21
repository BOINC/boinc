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

// Memory bandwidth benchmark derived from STREAM.  Original copyright
// notice follows.  Notice that we cannot call our results "STREAM
// benchmark results"
// Original Copyright Notice:
/*-----------------------------------------------------------------------*/
/* Program: Stream                                                       */
/* Revision: Id: stream.c,v 5.6 2005/10/04 00:19:59 mccalpin             */
/* Original code developed by John D. McCalpin                           */
/* Programmers: John D. McCalpin                                         */
/*              Joe R. Zagar                                             */
/*                                                                       */
/* This program measures memory transfer rates in MB/s for simple        */
/* computational kernels coded in C.                                     */
/*-----------------------------------------------------------------------*/
/* Copyright 1991-2005: John D. McCalpin                                 */
/*-----------------------------------------------------------------------*/
/* License:                                                              */
/*  1. You are free to use this program and/or to redistribute           */
/*     this program.                                                     */
/*  2. You are free to modify this program for your own use,             */
/*     including commercial use, subject to the publication              */
/*     restrictions in item 3.                                           */
/*  3. You are free to publish results obtained from running this        */
/*     program, or from works that you derive from this program,         */
/*     with the following limitations:                                   */
/*     3a. In order to be referred to as "STREAM benchmark results",     */
/*         published results must be in conformance to the STREAM        */
/*         Run Rules, (briefly reviewed below) published at              */
/*         http://www.cs.virginia.edu/stream/ref.html                    */
/*         and incorporated herein by reference.                         */
/*         As the copyright holder, John McCalpin retains the            */
/*         right to determine conformity with the Run Rules.             */
/*     3b. Results based on modified source code or on runs not in       */
/*         accordance with the STREAM Run Rules must be clearly          */
/*         labelled whenever they are published.  Examples of            */
/*         proper labelling include:                                     */
/*         "tuned STREAM benchmark results"                              */
/*         "based on a variant of the STREAM benchmark code"             */
/*         Other comparable, clear and reasonable labelling is           */
/*         acceptable.                                                   */
/*     3c. Submission of results to the STREAM benchmark web site        */
/*         is encouraged, but not required.                              */
/*  4. Use of this program or creation of derived works based on this    */
/*     program constitutes acceptance of these licensing restrictions.   */
/*  5. Absolutely no warranty is expressed or implied.                   */
/*-----------------------------------------------------------------------*/
# include <cstdio>
# include <cmath>
# include <float.h>
# include <climits>
# include <sys/time.h>
# include <cstdlib>
# include <algorithm>

static int N=64;
static const int NTIMES(10);
static const int OFFSET(0);

static double avgtime[4] = {0}, maxtime[4] = {0},
                           mintime[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};

static char *label[4] = {"Copy:      ", "Scale:     ",
                         "Add:       ", "Triad:     "};

static double bytes[4] = {
                           2 * sizeof(double) * N,
                           2 * sizeof(double) * N,
                           3 * sizeof(double) * N,
                           3 * sizeof(double) * N
                         };

extern double mysecond();
extern double checktick();
extern void checkSTREAMresults(double *a,double *b, double *c);

void mem_bw(double &avg_bw, double &cache_size) {
  avg_bw=0;
  double  *a=(double *)malloc((N+OFFSET)*sizeof(double));
  double  *b=(double *)malloc((N+OFFSET)*sizeof(double));
  double  *c=(double *)malloc((N+OFFSET)*sizeof(double));
  double   quantum=checktick();
  int   BytesPerWord;
  register int j, k;
  double  scalar, t, times[4][NTIMES];
  double rv=0;


  double tt[30],t_max;
  int cache_ratio=1,cache_level=1;
  int i=0;

  int check_cache=(cache_size==0);
  do {
    N*=2;
    a=(double *)realloc(a,(N+OFFSET)*sizeof(double));
    b=(double *)realloc(b,(N+OFFSET)*sizeof(double));
    c=(double *)realloc(c,(N+OFFSET)*sizeof(double));
    if ( !a || !b || !c ) return;

    for (j=0; j<N; j++) {
      a[j] = 1.0;
      b[j] = 2.0;
      c[j] = 0.0;
    }

    long transfer=0; 

    // get as close to a clock boundary as possible
    t = mysecond();
    while ((t-mysecond())==0); // do nothing

    double t0 = mysecond();
    do {
      for (j = 0; j < N; j++)
        a[j] = 2.0E0 * a[j];
      t = 1.0E6 * (mysecond() - t0);
      transfer++;
    } while (t < 10*quantum);  // run at least a 10 ticks.

    t/=transfer; // change to "per run" time
    tt[i++]=t;
    t_max=std::max(N/t,t_max);
    if (check_cache) {
      if ((cache_ratio*N)<(t*t_max)) {
        cache_size=N/2*sizeof(double);
	cache_ratio*=2;
//        printf("Level %d Cache Size = %f bytes\n",cache_level++,*cache_size);
      }
    }
  } while (t<1e5);

  bytes[0]=bytes[1]=2 * sizeof(double) * N;
  bytes[2]=bytes[3]=3 * sizeof(double) * N;

  /* --- MAIN LOOP --- repeat test cases NTIMES times --- */

  scalar = 3.0;
  for (k=0; k<NTIMES; k++) {
    times[0][k] = mysecond();

    for (j=0; j<N; j++)
      c[j] = a[j];
    times[0][k] = mysecond() - times[0][k];

    times[1][k] = mysecond();

    for (j=0; j<N; j++)
      b[j] = scalar*c[j];

    times[1][k] = mysecond() - times[1][k];

    times[2][k] = mysecond();

    for (j=0; j<N; j++)
      c[j] = a[j]+b[j];

    times[2][k] = mysecond() - times[2][k];

    times[3][k] = mysecond();

    for (j=0; j<N; j++)
      a[j] = b[j]+scalar*c[j];

    times[3][k] = mysecond() - times[3][k];
  }

  /* --- SUMMARY --- */

  for (k=1; k<NTIMES; k++) /* note -- skip first iteration */
  {
    for (j=0; j<4; j++) {
      avgtime[j] = avgtime[j] + times[j][k];
      mintime[j] = std::min(mintime[j], times[j][k]);
      maxtime[j] = std::max(maxtime[j], times[j][k]);
    }
  }

  printf("Function      Rate (MB/s)   Avg time     Min time     Max time\n");
  for (j=0; j<4; j++) {
    avgtime[j] = avgtime[j]/(double)(NTIMES-1);

    printf("%s%11.4f  %11.4f  %11.4f  %11.4f\n", label[j],
           1.0E-06 * bytes[j]/mintime[j],
           avgtime[j],
           mintime[j],
           maxtime[j]);

    avg_bw+=(double)bytes[j]/mintime[j];
  }
  avg_bw/=4;

  /* --- Check Results --- */
  checkSTREAMresults(a,b,c);

}

# define M 20

double checktick() {
    int  i;
    double minDelta, Delta;
    double t1, t2, timesfound[M];

    /*  Collect a sequence of M unique time values from the system. */

    for (i = 0; i < M; i++) {
      t1 = mysecond();
      while( ((t2=mysecond()) - t1) < 1.0E-6 )
	;
      timesfound[i] = t1 = t2;
    }

    /*
     * Determine the minimum difference between these M values.
     * This result will be our estimate (in microseconds) for the
     * clock granularity.
     */

    minDelta = 1000000;
    for (i = 1; i < M; i++) {
      Delta = ( 1.0E6 * (timesfound[i]-timesfound[i-1]));
      minDelta = std::min(minDelta, std::max(Delta,0.0));
    }

    return(minDelta);
  }



  /* A gettimeofday routine to give access to the wall
     clock timer on most UNIX-like systems.  */

#include <sys/time.h>

  double mysecond() {
    struct timeval tp;
    struct timezone tzp;
    int i;

    i = gettimeofday(&tp,&tzp);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
  }

  void checkSTREAMresults (double *a, double *b, double *c) {
    double aj,bj,cj,scalar;
    double asum,bsum,csum;
    double epsilon;
    int j,k;

    /* reproduce initialization */
    aj = 1.0;
    bj = 2.0;
    cj = 0.0;
    /* a[] is modified during timing check */
    aj = 2.0E0 * aj;
    /* now execute timing loop */
    scalar = 3.0;
    for (k=0; k<NTIMES; k++) {
      cj = aj;
      bj = scalar*cj;
      cj = aj+bj;
      aj = bj+scalar*cj;
    }
    aj = aj * (double) (N);
    bj = bj * (double) (N);
    cj = cj * (double) (N);

    asum = 0.0;
    bsum = 0.0;
    csum = 0.0;
    for (j=0; j<N; j++) {
      asum += a[j];
      bsum += b[j];
      csum += c[j];
    }
#ifdef VERBOSE
    printf ("Results Comparison: \n");
    printf ("        Expected  : %f %f %f \n",aj,bj,cj);
    printf ("        Observed  : %f %f %f \n",asum,bsum,csum);
#endif

#ifndef abs
#define abs(a) ((a) >= 0 ? (a) : -(a))
#endif
    epsilon = 1.e-8;

    if (abs(aj-asum)/asum > epsilon) {
      printf ("Failed Validation on array a[]\n");
      printf ("        Expected  : %f \n",aj);
      printf ("        Observed  : %f \n",asum);
    } else if (abs(bj-bsum)/bsum > epsilon) {
      printf ("Failed Validation on array b[]\n");
      printf ("        Expected  : %f \n",bj);
      printf ("        Observed  : %f \n",bsum);
    } else if (abs(cj-csum)/csum > epsilon) {
      printf ("Failed Validation on array c[]\n");
      printf ("        Expected  : %f \n",cj);
      printf ("        Observed  : %f \n",csum);
    } else {
      printf ("Solution Validates\n");
    }
  }

#ifdef STREAM_TEST

int main() {
  double cache_size=0;
  double avg_bw=0;
  mem_bw(avg_bw,cache_size);
  printf("Average bandwidth=%f MB/s\n",avg_bw/1024/1024);
  printf("Cache Size=%f kB\n",cache_size/1024);
}

#endif
