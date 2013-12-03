/*
 * condor/ts.c
 *
 * DC-API test application, slave
 *
 * (c) Daniel Drotos, 2006
 */

#include "dc_client.h"

#include <stdio.h>
#include <ctype.h>

static char infile_logical[]   = "in.txt";
static char outfile_logical[]  = "out.txt";
static char ckptfile_logical[] = "ckpt.txt";
static FILE *infile;
static FILE *outfile;
static FILE *ckptfile;

static int frac_input_size;
static int frac_current_pos = 0;

static void frac_init(void){
	frac_current_pos = ftell(infile);
	fseek(infile, 0, SEEK_END);
	frac_input_size = ftell(infile);
	fseek(infile, 0, SEEK_SET);
}

static int do_checkpoint(int frac_current_pos){
	FILE *ckptfile;
	char *ckptfile_physical;
	int retval;

	ckptfile_physical = DC_resolveFileName(DC_FILE_OUT, DC_CHECKPOINT_FILE);
	ckptfile = fopen(ckptfile_physical, "w");
	if (ckptfile == NULL){
		fprintf(stderr, "APP: cannot open checkpoint file for write!!\n");
		DC_finishClient(6);
	}
	fprintf(ckptfile, "%d", frac_current_pos);
	fclose(ckptfile);

	retval = fflush(outfile);
	if (retval) {
		fprintf(stderr, "APP: uppercase flush failed %d\n", retval);
		DC_finishClient(5);
	}
    
	return 0;
}

static void init_files(void){
	int ckpt_position;
	char *infile_physical;
	char *outfile_physical;
	char *ckptfile_physical;
	const char *outfile_openmode = "w";

	/* Open input file */
	infile_physical = DC_resolveFileName(DC_FILE_IN, infile_logical);
	infile = fopen(infile_physical, "rb");
	if (infile == NULL){
		fprintf(stderr, "APP: Cannot open input file! Logical name: %s, physical name: %s.\n",
			infile_logical, infile_physical);
		DC_finishClient(1);
	}
	frac_init();

	/* check ckpt file */
	ckptfile_physical = DC_resolveFileName(DC_FILE_IN, DC_CHECKPOINT_FILE);
	ckptfile = fopen(ckptfile_physical, "r");
	if (ckptfile != NULL){
		/* ckpt file exists: read and set everything according to it */
		fscanf(ckptfile, "%d", &ckpt_position);
		fclose(ckptfile);
		fprintf(stderr, "APP: Found checkpoint file. Checkpoint position: %d.\n", ckpt_position);
		fseek(infile, ckpt_position, SEEK_SET);
		frac_current_pos = ckpt_position;
		outfile_openmode = "r+";
	}

	/* open output file */
	outfile_physical = DC_resolveFileName(DC_FILE_OUT, outfile_logical);
	outfile = fopen(outfile_physical, outfile_openmode);
	if (outfile == NULL){
		fprintf(stderr, "APP: Cannot open output file! Logical name: %s, physical name: %s.\n",
			outfile_logical, outfile_physical);
		DC_finishClient(2);
	}
	fseek(outfile, frac_current_pos, SEEK_SET);
}

static void do_work(void){
	int c;
	int retval;

	while (1) {
		c = fgetc(infile);

		if (c == EOF) break;
		c = toupper(c);
		fputc(c, outfile);
		frac_current_pos++;

		//        if (DC_timeToCheckpoint()){
		if (1){
			if(do_checkpoint(frac_current_pos)){
				fprintf(stderr, "APP: uppercase checkpoint failed.\n");
				DC_finishClient(3);
			}
			//          DC_checkpointMade();
		}

		DC_fractionDone(frac_current_pos/frac_input_size);
	}

	retval = fflush(outfile);
	if (retval) {
		fprintf(stderr, "APP: uppercase flush failed %d\n", retval);
		DC_finishClient(5);
	}
	fclose(outfile);
}

int main(int argc, char **argv) {
	int retval = 0;

	retval = DC_initClient();
	if (retval) {
		fprintf(stderr, "APP: Error while the initialize. Return value = %d.\n", retval);
		DC_finishClient(retval);
	}
	fprintf(stdout, "APP: Init successful.\n");

	init_files();
	fprintf(stdout, "APP: Starting from line %d.\n", frac_current_pos+1);

	do_work();
	fprintf(stdout, "APP: Work finished.\n");
	fprintf(stderr, "APP: This is only a test error message.\n");

	DC_sendMessage("Client completed. Exiting.");

	DC_finishClient(0);
	return(0);	// Tho' we never reach this line
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

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
