/*
*     C/C++ Whetstone Benchmark Single or Double Precision
*
*     Original concept        Brian Wichmann NPL      1960's
*     Original author         Harold Curnow  CCTA     1972
*     Self timing versions    Roy Longbottom CCTA     1978/87
*     Optimisation control    Bangor University       1987/90
*     C/C++ Version           Roy Longbottom          1996
*     Compatibility & timers  Al Aburto               1996
*
************************************************************
*
*              Official version approved by:
*
*         Harold Curnow  100421.1615@compuserve.com
*
*      Happy 25th birthday Whetstone, 21 November 1997
*/

// Modified a little to work with BOINC

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#endif

#include "boinc_api.h"
#include "cpu_benchmark.h"

#define SPDP double


void pa(SPDP e[4], SPDP t, SPDP t2)
      {
	 long j;
	 for(j=0;j<6;j++)
	    {
	       e[0] = (e[0]+e[1]+e[2]-e[3])*t;
	       e[1] = (e[0]+e[1]-e[2]+e[3])*t;
	       e[2] = (e[0]-e[1]+e[2]+e[3])*t;
	       e[3] = (-e[0]+e[1]+e[2]+e[3])/t2;
	    }

	 return;
}

void po(SPDP e1[4], long j, long k, long l)
      {
	 e1[j] = e1[k];
	 e1[k] = e1[l];
	 e1[l] = e1[j];
	 return;
}

void p3(SPDP *x, SPDP *y, SPDP *z, SPDP t, SPDP t1, SPDP t2)
      {
	 *x = *y;
	 *y = *z;
	 *x = t * (*x + *y);
	 *y = t1 * (*x + *y);
	 *z = (*x + *y)/t2;
	 return;
}

void whetstone(double& flops) {
	long n1,n2,n3,n4,n5,n6,n7,n8,i,ix,n1mult;
	SPDP x,y,z;
	long j,k,l;
	SPDP e1[4];
	double startsec, finisec, ws;
	double KIPS;
    int xtra, ii;
    int x100 = 10000;   // chosen to make each pass take about 1 sec
            // on my current computer (2.2 GHz celeron)
            // Non-critical.

    benchmark_wait_to_start(BM_TYPE_FP);

    boinc_calling_thread_cpu_time(startsec, ws);

	SPDP t =  0.49999975;
	SPDP t0 = t;
	SPDP t1 = 0.50000025;
	SPDP t2 = 2.0;

	n1 = 12*x100;
	n2 = 14*x100;
	n3 = 345*x100;
	n4 = 210*x100;
	n5 = 32*x100;
	n6 = 899*x100;
	n7 = 616*x100;
	n8 = 93*x100;

    xtra = 1;
	n1mult = 10;
    ii = 0;

    do {

	/* Section 1, Array elements */

	e1[0] = 1.0;
	e1[1] = -1.0;
	e1[2] = -1.0;
	e1[3] = -1.0;
	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=0; i<n1*n1mult; i++)
		  {
		      e1[0] = (e1[0] + e1[1] + e1[2] - e1[3]) * t;
		      e1[1] = (e1[0] + e1[1] - e1[2] + e1[3]) * t;
		      e1[2] = (e1[0] - e1[1] + e1[2] + e1[3]) * t;
		      e1[3] = (-e1[0] + e1[1] + e1[2] + e1[3]) * t;
		  }
		t = 1.0 - t;
	      }
	    t =  t0;
	 }

	/* Section 2, Array as parameter */

	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=0; i<n2; i++)
		  {
		     pa(e1,t,t2);
		  }
		t = 1.0 - t;
	      }
	    t =  t0;
	 }

	/* Section 3, Conditional jumps */
	j = 1;
	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=0; i<n3; i++)
		  {
		     if(j==1)       j = 2;
		     else           j = 3;
		     if(j>2)        j = 0;
		     else           j = 1;
		     if(j<1)        j = 1;
		     else           j = 0;
		  }
	      }
	 }

	/* Section 4, Integer arithmetic */
	j = 1;
	k = 2;
	l = 3;
	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=0; i<n4; i++)
		  {
		     j = j *(k-j)*(l-k);
		     k = l * k - (l-j) * k;
		     l = (l-k) * (k+j);
		     e1[l-2] = j + k + l;
		     e1[k-2] = j * k * l;
		  }
	      }
	 }

	/* Section 5, Trig functions */
	x = 0.5;
	y = 0.5;
	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=1; i<n5; i++)
		  {
		     x = t*atan(t2*sin(x)*cos(x)/(cos(x+y)+cos(x-y)-1.0));
		     y = t*atan(t2*sin(y)*cos(y)/(cos(x+y)+cos(x-y)-1.0));
		  }
		t = 1.0 - t;
	      }
	    t = t0;
	 }

	/* Section 6, Procedure calls */
	x = 1.0;
	y = 1.0;
	z = 1.0;
	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=0; i<n6; i++)
		  {
		     p3(&x,&y,&z,t,t1,t2);
		  }
	      }
	 }

	/* Section 7, Array refrences */
	j = 0;
	k = 1;
	l = 2;
	e1[0] = 1.0;
	e1[1] = 2.0;
	e1[2] = 3.0;
	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=0;i<n7;i++)
		  {
		     po(e1,j,k,l);
		  }
	      }
	 }

	/* Section 8, Standard functions */
	x = 0.75;
	 {
	    for (ix=0; ix<xtra; ix++)
	      {
		for(i=0; i<n8; i++)
		  {
		     x = sqrt(exp(log(x)/t1));
		  }
	      }
	 }

     ii++;
    }
    while (!benchmark_time_to_stop(BM_TYPE_FP));

    boinc_calling_thread_cpu_time(finisec, ws);

	KIPS = (100.0*x100*ii)/(double)(finisec-startsec);
#if 0
	if (KIPS >= 1000.0)
		printf("C Converted Double Precision Whetstones: %.1f MIPS\n", KIPS/1000.0);
	else
		printf("C Converted Double Precision Whetstones: %.1f KIPS\n", KIPS);
#endif

    // convert from thousands of instructions a second to instructions a second.
    flops = KIPS*1000.0;
}
