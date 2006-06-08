/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#ifndef __DC_API_CONDOR_DEFS_H_
#define __DC_API_CONDOR_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <sys/time.h>
#include <uuid/uuid.h>
#include "glib.h"

#include "dc.h"
#include "dc_internal.h"

extern char project_uuid_str[37];

struct _DC_condor_event
{
	int event;
	int cluster;
	int proc;
	int subproc;
	time_t time;
};

struct _DC_wu_data
{
	char *client_name;
	int argc;
};

struct _DC_Workunit
{
	int magic;
	int chk;
	struct _DC_wu_data data;
	char **argv;
	char *tag;
	int subresults;
    
	char *name;
	uuid_t uuid;
	char *uuid_str;
	DC_WUState state;
    
	/*char *condor_id;*/
	GArray *condor_events;
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

/* DCAPI configuration keys */
#define CFG_SLEEPINTERVAL	"SleepingInterval"
#define CFG_ARCHITECTURES	"Architectures"
#define CFG_CLIENT_ARCH_NAME	"Client_%s_%s"	/* client_name, architecture */
/*#define CFG_WORKDIR		"WorkingDirectory"*/
/*#define CFG_INSTANCEUUID	"InstanceUUID"*/

#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_defs.h */
