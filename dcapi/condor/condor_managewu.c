/*
 * condor/condor_managewu.c
 *
 * DC-API wu management functions
 *
 * (c) Daniel Drotos, 2006
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>

#include "dc.h"

#include "condor_log.h"
#include "condor_wu.h"
#include "condor_utils.h"


/*************************************************************** Manage WUs */

static int
_DC_start_condor_job(DC_Workunit *wu)
{
	int ret;
	GString *cmd;
	gchar *act, *act2;

	cmd= g_string_new("condor_submit");
	cmd= g_string_append(cmd, " ");
	cmd= g_string_append(cmd, _DC_wu_cfg(wu, cfg_submit_file));
	act= getcwd(NULL, 0);
	chdir(wu->data.workdir);
	act2= getcwd(NULL, 0);
	DC_log(LOG_DEBUG, "Calling \"%s\" in %s...",
	       cmd->str, act2);
	ret= system(cmd->str);
	DC_log(LOG_DEBUG, "Returned %d", ret);
	chdir(act);
	g_free(act);
	g_free(act2);
	g_string_free(cmd, TRUE);

	return ret;
}

/* Submits a work unit. */
int
DC_submitWU(DC_Workunit *wu)
{
	int ret;
	/*GString *fn;*/
	char *id;
    int tries;
    int sleeptime;
    int maxtries;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	DC_log(LOG_DEBUG, "DC_submitWU(%p-\"%s\")", wu, wu->data.name);

	if (wu->data.state != DC_WU_READY)
	{
		DC_log(LOG_INFO, "Re-submission of %s", wu->data.name);
		return(DC_ERR_BADPARAM);
	}

	ret= _DC_wu_gen_condor_submit(wu);
	if (ret)
	{
		DC_log(LOG_ERR, "Submit file generation failed");
		return(ret);
	}

	/*fn= g_string_new(wu->workdir);*/
	/*fn= g_string_append(fn, "/condor_submit.txt");*/
    tries=0;
    sleeptime= DC_getCfgInt("SubmitRetrySleepTime", 2); 
    maxtries = DC_getCfgInt("SubmitRetry", 5);    
    do 
    {
    	ret= _DC_start_condor_job(wu);
        tries++;
        if (ret != 0)
        {
            if (tries > maxtries)
                break;
            DC_log(LOG_INFO, "Failed to submit WU to Condor. Sleeping for %d seconds and retrying (%d of %d)", 
                sleeptime, tries, maxtries);
            sleep(sleeptime);
            sleeptime*=2;
        }        
    } while (ret != 0);
	if (ret == 0)
	{
		/* Fix #1105 */
		_DC_wu_set_state(wu, DC_WU_RUNNING);
		_DC_wu_update_condor_events(wu);
		while (wu->condor_events->len == 0)
		{
			sleep(1);
			_DC_wu_update_condor_events(wu);
		}
		/*_DC_wu_set_state(wu, DC_WU_RUNNING);*/
		id= DC_getWUId(wu);
		DC_log(LOG_INFO, "Condor id of wu's job: %s", id);
		g_free(id);
	}
	return(ret);
}


static int
_DC_stop_condor_job(DC_Workunit *wu)
{
	char *id;
	GString *cmd;
	int ret;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	id= DC_getWUId(wu);
	if (!id)
		return(DC_ERR_UNKNOWN_WU);
	cmd= g_string_new("condor_rm ");
	cmd= g_string_append(cmd, id);
	DC_log(LOG_DEBUG, "Calling \"%s\"...", cmd->str);
	ret= system(cmd->str);
	DC_log(LOG_DEBUG, "Returned %d", ret);
	g_string_free(cmd, TRUE);
	g_free(id);
	return((ret==0)?DC_OK:DC_ERR_BADPARAM);
}


/* Cancels all computations for a given work unit. */
int
DC_cancelWU(DC_Workunit *wu)
{
	int ret;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	DC_log(LOG_DEBUG, "DC_cancelWU(%p-\"%s\")", wu, wu->data.name);

	if (wu->data.state != DC_WU_RUNNING &&
	    wu->data.state != DC_WU_SUSPENDED)
	{
		DC_log(LOG_NOTICE, "Can not cancel a non-running/non-suspended wu");
		return(DC_ERR_INTERNAL);
	}
	ret= _DC_stop_condor_job(wu);
	if (wu->data.state == DC_WU_SUSPENDED)
		ret= 0;
	_DC_wu_set_state(wu, DC_WU_ABORTED);
	return((ret==0)?DC_OK:DC_ERR_BADPARAM);
}


/* Temporarily suspends the execution of a work unit. */
int
DC_suspendWU(DC_Workunit *wu)
{
	GString *dn;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	DC_log(LOG_DEBUG, "DC_suspendWU(%p-\"%s\")", wu, wu->data.name);

	if (wu->data.state != DC_WU_RUNNING)
	{
		DC_log(LOG_NOTICE, "Can not suspend a non-running wu");
		return(DC_ERR_INTERNAL);
	}
	dn= g_string_new(wu->data.workdir);
	g_string_append(dn, "/");
	g_string_append(dn, _DC_wu_cfg(wu, cfg_management_box));
	_DC_create_message(dn->str, _DCAPI_MSG_COMMAND, _DCAPI_CMD_SUSPEND,
			   NULL);
	g_string_free(dn, TRUE);
	/*wu->asked_to_suspend= TRUE;*/
	return(DC_OK);
}


/* Resumes computation of a previously suspended work unit. */
int
DC_resumeWU(DC_Workunit *wu)
{
	int ret;
	char *id;
	GString *dn;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	DC_log(LOG_DEBUG, "DC_resumeWU(%p-\"%s\")", wu, wu->data.name);

	if (wu->data.state != DC_WU_SUSPENDED)
	{
		DC_log(LOG_NOTICE, "Can not resume a non-suspended wu");
		return(DC_ERR_INTERNAL);
	}
	dn= g_string_new(wu->data.workdir);
	g_string_append(dn, "/");
	g_string_append(dn, _DC_wu_cfg(wu, cfg_management_box));
	_DC_create_message(dn->str, _DCAPI_MSG_COMMAND, _DCAPI_CMD_RESUME,
			   NULL);
	g_string_free(dn, TRUE);
	ret= _DC_start_condor_job(wu);
	if (ret == 0)
	{
		_DC_wu_update_condor_events(wu);
		while (wu->condor_events->len == 0)
		{
			sleep(1);
			_DC_wu_update_condor_events(wu);
		}
		DC_log(LOG_DEBUG, "DC_ResumeWU...");
		_DC_wu_set_state(wu, DC_WU_RUNNING);
		id= DC_getWUId(wu);
		DC_log(LOG_INFO, "Condor id of wu's job: %s", id);
		g_free(id);
	}
	return(ret);
}


/* End of condor/condor_managewu.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
