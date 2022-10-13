#ifndef COMPAT_H
#define COMPAT_H

#include "wrapper_config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char*, const char*, size_t);
#endif

#ifndef HAVE_STRLCAT
size_t strlcat(char*, const char*, size_t);
#endif

#endif /* COMPAT_H */
