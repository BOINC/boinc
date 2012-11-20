/*
Copyright (c) 1986, 1993, 1995 by University of Toronto.
Written by Henry Spencer.  Not derived from licensed software.

Permission is granted to anyone to use this software for any
purpose on any computer system, and to redistribute it in any way,
subject to the following restrictions:

1. The author is not responsible for the consequences of use of
	this software, no matter how awful, even if they arise
	from defects in it.

2. The origin of this software must not be misrepresented, either
	by explicit claim or by omission.

3. Altered versions must be plainly marked as such, and must not
	be misrepresented (by explicit claim or omission) as being
	the original software.
 *** THIS IS AN ALTERED VERSION.  It was altered by Benoit
 *** Goudreault-Emond, bge@videotron.ca, on 2002/01/03, to add \< and
 *** \> for word matching, as well as pseudo character classes.  It
 *** also handles wide characters quite well now. Also, the API was
 *** changed so an arbitrary number of parenthesized expressions can
 *** be used (and so were the internals of the regexp engine).
 ***
 *** In general, the new API should be more re-entrant.

4. This notice must not be removed or altered.
*/

/*
 * regerror
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regexp.h"

struct error_message
{
    int err;
    const char* msg;
};

const struct error_message errors[] = {
    { 0, "no errors detected" },
    { REGEXP_BADARG,  "invalid argument" },
    { REGEXP_ESIZE,   "regular expression too big" },
    { REGEXP_ESPACE,  "out of memory" },
    { REGEXP_EPAREN,  "parenteses () not balanced" },
    { REGEXP_ERANGE,  "invalid character range" },
    { REGEXP_EBRACK,  "brackets [] not balanced" },
    { REGEXP_BADRPT,  "quantifier operator invalid" },
    { REGEXP_EESCAPE, "invalid escape \\ sequence" },
    { REGEXP_EEND,    "internal error!" },
    { 1,              "unknown error code (0x%x)!" }
};

#define ERROR_BUFFER_SIZE 80

void re_error(int errcode, const regexp* re, char* buffer, size_t bufsize)
{
    char convbuf[ERROR_BUFFER_SIZE];
    
    if(errcode >= 0)
    {
        strcpy(convbuf, errors[0].msg);
    }
    else
    {
        register int i;
        for(i = 1; errors[i].err != 1; ++i)
        {
            if(errors[i].err == errcode)
            {
                strcpy(convbuf, errors[i].msg);
                break;
            }
        }
        if(errors[i].err == 1)
            sprintf(convbuf, errors[i].msg, -errcode);
    }
    strncpy(buffer, convbuf, bufsize-1);
    buffer[bufsize-1] = '\0';    
}
