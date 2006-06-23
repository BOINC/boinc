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
#include "condor_event.h"
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
			if (e.event == ULOG_JOB_TERMINATED)
			{
				class TerminatedEvent *te=
					dynamic_cast<class TerminatedEvent *>
					(event);
				if (te)
				{
					e.exit_info.normal= te->normal;
					e.exit_info.exit_code= te->returnValue;
				}
			}
			if (e.event == ULOG_JOB_ABORTED)
			{
				class JobAbortedEvent *ae=
					dynamic_cast<class JobAbortedEvent *>
					(event);
				if (ae)
				{
					e.abort_info.reason=
						g_strdup(ae->getReason());
				}
			}
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
			/*e= g_new0(DC_MasterEvent, 1);*/
			e= _DC_event_create(wu, _DC_result_create(wu),
					    NULL, NULL);
			DC_log(LOG_DEBUG, "Result event created: %p for "
			       "wu (%p-\"%s\")", e, wu, wu->name);
			/*e->type= DC_MASTER_RESULT;
			e->wu= wu;
			e->result= _DC_result_create(wu);*/
			DC_log(LOG_DEBUG, "Result of the event: %p",
			       e->result);
			wu->state= DC_WU_FINISHED;
			return(e);
		}
	}
	return(NULL);
}


int
_DC_wu_exit_code(DC_Workunit *wu, int *res)
{
	unsigned int i;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	if (!res)
		return(DC_OK);
	for (i= 0; i < wu->condor_events->len; i++)
	{
		struct _DC_condor_event *ce;
		ce= &g_array_index(wu->condor_events,
				   struct _DC_condor_event,
				   i);
		if (ce->event == ULOG_JOB_TERMINATED)
		{
			*res= ce->exit_info.exit_code;
		}
	}
	return(DC_OK);
}


/* End of condor_log.cc */
