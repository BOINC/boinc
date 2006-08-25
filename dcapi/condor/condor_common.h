/* Local variables: */
/* c-file-style: "linux" */
/* End: */

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
	cfg_nuof
};

struct _DC_s_param {
	char *name;
	char *def;
};


extern struct _DC_s_param _DC_params[cfg_nuof];

extern void _DC_init_common(void);


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_common.h */
