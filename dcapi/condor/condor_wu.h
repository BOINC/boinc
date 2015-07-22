/*
 * condor/condor_wu.h
 *
 * DC-API functions to handle DC_Workunit data type
 *
 * (c) Daniel Drotos, 2006
 */

#ifndef _DC_API_CONDOR_WU_H_
#define _DC_API_CONDOR_WU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "condor_defs.h"
#include "condor_common.h"


extern void _DC_wu_changed(DC_Workunit *wu);
extern int _DC_wu_check(const DC_Workunit *wu);

extern int _DC_wu_set_client_name(DC_Workunit *wu, const char *new_name);
extern int _DC_wu_set_argc(DC_Workunit *wu, int new_argc);
extern int _DC_wu_set_uuid_str(DC_Workunit *wu, char *new_uuid_str);
extern int _DC_wu_set_name(DC_Workunit *wu, char *new_name);
extern int _DC_wu_set_tag(DC_Workunit *wu, char *new_tag);
extern int _DC_wu_set_subresults(DC_Workunit *wu, int new_subresults);
extern int _DC_wu_set_workdir(DC_Workunit *wu, char *new_workdir);

extern DC_WUState _DC_wu_set_state(DC_Workunit *wu,
				   DC_WUState new_state);

extern char *_DC_wu_cfg(DC_Workunit *wu,
			enum _DC_e_param what);

extern int _DC_wu_check_logical_name(DC_Workunit *wu,
				     const char *logicalFileName);
extern char *_DC_wu_get_workdir_path(DC_Workunit *wu,
				     const char *label,
				     WorkdirFile type);
extern int _DC_wu_gen_condor_submit(DC_Workunit *wu);
extern int _DC_wu_make_client_executables(DC_Workunit *wu);
extern int _DC_wu_make_client_config(DC_Workunit *wu);
extern DC_MasterEvent *_DC_wu_check_client_messages(DC_Workunit *wu);
extern void _DC_wu_input_list_foreach(gpointer data, gpointer user_data);
extern void _DC_wu_output_list_foreach(gpointer data, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_wu.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
