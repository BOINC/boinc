/***************
 *
 *	optimize.hpp:	test bench code for seti@home client
 *	Originator:		Ben Herndon
 *
 */


#ifndef OPTIMIZE_H
#define OPTIMIZE_H

//---------- Constants ---------//

#include <math.h>

#if !defined(UINT64)
//#if (defined(__GNUC__) && defined(__i386__)) || (defined(_MSC_VER) && _MSC_VER >= 700)
typedef unsigned long long int UINT64;
#endif

#if !defined(UINT32)
//#if (defined(__GNUC__) && defined(__i386__)) || (defined(_MSC_VER) && _MSC_VER >= 700)
typedef unsigned int UINT32;
#endif


#ifndef max
#define max(a, b)  ((a > b) ? a : b)
#endif
#ifndef min
#define min(a, b)  ((a < b) ? a : b)
#endif

//---------- Prototypes ---------//

//**********************
//
// FFT8g  routines that would speed up the code most from optimization
//

void  optPreFFT ( int len, float *cplxBuf, float *coEf, int *ip );
void  optPostFFT( int len, float *cplxBuf, float *coEf, int *ip, int isgn);

typedef void makewt_t		(int nw,  int *ip, float *coEf) ;
typedef void bitrv2_t		(int len, int *ip, float *cplxBuf);
typedef void bitrv2conj_t	(int len, int *ip, float *cplxBuf);
typedef void cfftFwd_t		(int len, float *cplxBuf, float *coEf);
typedef void cfftBwd_t		(int len, float *cplxBuf, float *coEf);
typedef void cft1st_t		(int len, float *cplxBuf, float *coEf);
typedef void cftmdl_t		(int len, int pwr8, float *cplxBuf, float *coEf);

extern makewt_t		*makewt;		// For real world use - each call
extern bitrv2_t		*bitrv2;		//  becomes a call to a pointer
extern bitrv2conj_t	*bitrv2conj;
extern cfftFwd_t	*cfftFwd;
extern cfftBwd_t	*cfftBwd;
extern cft1st_t		*cft1st;
extern cftmdl_t		*cftmdl;

extern makewt_t		orig_makewt;	// Original unmodified versions
extern bitrv2_t		orig_bitrv2;
extern bitrv2conj_t	orig_bitrv2conj;
extern cfftFwd_t	orig_cftfsub;
extern cfftBwd_t	orig_cftbsub;
extern cft1st_t		orig_cft1st;
extern cftmdl_t		orig_cftmdl;

extern makewt_t  	std_makewt;		// Improved standard C++ versions
extern bitrv2_t		std_bitrv2;
extern bitrv2conj_t	std_bitrv2conj;
extern cfftFwd_t	std_cfftFwd;
extern cfftBwd_t	std_cfftBwd;
extern cft1st_t		std_cft1st;
extern cftmdl_t		std_cftmdl;

//**********************
//
// find_pulse  -- variables/prototypes
//

typedef int find_pulse_t (const float * fp_PulsePot, int PulsePotLen,
		  		         float pulse_thresh, int TOffset, int FOffset);

//
//  multiple table averaging routines - Used by find_pulse
//
typedef float sum2tables_t	(int len, float *one, float *two, float *summed);
typedef float sum3tables_t	(int len, float *one, float *two, float *three, float *summed);
typedef float sum4tables_t	(int len, float *one, float *two, float *three, float *four, float *summed);
typedef float sum5tables_t	(int len, float *one, float *two, float *three, float *four, float *five
		, float *summed);

extern sum2tables_t std_sum2tables;	// Function Prototypes
extern sum3tables_t std_sum3tables;
extern sum4tables_t	std_sum4tables;
extern sum5tables_t	std_sum5tables;

extern sum2tables_t *sum2tables;	// Function Pointers
extern sum3tables_t *sum3tables;
extern sum4tables_t	*sum4tables;
extern sum5tables_t	*sum5tables;

//**********************
//
// v_ChirpData  -- variables/prototypes
//

typedef int v_ChirpData_t(
  float* fp_DataArray,
  float* fp_ChirpDataArray,
  float chirp_rate,
  int  ul_NumDataPoints,
  double sample_rate);

extern v_ChirpData_t	*v_ChirpData;

extern v_ChirpData_t	orig_v_ChirpData;

//**********************
//
// v_GetPowerSpectrum  -- variables/prototypes
//
typedef void v_GetPowerSpectrum_t(
    float * fp_FreqData,
    float * fp_PowerSpectrum,
    int ul_NumDataPoints
  );

extern v_GetPowerSpectrum_t	*v_GetPowerSpectrum;		// Func Ptr
extern v_GetPowerSpectrum_t	orig_v_GetPowerSpectrum;	// Orig

//**********************
//
// GaussFit  -- variables/prototypes
//

#if defined(_MSC_VER)
#pragma intrinsic (sin, cos)
#endif

#endif // #ifndef OPTIMIZE_H

