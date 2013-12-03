/*
 * condor/tm.c
 *
 * DC-API test application, master
 *
 * (c) Daniel Drotos, 2006
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "dc.h"

#include "tc.h"

#define MAX_WU 100

DC_Workunit *wut[MAX_WU];

extern char *_DC_state_name(DC_WUState state);

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
create_long(int how_long)
{
	DC_Workunit *wu;
	char *argv[]= {
		"15",
		"2",
		"3",
		"2",
		NULL
	};
	char l[100];
	
	printf("Creating long...\n");
	if (how_long > 0)
	{
		sprintf(l, "%d", how_long);
		argv[0]= l;
	}
	wu= DC_createWU("long", (const char **)argv, 0, NULL);
	printf("Created long wu: %p\n", wu);
	if (!wu)
		fail("DC_createWU", 0);
	return(wu);
}

static void
result_cb(DC_Workunit *wu, DC_Result *result)
{
	printf("\nresult_cb, wustate=%s\n",
	       _DC_state_name(DC_getWUState(wu)));
	printf("exit= %d\n", DC_getResultExit(result));
}

static void
subresult_cb(DC_Workunit *wu,
	     const char *logical_file_name,
	     const char *path)
{
	printf("\nsubresult_cb, wustate=%s\n",
	       _DC_state_name(DC_getWUState(wu)));
	printf("logical= \"%s\"\n", logical_file_name);
	printf("path   = \"%s\"\n", path);
	printf("Content: \"%s\"\n", get_file((char*)path));
	printf("Deleting %s...\n", path);
	unlink(path);
}

static void
message_cb(DC_Workunit *wu, const char *message)
{
	printf("\nmessage_cb, wustate=%s\n",
	       _DC_state_name(DC_getWUState(wu)));
	printf("Message= \"%s\"\n", message);
}


static void
ls_lR(int i)
{
	char cmd[100];
	sprintf(cmd, "./ls_lR.sh %03d", i);
	//system(cmd);
}

int cycle= 0;

static void
process(DC_Workunit *wu, int timeout)
{
	printf("-- %3d %s\n", cycle, _DC_state_name(DC_getWUState(wu)));
	DC_processMasterEvents(timeout);
}

static void
procs(DC_Workunit *wu, DC_WUState state, int timeout)
{
	while (DC_getWUState(wu) != state)
	{
		cycle++;
		process(wu, timeout);
	}
}

static void
proci(DC_Workunit *wu, int nr, int timeout)
{
	int j;
	for (j= 0; j < nr; j++)
	{
		cycle++;
		process(wu, timeout);
	}
}

static void
t(int what)
{
	DC_Workunit *wu;

	switch (what)
	{
	case 1:
	{
		int i= 0;
		printf("test1: running a short wu\n");
		wu= create_short();
		printf("Submitting short...\n");
		if ((i= DC_submitWU(wu)) != 0)
			fail("DC_submitWU", i);
		DC_setMasterCb(result_cb, subresult_cb, message_cb);
		while (DC_getWUState(wu) != DC_WU_FINISHED)
		{
			printf("wu is in state %s\n",
			       _DC_state_name(DC_getWUState(wu)));
			printf("Processing events for 1 sec...\n");
			DC_processMasterEvents(1);
		}
		printf("Destroying short...\n");
		DC_destroyWU(wu);
		break;
	}
	case 2:
	{
		int i, j;
		printf("test2: running a long wu\n");
		wu= create_long(0);
		printf("Submitting long...\n");
		if ((i= DC_submitWU(wu)) != 0)
			fail("DC_submitWU", i);
		DC_setMasterCb(result_cb, subresult_cb, message_cb);
		while (DC_getWUState(wu) != DC_WU_FINISHED)
		{
			i++;
			printf("Processing events for 1 sec...\n");
			printf("-- %3d\n", i);
			ls_lR(i);
			DC_processMasterEvents(1);
		}
		printf("wu finished, but checking events for more 5 secs...\n");
		for (j= 0; j < 5; j++)
		{
			i++;
			printf("-- %3d\n", i);
			ls_lR(i);
			DC_processMasterEvents(1);
		}
		printf("Destroying long...\n");
		DC_destroyWU(wu);
		break;
	}
	case 3:
	{
		int r;
		printf("test3: suspend/resume\n");
		wu= create_long(20);
		printf("Submitting long...\n");
		if ((r= DC_submitWU(wu)) != 0)
			fail("DC_submitWU", r);
		DC_setMasterCb(result_cb, subresult_cb, message_cb);

		printf("waiting to be running...\n");
		procs(wu, DC_WU_RUNNING, 1);
		
		printf("now running, run a bit...\n");
		proci(wu, 5, 1);

		printf("Suspending wu...\n");
		if ((r= DC_suspendWU(wu)) != 0)
			fail("DC_suspendWU", r);

		printf("waiting to be suspended...\n");
		procs(wu, DC_WU_SUSPENDED, 1);

		printf("now suspended, wait a bit...\n");
		proci(wu, 3, 1);

		printf("Resuming wu...\n");
		if ((r= DC_resumeWU(wu)) != 0)
			fail("DC_resumeWU", r);

		printf("waiting to be running again...\n");
		procs(wu, DC_WU_RUNNING, 1);
		
		printf("waiting to finish...\n");
		procs(wu, DC_WU_FINISHED, 1);
		
		printf("Destroying long...\n");
		DC_destroyWU(wu);
		break;
	}
	case 4:
	{
		char *s;
		int r;
		wu= create_long(20);
		printf("Ser=\"%s\"\n", s= DC_serializeWU(wu));
		create_file("serialized_wu.txt", s);
		free(s);
		printf("WU serialized\nSubmitting...\n");
		if ((r= DC_submitWU(wu)) != 0)
			fail("DC_submitWU", r);
		DC_setMasterCb(result_cb, subresult_cb, message_cb);

		printf("waiting to be running...\n");
		procs(wu, DC_WU_RUNNING, 1);
		/*DC_destroyWU(wu);*/
		break;
	}
	case 5:
	{
		char *s= get_file("serialized_wu.txt");
		int r;
		wu= DC_deserializeWU(s);
		free(s);
		if (!wu)
		{
			printf("Deserialization failed\n");
			break;
		}

		printf("Submitting deserialized WU...\n");
		if ((r= DC_submitWU(wu)) != 0)
			fail("DC_submitWU", r);
		DC_setMasterCb(result_cb, subresult_cb, message_cb);

		printf("waiting to be running...\n");
		procs(wu, DC_WU_RUNNING, 1);
		
		printf("waiting to finish...\n");
		procs(wu, DC_WU_FINISHED, 1);
		
		printf("Destroying...\n");
		DC_destroyWU(wu);
		break;
	}
	case 6:
	{
		int i, nr= 40;
		int done= 0;
		DC_setMasterCb(result_cb, subresult_cb, message_cb);
		printf("Creating and submitting wus...\n");
		for (i= 0; i < nr; i++)
		{
			wut[i]= create_long(10);
			DC_submitWU(wut[i]);
		}
		while (!done)
		{
			done= 1;
			for (i= 0; i < nr; i++)
			{
				printf("waiting to finish %d...\n", i);
				if (wut[i] &&
				    DC_getWUState(wut[i]) != DC_WU_FINISHED &&
				    DC_getWUState(wut[i]) != DC_WU_UNKNOWN)
				{
					done= 0;
					process(wut[i], 1);
				}
			}
		}
		printf("All WUs finished, processing event for a while...\n");
		DC_processMasterEvents(10);
		printf("Destroying all WUs...\n");
		for (i= 0; i < nr; i++)
		{
			printf("%2d\n", i);
			DC_destroyWU(wut[i]);
			wut[i]= NULL;
		}
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
		t(3);
		t(4);
		printf("WU serialized, now wait a bit...\n");
		sleep(5);
		printf("Trying to deserialize...\n");
		t(5);
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

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
