#ifndef __DEFINES_H_
#define __DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of input files */
#define MAX_INFILES     4

/* Maximum number of output files */
#define MAX_OUTFILES    4

/* States of the work unit */
#define STATE_INVALID   0  // don't used space in the wu_table
#define STATE_CREATED   1  // used wu_table part, but is already doesn't submitted
#define STATE_SUBMITTED 2  // a submitted work unit
#define STATE_RUNNING   3  // when the work unit is stated to running on a host
#define STATE_FINISHED  4  // the work unit is finished so we can get output files, create result, and delete work unit

#define STATE_UNKNOWN   5  // don't know what is the state of the selected wu, especially before ask_state function call
#define STATE_UNDEFINED 6  // if the ask_status function cannot realize the status of the given work unit
#define STATE_SUSPENDED 7  // when the master suspended an already submitted work unit
	

/* parameters for getout function */	
#define GETOUT_FINISHED    0  // the work unit was finished so we can delete it from the info system
#define GETOUT_NOTFINISHED 1  // the work unit wasn't finished, only create subresults
	
#ifdef __cplusplus
}
#endif

#endif /* __DEFINES_H_ */
