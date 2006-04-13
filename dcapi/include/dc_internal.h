/*
 * DC-API: Distributed Computing Platform for Master-Worker Applications
 *
 * Internal definitions
 *
 * Authors:
 * 	Gabor Gombas <gombasg@sztaki.hu>
 *
 * Copyright MTA SZTAKI, 2006
 */

/* <Tell gtk-doc that this is a private_header> */

#ifndef __DC_INTERNAL_H_
#define __DC_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 * Function prototypes
 */

/* Parses the config file and stores its contents in memory.
 *
 * @cfgfile: the name of the config file. May not be %NULL.
 * 
 * @Returns: 0 if successful or an error code.
 */
int _DC_parseCfg(const char *cfgfile);

/* Copies a file */
int _DC_copyFile(const char *src, const char *dst);

#ifdef __cplusplus
}
#endif

#endif /* __DC_INTERNAL_H_ */
