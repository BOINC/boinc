/*
  Copyright (c) 1990-2001 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*---------------------------------------------------------------------------
    Unix specific configuration section:
  ---------------------------------------------------------------------------*/

#ifndef __unxcfg_h
#define __unxcfg_h

#include <sys/types.h>          /* off_t, time_t, dev_t, ... */
#include <sys/stat.h>

#ifndef COHERENT
#  include <fcntl.h>            /* O_BINARY for open() w/o CR/LF translation */
#else /* COHERENT */
#  ifdef _I386
#    include <fcntl.h>          /* Coherent 4.0.x, Mark Williams C */
#  else
#    include <sys/fcntl.h>      /* Coherent 3.10, Mark Williams C */
#  endif
#  define SHORT_SYMS
#  ifndef __COHERENT__          /* Coherent 4.2 has tzset() */
#    define tzset  settz
#  endif
#endif /* ?COHERENT */

#ifndef NO_PARAM_H
#  ifdef NGROUPS_MAX
#    undef NGROUPS_MAX      /* SCO bug:  defined again in <sys/param.h> */
#  endif
#  ifdef BSD
#    define TEMP_BSD        /* may be defined again in <sys/param.h> */
#    undef BSD
#  endif
#  include <sys/param.h>    /* conflict with <sys/types.h>, some systems? */
#  ifdef TEMP_BSD
#    undef TEMP_BSD
#    ifndef BSD
#      define BSD
#    endif
#  endif
#endif /* !NO_PARAM_H */

#ifdef __osf__
#  define DIRENT
#  ifdef BSD
#    undef BSD
#  endif
#endif /* __osf__ */

#ifdef BSD
#  include <sys/time.h>
#  include <sys/timeb.h>
#  if (defined(_AIX) || defined(__GNU__))
#    include <time.h>
#  endif
#else
#  include <time.h>
   struct tm *gmtime(), *localtime();
#endif

#if (defined(BSD4_4) || (defined(SYSV) && defined(MODERN)))
#  include <unistd.h>           /* this includes utime.h on SGIs */
#  if (defined(BSD4_4) || defined(linux) || defined(__GNU__))
#    include <utime.h>
#    define GOT_UTIMBUF
#  endif
#endif

#if (defined(V7) || defined(pyr_bsd))
#  define strchr   index
#  define strrchr  rindex
#endif
#ifdef V7
#  define O_RDONLY 0
#  define O_WRONLY 1
#  define O_RDWR   2
#endif

#ifdef MINIX
#  include <stdio.h>
#endif
#if (!defined(HAVE_STRNICMP) & !defined(NO_STRNICMP))
#  define NO_STRNICMP
#endif
#ifndef DATE_FORMAT
#  define DATE_FORMAT DF_MDY    /* GRR:  customize with locale.h somehow? */
#endif
#define lenEOL          1
#ifdef EBCDIC
#  define PutNativeEOL  *q++ = '\n';
#else
#  define PutNativeEOL  *q++ = native(LF);
#endif
#define SCREENSIZE(ttrows, ttcols)  screensize(ttrows, ttcols)
#define SCREENWIDTH     80
#define SCREENLWRAP     1
#define USE_EF_UT_TIME
#define SET_DIR_ATTRIB
#if (!defined(TIMESTAMP) && !defined(NOTIMESTAMP))   /* GRR 970513 */
#  define TIMESTAMP
#endif
#define RESTORE_UIDGID

/* Static variables that we have to add to Uz_Globs: */
#define SYSTEM_SPECIFIC_GLOBALS \
    int created_dir, renamed_fullpath;\
    char *rootpath, *buildpath, *end;\
    ZCONST char *wildname;\
    char *dirname, matchname[FILNAMSIZ];\
    int rootlen, have_dirname, dirnamelen, notfirstcall;\
    zvoid *wild_dir;

/* created_dir, and renamed_fullpath are used by both mapname() and    */
/*    checkdir().                                                      */
/* rootlen, rootpath, buildpath and end are used by checkdir().        */
/* wild_dir, dirname, wildname, matchname[], dirnamelen, have_dirname, */
/*    and notfirstcall are used by do_wild().                          */

#endif /* !__unxcfg_h */
