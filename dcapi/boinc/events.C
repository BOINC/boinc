#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <strings.h>

#include "dc_boinc.h"

#include <boinc_db.h>
#include <sched_util.h>
#include <sched_msgs.h>

int DC_processEvents(int timeout)
{
	DC_Result *result;
	DB_WORKUNIT wu;
	char *query;

	if (!_dc_resultcb || !_dc_subresultcb || !_dc_messagecb)
	{
		DC_log(LOG_ERR, "DC_processEvents: callbacks are not set up");
		return DC_ERR_CONFIG;
	}

	/* XXX Check LIMIT value */
	query = g_strdup_printf("WHERE name LIKE '%s\\_%%' "
		"AND assimilate_state = %d LIMIT 100", project_uuid_str,
		ASSIMILATE_READY);
	while (!wu.enumerate(query))
	{
		DB_RESULT canonical_result;

		/* We are only interested in the canonical result */
		if (!wu.canonical_resultid)
		{
			/* This should never happen */
			DC_log(LOG_ERR, "No canonical result for work "
				"unit %d - bug in validator?", wu.id);
			continue;
		}

		if (canonical_result.lookup_id(wu.canonical_resultid))
		{
			DC_log(LOG_ERR, "Result #%d is not in the database",
				wu.canonical_resultid);
			continue;
		}

		/* Call the callback function */
		result = _DC_createResult(wu.name, canonical_result.xml_doc_in);
		if (!result)
			continue;
		_dc_resultcb(result->wu, result);
		_DC_destroyResult(result);

		/* Mark the work unit as completed */
		wu.assimilate_state = ASSIMILATE_DONE;
		wu.transition_time = time(0);
		wu.update();
	}

	g_free(query);
	return 0;
}

/* Look for a single result that matches the filter */
static DC_Event *look_for_results(const char *wuFilter, const char *wuName,
	int timeout)
{
	DB_RESULT result;
	DC_Event *event;
	DB_WORKUNIT wu;
	char *query;

	if (wuFilter)
		query = g_strdup_printf("WHERE name LIKE '%s\\_%%\\_%s' "
			"AND assimilate_state = %d LIMIT 1", project_uuid_str,
			wuFilter, ASSIMILATE_READY);
	else if (wuName)
		query = g_strdup_printf("WHERE name LIKE '%s\\_%s%%' "
			"AND assimilate_state = %d LIMIT 1", project_uuid_str,
			wuName, ASSIMILATE_READY);
	else
		query = g_strdup_printf("WHERE name LIKE '%s\\_%%' "
			"AND assimilate_state = %d LIMIT 1", project_uuid_str,
			ASSIMILATE_READY);

	if (wu.enumerate(query))
	{
		g_free(query);
		return NULL;
	}
	g_free(query);

	if (result.lookup_id(wu.canonical_resultid))
	{
		DC_log(LOG_ERR, "Result #%d is not in the database",
			wu.canonical_resultid);
		return NULL;
	}

	event = g_new(DC_Event, 1);
	event->type = DC_EVENT_RESULT;
	event->result = _DC_createResult(wu.name, result.xml_doc_in);
	if (!event->result)
	{
		g_free(event);
		return NULL;
	}

	return event;
}

DC_Event *DC_waitEvent(const char *wuFilter, int timeout)
{
	DC_Event *event;

	event = look_for_results(wuFilter, NULL, timeout);
/*
	if (!event)
		event = look_for_notifications(...)
*/
	return event;
}

DC_Event *DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	DC_Event *event;

	event = look_for_results(NULL, wu->name, timeout);
/*
	if (!event)
		event = look_for_notifications(...)
*/
	return event;
}

void DC_DestroyEvent(DC_Event *event)
{
	if (!event)
		return;

	switch (event->type)
	{
		case DC_EVENT_RESULT:
			_DC_destroyResult(event->result);
			break;
		case DC_EVENT_SUBRESULT:
			_DC_destroyPhysicalFile(event->subresult);
			break;
		case DC_EVENT_MESSAGE:
			g_free(event->message);
			break;
		default:
			DC_log(LOG_ERR, "Unknown event type %d", event->type);
			break;
	}
}
