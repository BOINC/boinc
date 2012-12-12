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
// This program serves as both
// - An example BOINC-CUDA application, illustrating the use of the BOINC API
//   and CUDA API.
// - A program for testing various features of BOINC.
//
// The program reads the input nxn matrix from the "input" file, inverts the
// matrix NUM_ITERATIONS times and write to "output" file.
//
// command line options
// -run_slow: sleep 1 second after each character
// -cpu_time N: use about N CPU seconds after copying files
// -early_exit: exit(10) after 30 chars
// -early_crash: crash after 30 chars
//
// See http://boinc.berkeley.edu/trac/wiki/GPUApp for any compiling issues
// Contributor: Tuan Le (tuanle86@berkeley.edu)

#include "cuda.h"
#include "cuda_config.h"

using std::string;

/*** GLOBALS ***/
bool run_slow = false;
bool early_exit = false;
bool early_crash = false;
bool early_sleep = false;
double cpu_time = 20, comp_result;

int main(int argc, char** argv) {
    int i, retval, lastInversion=0, checkpointExists=0, dimension=0;
    double fd;
    char input_path[512], output_path[512], chkpt_path[512], buf[256];
    REAL* h_idata;
    MFILE out;
    FILE* state, *infile;
    
    generate_random_input_file(MATRIX_SIZE); //call this if you don't want to
                                             //construct the input file manually
	
    for (i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-early_exit")) early_exit = true;
        if (!strcmp(argv[i], "-early_crash")) early_crash = true;
        if (!strcmp(argv[i], "-early_sleep")) early_sleep = true;
        if (!strcmp(argv[i], "-run_slow")) run_slow = true;
        if (!strcmp(argv[i], "-cpu_time")) {
            cpu_time = atof(argv[++i]);
        }
    }
	
    retval = boinc_init();
    if (retval) {
        fprintf(stderr,
            "%s boinc_init returned %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit(retval);
    }
    
    // open the input file (resolve logical name first)
    //
    boinc_resolve_filename(INPUT_FILENAME, input_path, sizeof(input_path));
    infile = boinc_fopen(input_path, "r");
    if (!infile) {
        fprintf(stderr,
            "%s Couldn't find input file, resolved name %s.\n",
            boinc_msg_prefix(buf, sizeof(buf)), input_path
        );
        getchar();
        exit(-1);
    }
    
    boinc_resolve_filename(OUTPUT_FILENAME, output_path, sizeof(output_path));
    
    // See if there's a valid checkpoint file.
    // If so retrieve the current matrix and inversion number
    //
    boinc_resolve_filename(CHECKPOINT_FILE, chkpt_path, sizeof(chkpt_path));
    state = boinc_fopen(chkpt_path, "r");
    if (state) {
        printf("Checkpoint file is detected. Read from checkpoint file ... \n");
        checkpointExists=fscanf(state, "%d", &lastInversion); 
        if (checkpointExists == 1) {
            printf("Last inversion # is : %d\n",lastInversion);	
            fscanf(state,"%d",&dimension);
            cudaMallocHost((void **)&h_idata,dimension*dimension*sizeof(REAL));
            for (int i=0;i<dimension*dimension;++i) {
                fscanf(state, "%f", &h_idata[i]);
			}
        }
        fclose(state);
    } else {
        printf("There's no valid checkpoint file!\n");
    }
    
    retval = out.open(output_path, "wb");
    
    if (retval) {
        fprintf(stderr,
            "%s APP: matrix_inversion output open failed:\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        fprintf(stderr,
            "%s resolved name %s, retval %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), output_path, retval
        );
        perror("open");
        exit(1);
    }
	
#ifdef APP_GRAPHICS
    // create shared mem segment for graphics, and arrange to update it
    //
    shmem = (UC_SHMEM*)boinc_graphics_make_shmem("matrix_inversion", sizeof(UC_SHMEM));
    if (!shmem) {
        fprintf(stderr,
            "%s failed to create shared mem segment\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
    }
    update_shmem();
    boinc_register_timer_callback(update_shmem);
#endif

    if (checkpointExists != 1) {
        dimension=get_matrix_dimension(infile);
        printf("Matrix dimension: %d\n",dimension);
        cudaMallocHost((void **)&h_idata,dimension*dimension*sizeof(REAL));
        fetch_elements_into_host_memory(infile,h_idata);
        out.printf("\n----------------- Before being inversed ----------------\n\n");
        printf("Computation is running ... Inverse the matrix %d times. Start at inversion #1\n",
			   NUM_ITERATIONS);
    } else {
        out.printf("\n----------------- Last checkpointed inversion #%d ----------------\n\n",
			       lastInversion);
        printf("Computation is resumed ... Inverse the matrix %d more times. Start at inversion #%d\n",
			   NUM_ITERATIONS-lastInversion,lastInversion+1);
    }
    print_to_file(&out,h_idata,dimension);

    for (int i=lastInversion+1;i<=NUM_ITERATIONS;++i) {
        invert(h_idata,dimension);
        printf("Finish inversion #%d\n",i);
        if (run_slow) {
            boinc_sleep(1.);
        }
        if (early_exit && i>30) {
            exit(-10);
        }
        if (early_crash && i>30) {
            boinc_crash();
        }
        if (early_sleep && i>30) {
            g_sleep = true;
            while (1) boinc_sleep(1);
        }

        if (boinc_time_to_checkpoint()) {
            //if (i==7) {
            printf("Perform checkpointing at inversion # %d\n",i);
            //we'll need to write the current matrix to the state file.
            retval = do_checkpoint(out, i, h_idata, dimension); 
            if (retval) {
                fprintf(stderr,
                    "%s APP: matrix_inversion checkpoint failed %d\n",
                    boinc_msg_prefix(buf, sizeof(buf)), retval
                );
                exit(retval);
            }
            boinc_checkpoint_completed();
        }

        fd = i/NUM_ITERATIONS;
        if (cpu_time) fd /= 2;
        boinc_fraction_done(fd);
    }

    out.printf("\n\n----------------- Final inversion #%d----------------\n\n",
		       NUM_ITERATIONS);
    print_to_file(&out,h_idata,dimension);
    cudaFreeHost( h_idata );
    retval = out.flush(); //force the output file to be closed.
    if (retval) {
        fprintf(stderr,
            "%s APP: matrix_inversion flush failed %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit(1);
    }

    // burn up some CPU time if needed
    //
    if (cpu_time) {
        printf("\nBurning up some CPU time ... \n");
        double start = dtime();
        for (int i=0; ; i++) {
            double e = dtime()-start;
            if (e > cpu_time) break;
            fd = .5 + .5*(e/cpu_time);
            boinc_fraction_done(fd);
			
            if (boinc_time_to_checkpoint()) {
                retval = do_checkpoint(out, NUM_ITERATIONS, h_idata, dimension);
                if (retval) {
                    fprintf(stderr,
                        "%s APP: maxtrix_inversion checkpoint failed %d\n",
                        boinc_msg_prefix(buf, sizeof(buf)), retval
                    );
                    exit(1);
                }
                boinc_checkpoint_completed();
            }
            comp_result = do_a_giga_flop(i);
        }
    }
    boinc_fraction_done(1);
#ifdef APP_GRAPHICS
    update_shmem();
#endif

    printf("\nDone! Please press ENTER to exit. ");
    getchar();
    boinc_finish(0);
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;
	
    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif

/*** BOINC FUNCTION DEFINITIONS ***/

/* Do a billion floating-point ops */
static double do_a_giga_flop(int foo) {
    double x = 3.14159*foo;
    int i;
    for (i=0; i<500000000; i++) {
        x += 5.12313123;
        x *= 0.5398394834;
    }
    return x;
}

/* Save the computation state into checkpoint file */
int do_checkpoint(MFILE& mf, int n, REAL *h_idata, int dimension) {
    int retval;
    string resolved_name;
	
    FILE* f = fopen("temp", "w");
	if (!f) {
        return 1;
    }
    fprintf(f, "%d", n); //write inversion number
    fprintf(f, " ");
    fprintf(f, "%d", dimension); //write dimension
    fprintf(f, " ");
    for (int i=0;i<dimension*dimension;++i) {
        fprintf(f, " ");
        fprintf(f, "%f", h_idata[i]);
    }
    fclose(f);
    retval = mf.flush();
	if (retval) {
        return retval;
	}
    boinc_resolve_filename_s(CHECKPOINT_FILE, resolved_name);
    retval = boinc_rename("temp", resolved_name.c_str());
    
	if (retval) {
        return retval;
    }
    return 0; //return 0 to indicate success.
}

/*** FUNCTION DEFINITIONS ***/

/* Create an input file filled with random data of type cl_float. */
void generate_random_input_file(int n) {
    FILE *infile;
    infile=fopen(INPUT_FILENAME,"w");
    REAL *h_idata = (REAL *)malloc(sizeof(REAL)*n*n);
    srand(n);
    for( int i = 0; i < n; i++ ) {
        for (int j = 0; j < n; j++) {
            h_idata[i*n+j] = 2.0*(rand()%32768)/32768.0 - 1.0;
        }
        h_idata[i*n+i] += sqrt((float)n);
    }
    int j=0;
    for (int i=0;i<n*n;++i) {
        fprintf(infile,"%15f",h_idata[i]);
        if (j+1==n) {
            fprintf(infile,"\n");
            j=0;
        } else {
            ++j;
        }
    }
    fclose(infile);
    free(h_idata);
}

/*
 * Parse the input file and determine the size of the matrix.
 * This is an nxn matrix. Note: if width <> height, the matrix is
 * non-invertible.
 */
int get_matrix_dimension(FILE *infile) {
    int w=0;
    char c;

    fseek(infile,0,SEEK_SET);
    while (true) {
        do {
            c=fgetc(infile);
            if (c == EOF || c == '\n') {
                goto exitLoop;
            }
        } while (isspace(c));

        if (isdigit(c) || c=='.' || c=='-') {
            ++w;
        }

        do {
            c=fgetc(infile);
            if (c == EOF || c == '\n') {
                goto exitLoop;
            }
        } while (isdigit(c) || c=='.' || c=='-');

        if (c==EOF || c == '\n') {
            break;
        }
    }
    exitLoop:
    return w;
}

/* Read the REAL values from input file into host array. */
void fetch_elements_into_host_memory(FILE *infile, REAL *h_idata) {
    float num=0;
    int i=0;
    fseek(infile,0,SEEK_SET);
    while (fscanf(infile,"%f",&num)==1) {
        h_idata[i]=num;
        ++i;
    }
}

/* Write the result to output file */
void print_to_file(MFILE *out, float *h_odata, int dimension) {
    int count=0;
    int move=0;
    int num_elements=dimension*dimension;
    while (num_elements>0) {
        out->printf("%15f    ",h_odata[move]);
        ++count;
        ++move;
        if (count==dimension) {
            out->printf("\n");
            count=0;
        }
        --num_elements;
    }
}
