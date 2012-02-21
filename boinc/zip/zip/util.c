/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  util.c by Mark Adler.
 */
#define __UTIL_C

#include "zip.h"
#include "ebcdic.h"
#include <ctype.h>

#ifdef MSDOS16
#  include <dos.h>
#endif

uch upper[256], lower[256];
/* Country-dependent case map table */


#ifndef UTIL /* UTIL picks out namecmp code (all utils) */

/* Local functions */
local int recmatch OF((ZCONST uch *, ZCONST uch *, int));
local int count_args OF((char *s));

#ifdef MSDOS16
  local unsigned ident OF((unsigned chr));
#endif

#ifdef NO_MKTIME
#include "mktime.c"
#endif

#ifndef HAVE_FSEEKABLE
int fseekable(fp)
FILE *fp;
{
    long x;

    return (fp == NULL || (fseek(fp, -1L, SEEK_CUR) == 0 &&
            (x = ftell(fp)) >= 0 &&
            fseek(fp,  1L, SEEK_CUR) == 0 &&
            ftell(fp) == x + 1));
}
#endif /* HAVE_FSEEKABLE */

char *isshexp(p)
char *p;                /* candidate sh expression */
/* If p is a sh expression, a pointer to the first special character is
   returned.  Otherwise, NULL is returned. */
{
  for (; *p; INCSTR(p))
    if (*p == '\\' && *(p+1))
      p++;
#ifdef VMS
    else if (*p == '%' || *p == '*')
#else /* !VMS */
# ifdef RISCOS
    /* RISC OS uses # as its single-character wildcard */
    else if (*p == '#' || *p == '*' || *p == '[')
# else /* !RISC OS */
    else if (*p == '?' || *p == '*' || *p == '[')
# endif
#endif /* ?VMS */
      return p;
  return NULL;
}


local int recmatch(p, s, cs)
ZCONST uch *p;  /* sh pattern to match */
ZCONST uch *s;  /* string to match it to */
int cs;         /* flag: force case-sensitive matching */
/* Recursively compare the sh pattern p with the string s and return 1 if
   they match, and 0 or 2 if they don't or if there is a syntax error in the
   pattern.  This routine recurses on itself no deeper than the number of
   characters in the pattern. */
{
  unsigned int c;       /* pattern char or start of range in [-] loop */
  /* Get first character, the pattern for new recmatch calls follows */
  c = *POSTINCSTR(p);

  /* If that was the end of the pattern, match if string empty too */
  if (c == 0)
    return *s == 0;

  /* '?' (or '%' or '#') matches any character (but not an empty string) */
#ifdef VMS
  if (c == '%')
#else /* !VMS */
# ifdef RISCOS
  if (c == '#')
# else /* !RISC OS */
  if (c == '?')
# endif
#endif /* ?VMS */
#ifdef WILD_STOP_AT_DIR
    return (*s && *s != '/') ? recmatch(p, s + CLEN(s), cs) : 0;
#else
    return *s ? recmatch(p, s + CLEN(s), cs) : 0;
#endif

  /* '*' matches any number of characters, including zero */
#ifdef AMIGA
  if (c == '#' && *p == '?')            /* "#?" is Amiga-ese for "*" */
    c = '*', p++;
#endif /* AMIGA */
  if (c == '*')
  {
    if (*p == 0)
      return 1;
#ifdef WILD_STOP_AT_DIR
    for (; *s && *s != '/'; INCSTR(s))
      if ((c = recmatch(p, s, cs)) != 0)
        return (int)c;
    return (*p == '/' || (*p == '\\' && p[1] == '/'))
      ? recmatch(p, s, cs) : 2;
#else /* !WILD_STOP_AT_DIR */
    for (; *s; INCSTR(s))
      if ((c = recmatch(p, s, cs)) != 0)
        return (int)c;
    return 2;           /* 2 means give up--shmatch will return false */
#endif /* ?WILD_STOP_AT_DIR */
  }

#ifndef VMS             /* No bracket matching in VMS */
  /* Parse and process the list of characters and ranges in brackets */
  if (c == '[')
  {
    int e;              /* flag true if next char to be taken literally */
    ZCONST uch *q;      /* pointer to end of [-] group */
    int r;              /* flag true to match anything but the range */

    if (*s == 0)                        /* need a character to match */
      return 0;
    p += (r = (*p == '!' || *p == '^')); /* see if reverse */
    for (q = p, e = 0; *q; q++)         /* find closing bracket */
      if (e)
        e = 0;
      else
        if (*q == '\\')
          e = 1;
        else if (*q == ']')
          break;
    if (*q != ']')                      /* nothing matches if bad syntax */
      return 0;
    for (c = 0, e = *p == '-'; p < q; p++)      /* go through the list */
    {
      if (e == 0 && *p == '\\')         /* set escape flag if \ */
        e = 1;
      else if (e == 0 && *p == '-')     /* set start of range if - */
        c = *(p-1);
      else
      {
        uch cc = (cs ? *s : case_map(*s));
        if (*(p+1) != '-')
          for (c = c ? c : (unsigned)*p; c <= (unsigned)*p; c++)
            /* compare range */
            if ((cs ? c : case_map(c)) == cc)
              return r ? 0 : recmatch(q + CLEN(q), s + CLEN(s), cs);
        c = e = 0;                      /* clear range, escape flags */
      }
    }
    return r ? recmatch(q + CLEN(q), s + CLEN(s), cs) : 0;
                                        /* bracket match failed */
  }
#endif /* !VMS */

  /* If escape ('\'), just compare next character */
  if (c == '\\')
    if ((c = *p++) == '\0')             /* if \ at end, then syntax error */
      return 0;

  /* Just a character--compare it */
  return (cs ? c == *s : case_map(c) == case_map(*s)) ?
          recmatch(p, s + CLEN(s), cs) : 0;
}


int shmatch(p, s, cs)
ZCONST char *p;         /* sh pattern to match */
ZCONST char *s;         /* string to match it to */
int cs;                 /* force case-sensitive match if TRUE */
/* Compare the sh pattern p with the string s and return true if they match,
   false if they don't or if there is a syntax error in the pattern. */
{
//printf("Match %s %s %d\n", p, s, cs);
  return recmatch((ZCONST uch *) p, (ZCONST uch *) s, cs) == 1;
}


#if defined(DOS) || defined(WIN32) || defined(UNIX)
/* XXX  also suitable for OS2?  Atari?  Human68K?  TOPS-20?? */

int dosmatch(p, s, cs)
ZCONST char *p;         /* dos pattern to match    */
ZCONST char *s;         /* string to match it to   */
int cs;                 /* force case-sensitive match if TRUE */
/* Treat filenames without periods as having an implicit trailing period */
{
  char *s1;             /* revised string to match */
  int r;                /* result */

  if ((s1 = malloc(strlen(s) + 2)) == NULL)
    /* will usually be OK */
    return recmatch((ZCONST uch *) p, (ZCONST uch *) s, cs) == 1;
  strcpy(s1, s);
  if (strchr(p, '.') && !strchr(s1, '.'))
    strcat(s1, ".");
  r = recmatch((ZCONST uch *)p, (ZCONST uch *)s1, cs);
  free((zvoid *)s1);
  return r == 1;
}

#endif /* DOS || WIN32 */

zvoid far **search(b, a, n, cmp)
ZCONST zvoid *b;        /* pointer to value to search for */
ZCONST zvoid far **a;   /* table of pointers to values, sorted */
extent n;               /* number of pointers in a[] */
int (*cmp) OF((ZCONST zvoid *, ZCONST zvoid far *)); /* comparison function */

/* Search for b in the pointer list a[0..n-1] using the compare function
   cmp(b, c) where c is an element of a[i] and cmp() returns negative if
   *b < *c, zero if *b == *c, or positive if *b > *c.  If *b is found,
   search returns a pointer to the entry in a[], else search() returns
   NULL.  The nature and size of *b and *c (they can be different) are
   left up to the cmp() function.  A binary search is used, and it is
   assumed that the list is sorted in ascending order. */
{
  ZCONST zvoid far **i; /* pointer to midpoint of current range */
  ZCONST zvoid far **l; /* pointer to lower end of current range */
  int r;                /* result of (*cmp)() call */
  ZCONST zvoid far **u; /* pointer to upper end of current range */

  l = (ZCONST zvoid far **)a;  u = l + (n-1);
  while (u >= l) {
    i = l + ((unsigned)(u - l) >> 1);
    if ((r = (*cmp)(b, (ZCONST char far *)*(struct zlist far **)i)) < 0)
      u = i - 1;
    else if (r > 0)
      l = i + 1;
    else
      return (zvoid far **)i;
  }
  return NULL;          /* If b were in list, it would belong at l */
}

#endif /* !UTIL */

#ifdef MSDOS16

local unsigned ident(unsigned chr)
{
   return chr; /* in al */
}

void init_upper()
{
  static struct country {
    uch ignore[18];
    int (far *casemap)(int);
    uch filler[16];
  } country_info;

  struct country far *info = &country_info;
  union REGS regs;
  struct SREGS sregs;
  unsigned int c;

  regs.x.ax = 0x3800; /* get country info */
  regs.x.dx = FP_OFF(info);
  sregs.ds  = FP_SEG(info);
  intdosx(&regs, &regs, &sregs);
  for (c = 0; c < 128; c++) {
    upper[c] = (uch) toupper(c);
    lower[c] = (uch) c;
  }
  for (; c < sizeof(upper); c++) {
    upper[c] = (uch) (*country_info.casemap)(ident(c));
    /* ident() required because casemap takes its parameter in al */
    lower[c] = (uch) c;
  }
  for (c = 0; c < sizeof(upper); c++ ) {
    int u = upper[c];
    if (u != c && lower[u] == (uch) u) {
      lower[u] = (uch)c;
    }
  }
  for (c = 'A'; c <= 'Z'; c++) {
    lower[c] = (uch) (c - 'A' + 'a');
  }
}
#else /* !MSDOS16 */
#  ifndef OS2

void init_upper()
{
  unsigned int c;
#if defined(ATARI) || defined(CMS_MVS)
#include <ctype.h>
/* this should be valid for all other platforms too.   (HD 11/11/95) */
  for (c = 0; c< sizeof(upper); c++) {
    upper[c] = islower(c) ? toupper(c) : c;
    lower[c] = isupper(c) ? tolower(c) : c;
  }
#else
  for (c = 0; c < sizeof(upper); c++) upper[c] = lower[c] = (uch)c;
  for (c = 'a'; c <= 'z';        c++) upper[c] = (uch)(c - 'a' + 'A');
  for (c = 'A'; c <= 'Z';        c++) lower[c] = (uch)(c - 'A' + 'a');
#endif
}
#  endif /* !OS2 */

#endif /* ?MSDOS16 */

int namecmp(string1, string2)
  ZCONST char *string1, *string2;
/* Compare the two strings ignoring case, and correctly taking into
 * account national language characters. For operating systems with
 * case sensitive file names, this function is equivalent to strcmp.
 */
{
  int d;

  for (;;)
  {
    d = (int) (uch) case_map(*string1)
      - (int) (uch) case_map(*string2);

    if (d || *string1 == 0 || *string2 == 0)
      return d;

    string1++;
    string2++;
  }
}

#ifdef EBCDIC
char *strtoasc(char *str1, ZCONST char *str2)
{
  char *old;
  old = str1;
  while (*str1++ = (char)ascii[(uch)(*str2++)]);
  return old;
}

char *strtoebc(char *str1, ZCONST char *str2)
{
  char *old;
  old = str1;
  while (*str1++ = (char)ebcdic[(uch)(*str2++)]);
  return old;
}

char *memtoasc(char *mem1, ZCONST char *mem2, unsigned len)
{
  char *old;
  old = mem1;
  while (len--)
     *mem1++ = (char)ascii[(uch)(*mem2++)];
  return old;
}

char *memtoebc(char *mem1, ZCONST char *mem2, unsigned len)
{
  char *old;
  old = mem1;
  while (len--)
     *mem1++ = (char)ebcdic[(uch)(*mem2++)];
  return old;
}
#endif /* EBCDIC */

#ifdef IZ_ISO2OEM_ARRAY
char *str_iso_to_oem(dst, src)
  ZCONST char *src;
  char *dst;
{
  char *dest_start = dst;
  while (*dst++ = (char)iso2oem[(uch)(*src++)]);
  return dest_start;
}
#endif

#ifdef IZ_OEM2ISO_ARRAY
char *str_oem_to_iso(dst, src)
  ZCONST char *src;
  char *dst;
{
  char *dest_start = dst;
  while (*dst++ = (char)oem2iso[(uch)(*src++)]);
  return dest_start;
}
#endif



/* DBCS support for Info-ZIP's zip  (mainly for japanese (-: )
 * by Yoshioka Tsuneo (QWF00133@nifty.ne.jp,tsuneo-y@is.aist-nara.ac.jp)
 * This code is public domain!   Date: 1998/12/20
 */
#ifdef _MBCS

char *___tmp_ptr;

int lastchar(ptr)
    ZCONST char *ptr;
{
    ZCONST char *oldptr = ptr;
    while(*ptr != '\0'){
        oldptr = ptr;
        INCSTR(ptr);
    }
    return (int)(unsigned)*oldptr;
}

unsigned char *zmbschr(str, c)
    ZCONST unsigned char *str;
    unsigned int c;
{
    while(*str != '\0'){
        if (*str == c) {return (char*)str;}
        INCSTR(str);
    }
    return NULL;
}

unsigned char *zmbsrchr(str, c)
    ZCONST unsigned char *str;
    unsigned int c;
{
    unsigned char *match = NULL;
    while(*str != '\0'){
        if (*str == c) {match = (char*)str;}
        INCSTR(str);
    }
    return match;
}
#endif /* _MBCS */



#ifndef UTIL

extern char *getenv();

/* this is in unzip/envargs

 ****************************************************************
 | envargs - add default options from environment to command line
 |----------------------------------------------------------------
 | Author: Bill Davidsen, original 10/13/91, revised 23 Oct 1991.
 | This program is in the public domain.
 |----------------------------------------------------------------
 | Minor program notes:
 |  1. Yes, the indirection is a tad complex
 |  2. Parenthesis were added where not needed in some cases
 |     to make the action of the code less obscure.
 ***************************************************************

void envargs(Pargc, Pargv, envstr, envstr2)
    int *Pargc;
    char ***Pargv;
    char *envstr;
    char *envstr2;
{
    char *envptr;                     
    char *bufptr;                    
    int argc;                        
    register int ch;                 
    char **argv;                     
    char **argvect;                  

    envptr = getenv(envstr);
    if (envptr != NULL)                               
        while (isspace((uch)*envptr))     
            envptr++;
    if (envptr == NULL || *envptr == '\0')
        if ((envptr = getenv(envstr2)) != NULL)               
            while (isspace((uch)*envptr))
                envptr++;
    if (envptr == NULL || *envptr == '\0')
        return;

    argc = count_args(envptr);
    bufptr = malloc(1 + strlen(envptr));
    if (bufptr == NULL)
        ziperr(ZE_MEM, "Can't get memory for arguments");
    strcpy(bufptr, envptr);

    argv = (char **)malloc((argc + *Pargc + 1) * sizeof(char *));
    if (argv == NULL) {
        free(bufptr);
        ziperr(ZE_MEM, "Can't get memory for arguments");
    }
    argvect = argv;

    *(argv++) = *((*Pargv)++);
    do {
#if defined(AMIGA) || defined(UNIX)
        if (*bufptr == '"') {
            char *argstart = ++bufptr;
            *(argv++) = argstart;
            for (ch = *bufptr; ch != '\0' && ch != '\"';
                 ch = *PREINCSTR(bufptr))
                if (ch == '\\' && bufptr[1] != '\0')
                    ++bufptr;              
            if (ch != '\0')                
                *(bufptr++) = '\0';

            while ((argstart = MBSCHR(argstart, '\\')) != NULL) {
                strcpy(argstart, argstart + 1);
                if (*argstart)
                    ++argstart;
            }
        } else {
            *(argv++) = bufptr;
            while ((ch = *bufptr) != '\0' && !isspace((uch)ch)) INCSTR(bufptr);
            if (ch != '\0') *(bufptr++) = '\0';
        }
#else
#  ifdef WIN32
        if (*bufptr == '"') {
            *(argv++) = ++bufptr;
            while ((ch = *bufptr) != '\0' && ch != '\"') INCSTR(bufptr);
            if (ch != '\0') *(bufptr++) = '\0';
        } else {
            *(argv++) = bufptr;
            while ((ch = *bufptr) != '\0' && !isspace((uch)ch)) INCSTR(bufptr);
            if (ch != '\0') *(bufptr++) = '\0';
        }
#  else
        *(argv++) = bufptr;
        while ((ch = *bufptr) != '\0' && !isspace((uch)ch)) INCSTR(bufptr);
        if (ch != '\0') *(bufptr++) = '\0';
#  endif
#endif 
        while ((ch = *bufptr) != '\0' && isspace((uch)ch)) INCSTR(bufptr);
    } while (ch);

    argc += *Pargc;
    while (--(*Pargc)) *(argv++) = *((*Pargv)++);

    *argv = NULL;

    *Pargv = argvect;
    *Pargc = argc;
}

*/

static int count_args(s)
char *s;
{
    int count = 0;
    char ch;

    do {
        /* count and skip args */
        ++count;
#if defined(AMIGA) || defined(UNIX)
        if (*s == '\"') {
            for (ch = *PREINCSTR(s); ch != '\0' && ch != '\"';
                 ch = *PREINCSTR(s))
                if (ch == '\\' && s[1] != '\0')
                    INCSTR(s);
            if (*s) INCSTR(s);  /* trailing quote */
        } else
            while ((ch = *s) != '\0' && !isspace((uch)ch)) INCSTR(s);
#else
#  ifdef WIN32
        if (*s == '\"') {
            ++s;                /* leading quote */
            while ((ch = *s) != '\0' && ch != '\"') INCSTR(s);
            if (*s) INCSTR(s);  /* trailing quote */
        } else
            while ((ch = *s) != '\0' && !isspace((uch)ch)) INCSTR(s);
#  else
        while ((ch = *s) != '\0' && !isspace((uch)ch)) INCSTR(s);
#  endif
#endif /* ?(AMIGA || UNIX) */
        while ((ch = *s) != '\0' && isspace((uch)ch)) INCSTR(s);
    } while (ch);

    return(count);
}



/* Extended argument processing -- by Rich Wales
 * This function currently deals only with the MKS shell, but could be
 * extended later to understand other conventions.
 *
 * void expand_args(int *argcp, char ***argvp)
 *
 *    Substitutes the extended command line argument list produced by
 *    the MKS Korn Shell in place of the command line info from DOS.
 *
 *    The MKS shell gets around DOS's 128-byte limit on the length of
 *    a command line by passing the "real" command line in the envi-
 *    ronment.  The "real" arguments are flagged by prepending a tilde
 *    (~) to each one.
 *
 *    This "expand_args" routine creates a new argument list by scanning
 *    the environment from the beginning, looking for strings begin-
 *    ning with a tilde character.  The new list replaces the original
 *    "argv" (pointed to by "argvp"), and the number of arguments
 *    in the new list replaces the original "argc" (pointed to by
 *    "argcp").
 */
void expand_args(argcp, argvp)
      int *argcp;
      char ***argvp;
{
#ifdef DOS

/* Do NEVER include (re)definiton of `environ' variable with any version
   of MSC or BORLAND/Turbo C. These compilers supply an incompatible
   definition in <stdlib.h>.  */
#if defined(__GO32__) || defined(__EMX__)
      extern char **environ;          /* environment */
#endif /* __GO32__ || __EMX__ */
      char        **envp;             /* pointer into environment */
      char        **newargv;          /* new argument list */
      char        **argp;             /* pointer into new arg list */
      int           newargc;          /* new argument count */

      /* sanity check */
      if (environ == NULL
          || argcp == NULL
          || argvp == NULL || *argvp == NULL)
              return;
      /* find out how many environment arguments there are */
      for (envp = environ, newargc = 0;
           *envp != NULL && (*envp)[0] == '~';
           envp++, newargc++) ;
      if (newargc == 0)
              return;                 /* no environment arguments */
      /* set up new argument list */
      newargv = (char **) malloc(sizeof(char **) * (newargc+1));
      if (newargv == NULL)
              return;                 /* malloc failed */
      for (argp = newargv, envp = environ;
           *envp != NULL && (*envp)[0] == '~';
           *argp++ = &(*envp++)[1]) ;
      *argp = NULL;                   /* null-terminate the list */
      /* substitute new argument list in place of old one */
      *argcp = newargc;
      *argvp = newargv;
#else /* !DOS */
      if (argcp || argvp) return;
#endif /* ?DOS */
}

#endif /* UTIL */

#ifdef DEBUGNAMES
#undef free
int Free(x)
void *x;
{
    if (x == (void *) 0xdeadbeef)
        exit(-1);
    free(x);
    return 0;
}

int printnames()
{
     struct zlist far *z;

     for (z = zfiles; z != NULL; z = z->nxt)
           fprintf(stderr, "%s %s %s %p %p %p %08x %08x %08x\n",
                            z->name, z->zname, z->iname,
                            z->name, z->zname, z->iname,
                            *((int *) z->name), *((int *) z->zname),
                            *((int *) z->iname));
     return 0;
}

#endif /* DEBUGNAMES */

const char *BOINC_RCSID_a327698ed4 = "$Id: util.c 4979 2005-01-02 18:29:53Z ballen $";
