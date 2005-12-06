#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dc.h"
#include "result.h"
#include "wu.h"


DC_Result dc_result_create(char *name, char *wuname, char *dir, char *std_out, char *std_err, char *sys_log, int exitcode)
{
    dc_result *result;
    result = (dc_result *) malloc (sizeof(dc_result));
    result->name = strdup(name);
    result->wu = dc_wu_findByName(wuname);
    result->outfiles_dir = strdup(dir);
    if (std_out != NULL) result->std_out = strdup(std_out);
        else result->std_out = NULL;
    if (std_err != NULL) result->std_err = strdup(std_err);
        else result->std_err = NULL;
    if (sys_log != NULL) result->sys_log = strdup(sys_log);
        else result->sys_log = NULL;
    
    result->exitcode = exitcode;
	
    return (DC_Result) result;
}
/*
int dc_result_addOutputFile(DC_Result result, char *filename)
{
    dc_result *dcr = (dc_result *) result;
    if (dcr->noutfiles >= MAX_OUTFILES-1) {
	DC_log( LOG_ERR, "Too many output files for one result!");
	return DC_ERROR;
    }
    dcr->outfiles[dcr->noutfiles] = strdup(filename);
    dcr->noutfiles++;
    return DC_OK;
}
*/
void dc_result_free(DC_Result result)
{
    dc_result *res;
    if (result != NULL) {
	res = (dc_result *) result;
	if (res->name != NULL) {
	    free(res->name);
	    res->name = NULL;
	}
	if (res->outfiles_dir != NULL) {
	    free(res->outfiles_dir);
	    res->outfiles_dir = NULL;
	}
	if (res->std_out != NULL) {
	    free(res->std_out);
	    res->std_out = NULL;
	}
        if (res->std_err != NULL) {
            free(res->std_err);
            res->std_err = NULL;
        }
        if (res->sys_log != NULL) {
            free(res->sys_log);
            res->sys_log = NULL;
        }
	
	free(res);
    }
}
