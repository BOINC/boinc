/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dc.h"

DC_Workunit *wu[10];

static void
fail(char *what, int ret)
{
	printf("%s failed: %d\n", what, ret);
	exit(ret);
}

static DC_Workunit *
create_short(void)
{
	DC_Workunit *wu;
	
	printf("Creating short...\n");
	wu= DC_createWU("short", NULL, 0, NULL);
	printf("Created short wu: %p\n", wu);
	if (!wu)
		fail("DC_createWU", 0);
	return(wu);
}

static DC_Workunit *
create_long(void)
{
	DC_Workunit *wu;
	const char *argv[]= {
		"15",
		"2",
		"3",
		"2",
		NULL
	};
	printf("Creating long...\n");
	wu= DC_createWU("long", argv, 0, NULL);
	printf("Created short wu: %p\n", wu);
	if (!wu)
		fail("DC_createWU", 0);
	return(wu);
}

static void
result_cb(DC_Workunit *wu, DC_Result *result)
{
	printf("\nresult_cb, wustate=%d\n", DC_getWUState(wu));
	printf("exit= %d\n", DC_getResultExit(result));
}

static void
subresult_cb(DC_Workunit *wu,
	     const char *logical_file_name,
	     const char *path)
{
	printf("\nsubresult_cb, wustate=%d\n", DC_getWUState(wu));
	printf("logical= \"%s\"\n", logical_file_name);
	printf("path   = \"%s\"\n", path);
}

static void
message_cb(DC_Workunit *wu, const char *message)
{
	printf("\nmessage_cb, wustate=%d\n", DC_getWUState(wu));
	printf("Message= \"%s\"\n", message);
}


static void
t(int what)
{
	DC_Workunit *wu;
	int i;

	switch (what)
	{
	case 1:
	{
		printf("test1: running a short wu\n");
		wu= create_short();
		printf("Submitting short...\n");
		if ((i= DC_submitWU(wu)) != 0)
			fail("DC_submitWU", i);
		DC_setMasterCb(result_cb, subresult_cb, message_cb);
		while (DC_getWUState(wu) != DC_WU_FINISHED)
		{
			printf("Processing events for 5 sec...\n");
			DC_processMasterEvents(5);
		}
		printf("Destroying short...\n");
		DC_destroyWU(wu);
		break;
	}
	case 2:
	{
		printf("test2: running a long wu\n");
		wu= create_long();
		printf("Submitting long...\n");
		if ((i= DC_submitWU(wu)) != 0)
			fail("DC_submitWU", i);
		DC_setMasterCb(result_cb, subresult_cb, message_cb);
		while (DC_getWUState(wu) != DC_WU_FINISHED)
		{
			printf("Processing events for 5 sec...\n");
			DC_processMasterEvents(5);
		}
		printf("wu finished, but checking events for more 5 secs...\n");
		DC_processMasterEvents(5);
		printf("Destroying long...\n");
		DC_destroyWU(wu);
		break;
	}
	}
}


int
main(int argc, char *argv[])
{
	int i;

	if ((i= DC_initMaster("t.conf")) != 0)
		fail("DC_initMaster", i);

	if (argc < 2)
	{
		printf("Calling all tests...\n");
		t(1);
		t(2);
	}

	for (i= 1; i < argc; i++)
	{
		int l;
		l= strtol(argv[i], 0, 0);
		printf("Calling test %d...\n", l);
		t(l);
	}

	return 0;
}
