/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#ifndef _DC_API_CONDOR_WU_H_
#define _DC_API_CONDOR_WU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "condor_defs.h"


extern int _DC_wu_check_logical_name(DC_Workunit *wu,
				     const char *logicalFileName);
extern char *_DC_wu_get_workdir_path(DC_Workunit *wu,
				     const char *label,
				     WorkdirFile type);
extern int _DC_wu_gen_condor_submit(DC_Workunit *wu);
extern int _DC_wu_make_client_executables(DC_Workunit *wu);


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_wu.h */
