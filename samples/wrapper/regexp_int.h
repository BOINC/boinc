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

/* Internal regexp header */
#include "regexp.h"

#ifdef REGEXP_UNICODE
#define LIT(a) L##a
typedef unsigned short UCHAR_TYPE;
#else
#define LIT(a) a
typedef unsigned char UCHAR_TYPE;
#endif

/* NOTE: this structure is completely opaque. */
struct tag_regexp {
    int regnsubexp;			/* Internal use only. */
	CHAR_TYPE regstart;			/* Internal use only. */
	CHAR_TYPE reganch;			/* Internal use only. */
	CHAR_TYPE *regmust;			/* Internal use only. */
	int regmlen;			/* Internal use only. */
	CHAR_TYPE program[1];		/* Unwarranted chumminess with compiler. */
};


#ifdef REGEXP_UNICODE
#include <wchar.h>
#include <wctype.h>
#define cstrlen wcslen
#define cstrcspn wcscspn
#define cstrstr wcsstr
#define cstrchr wcschr
#define cstrncpy wcsncpy
#define cstrncmp wcsncmp
#define cstrspn wcsspn
#define cisalnum iswalnum
#define cisalpha iswalpha
#define cisblank iswblank
#define ciscntrl iswcntrl
#define cisdigit iswdigit
#define cisgraph iswgraph
#define cislower iswlower
#define cisprint iswprint
#define cispunct iswpunct
#define cisspace iswspace
#define cisupper iswupper
#define cisxdigit iswxdigit
#define ctolower towlower
#else
#include <ctype.h>
#define cstrlen strlen
#define cstrcspn strcspn
#define cstrstr strstr
#define cstrchr strchr
#define cstrncpy strncpy
#define cstrncmp strncmp
#define cstrspn strspn
#define cisalnum isalnum
#define cisalpha isalpha
#define cisblank isblank
#define ciscntrl iscntrl
#define cisdigit isdigit
#define cisgraph isgraph
#define cislower islower
#define cisprint isprint
#define cispunct ispunct
#define cisspace isspace
#define cisupper isupper
#define cisxdigit isxdigit
#define ctolower tolower
#endif

#define REGEXP_MAXEXP 0x7fff   /* max number of subexpressions */
