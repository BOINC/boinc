/*
 * local/local_events.c
 *
 * DC-API functions related to event handling
 *
 * (c) Gabor Vida 2005-2006, Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "local_master.h"
#include "local_utils.h"
#include "local_result.h"


static int _DC_wu_terminated(DC_Workunit *wu)
{
	char syscmd[256];
	int retval;

	snprintf(syscmd, 256, "ps -p %d >/dev/null", wu->pid);
	retval = system(syscmd);

	/* retval == 0 means that the process is still in output of ps
		but it can be <defunct>.
		So check it again.
	*/
	if (retval == 0) {
		snprintf(syscmd, 256, "ps -p %d | grep defunct >/dev/null",
			wu->pid);
		retval = system(syscmd);
		if (retval == 0) retval = 1; /* defunct means finished */
		else if (retval == 1) retval = 0;
	}
	if (retval == 1) { /* process finished (not exists) */
		/* create the result object */
		DC_log(LOG_INFO, "Work unit %s with pid %d is found to be finished",
			wu->name, wu->pid);
		wu->state = DC_WU_FINISHED;
		return 1;
	}
	return 0;
}

/* by DD */
static DC_MasterEvent *
_DC_event_create(DC_Workunit *wu,
		 DC_Result *result,
		 DC_PhysicalFile *subresult,
		 char *message)
{
	DC_MasterEvent *e;

	if ((e= g_new0(DC_MasterEvent, 1)) == NULL)
		return(NULL);
	e->wu= wu;
	if (result)
	{
		e->type= DC_MASTER_RESULT;
		e->result= result;
	}
	else if (subresult)
	{
		e->type= DC_MASTER_SUBRESULT;
		e->subresult= subresult;
	}
	else if (message)
	{
		e->type= DC_MASTER_MESSAGE;
		e->message= g_strdup(message);
	}
	else
	{
		DC_log(LOG_ERR, "Wrong invokation of _DC_event_create("
		       "%p,%p,%p,%p)", wu, result, subresult, message);
		g_free(e);
		return(NULL);
	}
	return(e);
}

/* by DD */
static DC_MasterEvent *
_DC_wu_infra2api_event(DC_Workunit *wu)
{
	if (_DC_wu_terminated(wu))
	{
		DC_log(LOG_DEBUG, "Job terminated, "
		       "asked to susp=%d",
		       wu->asked_to_suspend);
		GString *s;
		s= g_string_new("");
		g_string_printf(s, "%s/%s", wu->workdir,
				_DC_wu_cfg(wu, cfg_management_box));
		char *message;
		if ((message= _DC_read_message(s->str,
					       _DCAPI_MSG_ACK,
					       FALSE)))
		{
			DC_log(LOG_DEBUG, "Acknowledge exists: %s",
			       message);
			if (strcmp(message, _DCAPI_ACK_SUSPEND) == 0)
			{
				DC_log(LOG_DEBUG, "Suspend "
				       "acknowledged");
				_DC_read_message(s->str,
						 _DCAPI_MSG_ACK,
						 TRUE);
				wu->asked_to_suspend= TRUE;
			}
		}
		g_string_free(s, TRUE);
		
		if (wu->asked_to_suspend)
		{
			/*_DC_wu_set_state(wu, DC_WU_SUSPENDED);*/
			wu->state= DC_WU_SUSPENDED;
			wu->asked_to_suspend= FALSE;
		}
		else
		{
			DC_MasterEvent *e;
			e= _DC_event_create(wu, _DC_result_create(wu),
					    NULL, NULL);
			DC_log(LOG_DEBUG, "Result event created: "
			       "%p for wu (%p-\"%s\")",
			       e, wu, wu->name);
			DC_log(LOG_DEBUG, "Result of the event: %p",
			       e->result);
			/*_DC_wu_set_state(wu, DC_WU_FINISHED);*/
			wu->state= DC_WU_FINISHED;
			return(e);
		}
	}
	return(NULL);
}

static void testWUEvents(void *key, void *value, void *ptr) /* by VG */
{
	DC_Workunit *wu = (DC_Workunit *)value;
	DC_Result *result;
	char syscmd[256];
	int retval;

	snprintf(syscmd, 256, "ps -p %d >/dev/null", wu->pid);
	retval = system(syscmd);

	/* retval == 0 means that the process is still in output of ps
		but it can be <defunct>.
		So check it again.
	*/
	if (retval == 0) {
		snprintf(syscmd, 256, "ps -p %d | grep defunct >/dev/null",
			wu->pid);
		retval = system(syscmd);
		if (retval == 0) retval = 1; /* defunct means finished */
		else if (retval == 1) retval = 0;
	}
	if (retval == 1) { /* process finished (not exists) */
		/* create the result object */
		DC_log(LOG_INFO, "Work unit %s with pid %d is found to be finished",
			wu->name, wu->pid);
		wu->state = DC_WU_FINISHED;

		result = _DC_createResult(wu->name);
		if (result)
		{
			_DC_result_callback(result->wu, result);
			_DC_destroyResult(result);
		}
	}

	return;
}

int _DC_searchForEvents() /* by VG */
{
	if (!_DC_wu_table)
	{
		DC_log(LOG_WARNING, "Searching for events is only usefull if there is any running work unit!");
		return DC_ERR_BADPARAM;
	}

	g_hash_table_foreach(_DC_wu_table, (GHFunc)testWUEvents, NULL);

	return DC_OK;
}


/* by DD */
/*
static void
_DC_event_destroy(DC_MasterEvent *event)
{
	if (!event)
		return;
	switch (event->type)
	{
	case DC_MASTER_RESULT:
		if (event->result)
			_DC_result_destroy(event->result);
		break;
	case DC_MASTER_SUBRESULT:
		if (event->subresult)
			_DC_destroyPhysicalFile(event->subresult);
		break;
	case DC_MASTER_MESSAGE:
		if (event->message)
			g_free(event->message);
		break;
	default:
		break;
	}
	g_free(event);
}
*/

/* by DD */
static DC_MasterEvent *
_DC_wu_check_client_messages(DC_Workunit *wu)
{
	GString *s;
	char *message;
	DC_MasterEvent *e= NULL;

	/*if (!_DC_wu_check(wu))
	  return(NULL);*/

	s= g_string_new(wu->workdir);
	g_string_append_printf(s, "/%s",
			       _DC_wu_cfg(wu, cfg_client_message_box));
	/*DC_log(LOG_DEBUG, "Checking box %s for message\n", s->str);*/
	if ((message= _DC_read_message(s->str, _DCAPI_MSG_MESSAGE, TRUE)))
	{
		e= _DC_event_create(wu, NULL, NULL, message);
		DC_log(LOG_DEBUG, "Message event created: %p "
		       "for wu (%p-\"%s\")", e, wu, wu->name);
		DC_log(LOG_DEBUG, "Message of the event: %s", e->message);
	}

	/*
	g_string_printf(s, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_management_box));
	if ((message= _DC_read_message(s->str, _DCAPI_MSG_ACK, TRUE)))
	{
		DC_log(LOG_DEBUG, "Acknowledge arrived: %s", message);
		if (strcmp(message, _DCAPI_ACK_SUSPEND) == 0)
		{
			wu->asked_to_suspend= TRUE;
		}
	}
	*/

	if (e == NULL/*)
		_DC_counter++;
		  if (_DC_counter % 5 == 0*/)
	{
		g_string_printf(s, "%s/%s", wu->workdir,
				_DC_wu_cfg(wu, cfg_subresults_box));
		/*DC_log(LOG_DEBUG, "Checking box %s for logical_name\n",
		  s->str);*/
		if ((message= _DC_read_message(s->str, _DCAPI_MSG_LOGICAL,
					       TRUE)))
		{
			DC_PhysicalFile *f;
			g_string_append_printf(s, "/real_files/%s", message);
			DC_log(LOG_DEBUG, "Got subresult %s %s",
			       message, s->str);
			f= _DC_createPhysicalFile(message, s->str);
			e= _DC_event_create(wu, NULL, f, NULL);
		}
	}

	g_string_free(s, TRUE);
	return(e);
}


/* by DD */
static void
_DC_process_event(DC_MasterEvent *event)
{
	if (!event)
		return;

	DC_log(LOG_DEBUG, "Processing an event %d, calling callback...",
	       event->type);
	switch (event->type)
	{
	case DC_MASTER_RESULT:
	{
		if (_DC_result_callback)
			(*_DC_result_callback)(event->wu,
					       event->result);
		break;
	}
	case DC_MASTER_SUBRESULT:
	{
		if (_DC_subresult_callback)
			(*_DC_subresult_callback)(event->wu,
						  event->subresult->label,
						  event->subresult->path);
		break;
	}
	case DC_MASTER_MESSAGE:
	{
		if (_DC_message_callback)
			(*_DC_message_callback)(event->wu,
						event->message);
		break;
	}
	}
	DC_destroyMasterEvent(event);
}

int DC_processMasterEvents(int timeout)
{
	/* call callback and destroy event */
	time_t start, now;
	DC_MasterEvent *event;

	DC_log(LOG_DEBUG, "DC_processMasterEvents(%d)",
	       timeout);

	if ((event= DC_waitMasterEvent(NULL, 0)) != NULL)
		_DC_process_event(event);
	if (timeout==0)
		return(DC_OK);

	start= time(NULL);
	now= time(NULL);
	while (now-start <= timeout &&
	       g_hash_table_size(_DC_wu_table) > 0)
	{
		sleep(1);
		if ((event= DC_waitMasterEvent(NULL, 0)) != NULL)
			_DC_process_event(event);
		now= time(NULL);
	}
	return(DC_OK);
/*
	int retval;
	static time_t start, current;

	if (!_dc_resultcb || !_dc_subresultcb || !_dc_messagecb)
	{
		DC_log(LOG_ERR, "DC_processMasterEvents: callbacks are not set up");
		return DC_ERR_CONFIG;
	}

	DC_log(LOG_INFO, "DC_processMasterEvents: Searching for events ...");

	start = time(NULL);
	while (1)
	{
*/
 	/* implemted in rm.c */
/*
		retval = _DC_searchForEvents();
		if (retval)
			return retval;

		if (DC_getWUNumber(DC_WU_RUNNING) == 0)
		{
			DC_log(LOG_INFO, "DC_processMasterEvents: No more running workunits ... returning");
			return 0;
		}

		current = time(NULL);
		if (current - start > timeout)
		{
			DC_log(LOG_INFO, "DC_processMasterEvents: timeout is over ... returning");
			return 0;
		}
		sleep(sleep_interval);
	}
*/
}

/* by DD */
static DC_MasterEvent *_DC_filtered_event;
static int _DC_counter;

static gboolean
_DC_check_filtered_wu_event(gpointer wu_name, gpointer w, gpointer ptr)
{
	DC_Workunit *wu= (DC_Workunit *)w;
	char *filter= (char *)ptr;
	char *tag;
	int match= TRUE;
	gboolean found;

	_DC_counter++;
	tag= DC_getWUTag(wu);
	if (filter)
		if (strcmp(tag, filter) != 0)
			tag= FALSE;
	tag?free(tag):0;
	if (!match)
	{
		return(FALSE);
	}
	if ((_DC_filtered_event= DC_waitWUEvent(wu, 0)) != NULL)
		DC_log(LOG_DEBUG,
		       "DC_waitWUEvent found an event for wu %p-\"%s\"",
		       wu, wu->name);
	found= _DC_filtered_event != NULL;
	return(found);
}


DC_MasterEvent *DC_waitMasterEvent(const char *wuFilter, int timeout)
{
	/* no callback called! */
	DC_Workunit *wu;
	time_t start, now;

	if (timeout)
		DC_log(LOG_DEBUG, "DC_waitMasterEvent(%s, %d)",
		       wuFilter, timeout);

	start= time(NULL);
	now= time(NULL);
	do
	{
		_DC_filtered_event= NULL;
		_DC_counter= 0;
		wu= (DC_Workunit *)
			g_hash_table_find(_DC_wu_table,
					  _DC_check_filtered_wu_event,
					  (gpointer)wuFilter);
		if (_DC_filtered_event)
			return(_DC_filtered_event);
		if (timeout)
			sleep(1);
	}
	while (timeout != 0 &&
	       now-start <= timeout);
	return(NULL);
/*
	DC_MasterEvent *event;

//	event = look_for_results(wuFilter, NULL, timeout);

	return event;
*/
}

DC_MasterEvent *DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	/* no callback called! */
	time_t start, now;
	DC_MasterEvent *me= NULL;

	/*if (!_DC_wu_check(wu))
	  return(NULL);*/
	if (timeout)
		DC_log(LOG_DEBUG, "DC_waitWUEvent(%p-\"%s\", %d)",
		       wu, wu->name, timeout);

	start= time(NULL);
	now= start;
	do
	{
		if ((me= _DC_wu_check_client_messages(wu)) != NULL)
			DC_log(LOG_DEBUG, "DC_waitWUEvent found a message for "
			       "wu %p-\"%s\"", wu, wu->name);
		if (!me)
		{
			if ((me= _DC_wu_infra2api_event(wu)) != NULL)
				DC_log(LOG_DEBUG,
				       "DC_waitWUEvent found an "
				       "infrastructure event for wu %p-\"%s\"",
				       wu, wu->name);
		}
		if (me)
		{
			return(me);
		}
		now= time(NULL);
		if (timeout)
			sleep(1);
	}
	while (timeout !=0 &&
	       now-start <= timeout);
	return(NULL);
/*
	DC_MasterEvent *event;

	return event;
*/
}

/* by VG */
void DC_destroyMasterEvent(DC_MasterEvent *event)
{
	if (!event)
		return;

	switch (event->type)
	{
		case DC_MASTER_RESULT:
			_DC_destroyResult(event->result);
			break;
		case DC_MASTER_SUBRESULT:
			_DC_destroyPhysicalFile(event->subresult);
			break;
		case DC_MASTER_MESSAGE:
			g_free(event->message);
			break;
		default:
			DC_log(LOG_ERR, "Unknown event type %d", event->type);
			break;
	}
}


/* End of local/local_events.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
