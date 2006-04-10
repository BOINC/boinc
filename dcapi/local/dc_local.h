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
#define CFG_SLEEPINTERVAL	"SleepingInterval"
#define CFG_INSTANCEUUID        "InstanceUUID"


#define DEFAULT_SLEEP_INTERVAL		5
#define MAX_SUBRESULTS			20


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
	char			*client_path;
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
	int			pid;

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

extern char project_uuid_str[37]; 
extern uuid_t project_uuid;
extern int sleep_interval;

/********************************************************************
 * Function prototypes
 */

/* Allocates a physical file descriptor */
DC_PhysicalFile *_DC_createPhysicalFile(const char *label,
        const char *path);

/* De-allocates a physical file descriptor */
void _DC_destroyPhysicalFile(DC_PhysicalFile *file);

int _DC_searchForEvents();

/* Parses the project's config.xml */
int _DC_parseConfigXML(const char *file);

/* Returns the Boinc upload directory */
char *_DC_getUploadDir(void);

/* Returns the Boinc download directory */
char *_DC_getDownloadDir(void);

/* Returns the Boinc upload/download hashing */
int _DC_getUldlDirFanout(void);

/* Returns the database name */
char *_DC_getDBName(void);

/* Returns the database host */
char *_DC_getDBHost(void);

/* Returns the database user ID */
char *_DC_getDBUser(void);

/* Returns the database password */
char *_DC_getDBPasswd(void);

/* Initializes the database connection */
int _DC_initDB(void);

/* Allocates a physical file descriptor */
DC_PhysicalFile *_DC_createPhysicalFile(const char *label,
	const char *path);

/* De-allocates a physical file descriptor */
void _DC_destroyPhysicalFile(DC_PhysicalFile *file);

/* Creates a new DC_Result */
DC_Result *_DC_createResult(const char *wu_name);

/* Destroys a DC_Result */
void _DC_destroyResult(DC_Result *result);

/* Looks up a WU by name */
DC_Workunit *_DC_getWUByName(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __DC_BOINC_H_ */
