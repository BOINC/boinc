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

/* <private_header> */

#ifndef __DC_INTERNAL_H_
#define __DC_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 * Constants
 */

/* Master's working directory (absolute path) */
#define CFG_WORKDIR		"WorkingDirectory"
/* Application instance UUID */
#define CFG_INSTANCEUUID	"InstanceUUID"
/* Log level */
#define CFG_LOGLEVEL		"LogLevel"
/* Log file location */
#define CFG_LOGFILE		"LogFile"


/********************************************************************
 * Function prototypes
 */

/* Parses a configuration file */
int _DC_parseCfg(const char *cfgfile);

/* Copies a file */
int _DC_copyFile(const char *src, const char *dst);

/* Processes a unit suffix and adjust the value accordingly */
long long _DC_processSuffix(const char *suffix);

#ifdef __cplusplus
}
#endif

#endif /* __DC_INTERNAL_H_ */
