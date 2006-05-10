#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dc.h"

/***************************************
 * Definitions
 */

#define TEST_LOW_LVL		1
#define TEST_HIGH_LVL		2
#define TEST_ERRORS		3
#define TEST_STRESS		4

/***************************************
 * Global variables
 */

DC_GridCapabilities gc;
DC_Workunit *wu[10];

static int testlevel = 0;
static int finished = 0;

/***************************************
 * Function prototypes
 */

static int init(int argc, char *argv[]);
static int createWU(void);
static int submitWU(void);
static void countWUs(void);
static void callbackResult(DC_Workunit *wu_result, DC_Result *result);
static void callbackSubresult(DC_Workunit *wu, const char *label, const char *path);
static void callbackMessage(DC_Workunit *wu, const char *message);

/***************************************
 * Functions
 */

int main(int argc, char *argv[])
{
	int retval;

	retval = init(argc, argv);
	if (retval)
		return retval;

	retval = createWU();
	if (retval)
		return retval;

	retval = submitWU();
	if (retval)
		return retval;

	if (testlevel != TEST_LOW_LVL) countWUs();

	while(!finished)
	{
		printf("Starting DC_processEvents with parameter: 100\n");
		retval = DC_processEvents(100);
		if (retval)
		{
			printf("Error:  No event processed in 100 sec.  retval = %d\n", retval);
		}
	}
	printf("Exit application\n");

	return 0;
}


int init(int argc, char *argv[])
{
	int retval, i;
/*
	if (argc != 2)
	{
		printf("Test application need testing lvl argumentum:\n\n");
		printf("  -testlvl=low\n");
		printf("    Create only 1 WU, submit it, and wait for result\n");
		printf("    Using only the minimum DC-API functions\n");
		printf("  -testlvl=high\n");
		printf("    Create more WUs, submit them, ... etc\n");
		printf("    Use all available DC-API functions\n");
		printf("  -testlvl=errors\n");
		printf("    Create WUs for testing the abnormal working\n");
		printf("  -testlvl=stress\n");
		printf("    Under construction....\n");
		return 1;
	}
*/
	if (argc == 1)
	{
		testlevel = TEST_LOW_LVL;
	}

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-testlvl=low"))
		{
			testlevel = TEST_LOW_LVL;
		}
		else if (!strcmp(argv[i], "-testlvl=high"))
		{
			testlevel = TEST_HIGH_LVL;
		}
		else if (!strcmp(argv[i], "-testlvl=errors"))
		{
			testlevel = TEST_ERRORS;
		}
		else if (!strcmp(argv[i], "-testlvl=stress"))
		{
			testlevel = TEST_STRESS;
		}
		else 
		{
			printf("Undefined argument '%s'\n", argv[i]);
			return -1;
		}
	}

	printf("Initialise DC\n");
	retval = DC_init("uppercase-demo-master.conf");
	if (retval != DC_OK)
	{
		printf("DC_init returned with error. Exit application.\n");
		return -1;
	}
	printf("DC_init returned with OK.\n");

	printf("Set callback functions\n");
	DC_setcb(callbackResult, callbackSubresult, callbackMessage);

	printf("Get Grid capabilities:\n");
	gc = DC_getGridCapabilities();

	if (gc & DC_GC_EXITCODE) 
	{
		printf("exitcode ...     ok\n");
	}
	else
	{
		printf("exitcode ...     not ok\n");
	}
	if (gc & DC_GC_STDOUT) 
	{
		printf("stdout ...       ok\n");
	}
	else
	{
		printf("stdout ...       not ok\n");
	}
	if (gc & DC_GC_STDERR) 
	{
		printf("stderr ...       ok\n");
	}
	else
	{
		printf("stderr ...       not ok\n");
	}
	if (gc & DC_GC_LOG) 
	{
		printf("Log ...          ok\n");
	}
	else
	{
		printf("Log ...          not ok\n");
	}
	if (gc & DC_GC_SUSPEND) 
	{
		printf("Suspend ...      ok\n");
	}
	else
	{
		printf("Suspend ...      not ok\n");
	}
	if (gc & DC_GC_SUBRESULT) 
	{
		printf("Subresult ...    ok\n");
	}
	else
	{
		printf("Subresult ...    not ok\n");
	}
	if (gc & DC_GC_MESSAGING)
	{
		printf("Messaging ...    ok\n");
	}
	else
	{
		printf("Messaging ...    not ok\n");
	}

	printf("\n");

	return 0;
}

static void callbackResult(DC_Workunit *wu_result, DC_Result *result)
{
	char *outfile;
	FILE *f;
	char buf[512];

	if (testlevel != TEST_LOW_LVL)
	{
		int status = DC_getResultExit(result);
		printf("The exit status of the WU is: %d\n", status);
	}

	outfile = DC_getResultOutput(result, "out.txt");
	if (!outfile)
	{
		printf("DC_getResultOutput returned with NULL for out.txt\nExit application...\n");
		exit(1);
	}

	f = fopen(outfile, "r");
	if (!f)
	{
		printf("Cannot open output file: %s\nExit application...\n", outfile);
		exit(1);
	}

	int i = 0;
	while (fgets(buf, sizeof(buf), f))
	{
		printf("%d. line of out.txt: %s", ++i, buf);
		switch(i)
		{
			case 1: if (!strcmp(buf, "THIS IS A TEST INPUT FILE FOR THE UPPERCASE-DEMO APPLICATION!"))
				{
					printf("Error. The output file is not correct at line %d\n", i);
				}
				break;
			case 2: if (!strcmp(buf, "MESSAGE = HELLO MASTER APPLICATION"))
				{
					printf("Error. The output file is not correct at line %d\n", i);
				}
				break;
			default: printf("The %d line of out.txt is %s\n", i, buf);
		}
	}
	fclose(f);

	if (testlevel != TEST_LOW_LVL)
	{
		outfile = DC_getResultOutput(result, DC_RESULT_STDOUT);
		if (!outfile)
		{
			printf("callbackResult: DC_getResultOutput returned with NULL for stdout. Exit application.\n");
			exit(1);
		}

		f = fopen(outfile, "r");
		if (!f)
		{
			printf("callbackResult: Cannot open output file: %s\n", outfile);
			exit(1);
		}

		fgets(buf, sizeof(buf), f);
		fclose(f);
		printf("callbackResult: 1. line of stdout: %s", buf);
		//printf("callbackResult: destroying WU\n");
		//wu = DC_getResultWU(result);
	}

	finished = 1;
	DC_destroyWU(wu_result);
	return;
}

static void callbackSubresult(DC_Workunit *wu, const char *label,
	const char *path)
{
	// not impl yet

	return;
}

static void callbackMessage(DC_Workunit *wu, const char *message)
{
	char *tag;
	char *id;

	id = DC_getWUId(wu);
	tag = DC_getWUTag(wu);

	printf("callbackMessage: WU with id: %s and tag: %s sent message: %s\n", id, tag, message);

	return;
}

static int createWU(void)
{
	int i, retval, wu_num = 0;
	const char *arg[1];

	if (testlevel == TEST_LOW_LVL)
	{
		wu_num = 1;
		arg[0] = strdup("low_lvl");
	}
	else arg[0] = NULL;

	for (i = 0; i < wu_num; i++)
	{
		printf("Creating upper_case workunit\n");
		wu[i] = DC_createWU("upper_case", NULL, 5, "testing");
		if (!wu[i])
		{
			printf("DC_createWU returned with NULL\n");
			return -1;
		}
		if (testlevel == TEST_LOW_LVL) printf("Workunit created\n");
		else printf("Workunit created with id: %s\n", DC_getWUId(wu[i]));

		printf("Add persistent input file to the workunit test_input.txt -> in.txt \n");
		retval = DC_addWUInput(wu[i], "in.txt", "test_input.txt", DC_FILE_PERSISTENT);
		if (retval)
		{
			printf("DC_addWUInput returned with error\n");
			return -1;
		}
		printf("DC_addWUInput returned with OK\n");

		printf("Add output file out.txt to the workunit\n");
		retval = DC_addWUOutput(wu[i], "out.txt");
		if (retval)
		{
			printf("DC_addWUOutput returned with error\n");
			return -1;
		}
		printf("DC_addWUOutput returned with OK\n");

		if (testlevel != TEST_LOW_LVL)
		{
			printf("Set workunit priority to 100\n");
			retval = DC_setWUPriority(wu[i], 100);
			if (retval)
			{
				printf("DC_setWUPriority returned with error\n");
				return -1;
			}
			printf("DC_setWUPriority returned with OK\n");
		}
	}
	return 0;
}

static int submitWU(void)
{
	int i, retval, wu_num = 0;

	if (testlevel == TEST_LOW_LVL)
	{
		wu_num = 1;
	}

	for (i = 0; i < wu_num; i++)
	{
		printf("Submitting workunit\n");
		retval = DC_submitWU(wu[i]);
		if (retval)
		{
			printf("DC_submitWU returned with error. Exit application.\n");
			return -2;
		}
		printf("DC_submitWU returned with OK\n");
	}
	return 0;
}

static void countWUs(void)
{
	int i;

	printf("----------------------------\n");
	printf("countWUs:\n");

	i = DC_getWUNumber(DC_WU_READY);
	printf("    READY: %d\n",i);

	i = DC_getWUNumber(DC_WU_RUNNING);
	printf("    RUNNING: %d\n",i);

	i = DC_getWUNumber(DC_WU_FINISHED);
	printf("    FINISHED: %d\n",i);

	i = DC_getWUNumber(DC_WU_SUSPENDED);
	printf("    SUSPENDED: %d\n",i);

	i = DC_getWUNumber(DC_WU_ABORTED);
	printf("    ABORTED: %d\n",i);

	i = DC_getWUNumber(DC_WU_UNKNOWN);
	printf("    UNKNOWN: %d\n",i);

	printf("----------------------------\n");
}

