/*
 * condor/tc.c
 *
 * DC-API test application, utility for both sides
 *
 * (c) Daniel Drotos, 2006
 */

#include <stdio.h>
#include <stdlib.h>

#include "tc.h"

void
create_file(char *fn, char *what)
{
	FILE *f= fopen(fn, "w");
	if (f)
	{
		if (what)
			fprintf(f, "%s", what);
		fclose(f);
	}
}

char *
get_file(char *fn)
{
	FILE *f;
	char *buf= NULL;

	if ((f= fopen(fn, "r")) != NULL)
	{
		int bs= 100, i;
		int c;

		buf= malloc(bs);
		i= 0;
		buf[i]= '\0';
		while ((c= fgetc(f)) != EOF)
		{
			if (i > bs-2)
			{
				bs+= 100;
				buf= realloc(buf, bs);
			}
			buf[i]= (char)c;
			i++;
		}
		buf[i]= '\0';
		fclose(f);
	}
	return(buf);
}

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
