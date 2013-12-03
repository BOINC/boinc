/*
 * condor/condor_result.c
 *
 * DC-API functions to handle DC_Result data type
 *
 * (c) Daniel Drotos, 2006
 */

#include "condor_result.h"


DC_Result *
_DC_result_create(DC_Workunit *wu)
{
	DC_Result *r;
	
	r= g_new0(DC_Result, 1);
	r->wu= wu;
	return(r);
}


void
_DC_result_destroy(DC_Result *result)
{
	if (!result)
		return;
	g_free(result);
}


/* End of condor_result.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
