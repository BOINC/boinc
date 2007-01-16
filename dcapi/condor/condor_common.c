/*
 * condor/condor_common.c
 *
 * DC-API functions common for master and slave side
 *
 * (c) Daniel Drotos, 2006
 */

/*#include <glib.h>*/

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
*/

#include "dc_common.h"

#include "condor_common.h"


struct _DC_s_param _DC_params[cfg_nuof];


void
_DC_init_common(void)
{
	int i;
	
	for (i= 0; i < cfg_nuof; i++)
	{
		_DC_params[i].name= 0;
		_DC_params[i].def= 0;
		_DC_params[i].lvalue= 0;
		_DC_params[i].gvalue= 0;
	}
	_DC_params[cfg_client_message_box].name= "ClientMessageBox";
	_DC_params[cfg_client_message_box].def= "_dcapi_client_messages";

	_DC_params[cfg_master_message_box].name= "MasterMessageBox";
	_DC_params[cfg_master_message_box].def= "_dcapi_master_messages";

	_DC_params[cfg_subresults_box].name= "SubresultBox";
	_DC_params[cfg_subresults_box].def= "_dcapi_client_subresults";

	_DC_params[cfg_management_box].name= "SystemMessageBox";
	_DC_params[cfg_management_box].def= "_dcapi_system_messages";

	_DC_params[cfg_architectures].name= "Architectures";
	_DC_params[cfg_architectures].def= "Client_%s_%s";/* client_name, architecture */

	_DC_params[cfg_submit_file].name= "SubmitFile";
	_DC_params[cfg_submit_file].def= "_dcapi_condor_submit.txt";

	_DC_params[cfg_executable].name= "Executable";
	_DC_params[cfg_executable].def= 0/*NULL*/;
	
	_DC_params[cfg_leave_files].name= "LeaveFiles";
	_DC_params[cfg_leave_files].def= "0";

	_DC_params[cfg_condor_log].name= "CondorLog";
	_DC_params[cfg_condor_log].def= "_dcapi_internal_log.txt";

	_DC_params[cfg_checkpoint_file].name= "CheckpointFile";
	_DC_params[cfg_checkpoint_file].def= "_dcapi_checkpoint";

	_DC_params[cfg_output_cache].name= "SavedOutputs";
	_DC_params[cfg_output_cache].def= "_dcapi_saved_output";

	_DC_params[cfg_condor_submit_template].name= "CondorSubmitTemplate";
	_DC_params[cfg_condor_submit_template].def= 0;
}


/* Determines the maximum allowed message length. */
int DC_getMaxMessageSize(void)
{
	return(16384);
}


/* Determines the maximum number of sub-results. */
int DC_getMaxSubresults(void)
{
	return(100);
}


/* Determines the basic capabilities of the underlying grid infrastructure. */
unsigned DC_getGridCapabilities(void)
{
	int cap;

	cap= DC_GC_STDOUT | DC_GC_STDERR;
	cap|= DC_GC_MESSAGING | DC_GC_LOG | DC_GC_SUBRESULT;
	cap|= DC_GC_EXITCODE;

	return(cap);
}


/* End of condor/condor_common.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
