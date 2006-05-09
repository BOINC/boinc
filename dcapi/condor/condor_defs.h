#ifndef __DC_API_CONDOR_DEFS_H_
#define __DC_API_CONDOR_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <uuid/uuid.h>

#include "dc.h"
#include "dc_internal.h"

  extern char project_uuid_str[37];

  struct _DC_Workunit
  {
    char *client_name;
    char **argv;
    int argc;
    char *tag;
    int subresults;
    
    char *name;
    uuid_t uuid;
    char *uuid_str;
    DC_WUState state;
    
    char *condor_id;
    char *workdir;
     
    GList *input_files;
    GList *output_files;
  };

  struct _DC_Result
  {
    DC_Workunit *wu;
  };

  typedef enum
    {
      FILE_IN,
      FILE_OUT,
      FILE_CKPT,
      FILE_DCAPI
    } WorkdirFile;

#define CFG_WORKDIR	"WorkingDirectory"


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_defs.h */
