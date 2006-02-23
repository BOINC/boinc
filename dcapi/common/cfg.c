#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <dc_internal.h>

/* Private functions */
void dc_cfg_cutTrailingWhiteSpace(char *line);
static int tokenise(char *line, char *separators, char *tokens[], int max);

struct pair
{
	char *name;
	char *value;
};

static struct pair *pairs;
static int n_pairs;

/* Public functions */
int _DC_parseCfg(const char *cfgfile)
{
	FILE *cfg;
	char line[1024];
	char *str;
#   define MAX_TOKEN 3
	char *tokens[MAX_TOKEN];
	int tokenCount;

	if ((cfg = fopen(cfgfile, "r")) == NULL)
	{
		fprintf(stderr, "Config file %s cannot be opened: %s\n",
			cfgfile, strerror(errno));
		return -1;
	}

	while (fgets(line, 1024, cfg) != NULL)
	{
		/* Cut leading white space */
		for (str = line; *str == ' ' || *str == '\t'; str++)
			/* Nothing */;

		/* Skip empty lines and comments */
		if (!*str || *str == '\n' || *str == '#')
			continue;

		tokenCount = tokenise(line, "=\n", tokens, MAX_TOKEN);
		if (tokenCount == 2)
		{
			struct pair *tmp;

			tmp = realloc(pairs, (n_pairs + 1) * sizeof(*tmp));
			if (!tmp)
			{
				fprintf(stderr, "Out of memory while parsing "
					"config file\n");
				return -1;
			}
			pairs = tmp;
			pairs[n_pairs].name = strdup(tokens[0]);
			pairs[n_pairs].value = strdup(tokens[1]);
			n_pairs++;
		}
		else
		{
			fprintf(stderr, "Cannot understand in config file %s "
				"the line %s", cfgfile, line);
			return -1;
		}
	}
	return 0;
}

const char *_DC_getCfgStr(const char *name)
{
	int i;

	if (!name)
		return NULL;

	for (i = 0; i < n_pairs; i++)
	{
		if (pairs[i].name && !strcmp(pairs[i].name, name))
			return pairs[i].value;
	}
	return NULL;
}

void dc_cfg_cutTrailingWhiteSpace(char *line)
{
	/* go until white spaces are at the end of the string */
	char *str = line;
	int len = strlen(line);
	if (len > 0)
	{
		str = line + len - 1;
		while (len > 0 && (str[0] == ' ' || str[0] == '\t'))
		{
			str[0] = '\0';
			str--;
		}
	}
}

/* break string into tokens and save tokens in arrays */
static int tokenise(char *line, char *separators, char *tokens[], int max)
{
	int i = 0;
	char *pch;

	pch = strtok(line, separators);	/* get 1st token addr */
	while (pch)			/* repeat for each token */
	{
		if (i < max)
		{
			/* Cut leading white space */
			while (*pch == ' ' || *pch == '\t')
				pch++;
			tokens[i] = pch;
			dc_cfg_cutTrailingWhiteSpace(tokens[i]);
		}
		pch = strtok(NULL, separators);	/* next token address */
		i++;
	}
	return i;
}
