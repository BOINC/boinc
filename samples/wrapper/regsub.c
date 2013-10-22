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
 * regsub
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regexp_int.h"
#include "regexp_custom.h"
#include "regmagic.h"

static int internal_sub(const CHAR_TYPE* s, const CHAR_TYPE* source, regmatch matches[10], CHAR_TYPE* dest)
{
    register int length = 0;
    register int no;
    register const CHAR_TYPE* src = source;
    register int len = 0;
    register CHAR_TYPE* dst = dest;
    register CHAR_TYPE c;
    
	while ((c = *src++) != LIT('\0')) {
		if (c == LIT('&'))
			no = 0;
		else if (c == LIT('\\') && cisdigit(*src))
			no = *src++ - LIT('0');
		else
			no = -1;

		if (no < 0) {	/* Ordinary character. */
			if (c == LIT('\\') && (*src == LIT('\\') || *src == LIT('&')))
				c = *src++;
            ++length;
            if(dst)
                *dst++ = c;
        } else if (matches[no].begin != -1 && matches[no].end != -1 &&
                   matches[no].end > matches[no].begin) {
			len = matches[no].end - matches[no].begin;
            length += len;
            if(dst)
            {
                cstrncpy(dst, s + matches[no].begin, len);
                dst += len;
                if(*(dst - 1) == LIT('\0'))
                    return REGEXP_EEND;
            }
		}
	}
    if(dst)
    {
        *dst++ = LIT('\0');
        return 1;
    }
    else
        return length + 1;
}

int re_subcount_w(const regexp* rp, const CHAR_TYPE* s, const CHAR_TYPE* src, regmatch matches[10])
{
    register int error;
    
	if (rp == NULL || src == NULL || s == NULL || matches == NULL) {
		re_report("NULL parameter to regsub");
		return REGEXP_BADARG;
	}
	if ((UCHAR_TYPE)*(rp->program) != MAGIC) {
		re_report("damaged regexp");
		return REGEXP_BADARG;
	}
    
    if ((error = re_exec_w(rp, s, 10, matches)) < 1)
        return error;

    /* run count */
    return internal_sub(s, src, matches, NULL);
}

int re_dosub_w(const CHAR_TYPE* s, const CHAR_TYPE* src, regmatch matches[10], CHAR_TYPE* dest)
{
	if (src == NULL || s == NULL || matches == NULL || dest == NULL) {
		re_report("NULL parameter to regsub");
		return REGEXP_BADARG;
	}

    return internal_sub(s, src, matches, dest);
}

/*
 - reg_sub_w - perform substitutions
 */
int re_sub_w(const regexp* rp, const CHAR_TYPE* s, const CHAR_TYPE* source, CHAR_TYPE** dest)
{
    /* note: there can only be 10 expressions (\0 to \9) */
    regmatch matches[10];
    int error;

    if(dest)
        *dest = NULL;
	if (rp == NULL || source == NULL || s == NULL || dest == NULL) {
		re_report("NULL parameter to regsub");
		return REGEXP_BADARG;
	}
	if ((UCHAR_TYPE)*(rp->program) != MAGIC) {
		re_report("damaged regexp");
		return REGEXP_BADARG;
	}
    /* figure out how much room is needed */
    if((error = re_subcount_w(rp, s, source, matches)) < 1)
        return error;

    /* allocate memory */
    *dest = (CHAR_TYPE*)re_malloc(error * sizeof(CHAR_TYPE));
    if(!*dest)
    {
        re_report("out of memory allocating substitute destination");
        return REGEXP_ESPACE;
    }
    
    /* do actual substitution */
    if((error = re_dosub_w(s, source, matches, *dest)) < 0)
    {
        re_cfree(*dest);
        *dest = NULL;
        return error;
    }
    /* done */
    return error;
}
