#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dc_client.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "common.h"

static FILE *infile;
static FILE *outfile;

/* Open input and output files, and check is there is a checkpoint file */
static void init_files(void)
{
	char *file_name;

	/* Open the input file */
	file_name = DC_resolveFileName(DC_FILE_IN, INPUT_LABEL);
	if (!file_name)
	{
		fprintf(stderr, "APP: Could not resolve the input file name\n");
		DC_finishClient(1);
	}

	infile = fopen(file_name, "r");
	if (!infile)
	{
		perror("APP: Could not open the input file");
		DC_finishClient(1);
	}
	free(file_name);

	/* Open the output file */
	file_name = DC_resolveFileName(DC_FILE_OUT, OUTPUT_LABEL);
	if (!file_name)
	{
		fprintf(stderr, "APP: Could not resolve the output file name\n");
		DC_finishClient(1);
	}

	outfile = fopen(file_name, "w");
	if (!outfile)
	{
		perror("APP: Could not open/create the output file");
		DC_finishClient(1);
	}
	free(file_name);
}

/* The real work */
static void do_work(void)
{
	int c;

	while ((c = fgetc(infile)) != EOF)
	{
		DC_ClientEvent *event;

		c = toupper(c);
		if (fputc(c, outfile) == EOF)
		{
			perror("APP: fputc");
			DC_finishClient(1);
		}

		/* Real applications do real computation that takes time. Here
		 * we just emulate it */
		sleep(3);

		/* Check if either the master or the BOINC core client asked
		 * us to do something */
		event = DC_checkClientEvent();
		if (!event)
			continue;

		/* If it is time, do a checkpoint */
		if (event->type == DC_CLIENT_CHECKPOINT)
			DC_checkpointMade(NULL);

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
	fprintf(stdout, "APP: DC-API initialization was successful.\n");

	init_files();
	do_work();
	fprintf(stdout, "APP: Work finished.\n");

	DC_finishClient(0);
	return(0); // Tho' we never reach this line
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode)
{
	LPSTR command_line;
	char *argv[100];
	int argc;

	command_line = GetCommandLine();
	argc = parse_command_line(command_line, argv);
	return main(argc, argv);
}
#endif


