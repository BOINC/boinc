/* Definitions common for both the server and client side */
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Prefix of subresult file labels */
#define SUBRESULT_PFX		"_dc_subresult_"

/* Logical names of the checkpoint file */
#define CKPT_LABEL_IN		"_dc_checkpoint_in"
#define CKPT_LABEL_OUT		"_dc_checkpoint_out"

/* Maximum allowed message length */
#define MAX_MESSAGE_SIZE	16384

/* Prefix for internal messages between the client-side and master-side DC-API */
#define DCAPI_MSG_PFX		"__dcapi__"

#ifdef __cplusplus
}
#endif

#endif /* COMMON_DEFS_H */
