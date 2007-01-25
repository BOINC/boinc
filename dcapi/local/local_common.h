/* Definitions common for both the server and client side */
#ifndef __DC_API_LOCAL_COMMON_H
#define __DC_API_LOCAL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

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
#define MAX_SUBRESULTS		100

/* Prefix for internal messages between the client-side and master-side DC-API */
#define DCAPI_MSG_PFX		"__dcapi__"

/* Must match the definition of WorkdirFile */
static const char *const workdir_prefixes[] =
{
        "in_", "out_", "checkpoint", "dc_"
};

#ifdef __cplusplus
}
#endif

#endif /* __DC_API_LOCAL_COMMON_H */
