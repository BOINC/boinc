/**************************************************
 * Checkpointing example Master program
 *
 * Created by: Gabor Vida
 *
 * The aim of the program is to test the serialize and deserialize
 * functions of the master side DC API. The program follows the
 * client server model, therefore having a server-side and
 * a client-side part.
 *
 * This is the server-side part of the program. It creates a WorkUnit (WU) for
 * a Boinc project using the DC Application Programming Interface. The program
 * takes a text file (input.txt) and submits it into the PRC-infrastructure,
 * where it get processed by a client. After the submittion, the master makes a
 * checkpoint with the DC_serializeWU function, than exits.
 * At the next start is tries to deserialize the saved WU, and waits for the result.
 * If everything goes well, the master will get the result of the serialized and
 * deserialized WorkUnit.
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
#include <unistd.h>

#include "common.h"

#define CHECKPOINT_LABEL	"ckpt.txt"

static int finished_wus;

static DC_Workunit *workunit;

/* Command line options */
static const struct option longopts[] =
{
	{ "config",	required_argument,	NULL,	'c' },
	{ "help",	no_argument,		NULL,	'h' },	
	{ NULL }
};

static void create_ckpt(void)
{
	FILE *f;
	char *ckpt;

	f = fopen(CHECKPOINT_LABEL, "w");
	if (!f)
	{
		DC_log(LOG_ERR, "Failed to open ckpt file (%s) for writing.", CHECKPOINT_LABEL);
		exit(1);
	}

	ckpt = DC_serializeWU(workunit);

	fprintf(f, "%s", ckpt);
	fclose(f);

	DC_log(LOG_NOTICE, "Master: serialized: %s", ckpt);
}

static void create_work(void)
{
	workunit = DC_createWU("serialize-example", NULL, 0, "serialize-test");
	if (!workunit)
	{
		DC_log(LOG_ERR, "Work unit creation has failed");
		exit(1);
	}

	if (DC_addWUInput(workunit, INPUT_LABEL, "input.txt",	DC_FILE_PERSISTENT))
	{
		DC_log(LOG_ERR, "Failed to register WU input file");
		exit(1);
	}

	if (DC_addWUOutput(workunit, OUTPUT_LABEL))
	{
		DC_log(LOG_ERR, "Failed to register WU output file");
		exit(1);
	}

	if (DC_submitWU(workunit))
	{
		DC_log(LOG_ERR, "Failed to submit WU");
		exit(1);
	}
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
	DC_MasterEvent *event;
	FILE *f;
	char buf[512];
	char *config_file = NULL;
	int c;

	while ((c = getopt_long(argc, argv, "ch", longopts, NULL)) != -1)
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
		fprintf(stderr, "Error: the config file is not specified\n");
		exit(1);
	}

	/* Initialize the DC-API */
	if (DC_initMaster(config_file))
	{
		fprintf(stderr, "Master: DC_initMaster failed, exiting.\n");
		exit(1);
	}

	/* Try to open checkpoint file */
	f = fopen(CHECKPOINT_LABEL, "r");
	if (f)
	{
		DC_log(LOG_NOTICE, "Master: Found checkpoint!");
		fgets(buf, sizeof(buf), f);
		workunit = DC_deserializeWU(buf);
		if(!workunit)
		{
			DC_log(LOG_ERR, "DC_desirializeWU returned NULL for WU: %s", buf);
			exit(1);
		}
		fclose(f);
		DC_log(LOG_NOTICE, "Master: deserialized: %s", buf);
	}
	else
	{
		DC_log(LOG_NOTICE, "Master: Creating work unit");
		create_work();
		DC_log(LOG_NOTICE, "Master: Work unit has been created and submitted.");
		DC_log(LOG_NOTICE, "Creating checkpoint.");
		create_ckpt();
		DC_log(LOG_NOTICE, "Master: Exit application. Please restart it!");
		exit(0);
	}

	/* Wait for the work units to finish */
	while (finished_wus == 0)
	{
		DC_log(LOG_NOTICE, "Master: Starting waitWUEvent with timeout 600 for the deserialized WU");
		event = DC_waitWUEvent(workunit, 600);
		if(!event)
			continue;

		if (event->type == DC_MASTER_RESULT)
		{
			char *tag;

			/* Extract our own private identifier */
			tag = DC_getWUTag(workunit);
			DC_log(LOG_NOTICE, "Work unit %s has completed", tag);

			/* We no longer need the work unit */
			DC_destroyWU(workunit);

			finished_wus++;

			free(tag);
		}
		DC_destroyMasterEvent(event);
	}

	unlink(CHECKPOINT_LABEL);

	return 0;
}
