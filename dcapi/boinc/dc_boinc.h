/*
 * dc_boinc.h - Common definitions for the master side of the BOINC DC-API backend
 *
 * Authors:
 *	Gábor Gombás <gombasg@sztaki.hu>
 *
 * Copyright (c) 2006 MTA SZTAKI
 */
#ifndef __DC_BOINC_H_
#define __DC_BOINC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dc.h>
#include <dc_internal.h>
#include <uuid/uuid.h>
#include <glib.h>

#include "common_defs.h"

/********************************************************************
 * Constants
 */

/* The root direcory of the BOINC project */
#define CFG_PROJECTROOT		    "ProjectRootDir"
/* Level of redundancy (per-client) variant 1. */
#define CFG_REDUNDANCY		    "Redundancy"
/* Level of redundancy (per-client) variant 2.: fine tune parameters */
#define CFG_MIN_QUORUM          "MinQuorum"
#define CFG_TARGET_NRESULTS     "TargetNResults"
#define CFG_MAX_ERROR_RESULTS   "MaxErrorResults"
#define CFG_MAX_TOTAL_RESULTS   "MaxTotalResults"
#define CFG_MAX_SUCCESS_RESULTS "MaxSuccessResults"
/* Max. size of output files, including subresults & checkpoint (per-client) */
#define CFG_MAXOUTPUT		"MaxOutputSize"
/* Max. memory usage (per-client) */
#define CFG_MAXMEMUSAGE		"MaxMemUsage"
/* Max. disk usage (per-client) */
#define CFG_MAXDISKUSAGE	"MaxDiskUsage"
/* Estimated number of floating point operations */
#define CFG_FPOPS_EST		"EstimatedFPOps"
/* Max. number of floating point operations */
#define CFG_MAXFPOPS		"MaxFPOps"
/* WU deadline */
#define CFG_DELAYBOUND		"DelayBound"
/* Suspending enabled or not */
#define CFG_ENABLESUSPEND	"EnableSuspend"
/* Client uses native BOINC API instead of DC-API */
#define CFG_NATIVECLIENT	"NativeClient"
/* Remote file rewrite regexp match string */
#define CFG_REGEXPMATCH		"InputURLRewriteRegExpMatch"
/* Remote file rewrite regexp replace string */
#define CFG_REGEXPREPLACE	"InputURLRewriteRegExpReplace"
/* Upload URL configuration option */
#define CFG_UPLOADURL		"UploadURL"

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
	char			**argv;
	int			argc;
	char			*tag;
	int			subresults;
	int			priority;

	int			serialized : 1;
	int			submitted : 1;
	int			suspended : 1;
	int			nosuspend : 1;
	int			nativeclient : 1;

	/* The WU's UUID */
	uuid_t			uuid;
	/* State of the WU */
	DC_WUState		state;
	/* The WU's working directory */
	char			*workdir;
	/* The WU's ID in the Boinc database */
	int			db_id;
	
	/* Batch value of the workunit */
	int 			batch;

	/* Input file definitions. Elements are of type DC_PhysicalFile */
	GList			*input_files;
	int			num_inputs;

	/* Remote input file definitions. Elements are of type DC_RemoteFile */
	GList			*remote_input_files;
	int			num_remote_inputs;

	/* Output file definitions. Elements are of type char * */
	GList			*output_files;
	int			num_outputs;

	/* Name of the checkpoint file, if exists (relative to workdir) */
	char			*ckpt_name;

	/* Reference count */
	int			refcnt;

	/* Mapping <optional/> attribute to output files  */
	GHashTable              *is_optional;
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

extern DC_ResultCallback _dc_resultcb G_GNUC_INTERNAL;
extern DC_SubresultCallback _dc_subresultcb G_GNUC_INTERNAL;
extern DC_MessageCallback _dc_messagecb G_GNUC_INTERNAL;

extern char project_uuid_str[] G_GNUC_INTERNAL;
extern uuid_t project_uuid G_GNUC_INTERNAL;


/********************************************************************
 * Function prototypes
 */

/* Parses the project's config.xml */
int _DC_parseConfigXML(void) G_GNUC_INTERNAL;

/* Returns the Boinc upload directory */
char *_DC_getUploadDir(void) G_GNUC_INTERNAL;

/* Returns the Boinc download directory */
char *_DC_getDownloadDir(void) G_GNUC_INTERNAL;

/* Returns the Boinc upload/download hashing */
int _DC_getUldlDirFanout(void) G_GNUC_INTERNAL;

/* Returns the database name */
char *_DC_getDBName(void) G_GNUC_INTERNAL;

/* Returns the database host */
char *_DC_getDBHost(void) G_GNUC_INTERNAL;

/* Returns the database user ID */
char *_DC_getDBUser(void) G_GNUC_INTERNAL;

/* Returns the database password */
char *_DC_getDBPasswd(void) G_GNUC_INTERNAL;

/* Initializes the database connection */
int _DC_initDB(void) G_GNUC_INTERNAL;

/* Initializes the WU manager */
int _DC_initWUs(void) G_GNUC_INTERNAL;

/* Creates a new DC_Result */
DC_Result *_DC_createResult(const char *wu_name, int db_id,
	const char *xml_doc_in, double cpu_time) G_GNUC_INTERNAL;

/* Destroys a DC_Result */
void _DC_destroyResult(DC_Result *result) G_GNUC_INTERNAL;

/* Get the name of the WU used in the database */
char *_DC_getWUName(const DC_Workunit *wu) G_GNUC_INTERNAL;

/* Looks up a WU by name */
DC_Workunit *_DC_getWUByName(const char *name) G_GNUC_INTERNAL;

/* Looks up the id of a wu */
int _DC_getDBid(DC_Workunit *wu) G_GNUC_INTERNAL;

/* Parses <file_ref> definitions in an XML document */
GList *_DC_parseFileRefs(const char *xml_doc, int *num_files) G_GNUC_INTERNAL;

/* Marks a work unit as completed in the database */
void _DC_resultCompleted(DC_Result *result) G_GNUC_INTERNAL;

/* Updates the WU state from the database */
void _DC_updateWUState(DC_Workunit *wu) G_GNUC_INTERNAL;

/* C wrapper around dir_hier_path() */
char *_DC_hierPath(const char *src, int upload) G_GNUC_INTERNAL;

/* Get the full path of a file in the WU's working directory */
char *_DC_workDirPath(const DC_Workunit *wu, const char *label, WorkdirFile type)
	G_GNUC_INTERNAL;

#ifdef __cplusplus
}
#endif

#endif /* __DC_BOINC_H_ */
