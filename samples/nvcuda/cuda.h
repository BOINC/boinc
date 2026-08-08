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
// See https://github.com/BOINC/boinc/wiki/GPUApp
// Contributor: Tuan Le (tuanle86@berkeley.edu)

#ifndef CUDA_H_
#define CUDA_H_

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#endif

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"
#include "cuda_config.h"

#define CHECKPOINT_FILE "matrix_inversion_state"
#define INPUT_FILENAME "input"
#define OUTPUT_FILENAME "output"
#define MATRIX_SIZE 10
#define NUM_ITERATIONS 501 // execute the kernel NUM_ITERATIONS times

struct UC_SHMEM {
    double update_time;
    double fraction_done;
    double cpu_time;
    BOINC_STATUS status;
    int countdown;
    // graphics app sets this to 5 repeatedly,
    // main program decrements it once/sec.
    // If it's zero, don't bother updating shmem
};

/*** BOINC FUNCTION DECLARATIONS  ***/

/* Do a billion floating-point ops */
static double do_a_giga_flop(int foo);

/* Save the computation state into checkpoint file */
int do_checkpoint(MFILE& mf, int n, REAL *h_idata, int dimension);

#ifdef APP_GRAPHICS
void update_shmem() {
    if (!shmem) return;

    // always do this; otherwise a graphics app will immediately
    // assume we're not alive
    shmem->update_time = dtime();

    // Check whether a graphics app is running,
    // and don't bother updating shmem if so.
    // This doesn't matter here,
    // but may be worth doing if updating shmem is expensive.
    //
    if (shmem->countdown > 0) {
        // the graphics app sets this to 5 every time it renders a frame
        shmem->countdown--;
    } else {
        return;
    }
    shmem->fraction_done = boinc_get_fraction_done();
    shmem->cpu_time = boinc_worker_thread_cpu_time();;
    boinc_get_status(&shmem->status);
}
#endif

/*** FUNCTION DECLARATIONS ***/

/*
 * Parse the input file and determine the size of the matrix.
 * This is an nxn matrix. Note: if width <> height, the matrix is
 * non-invertible.
 */
int get_matrix_dimension(FILE *infile);

/* Create an input file filled with random data of type cl_float. */
void generate_random_input_file(int n);

/* Read the REAL values from input file into host array. */
void fetch_elements_into_host_memory(FILE *infile, REAL *h_idata);

/* Write the result to output file */
void print_to_file(MFILE *out, float *h_odata, int dimension);

/* invert the given matrix A */
extern void invert(REAL * A, int n);

#endif  /* #ifndef CUDA_H_ */
