/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#ifndef _DC_API_CONDOR_LOG_H_
#define _DC_API_CONDOR_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dc.h"


extern void _DC_wu_update_condor_events(DC_Workunit *wu);
extern DC_MasterEvent *_DC_wu_condor2api_event(DC_Workunit *wu);
extern int _DC_wu_exit_code(DC_Workunit *wu, int *res);


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_log.h */
