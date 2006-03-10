#ifndef __DC_BOINC_H_
#define __DC_BOINC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dc.h>
#include <dc_internal.h>
#include <uuid/uuid.h>
#include <glib.h>

/********************************************************************
 * Constants
 */

/* DCAPI configuration keys */
#define CFG_WORKDIR		"WorkingDirectory"
#define CFG_INSTANCEUUID	"InstanceUUID"
#define CFG_CONFIGXML		"BoincConfigXML"

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

	/* State of the WU */
	uuid_t			uuid;
	char			*uuid_str;
	DC_WUState		state;
	char			*workdir;

	/* Input file definitions. Elements are of type char * */
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


/********************************************************************
 * Function prototypes
 */

/* Parses the project's config.xml */
int _DC_parseConfigXML(const char *file) G_GNUC_INTERNAL;

/* Get the project's root directory */
char *_DC_getProjectRoot(void) G_GNUC_INTERNAL;

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

/********************************************************************
 * XXX Everything below needs to be checked & redesigned
 */

/**
 *  'projectroot' is the path to the directory where the
 *  actual project is installed e.g. /usr/boinc/projects/proba.
 *  'appname' should be the application client name!
*/
int dc_assimilator_init(char * projectroot, char * appname);

/** Check for results.
 *  Waits for available results and returns the first.
 *  Return:  DC_OK on success
 *              >1 on error
 *  Parameters:
 *     func1      callback function for processing a result by the master app
 */
int dc_assimilator_dopass(void (*cb_assimilate_result)(DC_Result result));

DC_Result  *dc_result_create(char *name, char *wuname, char *dir);
int        dc_result_addOutputFile(DC_Result *result, char *filename);
void       dc_result_free(DC_Result *result);

/** add input files to a wu */
int dc_wu_setInput(DC_Workunit *wu, const char *url, const char* localfilename);

/** Create the wu within Boinc (i.e. submit from application into Boinc)
 *  uploadkeyfile: upload_private key
 *  boincRootDir: the actual projects dir. config.xml should be there
 */
int dc_wu_createBoincWU(DC_Workunit *wu, char *uploadkeyfile, char *boincRootDir);

/* Return the workunit index from the WUtable that matches the name */
int dc_wu_findByName(char *wuname);

#ifdef __cplusplus
}
#endif

#endif /* __DC_BOINC_H_ */
