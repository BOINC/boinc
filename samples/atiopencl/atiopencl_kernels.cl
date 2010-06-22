/*
 * atiopencl_kernels.cl
 * Author: Tuan Le
 * Date: 06/14/2010
 * University of California, Berkeley
 * Berkeley Space Sciences Lab
 * tuanle86@berkeley.edu
 */

__kernel void GEStep1A(__global float * AI, int i, int n2, int lda2) {
	//int k = get_group_id(0) * get_local_size(0) + get_local_id(0);
	int k=get_global_id(0);
	if (k>i && k < n2 && AI[i*lda2+k]!=0) {
		float multiplyer = -AI[i*lda2+k]/AI[i*lda2+i];
		int n = n2 / 2;
		for (int j = i+1; j < n; j++) {
			AI[j*lda2+k] += multiplyer*AI[j*lda2+i];
		}
	}
}

__kernel void GEStep2(__global float * AI,float diag,int i, int n2, int lda2) {
	int k = get_global_id(0);
	if (k < n2) {
		AI[i*lda2+k] /= diag;
	}
}

__kernel void GEStep3(__global float * AI,int i, int n2, int lda2) {
	int k = get_global_id(0);
	if (k > i && k < n2) {
		float multiplyer = -AI[i*lda2+k];
		for (int j = 0; j < i; j++) {
			AI[j*lda2+k] += multiplyer*AI[j*lda2+i];
		}
	}
}
