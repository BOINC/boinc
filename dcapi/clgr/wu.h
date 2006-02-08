#ifndef __WU_H_
#define __WU_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "dc.h"

/** init workunit module.
 *  'projectroot' is the path to the directory where the
 *  actual project is installed e.g. /usr/boinc/projects/proba.
 *  'uploadkeyfile' is the upload private key file for the 
 *  installed boinc project, e.g. /usr/boinc/keys/upload_private
 */
//void dc_wu_init(const char *projectroot, const char *uploadkeyfile);
void dc_wu_init(const char *projectroot,const char *workdir_id, const char* exec_dir);

/** add new wu which has been created */
DC_Workunit dc_wu_create(const char *clientname, 
		 const char *arguments);

/** add input files to a wu */
int dc_wu_setInput(DC_Workunit wu, const char *url, const char* localfilename);

/** set priority of a wu */
int dc_wu_setPriority(DC_Workunit wu, int priority);

/** Create the wu within Boinc (i.e. submit from application into Boinc)
 *  uploadkeyfile: upload_private key
 *  boincRootDir: the actual projects dir. config.xml should be there
 */
//int dc_wu_createBoincWU(DC_Workunit wu, char *uploadkeyfile, char *boincRootDir);
int dc_wu_submitWU(DC_Workunit wu);

int dc_wu_destroy(DC_Workunit wu);
int dc_wu_cancel(DC_Workunit wu);

int dc_wu_suspend(DC_Workunit wu);
int dc_wu_resubmit(DC_Workunit wu);

/* Return the workunit index from the WUtable that matches the name */
int dc_wu_findByName(char *wuname);

void dc_wu_log(void); /* print internal data */

/* */
int dc_wu_checkForResult(void (*cb_assimilate_result)(DC_Result result));

#ifdef __cplusplus
}
#endif


#endif /* __WU_H_ */
