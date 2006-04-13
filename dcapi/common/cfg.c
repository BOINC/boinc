/*
 * cfg-client.c
 *
 * Simple config file parser for the client
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include <dc_internal.h>
#include <dc_common.h>

/********************************************************************
 * Data type definitions
 */

struct pair
{
	char			*key;
	char			*value;
};


/********************************************************************
 * Global variables
 */

static struct pair *pairs;
static int n_pairs;


/********************************************************************
 * Implementation
 */

static void cut_trailing_whitespace(char *p)
{
	char *end;

	end = p + strlen(p) - 1;
	while (end >= p && *end && isspace(*end))
		*end-- = '\0';
}

int _DC_parseCfg(const char *cfgfile)
{
	char line[1024], *key, *value;
	struct pair *tmp;
	int linecnt, ret;
	FILE *f;

	/* Should not happen */
	if (!cfgfile)
		return DC_ERR_INTERNAL;

	f = fopen(cfgfile, "r");
	if (!f)
	{
		ret = errno;
		DC_log(LOG_ERR, "Config file %s cannot be opened: %s",
			cfgfile, strerror(ret));
		errno = ret;
		return DC_ERR_SYSTEM;
	}

	linecnt = 0;
	while (fgets(line, 1024, f) != NULL)
	{
		linecnt++;

		/* Cut leading white space */
		for (key = line; isspace(*key); key++)
			/* Nothing */;

		/* Skip empty lines and comments */
		if (!*key || *key == '\n' || *key == '#')
			continue;

		value = strchr(line, '=');
		if (!value)
		{
			DC_log(LOG_ERR, "Syntax error in config file %s "
				"line %d: '=' is missing", cfgfile, linecnt);
			fclose(f);
			return DC_ERR_CONFIG;
		}

		*value++ = '\0';
		/* Cut leading white space */
		while (isspace(*value))
			value++;

		cut_trailing_whitespace(key);
		cut_trailing_whitespace(value);

		tmp = realloc(pairs, (n_pairs + 1) * sizeof(*tmp));
		if (!tmp)
		{
			DC_log(LOG_ERR, "Out of memory while parsing the "
				"config file");
			fclose(f);
			errno = ENOMEM;
			return DC_ERR_SYSTEM;
		}

		tmp[n_pairs].key = strdup(key);
		tmp[n_pairs].value = strdup(value);
		pairs = tmp;
		n_pairs++;
	}

	fclose(f);
	return 0;
}

const char *DC_getCfgStr(const char *key)
{
	int i;

	if (!key)
		return NULL;

	for (i = 0; i < n_pairs; i++)
	{
		if (pairs[i].key && !strcmp(pairs[i].key, key))
			return pairs[i].value;
	}
	return NULL;
}
