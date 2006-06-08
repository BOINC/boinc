/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include "condor_result.h"


DC_Result *
_DC_create_result(DC_Workunit *wu)
{
	DC_Result *r;
	
	r= g_new0(DC_Result, 1);
	r->wu= wu;
	return(r);
}


/* End of condor_result.c */
