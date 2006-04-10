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

/* DCAPI configuration keys */
#define CFG_WORKDIR		"WorkingDirectory"
#define CFG_INSTANCEUUID	"InstanceUUID"
#define CFG_CONFIGXML		"BoincConfigXML"
#define CFG_PROJECTROOT		"ProjectRootDir"

/* File types in the working directory */
typedef enum
{
	FILE_IN,
	FILE_OUT,
	FILE_CKPT,
	FILE_DCAPI
} WorkdirFile;

/* Name of the result template file */
#define RESULT_TEMPLATE		"result_template.xml"


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

	/* The WU's UUID */
	uuid_t			uuid;
	/* State of the WU */
	DC_WUState		state;
	/* The WU's working directory */
	char			*workdir;
	/* The WU's ID in the Boinc database */
	int			db_id;

	/* Input file definitions. Elements are of type DC_LogicalFile */
	GList			*input_files;
	int			num_inputs;

	/* Output file definitions. Elements are of type char * */
	GList			*output_files;
	int			num_outputs;

	/* Name of the checkpoint file, if exists (relative to workdir) */
	char			*ckpt_name;
};

struct _DC_Result
{
	DC_Workunit		*wu;

	int			exit_code;

	/* List of output files. Elements are of type DC_PhysicalFile */
	GList			*output_files;
	int			num_outputs;
};


/********************************************************************
 * Global variables
 */

extern DC_ResultCallback	_dc_resultcb;
extern DC_SubresultCallback	_dc_subresultcb;
extern DC_MessageCallback	_dc_messagecb;

extern char project_uuid_str[];
extern uuid_t project_uuid;


/********************************************************************
 * Function prototypes
 */

/* Parses the project's config.xml */
int _DC_parseConfigXML(const char *file) G_GNUC_INTERNAL;

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

/* Allocates a physical file descriptor */
DC_PhysicalFile *_DC_createPhysicalFile(const char *label,
	const char *path) G_GNUC_INTERNAL;

/* De-allocates a physical file descriptor */
void _DC_destroyPhysicalFile(DC_PhysicalFile *file) G_GNUC_INTERNAL;

/* Creates a new DC_Result */
DC_Result *_DC_createResult(const char *wu_name, int db_id,
	const char *xml_doc_in) G_GNUC_INTERNAL;

/* Destroys a DC_Result */
void _DC_destroyResult(DC_Result *result) G_GNUC_INTERNAL;

/* Get the name of the WU used in the database */
char *_DC_getWUName(DC_Workunit *wu) G_GNUC_INTERNAL;

/* Looks up a WU by name */
DC_Workunit *_DC_getWUByName(const char *name) G_GNUC_INTERNAL;

/* Parses <file_ref> definitions in an XML document */
GList *_DC_parseFileRefs(const char *xml_doc, int *num_files) G_GNUC_INTERNAL;

/* Marks a work unit as completed in the database */
void _DC_resultCompleted(DC_Result *result) G_GNUC_INTERNAL;

#ifdef __cplusplus
}
#endif

#endif /* __DC_BOINC_H_ */
