// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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
// This file contains kernel definition for matrix inversion.
//
// See http://boinc.berkeley.edu/trac/wiki/GPUApp for any compiling issues
// Contributor: Tuan Le (tuanle86@berkeley.edu)

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