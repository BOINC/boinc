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

/* <private_header> */

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

/* No ssize_t */
typedef long ssize_t;

#define strcasecmp(a, b)	stricmp(a, b)

#ifdef __cplusplus
}
#endif

#endif /* _DC_WIN32_H_ */
