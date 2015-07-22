/*
 * condor/condor_master.h
 *
 * DC-API functions of master side
 *
 * (c) Daniel Drotos, 2006
 */

#ifndef _DC_API_CONDOR_MASTER_H_
#define _DC_API_CONDOR_MASTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dc.h"

#include "condor_common.h"


extern char *_DC_config_file;

extern char *_DC_acfg(enum _DC_e_param what);

extern char *_DC_state_name(DC_WUState state);

#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_master.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
