/***************
 *
 *	optBench.hpp:	test bench code for seti@home client
 *	Originator:		Ben Herndon
 *
 *	Test bench for optimization and benchmarking routines
 *   to be used with seti @ home client code
 *
 */

#ifndef OPT_BENCH_H
#define OPT_BENCH_H
#if defined(OPT_DEBUG)



#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include "../pulsefind.h"
#include "opt_x86.h"


/*	Different compilers
#if (defined(_MSC_VER) && _MSC_VER >= 700)
#if (defined(_M_IX86) && _M_IX86 >= 300)
#if (defined(__BORLANDC__) && __BORLANDC__ >= 452)
#if defined(_WIN32)
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#   if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#if (defined(__GNUC__) && defined(__i386__)) || (defined(_MSC_VER) && _MSC_VER >= 700)

*/

// -----------------------------
void  findRoutineTimes(CPU_INFO_t &cpu_info);
void  getCycleFix(void);

extern find_pulse_t orig_find_pulse;

//---------- Globals ---------//


void optSave(int index, float *buffer);
void optInfo1(float avg, float rms);// Debug function pointers

extern float *optDiv;				// Debug globals
extern float *optFolded;
extern float optMax[6];
extern float optSample[10*16];


#endif	// Is debug flag set?

#endif	// #ifndef OPT_BENCH_H

