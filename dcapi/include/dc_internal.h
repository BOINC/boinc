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

#include <stdio.h>
#include <dc_common.h>

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
/* Configuration items to send to clients */
#define CFG_SENDKEYS		"SendCfgKeys"
/* Default WU priority */
#define CFG_DEFAULTPRIO		"DefaultPriority"


/********************************************************************
 * Function prototypes
 */

/* Parses a configuration file */
int _DC_parseCfg(const char *cfgfile) __attribute__((visibility("hidden")));

/* Copies a file */
int _DC_copyFile(const char *src, const char *dst) __attribute__((visibility("hidden")));

/* Processes a unit suffix and adjust the value accordingly */
long long _DC_processSuffix(const char *suffix) __attribute__((visibility("hidden")));

/* Parses a boolean value */
int _DC_parseBoolean(const char *value) __attribute__((visibility("hidden")));

/* Allocates a physical file descriptor */
DC_PhysicalFile *_DC_createPhysicalFile(const char *label, const char *path)
	__attribute__((visibility("hidden")));

/* Allocates a remote file descriptor */
DC_RemoteFile *_DC_createRemoteFile(const char *label, const char *url, const char *md5, const size_t size)
	__attribute__((visibility("hidden")));

/* De-allocates a physical file descriptor */
void _DC_destroyPhysicalFile(DC_PhysicalFile *file)
	__attribute__((visibility("hidden")));

/* De-allocates a remote file descriptor */
void _DC_destroyRemoteFile(DC_RemoteFile *file)
	__attribute__((visibility("hidden")));

/* Initializa a config file for a client */
int _DC_initClientConfig(const char *clientName, FILE *f)
	__attribute__((visibility("hidden")));

#ifdef __cplusplus
}
#endif

#endif /* __DC_INTERNAL_H_ */
