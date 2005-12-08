#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#include "defines.h"
#include "dc.h"


static int remove_wu(char *wuname, char *workdir)
{
	FILE *f;
	char syscmd[512], buf[512];
	char *id;

	buf[0] = 0;

	DC_log(LOG_DEBUG,"REMOVE: %s work unit is under removing.", wuname);
	
	sprintf(syscmd, "clgr_rm -i %s", wuname);
	f = popen(syscmd, "r");
	if(!feof(f))
	{
	    fgets(buf, sizeof(buf), f);
	    pclose(f);
	}
	else
	{
	    DC_log(LOG_ERR,"REMOVE: Cannot accomplish clgr_rm system-command!");
	    return DC_ERROR;
	}	

	if (strlen(buf) == 0)
	{
	    DC_log(LOG_ERR,"REMOVE: Cannot accomplish clgr_rm system-command!");
	    return DC_ERROR;
	}

	id = strdup(buf);
	id = strtok(buf, " ");
	id = strtok(NULL, " ");
	id = strtok(NULL, " ");
	id = strtok(NULL, " ");
	
	if (!strcmp(id, "ok"))
	{
	    DC_log(LOG_ERR,"REMOVE: clgr_rm error message: '%s'", id);
	    return DC_ERROR;
	}

	DC_log(LOG_DEBUG,"REMOVE: %s workunit was removed.", wuname);
	return DC_OK;
	
}
