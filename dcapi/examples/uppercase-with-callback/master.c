/**************************************************
 * Uppercase Master program
 *
 * Created by: Gabor Vida
 * Fine tuned by: Adam Kornafeld
 * Upgraded to DC-API 2.0 by: Gabor Vida
 *
 * The aim of the program is to process text files (make all of their
 * characters uppercase) in a Public Resource Computing environment. The
 * program follows the client server model, therefore having a server-side and
 * a client-side part.
 *
 * This is the server-side part of the program. It creates WorkUnits (WUs) for
 * a Boinc project using the DC Application Programming Interface.  The program
 * takes a text file (master_input) divides it to smaller parts of LINES_PER_WU
 * lines each and submits them into the PRC-infrastructure, where they get
 * processed by the clients. After the clients have returned the results of the
 * WUs, the program assimilates them and constructs a final output file, by
 * concatenating the output of the results.
 *
 *****************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dc.h>

#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

#include "common.h"

#define LINES_PER_WU		20

/* Number of WUs we have created */
static int created_wus;
/* Number of results we have received so far */
static int processed_wus;

/* Command line options */
static const struct option longopts[] =
{
	{ "config",	required_argument,	NULL,	'c' },
	{ "help",	no_argument,		NULL,	'h' },	
	{ NULL }
};

/* Concatenate all results in their original order to form the final output */
static void calculate_final_result(void)
{
	FILE *output;
	int i;

	output = fopen("final_output.txt", "w");
	if (!output)
	{
		DC_log(LOG_ERR, "Cannot open final_output.txt for writing: %s",
			strerror(errno));
		exit(1);
	}

	for(i = 0; i < created_wus; i++)
	{
		char result_filename[PATH_MAX], buf[256];
		FILE *result;

		snprintf(result_filename, sizeof(result_filename),
			"result_%d.txt", i);
		result = fopen(result_filename, "r");
		if (!result)
			DC_log(LOG_WARNING, "The result for WU %d is missing "
				"from the final output", i);
		else
		{
			while (fgets(buf, sizeof(buf), result))
				fprintf(output, buf);
			fclose(result);
		}
	}

	fclose(output);
}

/* Copies the output files into the master working directory */
static void process_result(DC_Workunit *wu, DC_Result *result)
{
	char *output_filename, *tag, cmd[256];

	/* Internal housekeeping of the number of results we have seen */
	processed_wus++;

	/* Extract our own private identifier */
	tag = DC_getWUTag(wu);

	if (!result)
	{
		DC_log(LOG_WARNING, "Work unit %s has failed", tag);
		free(tag);

		/* In a real application we would create a new work unit
		 * instead of the failed one and submit it again. Here we
		 * just let it fail. */
		return;
	}

	output_filename = DC_getResultOutput(result, OUTPUT_LABEL);
	if (!output_filename)
	{
		DC_log(LOG_WARNING, "Work unit %s contains no output file",
			tag);
		free(tag);

		/* The same comment applies here as above for the no result
		 * case */
		return;
	}

	DC_log(LOG_NOTICE, "Work unit %s has completed", tag);

	/* The result file is available only as long as the work unit exists,
	 * so we have to save it if we want to use it later */
	snprintf(cmd, sizeof(cmd), "/bin/cp '%s' 'result_%s.txt'",
		output_filename, tag);
	system(cmd);

	/* We no longer need the work unit */
	DC_destroyWU(wu);

	free(output_filename);
	free(tag);
}

static void create_work(void)
{
	FILE *input;

	input = fopen("input.txt", "r");
	if (!input)
	{
		DC_log(LOG_ERR, "Failed to open the master input file: %s",
			strerror(errno));
		exit(1);
	}

	while (!feof(input))
	{
		char wu_tag[16], buf[1024];
		DC_Workunit *wu;
		FILE *f;
		int i;

		/* Tag every WU with its index, so we can reconstruct the
		 * final output file in the correct order */
		snprintf(wu_tag, sizeof(wu_tag), "%d", created_wus + 1);

		wu = DC_createWU("uppercase-example", NULL, 0, wu_tag);
		if (!wu)
		{
			DC_log(LOG_ERR, "Work unit creation has failed");
			exit(1);
		}

		f = fopen("wu-input.txt", "w");
		if (!f)
		{
			DC_log(LOG_ERR, "Failed to create wu-input.txt: %s",
				strerror(errno));
			exit(1);
		}

		for (i = 0; i < LINES_PER_WU; i++)
		{
			if (!fgets(buf, sizeof(buf), input))
				break;
			fprintf(f, "%s", buf);
		}
		fclose(f);

		if (DC_addWUInput(wu, INPUT_LABEL, "wu-input.txt",
				DC_FILE_VOLATILE))
		{
			DC_log(LOG_ERR, "Failed to register WU input file");
			exit(1);
		}
		if (DC_addWUOutput(wu, OUTPUT_LABEL))
		{
			DC_log(LOG_ERR, "Failed to register WU output file");
			exit(1);
		}

		if (DC_submitWU(wu))
		{
			DC_log(LOG_ERR, "Failed to submit WU");
			exit(1);
		}

		/* If everything went well, increment the number of WUs we have
		 * created */
		++created_wus;
	}
	fclose(input);
}

static void print_help(const char *prog) __attribute__((noreturn));
static void print_help(const char *prog)
{
	const char *p;

	/* Strip the path component if present */
	p = strrchr(prog, '/');
	if (p)
		prog = p;

	printf("Usage: %s {-c|--config} <config file>\n", prog);
	printf("Available options:\n");
	printf("\t-c <file>, --config <file>\tUse the specified config. file\n");
	printf("\t-h, --help\t\t\tThis help text\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	char *config_file = NULL;
	int c;

	while ((c = getopt_long(argc, argv, "c:h", longopts, NULL)) != -1)
	{
		switch (c)
		{
			case 'c':
				config_file = optarg;
				break;
			case 'h':
				print_help(argv[0]);
				break;
			default:
				exit(1);
		}
	}

	if (optind != argc)
	{
		fprintf(stderr, "Extra arguments on the command line\n");
		exit(1);
	}

	/* Specifying the config file is mandatory, otherwise the master can't
	 * run as a BOINC daemon */
	if (!config_file)
	{
		fprintf(stderr, "You must specify the config file\n");
		exit(1);
	}

	/* Initialize the DC-API */
	if (DC_initMaster(config_file))
	{
		fprintf(stderr, "Master: DC_initMaster failed, exiting.\n");
		exit(1);
	}

	/* We need the result callback function only */
	DC_setMasterCb(process_result, NULL, NULL);

	DC_log(LOG_NOTICE, "Master: Creating work units");
	create_work();
	DC_log(LOG_NOTICE, "Master: %d work units have been created. Waiting "
		"for results...", created_wus);

	/* Wait for the work units to finish */
	while (processed_wus < created_wus)
		DC_processMasterEvents(600);

	/* Got all the results -> build the final output file */
	calculate_final_result();

	return 0;
}
