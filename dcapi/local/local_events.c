#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <strings.h>

#include "dc_local.h"

int DC_processMasterEvents(int timeout)
{
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
		retval = _DC_searchForEvents(); 	/* implemted in rm.c */
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
}

DC_MasterEvent *DC_waitMasterEvent(const char *wuFilter, int timeout)
{
	DC_MasterEvent *event;

//	event = look_for_results(wuFilter, NULL, timeout);

	return event;
}

DC_MasterEvent *DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	DC_MasterEvent *event;

	return event;
}

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
