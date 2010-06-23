/*
 * Tuan Le
 * University of California, Berkeley
 * Berkeley Space Sciences Lab
 * tuanle86@berkeley.edu
 */

// When VERIFY is defined, the sum of squared errors is calculated between the
// identity matrix and the product A * incerse(A). For debugging...
//#define VERIFY 1

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "config.h"

void mathdispAI(const REAL *mat, int lda, int MAT_SIZE_h) {
    fprintf(stderr, "\n");
	int i,j;
    for (j=0;j<MAT_SIZE_h;j++)  {
		for (i=0;i<MAT_SIZE_h*2;i++) {
	    	fprintf(stderr, "%6.3f",mat[j*lda*2+i]);
	}
	fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
} // mathdisp2

void mathdispAId(const REAL * AId, int lda, int n) {
	REAL * AI = (REAL *)malloc(sizeof(REAL)*(n*lda*2));
	cudaMemcpy(AI,AId,sizeof(REAL)*n*lda*2,cudaMemcpyDeviceToHost);
	mathdispAI(AI, lda, n);
	delete [] AI;
}

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

//extern void invert(REAL * A, int n);
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
} // invertge


/* inverts nxn matrix A and stores result back in A */
void invert(REAL * A, int n) {
fprintf(stderr,"starting inversion n = %d ", n);
    volatile clock_t gputime, gputime0;
    gputime=clock();
    gputime0 = gputime;

    int lda = ((n+15)&~15|16);
//lda=n;
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
} // invert
