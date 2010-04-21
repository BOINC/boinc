/*
 * local/local_master.h
 *
 * DC-API definitions of master side
 *
 * (c) Gabor Vida 2005-2006, Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#ifndef __DC_API_LOCAL_MASTER_H
#define __DC_API_LOCAL_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#include <dc.h>
#include <dc_internal.h>
#include <uuid/uuid.h>
#include <glib.h>

#include "local_common.h"

/********************************************************************
 * Constants
 */

/* DCAPI configuration keys */
#define CFG_SLEEPINTERVAL	"SleepingInterval"

#define DEFAULT_SLEEP_INTERVAL	5

/* File types in the working directory */
typedef enum
{
	FILE_IN,
	FILE_OUT,
	FILE_CKPT,
	FILE_DCAPI
} WorkdirFile;


/********************************************************************
 * Data types
 */

struct _DC_Workunit
{
	/* Arguments passed to DC_createWU() */
	char			*client_name;
	/*char			*client_path;*/
	char			**argv;
	int			argc;
	char			*tag;
	int			subresults;

	/* State of the WU */
	char			*name;
	uuid_t			uuid;
	char			*uuid_str;
	DC_WUState		state;
	char			*workdir;
	pid_t			pid;

	/* Input file definitions. Elements are of type DC_PhysicalFile */
	GList			*input_files;
	int			num_inputs;

	/* Output file definitions. Elements are of type char * */
	GList			*output_files;
	int			num_outputs;

	/* by DD */
	int			asked_to_suspend;
};

struct _DC_Result
{
	DC_Workunit		*wu;

	int			exit_code;
	double			cpu_time;

	/* List of output files. Elements are of type DC_PhysicalFile */
	GList			*output_files;
	int			num_outputs;
};


/********************************************************************
 * Global variables
 */

extern DC_ResultCallback _DC_result_callback G_GNUC_INTERNAL;
extern DC_SubresultCallback _DC_subresult_callback G_GNUC_INTERNAL;
extern DC_MessageCallback _DC_message_callback G_GNUC_INTERNAL;

extern GHashTable *_DC_wu_table G_GNUC_INTERNAL;

extern char project_uuid_str[37] G_GNUC_INTERNAL; 
extern uuid_t project_uuid G_GNUC_INTERNAL;
extern int sleep_interval G_GNUC_INTERNAL;

/********************************************************************
 * Function prototypes
 */

/* Allocates a physical file descriptor */
DC_PhysicalFile *_DC_createPhysicalFile(const char *label,
        const char *path) G_GNUC_INTERNAL;

/* De-allocates a physical file descriptor */
void _DC_destroyPhysicalFile(DC_PhysicalFile *file) G_GNUC_INTERNAL;

/* Calls a g_hash_table_foreach function for the wu_table */
int _DC_searchForEvents(void) G_GNUC_INTERNAL;

/* Creates a new DC_Result */
DC_Result *_DC_createResult(const char *wu_name) G_GNUC_INTERNAL;

/* Destroys a DC_Result */
void _DC_destroyResult(DC_Result *result) G_GNUC_INTERNAL;

/* Looks up a WU by name */
DC_Workunit *_DC_getWUByName(const char *name) G_GNUC_INTERNAL;

char *_DC_state_name(DC_WUState state) G_GNUC_INTERNAL;

char *_DC_wu_cfg(DC_Workunit *wu, enum _DC_e_param what) G_GNUC_INTERNAL;

#ifdef __cplusplus
}
#endif

#endif /* __DC_API_LOCAL_MASTER_H */


/* End of local/local_master.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
