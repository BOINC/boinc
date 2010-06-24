/*
 * nvopencl.hpp
 * Author: Tuan Le
 * Date: 06/24/2010
 * University of California, Berkeley
 * Berkeley Space Sciences Lab
 * tuanle86@berkeley.edu
 */

#ifndef NVOPENCL_H_
#define NVOPENCL_H_

#include <CL/cl.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

#define INPUT_FILENAME "input"
#define OUTPUT_FILENAME "output"
#define KERNELS_FILENAME "nvopencl_kernels.cl"
#define KERNELS_FILEPATH "../samples/nvopencl/nvopencl_kernels.cl"
#define CHECKPOINT_FILE "matrix_inversion_state"
#define MATRIX_SIZE 20
#define NUM_ITERATIONS 51 // execute the kernel NUM_ITERATIONS times

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

//#include <cuda_runtime.h>
//#include <cublas.h>

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"
#include "graphics2.h"

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
cl_mem   inputBuffer; //in this sample app, we will read the result from the device back to host from inputBuffer as well.

cl_context          context;
cl_device_id        *devices;
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
void generateRandomInputFile(int n);

/*
 * Parse the input file and determine the size of the matrix.
 * This is an nxn matrix. Note: if width<> height, the matrix is
 * non-invertible.
 */
int getMatrixSize(FILE *infile);

/*
 * Read the float values from input file into "input" array.
 */
void fetchElementsIntoHostMemory(FILE *infile, cl_float *input);

/*
 * BOINC functions
 */
static double do_a_giga_flop(int foo);
int do_checkpoint(MFILE& mf, int n, cl_float *input, int matrixSize);
void update_shmem();

/*
 * OpenCL related initialisations are done here.
 * Context, Device list, Command Queue are set up.
 * Calls are made to set up OpenCL memory buffers that this program uses
 * and to load the programs into memory and get kernel handles.
 */
int initializeCL(void);

int initializeHost(FILE *infile);

/*
 * Read the file which contains kernel definitions, and stores the file content
 * into a char array which is used as an argument to clCreateProgramWithSource.
 */
char *convertToString(const char * filename);

/*
 * This is called once the OpenCL context, memory etc. are set up,
 * the program is loaded into memory and the kernel handles are ready.
 * 
 * It sets the values for kernels' arguments and enqueues calls to the kernels
 * on to the command queue and waits till the calls have finished execution.
 *
 * It also gets kernel start and end time if profiling is enabled.
 */
int runCLKernels(void);

/* Releases OpenCL resources (Context, Memory etc.) */
int cleanupCL(void);

/* Releases program's resources */
void cleanupHost(void);

/* Write the result to output file */
void printToFile(MFILE *out, float *h_odata, int n);

/*
 * Check if the device is able to support the requested number of work items.
 */
int checkDeviceCapability(size_t *globalThreads,
						  size_t *localThreads);

/*
 *	Functions used to inverst matrix. Call kernels inside.
 */
void invert(cl_float * input,
			cl_float *output,
			int n);

void invertge(cl_float * AI_d,
			  int lda,
			  int n);

#endif  /* #ifndef NVOPENCL_H_ */
