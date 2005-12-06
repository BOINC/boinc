#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>

#include"defines.h"
#include"dc.h"


int ask_status(const char *wu_name, int *status, time_t *subresult_time, time_t *ckpt_time)
{
    FILE *clgr_status;
    char command[512];
    char result[512];
    char variable[512];
    char *value;
    char clgr_string[32];
    int len; //n_of_lines = 0;
    int back = STATE_UNKNOWN;
    //int i;
    
    DC_log(LOG_DEBUG,"STATUS: wu_name: '%s'", wu_name);
    
    sprintf(command, "clgr_status -l -i %s", wu_name);

    clgr_status = popen(command, "r");
    
        
    /********************/
    /* reset variables */
    /******************/
    *subresult_time = *ckpt_time = 0;

    /*********************************/
    /* read through the status info */
    /*******************************/
    while(!feof(clgr_status))
    {    
	//n_of_lines++;
	fgets(result, sizeof(result), clgr_status);
	
	if ((strchr(result,'=')) == NULL) break;
	
	//DC_log(LOG_DEBUG,"%s", result);

	/*****************************/
	/* retreive variable string */
	/***************************/
	
	len = strcspn(result, " =");
	if( len < 512)
	{
	    snprintf(variable, len+1, "%s", result);
	    //DC_log(LOG_DEBUG,"STATUS: variable_name: '%s'", variable);
	}
	else {
            DC_log(LOG_ERR, "status.c: status field longer than 512 chars");
	    return DC_ERROR;
	}

	if(!strcmp(variable, "status"))
	{    
    	    /***************************/
    	    /* retreive STATUS string */
    	    /*************************/
   
	    /*value = result;*/
    	    value = (char *)strchr(result, '=');
    	    value++;
    	    value++;
    	    len = strcspn(value, "\n");
    	    if (len > 31) len = 31;	
            snprintf(clgr_string, len+1, "%s", value);
	    DC_log(LOG_DEBUG,"STATUS: retreived_status: '%s'", clgr_string);

		/*********************/
                /* set return state */
    		/*******************/
            if(strcmp(clgr_string, "DONE") == 0)	
               back = STATE_FINISHED;	
	    else if(strcmp(clgr_string, "RUNNING") == 0)
	       back = STATE_RUNNING;
            else if(strcmp(clgr_string, "QUEUED_ACTIVE") == 0)
	       back = STATE_SUBMITTED;
	    else 
	       back = STATE_UNDEFINED;
    	    //DC_log(LOG_DEBUG,"STATUS: status retreived successfully!");
	}
	else if(!strcmp(variable, "subresult_time"))
	{
    	    /***********************************/
    	    /* retreive SUBRESULT_TIME string */
    	    /*********************************/
    
    	    value = result;
	    value = (char *)strchr(result, '=');
    	    value++;
    	    value++;
    	    len = strcspn(value, "\n");
    	    if (len > 31) len = 31;
   	    snprintf(clgr_string, len+1, "%s", value);
	    *subresult_time = (time_t) atoi(clgr_string);
	    DC_log(LOG_DEBUG,"STATUS: subresult time (%d) retreived successfully!", subresult_time);
	}
	else if(!strcmp(variable, "ckpt_time"))
	{
    	    /***********************************/
    	    /* retreive CHEKPOINT_TIME string */
    	    /*********************************/
    
    	    value = result;
	    value = (char *)strchr(result, '=');
    	    value++;
    	    value++;
    	    len = strcspn(value, "\n");
    	    if (len > 31) len = 31;
	    snprintf(clgr_string, len+1, "%s", value);
	    *ckpt_time = (time_t) atoi(clgr_string);
	    DC_log(LOG_DEBUG,"STATUS: checkpoint time (%d) retreived successfully!", ckpt_time);
	}
    }
    pclose(clgr_status);

    // If the CLGR system doesn't found target wu.
 /*   if (n_of_lines == 1){
        DC_log(LOG_ERR,"STATUS: No status found for %s workunit.", wu_name);
	*status = STATE_UNKNOWN;
	return DC_ERROR;
    }
 */ 
    *status = back;
    DC_log(LOG_DEBUG, "STATUS: status retreived successfully!");
    return DC_OK;

}
