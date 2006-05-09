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
  return(0);
}


/* Determines the maximum number of sub-results. */
int DC_getMaxSubresults(void)
{
  return(0);
}


/* Determines the basic capabilities of the underlying grid infrastructure. */
DC_GridCapabilities DC_getGridCapabilities(void)
{
  return(0);
}


/****************************************************************************/

#define COPY_BUFSIZE 65536
int copy_file(const char *src, const char *dst)
{
  int sfd, dfd;
  ssize_t ret;
  char *buf;

  buf= (char *)g_malloc(COPY_BUFSIZE);
  sfd= open(src, O_RDONLY);
  if (sfd == -1)
    {
      DC_log(LOG_ERR, "Failed to open %s for copying: %s", src,
	     strerror(errno));
      g_free(buf);
      return(-1);
    }
  dfd= open(dst, O_WRONLY | O_CREAT | O_TRUNC);
  if (dfd == -1)
    {
      DC_log(LOG_ERR, "Failed to create %s: %s", dst, strerror(errno));
      g_free(buf);
      close(sfd);
      return(-1);
    }
  
  while ((ret= read(sfd, buf, COPY_BUFSIZE)) > 0)
    {
      char *ptr= buf;
      while (ret)
	{
	  ssize_t ret2= write(dfd, ptr, ret);
	  if (ret2 < 0)
	    {
	      DC_log(LOG_ERR, "Error writing to %s: %s", dst,
		     strerror(errno));
	      close(sfd);
	      close(dfd);
	      unlink(dst);
	      g_free(buf);
	      return(-1);
	    }
	  ret -= ret2;
	  ptr += ret2;
	}
    }
  
  if (ret < 0)
    {
      DC_log(LOG_ERR, "Error reading from %s: %s", src, strerror(errno));
      close(sfd);
      close(dfd);
      g_free(buf);
      unlink(dst);
      return(-1);
    }
  
  g_free(buf);
  close(sfd);
  if (close(dfd))
    {
      DC_log(LOG_ERR, "Error writing to %s: %s", dst, strerror(errno));
      unlink(dst);
      return(-1);
    }
  return(0);
}


/* End of condor/common.c */
