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
// See http://boinc.berkeley.edu/trac/wiki/GPUApp for any compiling issues.
// Original contributor: Tuan Le (tuanle86@berkeley.edu)

#ifndef OPENCLAPP_H_
#define OPENCLAPP_H_

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include <string.h>
#include <cmath>
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

#define INPUT_FILENAME "input"
#define OUTPUT_FILENAME "output"
#define KERNELS_FILENAME "openclapp_kernels.cl"
#define KERNELS_FILEPATH "../../openclapp_kernels.cl" // for Linux and Mac
#define CHECKPOINT_FILE "matrix_inversion_state"

#define LOCAL_WORK_SIZE 1
#define GLOBAL_WORK_SIZE 400
#define MATRIX_SIZE 10
#define NUM_ITERATIONS 501 // execute the kernel NUM_ITERATIONS times

#ifdef _WIN32
#include "boinc_win.h"
#else
#ifndef __APPLE__
#include "config.h"
#endif
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

#ifdef APP_GRAPHICS
UC_SHMEM* shmem;
#endif

/*** GLOBALS ***/

bool run_slow = false;
bool early_exit = false;
bool early_crash = false;
bool early_sleep = false;
double cpu_time = 20, comp_result;
bool isStateFileInUse = false;
const char *source;

size_t globalThreads[1]; // 1D var for Total # of work items
size_t localThreads[1];  // 1D var for # of work items in the work group

/*
 * Input data is stored here.
 */
cl_float *input;

/*
 * Output data is stored here.
 */
cl_float *output;

/* problem size for a 2D matrix. */
// Note: we will handle the problem as a 1D matrix.
cl_uint width;
cl_uint height;

/* The memory buffer that is used as input/output for OpenCL kernel */
cl_mem   inputBuffer; //in this sample app, we will read the result
                      //from the device back to host from inputBuffer as well.
cl_context          context;
cl_command_queue    commandQueue;

cl_program program;

/* This program uses three kernels */
cl_kernel  GEStep1A_kernel;
cl_kernel  GEStep2_kernel;
cl_kernel  GEStep3_kernel;

/*** FUNCTION DECLARATIONS ***/

/*
 * Create an input file filled with random data of type cl_float.
 */
void generate_random_input_file(int n);

/*
 * Parse the input file and determine the size of the matrix.
 * This is an nxn matrix. Note: if width<> height, the matrix is
 * non-invertible.
 */
int get_matrix_size(FILE *infile);

/*
 * Read the float values from input file into "input" array.
 */
void fetch_elements_into_host_memory(FILE *infile, cl_float *input);

/*
 * BOINC functions
 */

/* Do a billion floating-point ops */
static double do_a_giga_flop(int foo);

/* Save the computation state into checkpoint file */
int do_checkpoint(MFILE& mf, int n, cl_float *input, int matrixSize);

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

/*
 * OpenCL related initialisations are done here.
 * Context, Device list, Command Queue are set up.
 * Calls are made to set up OpenCL memory buffers that this program uses
 * and to load the programs into memory and get kernel handles.
 */
int initialize_cl(int argc, char * argv[]);

int initialize_host(FILE *infile);

/*
 * Read the file which contains kernel definitions, and stores the file content
 * into a char array which is used as an argument to clCreateProgramWithSource.
 */
char *convert_to_string(const char * filename);

/*
 * This is called once the OpenCL context, memory etc. are set up,
 * the program is loaded into memory and the kernel handles are ready.
 *
 * It sets the values for kernels' arguments and enqueues calls to the kernels
 * on to the command queue and waits till the calls have finished execution.
 *
 * It also gets kernel start and end time if profiling is enabled.
 */
int run_cl_kernels(void);

/* Releases OpenCL resources (Context, Memory etc.) */
int cleanup_cl(void);

/* Releases program's resources */
void cleanup_host(void);

/* Write the result to output file */
void print_to_file(MFILE *out, float *h_odata, int n);

/*
 *	Functions used to inverst matrix. Call kernels inside.
 */
void invert(cl_float * input,
            cl_float *output,
            int n);

void invertge(cl_float * AI_d,
              int lda,
              int n);

#endif  /* #ifndef OPENCLAPP_H_ */
