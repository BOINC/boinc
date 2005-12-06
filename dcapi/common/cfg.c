#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "cfg.h"
#include "dc.h"

/* Private functions */
static char * dc_cfg_cutLeadingWhiteSpace(char *line);
void          dc_cfg_cutTrailingWhiteSpace(char *line);
static int tokenise(char *line, char *separators, char *tokens[], int max);

typedef struct {
    char *name;
    char *value;
} NameValuePair;

static NameValuePair *pairs;
static int n_pairs;

/* Public functions */
int dc_cfg_parse(const char *cfgfile)
{
    FILE *cfg;
    char line[1024];
    char *str;
#   define MAX_TOKEN 3
    char *tokens[MAX_TOKEN];
    int tokenCount;

    if ((cfg = fopen(cfgfile, "r")) == NULL) {
	fprintf(stderr, "Config file %s cannot be opened\nErrno=%d %s",
	       cfgfile, errno, strerror(errno));
	return DC_CFG_FILENOTEXISTS;
    }

    while (fgets(line, 1024, cfg) != NULL) {
	str = dc_cfg_cutLeadingWhiteSpace(line);

	if (str[0] == '\0' || str[0] == '\n') continue; /* empty */
	if (str[0] == '#') continue; /* comment */

	tokenCount = tokenise(line, "=\n", tokens, MAX_TOKEN);
	if (tokenCount == 2) {
	    NameValuePair *tmp;

	    tmp = realloc(pairs, (n_pairs + 1) * sizeof(*tmp));
	    if (!tmp) {
		fprintf(stderr, "Out of memory while parsing config file\n");
		return DC_CFG_OUTOFMEM;
	    }
	    pairs = tmp;
	    pairs[n_pairs].name = strdup(tokens[0]);
	    pairs[n_pairs].value = strdup(tokens[1]);
	    n_pairs++;
	}
	else {
	    fprintf(stderr, 
		   "Cannot understand in config file %s the line %s",
		   cfgfile, line);
	}
    }
    return DC_CFG_OK;
}

char *dc_cfg_get(char *name) 
{
  /* Return the value of the name=value pair */
    int i = 0;

    if (name == NULL) return NULL;

    while (i < n_pairs) { 
	if (pairs[i].name != NULL && 
	    !strcmp(pairs[i].name, name))
	    return pairs[i].value;
	i++;
    }

    return NULL;
}

static char *dc_cfg_cutLeadingWhiteSpace(char *line)
{
    /* go until white spaces are in front of the string */
    char *str = line;
    while (str[0] == ' ' || str[0] == '\t') str++;
    return str;
}

void dc_cfg_cutTrailingWhiteSpace(char *line)
{
    /* go until white spaces are at the end of the string */
    char *str = line;
    int len = strlen(line);
    if (len > 0) {
	str = line+len-1;
	while (len > 0 && 
	       (str[0] == ' ' || str[0] == '\t')) {
	    str[0]='\0';
	    str--;
	}
    }
}

/* break string into tokens and save tokens in arrays */
static int tokenise(char *line, char *separators, char *tokens[], int max)
{
   int i = 0;
   char *pch;
   
   pch = strtok( line, separators );  /* get 1st token addr */
   while( pch != NULL )               /* repeat for each token */
   {
      if ( i < max )
      {
         /* strip leading and trailing spaces and
	    save token in array 
	 */
	  tokens[i] = dc_cfg_cutLeadingWhiteSpace(pch);    
	  dc_cfg_cutTrailingWhiteSpace(tokens[i]);
      }
      pch = strtok( NULL, separators );  /* next token address */
      i++;
   }
   return( i );
}
