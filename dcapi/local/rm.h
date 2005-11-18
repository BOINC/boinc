#ifndef __RM_H_
#define __RM_H_

#include "dc.h"

/** init resource manager */
void dc_rm_init(void);

/** add new wu which has been created */
int dc_rm_wuCreate(const char *application_client,
		   const char *arguments, 
		   const char *workdir);

/** add input file to a wu) */
int dc_rm_setInput(DC_Workunit wu, char * URL, char * localFileName);

/** submit a wu */
int dc_rm_submitWU(DC_Workunit wu);

/** free memory used by this wu */
int dc_rm_destroyWU(DC_Workunit wu);

/** free memory within result allocate within this module */
void dc_rm_freeResult(dc_result result);


#define DC_RM_NORESULT     0
#define DC_RM_RESULTEXISTS 1

/** Check if there is at least one result already.
 *  Storage for result is allocated by the function, using
 *    dc_result_create().
 *  It should be deallocated by the caller using 
 *    dc_result_free().
 */
int dc_rm_checkForResult(dc_result *result);

void dc_rm_log(void); /* print internal data */
#endif /* __RM_H_ */
