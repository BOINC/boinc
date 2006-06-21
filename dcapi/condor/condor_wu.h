/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#ifndef _DC_API_CONDOR_WU_H_
#define _DC_API_CONDOR_WU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "condor_defs.h"


extern void _DC_wu_changed(DC_Workunit *wu);
extern int _DC_wu_check(const DC_Workunit *wu);
extern int _DC_wu_set_client_name(DC_Workunit *wu,
				  const char *new_name);
extern int _DC_wu_set_argc(DC_Workunit *wu,
			   int new_argc);
extern int _DC_wu_check_logical_name(DC_Workunit *wu,
				     const char *logicalFileName);
extern char *_DC_wu_get_workdir_path(DC_Workunit *wu,
				     const char *label,
				     WorkdirFile type);
extern int _DC_wu_gen_condor_submit(DC_Workunit *wu);
extern int _DC_wu_make_client_executables(DC_Workunit *wu);
extern int _DC_wu_make_client_config(DC_Workunit *wu);
extern DC_MasterEvent *_DC_wu_check_client_messages(DC_Workunit *wu);


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_wu.h */
