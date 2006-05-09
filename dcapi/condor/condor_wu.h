#ifndef _DC_API_CONDOR_WU_H_
#define _DC_API_CONDOR_WU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "condor_defs.h"


  extern int check_logical_name(DC_Workunit *wu,
				const char *logicalFileName);
  extern char *get_workdir_path(DC_Workunit *wu,
				const char *label,
				WorkdirFile type);


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_wu.h */
