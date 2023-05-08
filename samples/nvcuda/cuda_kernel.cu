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
// This file contains kernel definition for matrix inversion. The external function
// "invert" serves as an interface between cuda_kernel.cu and cuda.cpp
//
// See http://boinc.berkeley.edu/trac/wiki/GPUApp for any compiling issues
// Contributor: Tuan Le (tuanle86@berkeley.edu)

// When VERIFY is defined, the sum of squared errors is calculated between the
// identity matrix and the product A * incerse(A). For debugging...
//#define VERIFY 1
#include <stdio.h>
#include <cmath>
#include <time.h>
#include "cuda_config.h"

__global__ void GEStep1A(REAL * AI, int i, int n2, int lda2) {
    int k = blockIdx.x * blockDim.x + threadIdx.x;
    if (k>i && k < n2 && AI[i*lda2+k]!=0) {
        REAL multiplyer = -AI[i*lda2+k]/AI[i*lda2+i];
        int n = n2 / 2;
        for (int j = i+1; j < n; j++) {
            AI[j*lda2+k] += multiplyer*AI[j*lda2+i];
        }
    }
}

__global__ void GEStep2(REAL * AI,REAL diag,int i, int n2, int lda2) {
    int k = blockIdx.x * blockDim.x + threadIdx.x;
    if (k < n2) {
        AI[i*lda2+k] /= diag;
    }
}

__global__ void GEStep3(REAL * AI,int i, int n2, int lda2) {
    int k = blockIdx.x * blockDim.x + threadIdx.x;
    if (k > i && k < n2) {
        REAL multiplyer = -AI[i*lda2+k];
        for (int j = 0; j < i; j++) {
            AI[j*lda2+k] += multiplyer*AI[j*lda2+i];
        }
    }
}

/* Helper function for invert. Kernel calls are made in this function */
void invertge(REAL * AI_d, int lda, int n) {
    int lda2 = lda * 2;
    // perform elementary row operations till A in AI becomes identity matrix
    for (int i = 0; i < n; i++) {
        GEStep1A<<<(int)ceil((float)(1+(2*n-1)/32)),32>>>(AI_d,i,n*2, lda2);
        CUDACHECK;
        cudaThreadSynchronize();
    }

    for (int i = n-1; i >= 0; i--) {
        REAL diag = 1.0;
        SAFECALL(cudaMemcpy(&diag, &AI_d[i*lda2+i], sizeof(REAL), cudaMemcpyDeviceToHost));
        GEStep2<<<(int)ceil((float)(1+(n*2-1)/32)),32>>>(AI_d,diag,i,n*2, lda2);
        CUDACHECK;

        GEStep3<<<(int)ceil((float)(1+(n*2-1)/32)),32>>>(AI_d,i,n*2, lda2);
        CUDACHECK;
        cudaThreadSynchronize();
        CUDACHECK;
    }
}

/* inverts nxn matrix A and stores result back in A */
extern void invert(REAL * A, int n) {
    fprintf(stderr,"starting inversion n = %d ", n);
    volatile clock_t gputime;
    gputime=clock();

    int lda = ((n+15)&~15|16);
    REAL * AI = (REAL *)malloc(sizeof(REAL)*(n*lda*2));
    memset(AI,0,sizeof(REAL)*n*lda*2);
    for (int i = 0; i < n; i++) {
        memcpy(&AI[lda*i*2], &A[n*i], sizeof(REAL)*n);
        AI[lda*i*2+n+i] = 1;
    }

    REAL * AI_d;
    SAFECALL(cudaMalloc((void **) &AI_d, sizeof(REAL)*n*lda*2));
    SAFECALL(cudaMemcpy(AI_d, AI, sizeof(REAL)*n*lda*2, cudaMemcpyHostToDevice));

    invertge(AI_d, lda, n);
    SAFECALL(cudaMemcpy(AI, AI_d, sizeof(REAL)*n*lda*2, cudaMemcpyDeviceToHost));
    cudaFree(AI_d);
    gputime=clock()-gputime;fprintf(stderr, " %7.1f ms ",gputime/1.e3f);
    fprintf(stderr, " %7.2f Gflops", 1e-3*(3.0)*n*n*n/3.0/gputime);

#ifdef VERIFY
	// let's verify that
    REAL error=0.0;
    // multiply inverse*xcopy, should be Identity matrix
    for (int k = 0; k < n; k++) {
        for (int j = 0; j < n; j++) {
            REAL sum = 0;
            for (int i = 0; i < n; i++) {
                sum += AI[j*lda*2+n+i]*A[i*n+k];
            }
            if (j!=k) {
                error += sum * sum;
            } else {
                error += (1.0-sum) * (1.0-sum);
            }
        }
    }
    fprintf(stderr, " %6.2f SSE", error);
#endif

    for (int i = 0; i < n; i++) {
        memcpy(&A[n*i], &AI[lda*i*2+n], sizeof(REAL)*n);
    }
    free(AI);
    fprintf(stderr," done!\n");
}
