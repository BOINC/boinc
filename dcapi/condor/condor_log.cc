/*
 * condor/condor_log.cc
 *
 * DC-API utils to read and process condor "user log" file
 *
 * (c) Daniel Drotos, 2006
 */


#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "read_user_log.h"

#include "dc.h"
#include "dc_common.h"

#include "condor_log.h"

#include "condor_defs.h"
#include "condor_event.h"
#include "condor_result.h"
#include "condor_wu.h"
#include "condor_utils.h"


void
_DC_wu_update_condor_events(DC_Workunit *wu)
{
	ReadUserLog log;
	GString *fn_org, *fn_tmp;
	char t[100];
	int res;
	unsigned int i;

	if (!wu) {
        DC_log(LOG_DEBUG, "Invalid work unit passed.");
		return;	    
	}
	
	fn_org= g_string_new(wu->data.workdir);
	fn_org= g_string_append(fn_org, "/");
	fn_org= g_string_append(fn_org, _DC_wu_cfg(wu, cfg_condor_log));
	strcpy(t, "/tmp/condor_dc_api_XXXXXX");
	res= mkstemp(t);
	fn_tmp= g_string_new(t);
	close(res);
	_DC_copyFile(fn_org->str, fn_tmp->str);
	res= log.initialize(fn_tmp->str);
	if (res != 1)
	{
		DC_log(LOG_ERR, "Condor user log file reader "
		       "initialization failed for %s", fn_tmp->str);
		unlink(fn_tmp->str);
		g_string_free(fn_org, TRUE);
		g_string_free(fn_tmp, TRUE);
		return;
	}

	i= 1;
	ULogEvent *event;
	ULogEventOutcome result;
	while ((result = log.readEvent(event)) != ULOG_NO_EVENT)
	{
        switch (result) 
        {
	        case ULOG_RD_ERROR:
                DC_log(LOG_ERR, "read error in the log");
	            break;
	        case ULOG_UNK_ERROR:
                DC_log(LOG_ERR, "unknown error in the log");
                break;	        
	        case ULOG_OK: 
        		if (wu->condor_events->len < i)
        		{
        			struct _DC_condor_event e;
        			e.event= event->eventNumber;
        			e.cluster= event->cluster;
        			e.proc= event->proc;
        			e.subproc= event->subproc;
        			e.time= mktime(&(event->eventTime));
        			e.reported= FALSE;
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
        			if (e.event == ULOG_EXECUTE)
        			{
        				class ExecuteEvent *ae=
        					dynamic_cast<class ExecuteEvent *>
        					(event);
        				if (ae)
        				{
        					e.exec_info.host=
        						g_strdup(ae->getExecuteHost());
        				}
        			}
        			g_array_append_val(wu->condor_events, e);
        			DC_log(LOG_DEBUG, "Condor event %d %s of %p-\"%s\"",
        			       event->eventNumber,
        			       ULogEventNumberNames[event->eventNumber],
        			       wu, wu->data.name);
        		} 
        		i++;
                delete event;
                break;
	        default:
	            DC_log(LOG_ERR, "should never get here.");
                break;
	    }
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
		if (ce->reported)
			continue;
		ce->reported= TRUE;
		if (ce->event == ULOG_JOB_TERMINATED)
		{
			DC_log(LOG_DEBUG, "Job terminated, "
			       "asked to susp=%d",
			       wu->asked_to_suspend);
			GString *s;
			s= g_string_new("");
			g_string_printf(s, "%s/%s", wu->data.workdir,
					_DC_wu_cfg(wu, cfg_management_box));
			char *message;
			if ((message= _DC_read_message(s->str,
						       (char *)_DCAPI_MSG_ACK,
						       FALSE)))
			{
				DC_log(LOG_DEBUG, "Acknowledge exists: %s",
				       message);
				if (strcmp(message, _DCAPI_ACK_SUSPEND) == 0)
				{
					DC_log(LOG_DEBUG, "Suspend "
					       "acknowledged");
					_DC_read_message(s->str,
							 (char *)_DCAPI_MSG_ACK,
							 TRUE);
					wu->asked_to_suspend= TRUE;
				}
			}
			g_string_free(s, TRUE);

			if (wu->asked_to_suspend)
			{
				_DC_wu_set_state(wu, DC_WU_SUSPENDED);
				wu->asked_to_suspend= FALSE;
			}
			else
			{
				DC_MasterEvent *e;
				e= _DC_event_create(wu, _DC_result_create(wu),
						    NULL, NULL);
				DC_log(LOG_DEBUG, "Result event created: "
				       "%p for wu (%p-\"%s\")",
				       e, wu, wu->data.name);
				DC_log(LOG_DEBUG, "Result of the event: %p",
				       e->result);
				_DC_wu_set_state(wu, DC_WU_FINISHED);
				return(e);
			}
		}
		if (ce->event == ULOG_EXECUTE)
		{
			/* Fix #1105 */
			/*_DC_wu_set_state(wu, DC_WU_RUNNING);*/
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


/* Returns the CPU time used for computing the result, in seconds. */

double
DC_getResultCPUTime(const DC_Result *result)
{
	DC_Workunit *wu;
	unsigned int i;
	time_t start= (time_t)-1, end= (time_t)-1;

	if (result == NULL)
		return(0.0);
	wu= result->wu;
	if (wu == NULL)
		return(0.0);

	for (i= 0; i < wu->condor_events->len; i++)
	{
		struct _DC_condor_event *ce;
		ce= &g_array_index(wu->condor_events,
				   struct _DC_condor_event,
				   i);
		if (ce->event == ULOG_EXECUTE)
			start= ce->time;
		if (ce->event == ULOG_JOB_TERMINATED ||
		    ce->event == ULOG_JOB_ABORTED)
			end= ce->time;
	}
	if (start == (time_t)-1 ||
	    end == (time_t)-1)
		return(0.0);
	
	return difftime(end, start);
}


/* End of condor_log.cc */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
