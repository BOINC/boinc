/*
 * Tuan Le
 * University of California, Berkeley
 * Berkeley Space Sciences Lab
 * tuanle86@berkeley.edu
 */

#ifdef DOUBLE_PRECISION
#define REAL            double
#define jREAL           jdouble
#define jREALArray           jdoubleArray
#else
#define REAL            float
#define jREAL           jfloat
#define jREALArray           jfloatArray
#endif

inline void __cudaSafeCall( int err, const char *file, const int line )
{
  do {
    if( err != 0) {
      fprintf(stderr, "cudaSafeCall() Runtime API error in file <%s>, line %i : %s.\n",
	      file, line, cudaGetErrorString((cudaError_t) err) );
      exit(-1);
    }
  } while (0);
}

#define SAFECALL(err) __cudaSafeCall(err, __FILE__, __LINE__)
#define CUDACHECK {cudaError_t error = cudaGetLastError(); if(error != 0){fprintf(stderr, "Error code %d: %s file %s line %i.\n",error,cudaGetErrorString(error),__FILE__,__LINE__);}}

