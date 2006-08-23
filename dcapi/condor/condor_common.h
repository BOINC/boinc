/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#ifndef _DC_API_CONDOR_COMMON_H_
#define _DC_API_CONDOR_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif


#define CLIENT_CONFIG_NAME	"_dcapi_configfile.txt"

/* Config params used in both sides and their default values */
#define SCFG_CLIENT_MESSAGE_BOX	"ClientMessageBox"
#define SDEF_CLIENT_MESSAGE_BOX	"_dcapi_client_messages"

#define SCFG_MASTER_MESSAGE_BOX	"MasterMessageBox"
#define SDEF_MASTER_MESSAGE_BOX	"_dcapi_master_messages"

#define SCFG_SUBRESULTS_BOX	"SubresultBox"
#define SDEF_SUBRESULTS_BOX	"_dcapi_client_subresults"

#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_common.h */
