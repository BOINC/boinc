/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>

#include "dc.h"

#include "condor_log.h"
#include "condor_wu.h"


/*************************************************************** Manage WUs */

/* Submits a work unit. */
int
DC_submitWU(DC_Workunit *wu)
{
	int ret;
	//GString *fn;
	GString *cmd;
	gchar *act, *act2;

	ret= _DC_wu_gen_condor_submit(wu);
	if (ret)
	{
		DC_log(LOG_ERR, "Submit file generation failed");
		return(ret);
	}

	//fn= g_string_new(wu->workdir);
	//fn= g_string_append(fn, "/condor_submit.txt");
	cmd= g_string_new("condor_submit");
	cmd= g_string_append(cmd, " condor_submit.txt");
	act= getcwd(NULL, 0);
	chdir(wu->workdir);
	act2= getcwd(NULL, 0);
	DC_log(LOG_DEBUG, "Calling \"%s\" in %s...",
	       cmd->str, act2);
	ret= system(cmd->str);
	DC_log(LOG_DEBUG, "Returned %d", ret);
	chdir(act);
	g_free(act);
	g_free(act2);
	g_string_free(cmd, TRUE);
	if (ret == 0)
		wu->state= DC_WU_RUNNING;
	return(DC_OK);
}


/* Cancels all computations for a given work unit. */
int
DC_cancelWU(DC_Workunit *wu)
{
	return(0);
}


/* Temporarily suspends the execution of a work unit. */
int
DC_suspendWU(DC_Workunit *wu)
{
	return(0);
}


/* Resumes computation of a previously suspended work unit. */
int
DC_resumeWU(DC_Workunit *wu)
{
	return(0);
}


/* End of condor/condor_managewu.c */
