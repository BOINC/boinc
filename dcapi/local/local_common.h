/*
 * local/local_common.h
 *
 * DC-API definitions of both client and master side
 *
 * (c) Gabor Vida 2005-2006, Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

/* Definitions common for both the server and client side */
#ifndef __DC_API_LOCAL_COMMON_H
#define __DC_API_LOCAL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* For the local backend, it is OK to use glib on the client side as well */
#include <glib.h>

/* Prefix of subresult file labels */
#define SUBRESULT_PFX		"dc_subresult_"

/* Logical names of the standard output file */
#define STDOUT_LABEL		"dc_stdout"

/* Logical names of the standard error file */
#define STDERR_LABEL		"dc_stderr"

/* Logical names of the checkpoint file */
#define CKPT_LABEL		"dc_checkpoint"

/* Name of the client-side config. file */
#define CLIENTCONF_LABEL	"dc_client.conf"

/* Maximum allowed message length */
#define MAX_MESSAGE_SIZE	16384

/* Maximum allowed message length */
/*
 * local/local_master.c
 *
 * DC-API functions of master side
 *
 * (c) Gabor Vida 2005-2006, Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#define MAX_SUBRESULTS		100

/* Prefix for internal messages between the client-side and master-side DC-API */
#define DCAPI_MSG_PFX		"__dcapi__"

/* Must match the definition of WorkdirFile */
static const char *const workdir_prefixes[] =
{
        "in_", "out_", "checkpoint", "dc_"
};


/****************************************************************************/

#define CLIENT_CONFIG_NAME	"_dcapi_configfile.txt"


enum _DC_e_param {
	cfg_client_message_box= 0,
	cfg_master_message_box,
	cfg_subresults_box,
	cfg_management_box,
	cfg_executable,
	cfg_leave_files,
	cfg_checkpoint_file,
	cfg_output_cache,
	cfg_nuof
};

struct _DC_s_param {
	char *name;
	char *def;
	char *lvalue;
	char *gvalue;
};


extern struct _DC_s_param _DC_params[cfg_nuof] G_GNUC_INTERNAL;

extern void _DC_init_common(void) G_GNUC_INTERNAL;
#define _DCAPI_MSG_MESSAGE      "message"
#define _DCAPI_MSG_LOGICAL      "logical_name"
#define _DCAPI_MSG_COMMAND      "command"
#define _DCAPI_MSG_ACK          "acknowledge"

#define _DCAPI_CMD_SUSPEND      "suspend"
#define _DCAPI_ACK_SUSPEND      "suspending"

#define _DCAPI_CMD_RESUME       "resume"

#define _DCAPI_SIG_SERIALIZED   "serialized"


#ifdef __cplusplus
}
#endif

#endif /* __DC_API_LOCAL_COMMON_H */


/* End of local/local_common.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
