/*
 * cfg-client.c
 *
 * Simple config file parser for the client
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <dc.h>
#include <dc_internal.h>

#include <glib.h>

/********************************************************************
 * Constants
 */

/* Name of the group holding the master's configuration */
#define MASTER_GROUP		"Master"


/********************************************************************
 * Global variables
 */

static GKeyFile *config;


/********************************************************************
 * Implementation
 */

int _DC_parseCfg(const char *cfgfile)
{
	GError *error = NULL;
	int ret;

	/* Should not happen */
	if (!cfgfile)
		return DC_ERR_INTERNAL;

	config = g_key_file_new();
	ret = g_key_file_load_from_file(config, cfgfile, G_KEY_FILE_NONE,
		&error);
	if (!ret)
	{
		DC_log(LOG_ERR, "Failed to load the config file %s: %s",
			cfgfile, error->message);
		g_error_free(error);
		return DC_ERR_CONFIG;
	}
	return 0;
}

static char *getCfgStr(const char *group, const char *key)
{
	char *value, *tmp;

	if (!config || !key)
		return NULL;

	value = g_key_file_get_value(config, group, key, NULL);
	if (!value || g_mem_is_system_malloc())
		return value;
	tmp = strdup(value);
	g_free(value);
	return tmp;
}

static int getCfgInt(const char *group, const char *key, int defaultValue,
	int *err)
{
	char *value, *p;
	long retval;

	if (!config || !key)
		return defaultValue;

	value = g_key_file_get_value(config, group, key, NULL);
	if (!value)
	{
		*err = 1;
		return defaultValue;
	}

	retval = strtol(value, &p, 10);
	/* Check for unit suffixes */
	if (p && *p)
	{
		long mult = _DC_processSuffix(p);
		if (mult == -1)
		{
			DC_log(LOG_WARNING, "Configuration value for key %s "
				"is not a valid number, ignoring", key);
			g_free(value);
			*err = 1;
			return defaultValue;
		}
		retval *= mult;
	}

	g_free(value);
	*err = 0;
	return retval;
}

static double getCfgDouble(const char *group, const char *key,
	double defaultValue, int *err)
{
	char *value, *p;
	double retval;

	if (!config || !key)
		return defaultValue;

	value = g_key_file_get_value(config, group, key, NULL);
	if (!value)
	{
		*err = 1;
		return defaultValue;
	}

	retval = strtod(value, &p);
	if (p && *p)
	{
		long long mult = _DC_processSuffix(p);
		if (mult == -1)
		{
			DC_log(LOG_WARNING, "Configuration value for key %s "
				"is not a valid number, ignoring", key);
			g_free(value);
			*err = 1;
			return defaultValue;
		}
		retval *= mult;
	}

	g_free(value);
	*err = 0;
	return retval;
}

static int getCfgBool(const char *group, const char *key, int defaultValue,
	int *err)
{
	char *value;
	int retval;

	if (!config || !key)
		return !!defaultValue;

	value = g_key_file_get_value(config, group, key, NULL);
	if (!value)
	{
		*err = 1;
		return !!defaultValue;
	}

	retval = _DC_parseBoolean(value);
	g_free(value);
	if (retval == -1)
	{
		*err = 1;
		return !!defaultValue;
	}
	*err = 0;
	return retval;
}

char *DC_getCfgStr(const char *key)
{
	return getCfgStr(MASTER_GROUP, key);
}

int DC_getCfgInt(const char *key, int defaultValue)
{
	int err;

	return getCfgInt(MASTER_GROUP, key, defaultValue, &err);
}

int DC_getCfgBool(const char *key, int defaultValue)
{
	int err;

	return getCfgBool(MASTER_GROUP, key, defaultValue, &err);
}

char *DC_getClientCfgStr(const char *clientName, const char *key,
	int fallbackGlobal)
{
	char *group, *val;

	if (!clientName)
		return NULL;

	group = g_strdup_printf("Client-%s", clientName);
	val = getCfgStr(group, key);
	g_free(group);
	if (!val && fallbackGlobal)
		val = getCfgStr(MASTER_GROUP, key);
	return val;
}

int DC_getClientCfgInt(const char *clientName, const char *key,
	int defaultValue, int fallbackGlobal)
{
	int val, err;
	char *group;

	if (!clientName)
		return defaultValue;

	group = g_strdup_printf("Client-%s", clientName);
	val = getCfgInt(group, key, defaultValue, &err);
	g_free(group);
	if (err && fallbackGlobal)
		val = getCfgInt(MASTER_GROUP, key, defaultValue, &err);
	return val;
}

double DC_getClientCfgDouble(const char *clientName, const char *key,
	double defaultValue, int fallbackGlobal)
{
	char *group;
	double val;
	int err;

	if (!clientName)
		return defaultValue;

	group = g_strdup_printf("Client-%s", clientName);
	val = getCfgDouble(group, key, defaultValue, &err);
	g_free(group);
	if (err && fallbackGlobal)
		val = getCfgDouble(MASTER_GROUP, key, defaultValue, &err);
	return val;
}

int DC_getClientCfgBool(const char *clientName, const char *key,
	int defaultValue, int fallbackGlobal)
{
	int val, err;
	char *group;

	if (!clientName)
		return !!defaultValue;

	group = g_strdup_printf("Client-%s", clientName);
	val = getCfgBool(group, key, defaultValue, &err);
	g_free(group);
	if (err && fallbackGlobal)
		val = getCfgBool(MASTER_GROUP, key, defaultValue, &err);
	return val;
}

int _DC_initClientConfig(const char *clientName, FILE *f)
{
	char **keys, *group, *val;
	gsize i, cnt;

	if (!clientName)
		return DC_ERR_BADPARAM;

	group = g_strdup_printf("Client-%s", clientName);
	keys = g_key_file_get_string_list(config, group, CFG_SENDKEYS, &cnt,
		NULL);

	/* Always set LogLevel */
	val = g_key_file_get_value(config, group, CFG_LOGLEVEL, NULL);
	if (!val)
		val = g_key_file_get_value(config, MASTER_GROUP, CFG_LOGLEVEL,
			NULL);
	if (val)
	{
		fprintf(f, "%s = %s\n", CFG_LOGLEVEL, val);
		g_free(val);
	}

	/* Copy the values of the requested keys */
	for (i = 0; keys && i < cnt; i++)
	{
		char *p;

		/* Strip white space from the key name */
		p = keys[i] + strlen(keys[i]) - 1;
		while (*p == ' ' || *p == '\t')
			*p-- = '\0';
		p = keys[i];
		while (*p == ' ' || *p == '\t')
			p++;

		val = g_key_file_get_value(config, group, p, NULL);
		if (!val)
			val = g_key_file_get_value(config, MASTER_GROUP, p,
				NULL);
		if (val)
		{
			fprintf(f, "%s = %s\n", p, val);
			g_free(val);
		}
	}

	g_strfreev(keys);
	g_free(group);
	return 0;
}
