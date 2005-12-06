#ifndef __SUBMIT_H_
#define __SUBMIT_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define MAX_INFILES 4

/**
 * Function: Submit() - creates directory and file structure for
 * 			the job to be submitted and than submits
 *			the job in the ClusterGrid for processing
 *
 * Arguments:
 * 	work_dir_base   - the base directory for the job (eg.: wrokdir)
 *	work_dir_id     - the identifier for the job (eg.: )
 *	work_dir_num    - number of the job (eg.: 1)
 *	executable_dir  - function gets the executables for the job from this directory (eg.: ./executable)
 *	executable 	- the name of the executable of the job (eg.: worker)
 *	job_name	- name of the job (eg.: test_job)
 *	args		- arguments of the job
 *	arg_type	- type of the arguments
 *	num_of_infiles  - number of the input files the executable processes
 *	infiles[]	- input files with path
 *	localfilenames  - input filenames gets renamed for these names
 *
 * Output:
 *	Submit() returns a job identifier string upon success
 *	or returns NULL upon faliure
 *
 * Remarks:
 * 	Submit uses function retreive_id() to extract the job identifier from the string
 * 	that is returned by clgr_submit command and it reserves memory for the id with a malloc
 *	To free the memory allocated for the id is the job of the caller of Submit()
 *
 **/

char *submit(const char *work_dir_base, const char *work_dir_id, const char *work_dir_num, 
		const char *executable_dir, const char *executale, const char *job_name,
		const char *args, int num_of_infiles,
		const char *infiles[MAX_INFILES], const char *localfilenames[MAX_INFILES]);

int resubmit(char *workdir, char *wu_name[1]);
/*
  int create_work_dir(const char *work_dir_base, const char *workdir_id, const char *work_dir_num);
  int create_jobdir_structure();
  int create_submit_file(const char *job_name, const char *executable, const char *args, const char *arg_type);
  int copy_executables(const char *executable_dir, const char *work_dir);
  int copy_inputfiles(int num_of_infiles, char* infiles[MAX_INFILES], 
  		      char* localfilenames[MAX_INFILES], const char *work_dir); 
  char *job_submit(const char *work_dir);
  char *retreive_id(const char *result);
*/

#ifdef __cplusplus
}
#endif

#endif /* __SUBMIT_H_ */
