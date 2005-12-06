#ifndef __STATUS_H_
#define __STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function: ask_status(const char *wu_name, int *status, 
 *		    time_t *subresult_time, time_t *ckpt_time) - 
 *	retreives the status info of a submitted job identified by
 *	the Work Unit Name (wu_name) of the job
 *	In addition it also retreives 
 *	- Subresult Time (subresult_time) and
 *	- Chekpoint Time (ckpt_time)
 *	of the job, if any, or returns NULL if missing.
 *			
 *
 * Argumets:
 *	char *wu_name: work_unit name
 *	int *status: return argument, status of the job
 *	int *subresult_time: return argument, subresult time info
 *	int *ckpt_time: return argument, chekpoint time info
 *
 * Output:
 *	Submit() returns zero upon success and the collected infos can
 *	be retreived from it's return arguments
 *	
 **/

int ask_status(const char *wu_name, int *status, time_t *subresult_time, time_t *ckpt_time);

#ifdef __cplusplus
}
#endif

#endif /* __STATUS_H_ */
