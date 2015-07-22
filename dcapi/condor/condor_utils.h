/*
 * condor/condor_utils.h
 *
 * DC-API usefull functions
 *
 * (c) Daniel Drotos, 2006
 */

#ifndef __DC_API_CONDOR_UTILS_H_
#define __DC_API_CONDOR_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>

#include "condor_common.h"


extern void _DC_init_utils(void);

/* Usefull funcs */
extern int _DC_mkdir_with_parents(char *dn, mode_t mode);
extern int _DC_rm(char *name);
extern int _DC_file_exists(char *fn);
extern int _DC_file_empty(char *fn);
extern int _DC_create_file(char *fn, char *what);
extern char *_DC_get_file(char *fn);

/* Message passing utilities */
extern int _DC_create_message(char *box,
			      char *name,
			      const char *message,
			      char *msgfile);
extern int _DC_nuof_messages(char *box, char *name);
extern char *_DC_message_name(char *box, char *name);
extern char *_DC_read_message(char *box, char *name, int del_msg);

extern char *_DC_quote_string(char *str);
extern char *_DC_unquote_string(char *str);


#ifdef __cplusplus
}
#endif

#endif

/* End of condor/condor_utils.h */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
