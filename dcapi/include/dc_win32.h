/*
 * DC-API: Distributed Computing Platform for Master-Worker Applications
 *
 * Compatibility definitions for Windows
 *
 * Authors:
 * 	Gabor Gombas <gombasg@sztaki.hu>
 *
 * Copyright MTA SZTAKI, 2006
 */
#ifndef _DC_WIN32_H_
#define _DC_WIN32_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Windows does not have syslog.h */
enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_NOTICE,
	LOG_WARNING,
	LOG_ERR,
	LOG_CRIT
};

/* The access() macros are missing on Windows */
#define X_OK		1
#define W_OK		2
#define R_OK		4

#ifdef __cplusplus
}
#endif

#endif /* _DC_WIN32_H_ */
