/*
 * condor/tss.c
 *
 * DC-API test application, slave
 *
 * (c) Daniel Drotos, 2006
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "dc_client.h"

#include "tc.h"


int cyc= 1;
int sub= 0;
int ckp= 0;
int msg= 0;

static void
send_subresult(int c)
{
	char l[100];
	char p[100];
	char s[100];

	printf("cyc=%d Sending subresult...\n", c);
	sprintf(l, "l_%d", c);
	sprintf(p, "p_%d", c);
	sprintf(s, "subresult %d", c);
	create_file(p, s);
	DC_sendResult(l, p, DC_FILE_REGULAR);
}

static void
mk_checkpoint(int c)
{
	char *fn;
	char buf[100];

	printf("cyc=%d Making checkpoint...\n", c);
	fn= DC_resolveFileName(DC_FILE_OUT, DC_CHECKPOINT_FILE);
	if (fn)
	{
		sprintf(buf, "%d\n", c);
		create_file(fn, buf);
		printf("Checkpoint created in file %s\n", fn);
		DC_checkpointMade(fn);
		free(fn);
	}
	else
		printf("Failed\n");
}

static void
send_message(int c)
{
	char s[100];

	printf("cyc=%d Sending message...\n", c);
	sprintf(s, "Message=%d, cyc=%d", msg, c);
	DC_sendMessage(s);
}

int
main(int argc, char *argv[])
{
	DC_initClient();
	int i;

	if (argc > 1)
	{
		cyc= strtol(argv[1], 0, 0);
		printf("cyc defined= %d\n", cyc);
	}
	if (argc > 2)
	{
		sub= strtol(argv[2], 0, 0);
		printf("sub defined= %d\n", sub);
	}
	if (argc > 3)
	{
		ckp= strtol(argv[3], 0, 0);
		printf("ckp defined= %d\n", ckp);
	}
	if (argc > 4)
	{
		msg= strtol(argv[4], 0, 0);
		printf("msg defined= %d\n", msg);
	}

	i= 0;
	{
		char *fn= DC_resolveFileName(DC_FILE_IN, DC_CHECKPOINT_FILE);
		if (fn)
		{
			char *s= get_file(fn);
			printf("Checkpoint file found %s\n", fn);
			printf("Content: \"%s\"\n", s);
			i= strtol(s, 0, 0);
			printf("Continuing from cycle %d\n", i);
			cyc-= i;
			free(s);
			free(fn);
		}
		else
			printf("No checkpoint file found, start from the beginning...\n");
	}
	while (cyc)
	{
		cyc--;
		i++;
		printf("\nCycle %d. starting...\n", i);

		{
			DC_ClientEvent *e= DC_checkClientEvent();
			if (e)
			{
				switch (e->type)
				{
				case DC_CLIENT_CHECKPOINT:
				{
					break;
				}
				case DC_CLIENT_FINISH:
				{
					break;
				}
				case DC_CLIENT_MESSAGE:
				{
					printf("At cyc=%d got a message: \"%s\"\n", i,
					       e->message);
					break;
				}
				}
				DC_destroyClientEvent(e);
			}
		}
		sleep(1);

		if (sub &&
		    i%sub == 0)
			send_subresult(i);
		if (ckp &&
		    i%ckp == 0)
			mk_checkpoint(i);
		if (msg)
		{
			send_message(i);
			msg--;
		}
		printf("Done, remaining cycles: %d\n", cyc);
	}

	DC_finishClient(0);
}

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
