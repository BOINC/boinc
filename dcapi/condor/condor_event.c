/*
 * condor/condor_event.c
 *
 * DC-API functions to handle DC_MasterEvent data type
 *
 * (c) Daniel Drotos, 2006
 */


#include <glib.h>

#include "dc.h"

#include "condor_result.h"
#include "condor_event.h"


DC_MasterEvent *
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


void
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


/* End of condor/condor_event.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
