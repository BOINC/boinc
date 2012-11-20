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

#ifndef REGEXP_H_
#define REGEXP_H_

#include <stddef.h>

/*
 * Definitions etc. for regexp(3) routines.
 *
 * NOTE: this was completely redone from the old code to have a
 * saner interface (with pretty much the same functionality).
 */
#ifdef REGEXP_UNICODE
#include <wchar.h>
typedef wchar_t CHAR_TYPE;
#else
typedef char CHAR_TYPE;
#endif

typedef int regoffset;
typedef struct regmatch
{
    regoffset begin;
    regoffset end;
} regmatch;

struct tag_regexp;
typedef struct tag_regexp regexp;

/* error codes -- similar to POSIX codes */
#define REGEXP_BADARG  -1         /* bad argument--NULL pointer or such */
#define REGEXP_ESIZE   -2         /* regexp too big */
#define REGEXP_ESPACE  -3         /* out of memory */
#define REGEXP_EPAREN  -4         /* unmatched ) */
#define REGEXP_ERANGE  -5         /* invalid [] range */
#define REGEXP_EBRACK  -6         /* unclosed ] */
#define REGEXP_BADRPT  -7         /* *+? follows nothing */
#define REGEXP_EESCAPE -8         /* trailing backslash */
#define REGEXP_EEND   -99         /* unspecified internal error */

#ifdef __cplusplus
extern "C" 
{
#endif
    
extern void re_error(int errcode, const regexp* re, char *buffer, size_t bufsize);
extern int re_nsubexp(const regexp *rp);
extern void re_free(void* object);

/* Unicode-dependent */
extern int re_comp_w(regexp **rpp, const CHAR_TYPE* regex);
extern int re_exec_w(const regexp *rp, const CHAR_TYPE *s, size_t nmatch, regmatch pmatch[]);
extern int re_sub_w(const regexp *rp, const CHAR_TYPE* s, const CHAR_TYPE *src, CHAR_TYPE** sub);
extern int re_subcount_w(const regexp* rp, const CHAR_TYPE* s, const CHAR_TYPE* src, regmatch matches[10]);
extern int re_dosub_w(const CHAR_TYPE* s, const CHAR_TYPE* src, regmatch matches[10], CHAR_TYPE* dest);
    
#if defined(REGEXP_UNICODE)
extern int re_comp(regexp **rpp, const char *regex);
extern int re_exec(const regexp *rp, const char *s, size_t nmatch, regmatch pmatch[]);
extern int re_sub(const regexp* rp, const char* s, const char* src, char** sub);

#elif defined(__cplusplus)
/* forward inline */
inline int re_comp(regexp** rpp, const char *re)
{ return re_comp_w(rpp, re); }
    
inline int re_exec(const regexp* rp, const char *s, size_t nmatch, regmatch pmatch[])
{ return re_exec_w(rp, s, nmatch, pmatch); }

inline int re_sub(const regexp *rp, const char* s, const char* src, char** sub)
{ return re_sub_w(rp, s, src, sub); }
    
#else
    
#define re_comp re_comp_w
#define re_exec re_exec_w
#define re_sub  re_sub_w

#endif

#ifdef __cplusplus
}
#endif

#endif /* REGEXP_H_ */
