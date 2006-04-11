#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <strings.h>

#include "dc_local.h"

int DC_processEvents(int timeout)
{
	int retval;
	static time_t start, current;

	if (!_dc_resultcb || !_dc_subresultcb || !_dc_messagecb)
	{
		DC_log(LOG_ERR, "DC_processEvents: callbacks are not set up");
		return DC_ERR_CONFIG;
	}

	DC_log(LOG_INFO, "DC_processEvents: Searching for events ...");

	start = time(NULL);
	while (1)
	{
		retval = _DC_searchForEvents(); 	/* implemted in rm.c */
		if (retval)
			return retval;

		current = time(NULL);
		if (current - start > timeout)
		{
			DC_log(LOG_INFO, "DC_processEvents: timeout is over ... returning");
			return 0;
		}
		sleep(sleep_interval);
	}
}

DC_Event *DC_waitEvent(const char *wuFilter, int timeout)
{
	DC_Event *event;

//	event = look_for_results(wuFilter, NULL, timeout);

	return event;
}

DC_Event *DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	DC_Event *event;

	return event;
}

void DC_destroyEvent(DC_Event *event)
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
