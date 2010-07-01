/*
 * Tuan Le
 * University of California, Berkeley
 * Berkeley Space Sciences Lab
 * tuanle86@berkeley.edu
 */

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

#include <cuda_runtime.h>
#include <cublas.h>
#include "cuda_config.h"

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
#include "uc2.h"
UC_SHMEM* shmem;
#endif

using std::string;

#define CHECKPOINT_FILE "matrix_inversion_state"
#define INPUT_FILENAME "input"
#define OUTPUT_FILENAME "output"
#define MATRIX_SIZE 10


// execute the kernel NUM_ITERATIONS times
#define NUM_ITERATIONS 51

bool run_slow = false;
bool early_exit = false;
bool early_crash = false;
bool early_sleep = false;
double cpu_time = 20, comp_result;

// do a billion floating-point ops
// (note: I needed to add an arg to this;
// otherwise the MS C++ compiler optimizes away
// all but the first call to it!)
//
static double do_a_giga_flop(int foo) {
    double x = 3.14159*foo;
    int i;
    for (i=0; i<500000000; i++) {
        x += 5.12313123;
        x *= 0.5398394834;
    }
    return x;
}

int do_checkpoint(MFILE& mf, int n, REAL *h_idata, int dimension) {
    int retval;
    string resolved_name;
	
    FILE* f = fopen("temp", "w");
    if (!f) return 1;
    fprintf(f, "%d", n); //write inversion number
    fprintf(f, " ");
    fprintf(f, "%d", dimension); //write dimension
    fprintf(f, " ");
    for (int i=0;i<dimension*dimension;++i) {
		fprintf(f, " ");
		fprintf(f, "%f", h_idata[i]);
	}
    fclose(f);
    retval = mf.flush(); //not really necessary since we have not yet written anything to output file when doing checkpointing
    if (retval) return retval;
    boinc_resolve_filename_s(CHECKPOINT_FILE, resolved_name); //resolved_name is a string object
    retval = boinc_rename("temp", resolved_name.c_str()); //c_str() will convert a string object to a char string with null terminator equivalent.
	//	because we do rename. Thus temp does not appear, but the CHECKPOINT_FILE appears instead on the disk.
    
    if (retval) return retval;
    return 0; //return 0 to indicate success.
}

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

void print(REAL *A, int n); //on commandline
void generateRandomInputFile(int n);
int getMatrixDimension(FILE *infile);
int countElementsInMatrix(FILE *infile);
void fetchElementsIntoHostMemory(FILE *infile, REAL *h_idata);
void printToFile(MFILE *out, float *h_odata, int dimension);
extern void invert(REAL * A, int n);

int main(int argc, char** argv)
{   
    int i, retval, lastInversion=0, checkpointExists=0, dimension=0;
    double fd;
    char input_path[512], output_path[512], chkpt_path[512], buf[256];
    REAL* h_idata;
    MFILE out;
    FILE* state, *infile;
    
    generateRandomInputFile(MATRIX_SIZE); //call this if you don't want to construct the input file manually
	
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
        fprintf(stderr, "%s boinc_init returned %d\n",
				boinc_msg_prefix(buf), retval
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
				boinc_msg_prefix(buf), input_path
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
				//printf("--%f\n",h_idata[i]);
			}
		}
        fclose(state);
    } else {
		printf("There's no valid checkpoint file!\n");
	}
    
	retval = out.open(output_path, "wb");
    
    if (retval) {
        fprintf(stderr, "%s APP: matrix_inversion output open failed:\n",
				boinc_msg_prefix(buf)
				);
        fprintf(stderr, "%s resolved name %s, retval %d\n",
				boinc_msg_prefix(buf), output_path, retval
				);
        perror("open");
        exit(1);
    }
	
#ifdef APP_GRAPHICS
    // create shared mem segment for graphics, and arrange to update it
    //
    shmem = (UC_SHMEM*)boinc_graphics_make_shmem("matrix_inversion", sizeof(UC_SHMEM));
    if (!shmem) {
        fprintf(stderr, "%s failed to create shared mem segment\n",
				boinc_msg_prefix(buf)
				);
    }
    update_shmem();
    boinc_register_timer_callback(update_shmem);
#endif
	
    if (checkpointExists != 1) {
		dimension=getMatrixDimension(infile);
		printf("Matrix dimension: %d\n",dimension);
		cudaMallocHost((void **)&h_idata,dimension*dimension*sizeof(REAL));
		fetchElementsIntoHostMemory(infile,h_idata);
		out.printf("\n----------------- Before being inversed ----------------\n\n");
		printf("Computation is running ... Inverse the matrix %d times. Start at inversion #1\n",NUM_ITERATIONS);
	} else {
		out.printf("\n----------------- Last checkpointed inversion #%d ----------------\n\n",lastInversion);
		printf("Computation is resumed ... Inverse the matrix %d more times. Start at inversion #%d\n",NUM_ITERATIONS-lastInversion,lastInversion+1);
	}
	printToFile(&out,h_idata,dimension);
	
	
	
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
                fprintf(stderr, "%s APP: matrix_inversion checkpoint failed %d\n",
						boinc_msg_prefix(buf), retval
						);
                exit(retval);
            }
            boinc_checkpoint_completed();
        }
		
        fd = i/NUM_ITERATIONS;
        if (cpu_time) fd /= 2;
        boinc_fraction_done(fd);
		
	}
    
    out.printf("\n\n----------------- Final inversion #%d----------------\n\n",NUM_ITERATIONS);
	printToFile(&out,h_idata,dimension);
	
    cudaFreeHost( h_idata );
    retval = out.flush(); //force the output file to be closed.
    if (retval) {
        fprintf(stderr, "%s APP: matrix_inversion flush failed %d\n",
				boinc_msg_prefix(buf), retval
				);
        exit(1);
    }
    
    // burn up some CPU time if needed
    //
    if (cpu_time) {
        double start = dtime();
        for (int i=0; ; i++) {
            double e = dtime()-start;
            if (e > cpu_time) break;
            fd = .5 + .5*(e/cpu_time);
            boinc_fraction_done(fd);
			
            if (boinc_time_to_checkpoint()) {
                retval = do_checkpoint(out, NUM_ITERATIONS, h_idata, dimension);
                if (retval) {
                    fprintf(stderr, "%s APP: maxtrix_inversion checkpoint failed %d\n",
							boinc_msg_prefix(buf), retval
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
	
	printf("Done!");
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

void print(REAL *h_idata, int n) {
	int j=0;
	for (int i=0;i<n*n;++i) {
		printf("%f    ",h_idata[i]);
		if (j+1==n) {
			printf("\n");
			j=0;
		} else {
			++j;
		}
	}
}

// nxn matrix
void generateRandomInputFile(int n) {
	FILE *infile;
	
	infile=fopen(INPUT_FILENAME,"w");
	REAL *h_idata = new REAL[n*n];
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
}

int getMatrixDimension(FILE *infile) {
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

void fetchElementsIntoHostMemory(FILE *infile, REAL *h_idata) {
	float num=0;
	int i=0;
	fseek(infile,0,SEEK_SET);
	while (fscanf(infile,"%f",&num)==1) {
		h_idata[i]=num;
		++i;
	}
}

void printToFile(MFILE *out, float *h_odata, int dimension) {
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
