#ifndef __RESULT_H_
#define __RESULT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dc.h"


DC_Result  dc_result_create(char *name, char *wuname, char *dir, char *std_out, char *std_err, char *sys_log, int exitcode);
//int        dc_result_addOutputFile(DC_Result result, char *filename);
void       dc_result_free(DC_Result result);

#ifdef __cplusplus
}
#endif

#endif /* __RESULT_H_ */
