/*
 * condor/condor_common.h
 *
 * DC-API functions common for master and slave side
 *
 * (c) Daniel Drotos, 2006
 */


#ifndef _DC_API_CONDOR_COMMON_H_
#define _DC_API_CONDOR_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif


#define CLIENT_CONFIG_NAME	"_dcapi_configfile.txt"


enum _DC_e_param {
	cfg_client_message_box= 0,
	cfg_master_message_box,
	cfg_subresults_box,
	cfg_management_box,
	cfg_architectures,
	cfg_submit_file,
	cfg_executable,
	cfg_leave_files,
	cfg_condor_log,
	cfg_checkpoint_file,
	cfg_output_cache,
	cfg_condor_submit_template,
	cfg_nuof
};

struct _DC_s_param {
	char *name;
	char *def;
	char *lvalue;
	char *gvalue;
};


extern struct _DC_s_param _DC_params[cfg_nuof];

extern void _DC_init_common(void);


#define _DCAPI_MSG_MESSAGE	"message"
#define _DCAPI_MSG_LOGICAL	"logical_name"
#define _DCAPI_MSG_COMMAND	"command"
#define _DCAPI_MSG_ACK		"acknowledge"

#define _DCAPI_CMD_SUSPEND	"suspend"
#define _DCAPI_ACK_SUSPEND	"suspending"

#define _DCAPI_CMD_RESUME	"resume"

#define _DCAPI_SIG_SERIALIZED	"serialized"


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_common.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
