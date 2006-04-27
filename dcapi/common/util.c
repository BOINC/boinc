#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <dc_internal.h>
#include <dc_common.h>

/********************************************************************
 * Constants
 */

/* Buffer size used when copying a file */
#define COPY_BUFSIZE		65536


/********************************************************************
 * Functions
 */

int _DC_copyFile(const char *src, const char *dst)
{
	int sfd, dfd;
	ssize_t ret;
	char *buf;

	buf = (char *)malloc(COPY_BUFSIZE);
	if (!buf)
		return DC_ERR_SYSTEM;

	sfd = open(src, O_RDONLY);
	if (sfd == -1)
	{
#if 0
		DC_log(LOG_ERR, "Failed to open %s for copying: %s", src,
			strerror(errno));
#endif
		free(buf);
		return -1;
	}
	dfd = open(dst, O_WRONLY | O_CREAT | O_TRUNC);
	if (dfd == -1)
	{
#if 0
		DC_log(LOG_ERR, "Failed to create %s: %s", dst, strerror(errno));
#endif
		free(buf);
		close(sfd);
		return -1;
	}

	while ((ret = read(sfd, buf, COPY_BUFSIZE)) > 0)
	{
		char *ptr = buf;
		while (ret)
		{
			ssize_t ret2 = write(dfd, ptr, ret);
			if (ret2 < 0)
			{
#if 0
				DC_log(LOG_ERR, "Error writing to %s: %s", dst,
					strerror(errno));
#endif
				goto error;
			}
			ret -= ret2;
			ptr += ret2;
		}
	}

	if (ret < 0)
	{
#if 0
		DC_log(LOG_ERR, "Error reading from %s: %s", src, strerror(errno));
#endif
		goto error;
	}

	free(buf);
	close(sfd);
	if (close(dfd))
	{
#if 0
		DC_log(LOG_ERR, "Error writing to %s: %s", dst, strerror(errno));
#endif
		unlink(dst);
		return -1;
	}
	return 0;

error:
	close(sfd);
	close(dfd);
	free(buf);
	unlink(dst);
	return -1;
}

int _DC_processSuffix(long *value, const char *suffix)
{
	while (*suffix == ' ' || *suffix == '\t')
		suffix++;
	if (!strcasecmp(suffix, "kb") || !strcasecmp(suffix, "kib"))
		*value <<= 10;
	else if (!strcasecmp(suffix, "mb") || !strcasecmp(suffix, "mib"))
		*value <<= 20;
	else if (!strcasecmp(suffix, "gb") || !strcasecmp(suffix, "gib"))
		*value <<= 30;
	else if (!strcasecmp(suffix, "min"))
		*value *= 60;
	else if (!strcasecmp(suffix, "h") || !strcasecmp(suffix, "hour"))
		*value *= 60 * 60;
	else if (!strcacecmp(suffix, "day"))
		*value *= 24 * 60 * 60;
	else
		return -1;
	return 0;
}
