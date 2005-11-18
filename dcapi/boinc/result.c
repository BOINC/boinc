#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dc.h"
#include "result.h"
#include "wu.h"


DC_Result dc_result_create(char *name, char *wuname, char *dir)
{
    dc_result *result;
    result = (dc_result *) malloc (sizeof(dc_result));
    result->name = strdup(name);
    result->wu = dc_wu_findByName(wuname);
    result->outfiles_dir = strdup(dir);
    result->noutfiles = 0;
    return (DC_Result) result;
}

int dc_result_addOutputFile(DC_Result result, char *filename)
{
    dc_result *dcr = (dc_result *) result;
    if (dcr->noutfiles >= MAX_OUTFILES-1) {
	DC_log(LOG_ERR, "Too many output files for one result!");
	return DC_ERROR;
    }
    dcr->outfiles[dcr->noutfiles] = strdup(filename);
    dcr->noutfiles++;
    return DC_OK;
}

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
	free(res);
    }
}
