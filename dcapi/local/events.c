#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <strings.h>

#include "dc_local.h"

int DC_processEvents(int timeout)
{

	if (!_dc_resultcb || !_dc_subresultcb || !_dc_messagecb)
	{
		DC_log(LOG_ERR, "DC_processEvents: callbacks are not set up");
		return DC_ERR_CONFIG;
	}


	return 0;
}

/* Look for a single result that matches the filter */
static DC_Event *look_for_results(const char *wuFilter, const char *wuName,
	int timeout)
{
	DC_Event *event;

	return event;
}

DC_Event *DC_waitEvent(const char *wuFilter, int timeout)
{
	DC_Event *event;

	event = look_for_results(wuFilter, NULL, timeout);

	return event;
}

DC_Event *DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	DC_Event *event;

	event = look_for_results(NULL, wu->name, timeout);

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
