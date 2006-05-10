#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <strings.h>
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

long long _DC_processSuffix(const char *suffix)
{
	while (*suffix == ' ' || *suffix == '\t')
		suffix++;
	if (!strcasecmp(suffix, "kb") || !strcasecmp(suffix, "kib"))
		return 1ll << 10;
	else if (!strcasecmp(suffix, "mb") || !strcasecmp(suffix, "mib"))
		return 1ll << 20;
	else if (!strcasecmp(suffix, "gb") || !strcasecmp(suffix, "gib"))
		return 1ll << 30;
	else if (!strcasecmp(suffix, "min"))
		return 60ll;
	else if (!strcasecmp(suffix, "h") || !strcasecmp(suffix, "hour"))
		return 60ll * 60;
	else if (!strcasecmp(suffix, "day"))
		return 24ll * 60 * 60;
	return -1;
}
