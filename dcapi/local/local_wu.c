/*
 * local/local_wu.c
 *
 * DC-API functions of master side
 *
 * (c) Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#include "local_master.h"


/* Get a configuration parameter */
char *
_DC_wu_cfg(DC_Workunit *wu,
	   enum _DC_e_param what)
{
	/*if (!_DC_wu_check(wu))
	  return(NULL);*/
	if (what >= cfg_nuof)
		return(NULL);
	if (!_DC_params[what].name)
		return(NULL);

	if (_DC_params[what].gvalue)
		return(_DC_params[what].gvalue);

	_DC_params[what].gvalue=
		DC_getClientCfgStr(wu->client_name,
				   _DC_params[what].name,
				   /*TRUE*/1);
	if (_DC_params[what].gvalue)
		return(_DC_params[what].gvalue);
	return(_DC_params[what].def);
}


/* End of local/local_wu.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
