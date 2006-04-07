/* Definitions common for both the server and client side */
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Prefix of subresult file labels */
#define SUBRESULT_PFX		"dc_subresult_"

/* Logical names of the checkpoint file */
#define CKPT_LABEL_IN		"dc_checkpoint.in"
#define CKPT_LABEL_OUT		"dc_checkpoint.out"

/* Name of the client-side config. file */
#define CLIENTCONF_LABEL	"dc_client.conf"

/* Maximum allowed message length */
#define MAX_MESSAGE_SIZE	16384

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

#endif /* COMMON_DEFS_H */
