/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "user_log.c++.h"

#include "dc.h"
#include "dc_common.h"

#include "condor_defs.h"
#include "condor_result.h"
#include "condor_wu.h"
#include "condor_log.h"


void
_DC_wu_update_condor_events(DC_Workunit *wu)
{
	ReadUserLog log;
	GString *fn_org, *fn_tmp;
	char t[100];
	int res;
	unsigned int i;

	if (!wu)
		return;

	fn_org= g_string_new(wu->workdir);
	fn_org= g_string_append(fn_org, "/internal_log.txt");
	strcpy(t, "/tmp/condor_dc_api_XXXXXX");
	res= mkstemp(t);
	fn_tmp= g_string_new(t);
	close(res);
	_DC_copyFile(fn_org->str, fn_tmp->str);
	res= log.initialize(fn_tmp->str);
	if (!res)
	{
		DC_log(LOG_ERR, "Condor user log file reader "
		       "initialization failed for %s", fn_tmp->str);
		g_string_free(fn_org, TRUE);
		g_string_free(fn_tmp, TRUE);
		return;
	}

	i= 1;
	ULogEvent *event;
	while (log.readEvent(event) == ULOG_OK)
	{
		/*printf("%d %s\n", event->eventNumber,
		  ULogEventNumberNames[event->eventNumber]);*/
		if (wu->condor_events->len < i)
		{
			struct _DC_condor_event e;
			e.event= event->eventNumber;
			e.cluster= event->cluster;
			e.proc= event->proc;
			e.subproc= event->subproc;
			e.time= mktime(&(event->eventTime));
			g_array_append_val(wu->condor_events, e);
			DC_log(LOG_DEBUG, "Condor event %d %s",
			       event->eventNumber,
			       ULogEventNumberNames[event->eventNumber]);
		}
		delete event;
		i++;
	}
	
	unlink(fn_tmp->str);
	g_string_free(fn_org, TRUE);
	g_string_free(fn_tmp, TRUE);
	return;
}


DC_MasterEvent *
_DC_wu_condor2api_event(DC_Workunit *wu)
{
	unsigned int i;

	if (!_DC_wu_check(wu))
		return(NULL);
	for (i= 0; i < wu->condor_events->len; i++)
	{
		struct _DC_condor_event *ce;
		ce= &g_array_index(wu->condor_events,
				   struct _DC_condor_event,
				   i);
		if (ce->event == ULOG_JOB_TERMINATED)
		{
			DC_MasterEvent *e;
			e= g_new0(DC_MasterEvent, 1);
			DC_log(LOG_DEBUG, "API event created: %p", e);	       
			e->type= DC_MASTER_RESULT;
			e->wu= wu;
			e->result= _DC_create_result(wu);
			DC_log(LOG_DEBUG, "Result of the event: %p",
			       e->result);
			wu->state= DC_WU_FINISHED;
			return(e);
		}
	}
	return(NULL);
}


/* End of condor_log.cc */
