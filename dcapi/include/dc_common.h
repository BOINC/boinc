/* API for Master-worker applications on Distributed Computing Platforms
   Common definitions for the client and server side
*/

#ifndef __DC_COMMON_H_
#define __DC_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/* API error codes */
enum {
	DC_OK,
	DC_ERROR
};

/* Support non-gcc compatible compilers */
#ifndef __GNUC__
#define __attribute__(x)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DC_COMMON_H_ */
