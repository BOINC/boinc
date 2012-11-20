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

/* invisible customization header for regexp implementation */

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* undefine this to remove some extra checks */
#define REGEXP_EXTRA_CHECKS
    
/* internal error handler -- you need to provide an implementation.
 * By default, this does nothing, but you may want to send stuff to
 * the debug log or something. */
void re_report(const char* error);

/* memory allocation routines. Those are defaulted to
 * the C heap management routines. */
void* re_malloc(size_t sz);
void  re_cfree(void* p);

/* unicode handlers for translating stuff.  Provide a sane implementation
 * for your character set.  A default implementation is provided and
 * it kind of sucks.
 *
 * If REGEXP_UNICODE is not defined, those functions won't be used. */
#ifdef REGEXP_UNICODE
CHAR_TYPE* re_ansi_to_unicode(const char* s);
char* re_unicode_to_ansi(const CHAR_TYPE* s);
#endif

#ifdef __cplusplus
}
#endif
