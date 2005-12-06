#ifndef __GET_OUT_H_
#define __GET_OUT_H_

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Function: get_out() - retreives the output of a completed job
 *			
 * Arguments:
 * 	wu_name: 
 *	work_dir:
 *	output_dir:
 *	result_name:
 *	outfiles[]:
 *	num_of_outfiles:
 *
 * Output:
 *	get_out() returns 0 on success and fills out its arguments
 *
 *
 **/

int get_out(const char *wu_name, const char *work_dir, char *output_dir[1], char *result_name[1],
	char *std_out[1], char *std_err[1], char *sys_log[1], int *exitcode, int IsFinished);
//char *extract_id(const char *wu_name);
		
#ifdef __cplusplus
}
#endif

#endif /* __GET_OUT_H_ */
