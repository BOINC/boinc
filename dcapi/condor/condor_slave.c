/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "dc_common.h"
#include "dc_client.h"
#include "dc_internal.h"

#include "condor_common.h"
#include "condor_slave.h"


/* Initializes the client API. */
int DC_initClient(void)
{
	int ret= _DC_parseCfg(CLIENT_CONFIG_NAME);
	if (ret)
		fprintf(stderr, "Error parsing configfile %s",
			CLIENT_CONFIG_NAME);
	DC_log(LOG_DEBUG, "Slave dcapi initialized");
	return(ret);
}


/* Resolves the local name of input/output files. */
char *DC_resolveFileName(DC_FileType type,
			 const char *logicalFileName)
{
	/* init_log calls this fn, so it is not possible to call DC_log
	   from here */
	/*DC_log(LOG_DEBUG, "DC_resolveFileName(%d,%s)",
	  type, logicalFileName);*/
	if (!strcmp(logicalFileName, DC_CHECKPOINT_FILE))
		return(strdup(CKPT_LABEL));
	return(strdup((char*)logicalFileName));
}


/* Sends a sub-result back to the master. */
int DC_sendResult(const char *logicalFileName,
		  const char *path,
		  DC_FileMode fileMode)
{
	DC_log(LOG_DEBUG, "DC_sendResult(%s,%s,%d)",
	       logicalFileName,
	       path,
	       fileMode);
	return(DC_ERR_NOTIMPL);
}


static int _DC_message_id= 0;

/* Sends a message to the master. */
int DC_sendMessage(const char *message)
{
	char fn[254];
	FILE *f;

	DC_log(LOG_DEBUG, "DC_sendMessage(%s)", message);
	return _DC_create_message("client_messages", "message", message, NULL);
}


/* Checks for application control events. */
DC_ClientEvent *DC_checkClientEvent(void)
{
	char *dn;
	DIR *d;
	struct dirent *de;
	int min_id;
	DC_ClientEvent *e= NULL;

	dn= strdup("master_messages");
	d= opendir(dn);
	min_id= -1;
	while (d &&
	       (de= readdir(d)) != NULL)
	{
		char *found= strstr(de->d_name, "message.");
		if (found == de->d_name)
		{
			char *pos= strrchr(de->d_name, '.');
			if (pos)
			{
				int id= 0;
				pos++;
				id= strtol(pos, NULL, 10);
				if (id > 0)
				{
					if (min_id < 0)
						min_id= id;
					else
						if (id < min_id)
							min_id= id;
				}
			}
		}
	}
	if (d)
		closedir(d);
	if (min_id >= 0)
	{
		FILE *f;
		dn= realloc(dn, 100);
		sprintf(dn, "master_messages/message.%d", min_id);
		DC_log(LOG_DEBUG, "Reading message from %s", dn);
		if ((f= fopen(dn, "r")) != NULL)
		{
			char *cont;
			int bs= 100, i;
			char c;

			cont= malloc(bs);
			i= 0;
			cont[i]= '\0';
			while ((c= fgetc(f)) != EOF)
			{
				if (i > bs-2)
				{
					bs+= 100;
					cont= realloc(cont, bs);
				}
				cont[i]= c;
				i++;
				cont[i]= '\0';
			}
			if (i > 0)
			{
				if ((e= calloc(1, sizeof(DC_ClientEvent))))
				{
					DC_log(LOG_DEBUG, "API event created: "
					       "%p", e);
					e->type= DC_CLIENT_MESSAGE;
					e->message= cont;
					DC_log(LOG_DEBUG, "Message of the "
					       "event: %s",
					       e->message);
				}
				else
					DC_log(LOG_ERR, "Failed to create "
					       "API event, memory allocation "
					       "error");
			}
			fclose(f);
			if (e)
				unlink(dn);
		}
	}
	if (dn)
		free(dn);
	return(e);
}


/* Destroys the event-specific data returned by DC_checkClientEvent(). */
void DC_destroyClientEvent(DC_ClientEvent *event)
{
	DC_log(LOG_DEBUG, "DC_destroyClientEvent(%p)", event);
	if (event)
	{
		if (event->message)
			free(event->message);
		free(event);
	}
}


/* Indicates that an application-level checkpoint has completed. */
void DC_checkpointMade(const char *fileName)
{
	DC_log(LOG_DEBUG, "DC_checkpointMade(%s)", fileName);
}


/* Informs the user interface about the fraction of work already done. */
void DC_fractionDone(double fraction)
{
	DC_log(LOG_DEBUG, "DC_fractionDone(%g)", fraction);
}


/* Finishes computation. */
void DC_finishClient(int exitcode)
{
	DC_log(LOG_DEBUG, "DC_finishClient(%d)", exitcode);
	exit(exitcode);
}


/* End of condor/condor_slave.c */
