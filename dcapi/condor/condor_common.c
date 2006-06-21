/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "dc_common.h"

#include "condor_common.h"


/* Determines the maximum allowed message length. */
int DC_getMaxMessageSize(void)
{
	return(16384);
}


/* Determines the maximum number of sub-results. */
int DC_getMaxSubresults(void)
{
	return(100);
}


/* Determines the basic capabilities of the underlying grid infrastructure. */
unsigned DC_getGridCapabilities(void)
{
	int cap;

	cap= DC_GC_STDOUT | DC_GC_STDERR | DC_GC_MESSAGING | DC_GC_LOG;

	return(cap);
}


/* End of condor/condor_common.c */
