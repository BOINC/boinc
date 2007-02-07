/*
 * local/local_result.h
 *
 * DC-API functions to handle DC_Result data type
 *
 * (c) Daniel Drotos, 2006
 */

#ifndef _DC_API_LOCAL_RESULT_H_
#define _DC_API_LOCAL_RESULT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "local_master.h"


extern DC_Result *_DC_result_create(DC_Workunit *wu);
extern void _DC_result_destroy(DC_Result *result);


#ifdef __cplusplus
}
#endif

#endif

/* End of local/local_result.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
