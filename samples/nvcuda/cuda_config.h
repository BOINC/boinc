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
//
// See http://boinc.berkeley.edu/trac/wiki/GPUApp for any compiling issues
// Contributor: Tuan Le (tuanle86@berkeley.edu)

#ifndef CUDA_CONFIG_H_
#define CUDA_CONFIG_H_

#include <cuda_runtime.h>
#include <cublas.h>

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

#endif  /* #ifndef CUDA_CONFIG_H_ */