/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  fileio.c by Mark Adler
 */
#define __FILEIO_C
#include "zip.h"

#ifdef MACOS
#  include "helpers.h"
#endif

#include <time.h>
#include <sys/stat.h>

#ifdef NO_MKTIME
time_t mktime OF((struct tm *));
#endif

#ifdef OSF
#define EXDEV 18   /* avoid a bug in the DEC OSF/1 header files. */
#else
#include <errno.h>
#endif

#ifdef NO_ERRNO
extern int errno;
#endif

#if defined(VMS) || defined(TOPS20)
#  define PAD 5
#else
#  define PAD 0
#endif

#ifdef NO_RENAME
int rename OF((ZCONST char *, ZCONST char *));
#endif


#ifndef UTIL    /* the companion #endif is a bit of ways down ... */

/* Local functions */
local int fqcmp  OF((ZCONST zvoid *, ZCONST zvoid *));
local int fqcmpz OF((ZCONST zvoid *, ZCONST zvoid *));

/* Local module level variables. */
char *label = NULL;                /* global, but only used in `system'.c */
local struct stat zipstatb;
#if (!defined(MACOS) && !defined(WINDLL))
local int zipstate = -1;
#else
int zipstate;
#endif
/* -1 unknown, 0 old zip file exists, 1 new zip file */

char *getnam(n, fp)
char *n;                /* where to put name (must have >=FNMAX+1 bytes) */
FILE *fp;
/* Read a \n or \r delimited name from stdin into n, and return
   n.  If EOF, then return NULL.  Also, if the name read is too big, return
   NULL. */
{
  int c;                /* last character read */
  char *p;              /* pointer into name area */

  p = n;
  while ((c = getc(fp)) == '\n' || c == '\r')
    ;
  if (c == EOF)
    return NULL;
  do {
    if (p - n >= FNMAX)
      return NULL;
    *p++ = (char) c;
    c = getc(fp);
  } while (c != EOF && (c != '\n' && c != '\r'));
#ifdef WIN32
/*
 * WIN32 strips off trailing spaces and periods in filenames
 * XXX what about a filename that only consists of spaces ?
 *     Answer: on WIN32, a filename must contain at least one non-space char
 */
  while (p > n) {
    if ((c = p[-1]) != ' ' && c != '.')
      break;
    --p;
  }
#endif
  *p = 0;
  return n;
}

struct flist far *fexpel(f)
struct flist far *f;    /* entry to delete */
/* Delete the entry *f in the doubly-linked found list.  Return pointer to
   next entry to allow stepping through list. */
{
  struct flist far *t;  /* temporary variable */

  t = f->nxt;
  *(f->lst) = t;                        /* point last to next, */
  if (t != NULL)
    t->lst = f->lst;                    /* and next to last */
  if (f->name != NULL)                  /* free memory used */
    free((zvoid *)(f->name));
  if (f->zname != NULL)
    free((zvoid *)(f->zname));
  if (f->iname != NULL)
    free((zvoid *)(f->iname));
  farfree((zvoid far *)f);
  fcount--;                             /* decrement count */
  return t;                             /* return pointer to next */
}


local int fqcmp(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to found entries */
/* Used by qsort() to compare entries in the found list by name. */
{
  return strcmp((*(struct flist far **)a)->name,
                (*(struct flist far **)b)->name);
}


local int fqcmpz(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to found entries */
/* Used by qsort() to compare entries in the found list by iname. */
{
  return strcmp((*(struct flist far **)a)->iname,
                (*(struct flist far **)b)->iname);
}


char *last(p, c)
char *p;                /* sequence of path components */
int c;                  /* path components separator character */
/* Return a pointer to the start of the last path component. For a directory
 * name terminated by the character in c, the return value is an empty string.
 */
{
  char *t;              /* temporary variable */

  if ((t = strrchr(p, c)) != NULL)
    return t + 1;
  else
#ifndef AOS_VS
    return p;
#else
/* We want to allow finding of end of path in either AOS/VS-style pathnames
 * or Unix-style pathnames.  This presents a few little problems ...
 */
  {
    if (*p == '='  ||  *p == '^')      /* like ./ and ../ respectively */
      return p + 1;
    else
      return p;
  }
#endif
}


char *msname(n)
char *n;
/* Reduce all path components to MSDOS upper case 8.3 style names. */
{
  int c;                /* current character */
  int f;                /* characters in current component */
  char *p;              /* source pointer */
  char *q;              /* destination pointer */

  p = q = n;
  f = 0;
  while ((c = (unsigned char)*POSTINCSTR(p)) != 0)
    if (c == ' ' || c == ':' || c == '"' || c == '*' || c == '+' ||
        c == ',' || c == ';' || c == '<' || c == '=' || c == '>' ||
        c == '?' || c == '[' || c == ']' || c == '|')
      continue;                         /* char is discarded */
    else if (c == '/')
    {
      *POSTINCSTR(q) = (char)c;
      f = 0;                            /* new component */
    }
#ifdef __human68k__
    else if (ismbblead(c) && *p)
    {
      if (f == 7 || f == 11)
        f++;
      else if (*p && f < 12 && f != 8)
      {
        *q++ = c;
        *q++ = *p++;
        f += 2;
      }
    }
#endif /* __human68k__ */
    else if (c == '.')
    {
      if (f == 0)
        continue;                       /* leading dots are discarded */
      else if (f < 9)
      {
        *POSTINCSTR(q) = (char)c;
        f = 9;                          /* now in file type */
      }
      else
        f = 12;                         /* now just excess characters */
    }
    else
      if (f < 12 && f != 8)
      {
        f += CLEN(p);                   /* do until end of name or type */
        *POSTINCSTR(q) = (char)(to_up(c));
      }
  *q = 0;
  return n;
}

int check_dup()
/* Sort the found list and remove duplicates.
   Return an error code in the ZE_ class. */
{
  struct flist far *f;          /* steps through found linked list */
  extent j, k;                  /* indices for s */
  struct flist far **s;         /* sorted table */
  struct flist far **nodup;     /* sorted table without duplicates */

  /* sort found list, remove duplicates */
  if (fcount)
  {
    extent fl_size = fcount * sizeof(struct flist far *);
    if ((fl_size / sizeof(struct flist far *)) != fcount ||
        (s = (struct flist far **)malloc(fl_size)) == NULL)
      return ZE_MEM;
    for (j = 0, f = found; f != NULL; f = f->nxt)
      s[j++] = f;
    /* Check names as given (f->name) */
    qsort((char *)s, fcount, sizeof(struct flist far *), fqcmp);
    for (k = j = fcount - 1; j > 0; j--)
      if (strcmp(s[j - 1]->name, s[j]->name) == 0)
        /* remove duplicate entry from list */
        fexpel(s[j]);           /* fexpel() changes fcount */
      else
        /* copy valid entry into destination position */
        s[k--] = s[j];
    s[k] = s[0];                /* First entry is always valid */
    nodup = &s[k];              /* Valid entries are at end of array s */

    /* sort only valid items and check for unique internal names (f->iname) */
    qsort((char *)nodup, fcount, sizeof(struct flist far *), fqcmpz);
    for (j = 1; j < fcount; j++)
      if (strcmp(nodup[j - 1]->iname, nodup[j]->iname) == 0)
      {
        zipwarn("  first full name: ", nodup[j - 1]->name);
        zipwarn(" second full name: ", nodup[j]->name);
#ifdef EBCDIC
        strtoebc(nodup[j]->iname, nodup[j]->iname);
#endif
        zipwarn("name in zip file repeated: ", nodup[j]->iname);
#ifdef EBCDIC
        strtoasc(nodup[j]->iname, nodup[j]->iname);
#endif
        return ZE_PARMS;
      }
    free((zvoid *)s);
  }
  return ZE_OK;
}

int filter(name, casesensitive)
  char *name;
  int casesensitive;
  /* Scan the -i and -x lists for matches to the given name.
     Return true if the name must be included, false otherwise.
     Give precedence to -x over -i.
   */
{
   unsigned int n;
   int slashes;
   int include = icount ? 0 : 1;
   char *p, *q;

   if (pcount == 0) return 1;

   for (n = 0; n < pcount; n++) {
      if (!patterns[n].zname[0])        /* it can happen... */
          continue;
      p = name;
      if (patterns[n].select == 'R') {
         /* With -R patterns, if the pattern has N path components (that is, */
         /* N-1 slashes), then we test only the last N components of name.   */
         slashes = 0;
         for (q = patterns[n].zname; (q = MBSCHR(q, '/')) != NULL; INCSTR(q))
            slashes++;
         for (q = p; (q = MBSCHR(q, '/')) != NULL; INCSTR(q))
            slashes--;
         if (slashes < 0)
            for (q = p; (q = MBSCHR(q, '/')) != NULL; INCSTR(q))
               if (slashes++ == 0) {
                  p = q + CLEN(q);
                  break;
               }
      }
      if (MATCH(patterns[n].zname, p, casesensitive)) {
         if (patterns[n].select == 'x') return 0;
         include = 1;
      }
   }
   return include;
}

int newname(name, isdir, casesensitive)
char *name;             /* name to add (or exclude) */
int  isdir;             /* true for a directory */
int  casesensitive;     /* true for case-sensitive matching */
/* Add (or exclude) the name of an existing disk file.  Return an error
   code in the ZE_ class. */
{
  char *iname, *zname;  /* internal name, external version of iname */
  char *undosm;         /* zname version with "-j" and "-k" options disabled */
  struct flist far *f;  /* where in found, or new found entry */
  struct zlist far *z;  /* where in zfiles (if found) */
  int dosflag;

  /* Search for name in zip file.  If there, mark it, else add to
     list of new names to do (or remove from that list). */
  if ((iname = ex2in(name, isdir, &dosflag)) == NULL)
    return ZE_MEM;

  /* Discard directory names with zip -rj */
  if (*iname == '\0') {
#ifndef AMIGA
/* A null string is a legitimate external directory name in AmigaDOS; also,
 * a command like "zip -r zipfile FOO:" produces an empty internal name.
 */
# ifndef RISCOS
 /* If extensions needs to be swapped, we will have empty directory names
    instead of the original directory. For example, zipping 'c.', 'c.main'
    should zip only 'main.c' while 'c.' will be converted to '\0' by ex2in. */

    if (pathput && !recurse) error("empty name without -j or -r");

# endif /* !RISCOS */
#endif /* !AMIGA */
    free((zvoid *)iname);
    return ZE_OK;
  }
  undosm = NULL;
  if (dosflag || !pathput) {
    int save_dosify = dosify, save_pathput = pathput;
    dosify = 0;
    pathput = 1;
    /* zname is temporarly mis-used as "undosmode" iname pointer */
    if ((zname = ex2in(name, isdir, NULL)) != NULL) {
      undosm = in2ex(zname);
      free(zname);
    }
    dosify = save_dosify;
    pathput = save_pathput;
  }
  if ((zname = in2ex(iname)) == NULL)
    return ZE_MEM;
  if (undosm == NULL)
    undosm = zname;
  if ((z = zsearch(zname)) != NULL) {
    if (pcount && !filter(undosm, casesensitive)) {
      /* Do not clear z->mark if "exclude", because, when "dosify || !pathput"
       * is in effect, two files with different filter options may hit the
       * same z entry.
       */
      if (verbose)
        fprintf(mesg, "excluding %s\n", zname);
      free((zvoid *)iname);
      free((zvoid *)zname);
    } else {
      z->mark = 1;
      if ((z->name = malloc(strlen(name) + 1 + PAD)) == NULL) {
        if (undosm != zname)
          free((zvoid *)undosm);
        free((zvoid *)iname);
        free((zvoid *)zname);
        return ZE_MEM;
      }
      strcpy(z->name, name);
      z->dosflag = dosflag;

#ifdef FORCE_NEWNAME
      free((zvoid *)(z->iname));
      z->iname = iname;
#else
      /* Better keep the old name. Useful when updating on MSDOS a zip file
       * made on Unix.
       */
      free((zvoid *)iname);
      free((zvoid *)zname);
#endif /* ? FORCE_NEWNAME */
    }
    if (name == label) {
       label = z->name;
    }
  } else if (pcount == 0 || filter(undosm, casesensitive)) {

    /* Check that we are not adding the zip file to itself. This
     * catches cases like "zip -m foo ../dir/foo.zip".
     */
#ifndef CMS_MVS
/* Version of stat() for CMS/MVS isn't complete enough to see if       */
/* files match.  Just let ZIP.C compare the filenames.  That's good    */
/* enough for CMS anyway since there aren't paths to worry about.      */
    struct stat statb;

    if (zipstate == -1)
       zipstate = strcmp(zipfile, "-") != 0 &&
                   stat(zipfile, &zipstatb) == 0;

    if (zipstate == 1 && (statb = zipstatb, stat(name, &statb) == 0
      && zipstatb.st_mode  == statb.st_mode
#ifdef VMS
      && memcmp(zipstatb.st_ino, statb.st_ino, sizeof(statb.st_ino)) == 0
      && strcmp(zipstatb.st_dev, statb.st_dev) == 0
      && zipstatb.st_uid   == statb.st_uid
#else /* !VMS */
      && zipstatb.st_ino   == statb.st_ino
      && zipstatb.st_dev   == statb.st_dev
      && zipstatb.st_uid   == statb.st_uid
      && zipstatb.st_gid   == statb.st_gid
#endif /* ?VMS */
      && zipstatb.st_size  == statb.st_size
      && zipstatb.st_mtime == statb.st_mtime
      && zipstatb.st_ctime == statb.st_ctime)) {
      /* Don't compare a_time since we are reading the file */
         if (verbose)
           fprintf(mesg, "file matches zip file -- skipping\n");
         if (undosm != zname)
           free((zvoid *)zname);
         if (undosm != iname)
           free((zvoid *)undosm);
         free((zvoid *)iname);
         return ZE_OK;
    }
#endif  /* CMS_MVS */

    /* allocate space and add to list */
    if ((f = (struct flist far *)farmalloc(sizeof(struct flist))) == NULL ||
        fcount + 1 < fcount ||
        (f->name = malloc(strlen(name) + 1 + PAD)) == NULL)
    {
      if (f != NULL)
        farfree((zvoid far *)f);
      if (undosm != zname)
        free((zvoid *)undosm);
      free((zvoid *)iname);
      free((zvoid *)zname);
      return ZE_MEM;
    }
    strcpy(f->name, name);
    f->iname = iname;
    f->zname = zname;
    f->dosflag = dosflag;
    *fnxt = f;
    f->lst = fnxt;
    f->nxt = NULL;
    fnxt = &f->nxt;
    fcount++;
    if (name == label) {
      label = f->name;
    }
  }
  if (undosm != zname)
    free((zvoid *)undosm);
  return ZE_OK;
}


ulg dostime(y, n, d, h, m, s)
int y;                  /* year */
int n;                  /* month */
int d;                  /* day */
int h;                  /* hour */
int m;                  /* minute */
int s;                  /* second */
/* Convert the date y/n/d and time h:m:s to a four byte DOS date and
   time (date in high two bytes, time in low two bytes allowing magnitude
   comparison). */
{
  return y < 1980 ? DOSTIME_MINIMUM /* dostime(1980, 1, 1, 0, 0, 0) */ :
        (((ulg)y - 1980) << 25) | ((ulg)n << 21) | ((ulg)d << 16) |
        ((ulg)h << 11) | ((ulg)m << 5) | ((ulg)s >> 1);
}


ulg unix2dostime(t)
time_t *t;              /* unix time to convert */
/* Return the Unix time t in DOS format, rounded up to the next two
   second boundary. */
{
  time_t t_even;
  struct tm *s;         /* result of localtime() */

  t_even = (time_t)(((unsigned long)(*t) + 1) & (~1));
                                /* Round up to even seconds. */
  s = localtime(&t_even);       /* Use local time since MSDOS does. */
  if (s == (struct tm *)NULL) {
      /* time conversion error; use current time as emergency value
         (assuming that localtime() does at least accept this value!) */
      t_even = (time_t)(((unsigned long)time(NULL) + 1) & (~1));
      s = localtime(&t_even);
  }
  return dostime(s->tm_year + 1900, s->tm_mon + 1, s->tm_mday,
                 s->tm_hour, s->tm_min, s->tm_sec);
}

int issymlnk(a)
ulg a;                  /* Attributes returned by filetime() */
/* Return true if the attributes are those of a symbolic link */
{
#ifndef QDOS
#ifdef S_IFLNK
#ifdef __human68k__
  int *_dos_importlnenv(void);

  if (_dos_importlnenv() == NULL)
    return 0;
#endif
  return ((a >> 16) & S_IFMT) == S_IFLNK;
#else /* !S_IFLNK */
  return (int)a & 0;    /* avoid warning on unused parameter */
#endif /* ?S_IFLNK */
#else
  return 0;
#endif
}

#endif /* !UTIL */


#if (!defined(UTIL) && !defined(ZP_NEED_GEN_D2U_TIME))
   /* There is no need for dos2unixtime() in the ZipUtils' code. */
#  define ZP_NEED_GEN_D2U_TIME
#endif
#if ((defined(OS2) || defined(VMS)) && defined(ZP_NEED_GEN_D2U_TIME))
   /* OS/2 and VMS use a special solution to handle time-stams of files. */
#  undef ZP_NEED_GEN_D2U_TIME
#endif
#if (defined(W32_STATROOT_FIX) && !defined(ZP_NEED_GEN_D2U_TIME))
   /* The Win32 stat()-bandaid to fix stat'ing root directories needs
    * dos2unixtime() to calculate the time-stamps. */
#  define ZP_NEED_GEN_D2U_TIME
#endif

#ifdef ZP_NEED_GEN_D2U_TIME

time_t dos2unixtime(dostime)
ulg dostime;            /* DOS time to convert */
/* Return the Unix time_t value (GMT/UTC time) for the DOS format (local)
 * time dostime, where dostime is a four byte value (date in most significant
 * word, time in least significant word), see dostime() function.
 */
{
  struct tm *t;         /* argument for mktime() */
  ZCONST time_t clock = time(NULL);

  t = localtime(&clock);
  t->tm_isdst = -1;     /* let mktime() determine if DST is in effect */
  /* Convert DOS time to UNIX time_t format */
  t->tm_sec  = (((int)dostime) <<  1) & 0x3e;
  t->tm_min  = (((int)dostime) >>  5) & 0x3f;
  t->tm_hour = (((int)dostime) >> 11) & 0x1f;
  t->tm_mday = (int)(dostime >> 16) & 0x1f;
  t->tm_mon  = ((int)(dostime >> 21) & 0x0f) - 1;
  t->tm_year = ((int)(dostime >> 25) & 0x7f) + 80;

  return mktime(t);
}

#undef ZP_NEED_GEN_D2U_TIME
#endif /* ZP_NEED_GEN_D2U_TIME */


#ifndef MACOS
int destroy(f)
char *f;                /* file to delete */
/* Delete the file *f, returning non-zero on failure. */
{
  return unlink(f);
}


int replace(d, s)
char *d, *s;            /* destination and source file names */
/* Replace file *d by file *s, removing the old *s.  Return an error code
   in the ZE_ class. This function need not preserve the file attributes,
   this will be done by setfileattr() later.
 */
{
  struct stat t;        /* results of stat() */
#if defined(CMS_MVS)
  /* cmsmvs.h defines FOPW_TEMP as memory(hiperspace).  Since memory is
   * lost at end of run, always do copy instead of rename.
   */
  int copy = 1;
#else
  int copy = 0;
#endif
  int d_exists;

#if defined(VMS) || defined(CMS_MVS)
  /* stat() is broken on VMS remote files (accessed through Decnet).
   * This patch allows creation of remote zip files, but is not sufficient
   * to update them or compress remote files */
  unlink(d);
#else /* !(VMS || CMS_MVS) */
  d_exists = (LSTAT(d, &t) == 0);
  if (d_exists)
  {
    /*
     * respect existing soft and hard links!
     */
    if (t.st_nlink > 1
# ifdef S_IFLNK
        || (t.st_mode & S_IFMT) == S_IFLNK
# endif
        )
       copy = 1;
    else if (unlink(d))
       return ZE_CREAT;                 /* Can't erase zip file--give up */
  }
#endif /* ?(VMS || CMS_MVS) */
#ifndef CMS_MVS
  if (!copy) {
      if (rename(s, d)) {               /* Just move s on top of d */
          copy = 1;                     /* failed ? */
#if !defined(VMS) && !defined(ATARI) && !defined(AZTEC_C)
#if !defined(CMS_MVS) && !defined(RISCOS) && !defined(QDOS)
    /* For VMS, ATARI, AMIGA Aztec, VM_CMS, MVS, RISCOS,
       always assume that failure is EXDEV */
          if (errno != EXDEV
#  ifdef THEOS
           && errno != EEXIST
#  else
#    ifdef ENOTSAM
           && errno != ENOTSAM /* Used at least on Turbo C */
#    endif
#  endif
              ) return ZE_CREAT;
#endif /* !CMS_MVS && !RISCOS */
#endif /* !VMS && !ATARI && !AZTEC_C */
      }
  }
#endif /* !CMS_MVS */

  if (copy) {
    FILE *f, *g;        /* source and destination files */
    int r;              /* temporary variable */

#ifdef RISCOS
    if (SWI_OS_FSControl_26(s,d,0xA1)!=NULL) {
#endif

    if ((f = fopen(s, FOPR)) == NULL) {
      fprintf(stderr," replace: can't open %s\n", s);
      return ZE_TEMP;
    }
    if ((g = fopen(d, FOPW)) == NULL)
    {
      fclose(f);
      return ZE_CREAT;
    }
    r = fcopy(f, g, (ulg)-1L);
    fclose(f);
    if (fclose(g) || r != ZE_OK)
    {
      unlink(d);
      return r ? (r == ZE_TEMP ? ZE_WRITE : r) : ZE_WRITE;
    }
    unlink(s);
#ifdef RISCOS
    }
#endif
  }
  return ZE_OK;
}
#endif /* !MACOS */


int getfileattr(f)
char *f;                /* file path */
/* Return the file attributes for file f or 0 if failure */
{
#ifdef __human68k__
  struct _filbuf buf;

  return _dos_files(&buf, f, 0xff) < 0 ? 0x20 : buf.atr;
#else
  struct stat s;

  return SSTAT(f, &s) == 0 ? (int) s.st_mode : 0;
#endif
}


int setfileattr(f, a)
char *f;                /* file path */
int a;                  /* attributes returned by getfileattr() */
/* Give the file f the attributes a, return non-zero on failure */
{
#if defined(TOPS20) || defined (CMS_MVS)
  return 0;
#else
#ifdef __human68k__
  return _dos_chmod(f, a) < 0 ? -1 : 0;
#else
  return chmod(f, a);
#endif
#endif
}


char *tempname(zip)
char *zip;              /* path name of zip file to generate temp name for */

/* Return a temporary file name in its own malloc'ed space, using tempath. */
{
  char *t = zip;   /* malloc'ed space for name (use zip to avoid warning) */

#ifdef CMS_MVS
  if ((t = malloc(strlen(tempath)+L_tmpnam+2)) == NULL)
    return NULL;
#  ifdef VM_CMS
  tmpnam(t);
  /* Remove filemode and replace with tempath, if any. */
  /* Otherwise A-disk is used by default */
  *(strrchr(t, ' ')+1) = '\0';
  if (tempath!=NULL)
     strcat(t, tempath);
  return t;
#  else   /* !VM_CMS */
  /* For MVS */
  tmpnam(t);
  if (tempath != NULL)
  {
    int l1 = strlen(t);
    char *dot;
    if (*t == '\'' && *(t+l1-1) == '\'' && (dot = strchr(t, '.')))
    {
      /* MVS and not OE.  tmpnam() returns quoted string of 5 qualifiers.
       * First is HLQ, rest are timestamps.  User can only replace HLQ.
       */
      int l2 = strlen(tempath);
      if (strchr(tempath, '.') || l2 < 1 || l2 > 8)
        ziperr(ZE_PARMS, "On MVS and not OE, tempath (-b) can only be HLQ");
      memmove(t+1+l2, dot, l1+1-(dot-t));  /* shift dot ready for new hlq */
      memcpy(t+1, tempath, l2);            /* insert new hlq */
    }
    else
    {
      /* MVS and probably OE.  tmpnam() returns filename based on TMPDIR,
       * no point in even attempting to change it.  User should modify TMPDIR
       * instead.
       */
      zipwarn("MVS, assumed to be OE, change TMPDIR instead of option -b: ",
              tempath);
    }
  }
  return t;
#  endif  /* !VM_CMS */
#else /* !CMS_MVS */
#ifdef TANDEM
  char cur_subvol [FILENAME_MAX];
  char temp_subvol [FILENAME_MAX];
  char *zptr;
  char *ptr;
  char *cptr = &cur_subvol[0];
  char *tptr = &temp_subvol[0];
  short err;

  t = (char *)malloc(NAMELEN); /* malloc here as you cannot free */
                               /* tmpnam allocated storage later */

  zptr = strrchr(zip, TANDEM_DELIMITER);

  if (zptr != NULL) {
    /* ZIP file specifies a Subvol so make temp file there so it can just
       be renamed at end */

    *tptr = *cptr = '\0';
    strcat(cptr, getenv("DEFAULTS"));

    strncat(tptr, zip, _min(FILENAME_MAX, (zptr - zip)) ); /* temp subvol */
    strncat(t,zip, _min(NAMELEN, ((zptr - zip) + 1)) );    /* temp stem   */

    err = chvol(tptr);
    ptr = t + strlen(t);  /* point to end of stem */
    tmpnam(ptr);          /* Add filename part to temp subvol */
    err = chvol(cptr);
  }
  else
    t = tmpnam(t);

  return t;

#else /* !CMS_MVS && !TANDEM */
/*
 * Do something with TMPDIR, TMP, TEMP ????
 */
  if (tempath != NULL)
  {
    if ((t = malloc(strlen(tempath)+12)) == NULL)
      return NULL;
    strcpy(t, tempath);
#if (!defined(VMS) && !defined(TOPS20))
#  ifdef MSDOS
    {
      char c = (char)lastchar(t);
      if (c != '/' && c != ':' && c != '\\')
        strcat(t, "/");
    }
#  else
#    ifdef AMIGA
    {
      char c = (char)lastchar(t);
      if (c != '/' && c != ':')
        strcat(t, "/");
    }
#    else /* !AMIGA */
#      ifdef RISCOS
    if (lastchar(t) != '.')
      strcat(t, ".");
#      else /* !RISCOS */
#        ifdef QDOS
    if (lastchar(t) != '_')
      strcat(t, "_");
#        else
    if (lastchar(t) != '/')
      strcat(t, "/");
#        endif /* ?QDOS */
#      endif /* ?RISCOS */
#    endif  /* ?AMIGA */
#  endif /* ?MSDOS */
#endif /* !VMS && !TOPS20 */
  }
  else
  {
    if ((t = malloc(12)) == NULL)
      return NULL;
    *t = 0;
  }
#ifdef NO_MKTEMP
  {
    char *p = t + strlen(t);
    sprintf(p, "%08lx", (ulg)time(NULL));
    return t;
  }
#else
  strcat(t, "ziXXXXXX"); /* must use lowercase for Linux dos file system */
#ifdef _WIN32
  return mktemp(t);
#else
    int fd = mkstemp(t);
    if (fd) {
        close(fd);
        return t;
    } else {
        return NULL; 
    }
#endif /* MKSTEMP */
#endif /* NO_MKTEMP */
#endif /* TANDEM */
#endif /* CMS_MVS */
}


int fcopy(f, g, n)
FILE *f, *g;            /* source and destination files */
ulg n;                  /* number of bytes to copy or -1 for all */
/* Copy n bytes from file *f to file *g, or until EOF if n == -1.  Return
   an error code in the ZE_ class. */
{
  char *b;              /* malloc'ed buffer for copying */
  extent k;             /* result of fread() */
  ulg m;                /* bytes copied so far */

  if ((b = malloc(CBSZ)) == NULL)
    return ZE_MEM;
  m = 0;
  while (n == (ulg)(-1L) || m < n)
  {
    if ((k = fread(b, 1, n == (ulg)(-1) ?
                   CBSZ : (n - m < CBSZ ? (extent)(n - m) : CBSZ), f)) == 0)
    {
      if (ferror(f))
      {
        free((zvoid *)b);
        return ZE_READ;
      }
      else
        break;
    }
    if (fwrite(b, 1, k, g) != k)
    {
      free((zvoid *)b);
      fprintf(stderr," fcopy: write error\n");
      return ZE_TEMP;
    }
    m += k;
  }
  free((zvoid *)b);
  return ZE_OK;
}

#ifdef NO_RENAME
int rename(from, to)
ZCONST char *from;
ZCONST char *to;
{
    unlink(to);
    if (link(from, to) == -1)
        return -1;
    if (unlink(from) == -1)
        return -1;
    return 0;
}

#endif /* NO_RENAME */


#ifdef ZMEM

/************************/
/*  Function memset()   */
/************************/

/*
 * memset - for systems without it
 *  bill davidsen - March 1990
 */

char *
memset(buf, init, len)
register char *buf;     /* buffer loc */
register int init;      /* initializer */
register unsigned int len;   /* length of the buffer */
{
    char *start;

    start = buf;
    while (len--) *(buf++) = init;
    return(start);
}


/************************/
/*  Function memcpy()   */
/************************/

char *
memcpy(dst,src,len)             /* v2.0f */
register char *dst, *src;
register unsigned int len;
{
    char *start;

    start = dst;
    while (len--)
        *dst++ = *src++;
    return(start);
}


/************************/
/*  Function memcmp()   */
/************************/

int
memcmp(b1,b2,len)                     /* jpd@usl.edu -- 11/16/90 */
register char *b1, *b2;
register unsigned int len;
{

    if (len) do {       /* examine each byte (if any) */
      if (*b1++ != *b2++)
        return (*((uch *)b1-1) - *((uch *)b2-1));  /* exit when miscompare */
    } while (--len);

    return(0);          /* no miscompares, yield 0 result */
}

#endif  /* ZMEM */

const char *BOINC_RCSID_cd0cef87cc = "$Id: z_fileio.c 14563 2008-01-16 04:43:19Z davea $";
