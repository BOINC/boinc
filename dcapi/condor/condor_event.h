/*
 * condor/condor_event.h
 *
 * DC-API functions to handle DC_MasterEvent data type
 *
 * (c) Daniel Drotos, 2006
 */


#ifndef _DC_API_CONDOR_EVENT_H_
#define _DC_API_CONDOR_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dc.h"


extern DC_MasterEvent *_DC_event_create(DC_Workunit *wu,
					DC_Result *result,
					DC_PhysicalFile *subresult,
					char *message);
extern void _DC_event_destroy(DC_MasterEvent *event);


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_event.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
