/*
 * local/local_wu.h
 *
 * DC-API functions of master side
 *
 * (c) Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#ifndef __DC_API_LOCAL_WU_H
#define __DC_API_LOCAL_WU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Get a configuration parameter */
char *_DC_wu_cfg(DC_Workunit *wu, enum _DC_e_param what) G_GNUC_INTERNAL;

#ifdef __cplusplus
}
#endif

#endif

/* End of local/local_wu.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
