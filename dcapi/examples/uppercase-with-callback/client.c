#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dc_client.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <errno.h>
#include <time.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "common.h"

static FILE *infile;
static FILE *outfile;

/* Needed to report the percentage of the work already done */
static int frac_total_size;
static int frac_current_pos;

/* Save the value of frac_current_pos to the ckpt file */
static void do_checkpoint(void)
{
	char *ckptfile_name;
	FILE *ckptfile;
	int retval;

	/* Ensure that the output file is really on the disk before we create
	 * the checkpoint file */
	retval = fflush(outfile);
	if (retval)
	{
		perror("APP: flushing the output file has failed");
		DC_finishClient(1);
	}

	ckptfile_name = DC_resolveFileName(DC_FILE_OUT, DC_CHECKPOINT_FILE);
	if (!ckptfile_name)
	{
		fprintf(stderr, "APP: Could not resolve the checkpoint file "
			"name\n");
		DC_finishClient(1);
	}

	ckptfile = fopen(ckptfile_name, "w");
	if (!ckptfile)
	{
		perror("APP: Could not create the checkpoint file");
		DC_finishClient(1);
	}

	fprintf(ckptfile, "%d", frac_current_pos);
	retval = fclose(ckptfile);
	if (retval)
	{
		perror("APP: Closing the checkpoint file has failed");
		DC_finishClient(1);
	}

	/* Tell the DC-API that the checkpoint is ready */
	DC_checkpointMade(ckptfile_name);

	/* Update the estimation about the work done */
	DC_fractionDone(frac_current_pos/frac_total_size);
}

/* Open input and output files, and check is there is a checkpoint file */
static void init_files(void)
{
	const char *outfile_openmode = "w";
	char *file_name;

	/* Open the input file */
	file_name = DC_resolveFileName(DC_FILE_IN, INPUT_LABEL);
	if (!file_name)
	{
		fprintf(stderr, "APP: Could not resolve the input file name\n");
		DC_finishClient(1);
	}

	infile = fopen(file_name, "rb");
	if (!infile)
	{
		perror("APP: Could not open the input file");
		DC_finishClient(1);
	}
	free(file_name);

	/* Determine the size of the input file */
	fseek(infile, 0, SEEK_END);
	frac_total_size = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	/* Check the checkpoint file */
	file_name = DC_resolveFileName(DC_FILE_IN, DC_CHECKPOINT_FILE);
	if (file_name)
	{
		FILE *ckptfile;
		int ret;

		ckptfile = fopen(file_name, "r");
		if (!ckptfile)
		{
			perror("APP: Could not open the initial checkpoint file");
			DC_finishClient(1);
		}
		free(file_name);

		/* ckpt file exists: read and set everything according to it */
		ret = fscanf(ckptfile, "%d", &frac_current_pos);
		if (ret != 1 || frac_current_pos < 0 ||
				frac_current_pos > frac_total_size)
		{
			fprintf(stderr, "APP: Failed to parse the contents of "
				"the checkpoint file, starting from the "
				"beginning\n");
			frac_current_pos = 0;
		}
		else
		{
			fprintf(stderr, "APP: Found checkpoint file, starting "
				"from position %d.\n", frac_current_pos);
			outfile_openmode = "r+";
		}
		fclose(ckptfile);

		fseek(infile, frac_current_pos, SEEK_SET);
	}

	/* Open the output file */
	file_name = DC_resolveFileName(DC_FILE_OUT, OUTPUT_LABEL);
	if (!file_name)
	{
		fprintf(stderr, "APP: Could not resolve the output file name\n");
		DC_finishClient(1);
	}

	outfile = fopen(file_name, outfile_openmode);
	if (!outfile)
	{
		perror("APP: Could not open/create the output file");
		DC_finishClient(1);
	}
	free(file_name);

	/* If we are starting from a checkpoint file, restore the state of
	 * the output file as well */
#ifdef _WIN32
	_chsize(fileno(outfile), frac_current_pos);
#else
	ftruncate(fileno(outfile), frac_current_pos);
#endif

	fseek(outfile, 0, SEEK_END);
}

/* The real work */
static void do_work(void)
{
	int c;
#ifndef _WIN32
	struct timespec time_to_sleep;
#endif

	while ((c = fgetc(infile)) != EOF)
	{
		DC_ClientEvent *event;

		c = toupper(c);
		if (fputc(c, outfile) == EOF)
		{
			perror("APP: fputc");
			DC_finishClient(1);
		}
		frac_current_pos++;

		/* Real applications do real computation here that takes time. Here
		 * we will emulate waiting a second at the end of each line */

		if (c == '\n')
#ifdef _WIN32
			::Sleep(1000);
#else
		{
			time_to_sleep.tv_sec=1;
			time_to_sleep.tv_nsec=0;
			while (0 > nanosleep(&time_to_sleep, &time_to_sleep))
				if (errno != EINTR) break;
		}
#endif

		/* Check if either the master or the BOINC core client asked
		 * us to do something */
		event = DC_checkClientEvent();
		if (!event)
			continue;

		/* If it is time, do a checkpoint */
		if (event->type == DC_CLIENT_CHECKPOINT)
			do_checkpoint();

		/* Make sure we do not leak memory */
		DC_destroyClientEvent(event);
	}

	if (fclose(outfile))
	{
		perror("APP: Closing the output file has failed");
		DC_finishClient(1);
	}
}

int main(int argc, char *argv[])
{
	int retval;

	retval = DC_initClient();
	if (retval)
	{
		fprintf(stderr, "APP: Failed to initialize the DC-API. Return "
			"value was %d\n", retval);
		DC_finishClient(1);
	}
	fprintf(stderr, "APP: DC-API initialization was successful.\n");

	init_files();
	do_work();
	fprintf(stderr, "APP: Work finished.\n");

	DC_finishClient(0);
	return(0); // Tho' we never reach this line
}

#ifdef _WIN32

extern int parse_command_line( char *, char ** );

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode)
{
	LPSTR command_line;
	char* argv[100];
	int argc;

	command_line = GetCommandLine();
	argc = parse_command_line( command_line, argv );
	return main(argc, argv);
}
#endif


