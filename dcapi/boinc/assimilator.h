#ifndef __ASSIMILATOR_H
#define __ASSIMILATOR_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
