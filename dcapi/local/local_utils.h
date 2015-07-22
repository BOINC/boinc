/*
 * local/local_utils.h
 *
 * DC-API usefull functions
 *
 * (c) Daniel Drotos, 2006-2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#ifndef __DC_API_LOCAL_UTILS_H_
#define __DC_API_LOCAL_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>

#include "local_common.h"


void _DC_init_utils(void) G_GNUC_INTERNAL;

/* Usefull funcs */
int _DC_mkdir_with_parents(char *dn, mode_t mode) G_GNUC_INTERNAL;
int _DC_rm(char *name) G_GNUC_INTERNAL;
int _DC_file_exists(char *fn) G_GNUC_INTERNAL;
int _DC_file_empty(char *fn) G_GNUC_INTERNAL;
int _DC_create_file(char *fn, char *what) G_GNUC_INTERNAL;
char *_DC_get_file(char *fn) G_GNUC_INTERNAL;

/* Message passing utilities */
int _DC_create_message(char *box,
			      char *name,
			      const char *message,
			      char *msgfile) G_GNUC_INTERNAL;
int _DC_nuof_messages(char *box, char *name) G_GNUC_INTERNAL;
char *_DC_message_name(char *box, char *name) G_GNUC_INTERNAL;
char *_DC_read_message(char *box, char *name, int del_msg) G_GNUC_INTERNAL;

char *_DC_quote_string(char *str) G_GNUC_INTERNAL;
char *_DC_unquote_string(char *str) G_GNUC_INTERNAL;


#ifdef __cplusplus
}
#endif

#endif /* __DCAPI_LOCAL_UTILS_H_ */
