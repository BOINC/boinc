/*
 * common_defs.h - Definitions common for both the server and client side
 *
 * Authors:
 *	Gábor Gombás <gombasg@sztaki.hu>
 *
 * Copyright (c) 2006 MTA SZTAKI
 */
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Prefix of subresult file labels */
#define SUBRESULT_PFX		"dc_subresult_"

/* Logical names of the checkpoint file */
#define CKPT_LABEL_IN		"dc_ckpt_in"
#define CKPT_LABEL_OUT		"dc_ckpt_out"

/* Maximum allowed message length */
#define MAX_MESSAGE_SIZE	16384

/* Prefix for internal messages between the client-side and master-side DC-API */
#define DCAPI_MSG_PFX		"__dcapi__"

/* Internal message that a subresult has been uploaded */
#define DC_MSG_UPLOAD		"UPLOAD"
/* Internal message telling that the client should suspend */
#define DC_MSG_SUSPEND		"SUSPEND"
/* Internal message for cancelling the client computation */
#define DC_MSG_CANCEL		"CANCEL"

#ifdef __cplusplus
}
#endif

#endif /* COMMON_DEFS_H */
