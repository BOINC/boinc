/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  globals.c by Mark Adler
 */
#define __GLOBALS_C

#define GLOBALS         /* include definition of errors[] in zip.h */
#ifndef UTIL
#define UTIL            /* do not declare the read_buf variable */
#endif

#include "zip.h"


/* Handy place to build error messages */
char errbuf[FNMAX+81];

/* Argument processing globals */
int recurse = 0;        /* 1=recurse into directories encountered */
int dispose = 0;        /* 1=remove files after put in zip file */
int pathput = 1;        /* 1=store path with name */
#ifdef RISCOS
int scanimage = 1;      /* 1=scan through image files */
#endif
int method = BEST;      /* one of BEST, DEFLATE (only), or STORE (only) */
int dosify = 0;         /* 1=make new entries look like MSDOS */
int verbose = 0;        /* 1=report oddities in zip file structure */
int fix = 0;            /* 1=fix the zip file */
int adjust = 0;         /* 1=adjust offsets for sfx'd file (keep preamble) */
int level = 6;          /* 0=fastest compression, 9=best compression */
int translate_eol = 0;  /* Translate end-of-line LF -> CR LF */
#ifdef VMS
   int vmsver = 0;      /* 1=append VMS version number to file names */
   int vms_native = 0;  /* 1=store in VMS format */
#endif /* VMS */
#if defined(OS2) || defined(WIN32)
   int use_longname_ea = 0; /* 1=use the .LONGNAME EA as the file's name */
#endif
int hidden_files = 0;   /* process hidden and system files */
int volume_label = 0;   /* add volume label */
int dirnames = 1;       /* include directory entries by default */
int linkput = 0;        /* 1=store symbolic links as such */
int noisy = 1;          /* 0=quiet operation */
int extra_fields = 1;   /* 0=do not create extra fields */
#ifdef WIN32
    int use_privileges = 0; /* 1=use security privilege overrides */
#endif
#ifndef RISCOS
#ifndef QDOS
#ifndef TANDEM
char *special = ".Z:.zip:.zoo:.arc:.lzh:.arj"; /* List of special suffixes */
#else /* TANDEM */
char *special = " Z: zip: zoo: arc: lzh: arj"; /* List of special suffixes */
#endif
#else /* QDOS */
char *special = "_Z:_zip:_zoo:_arc:_lzh:_arj"; /* List of special suffixes */
#endif
#else /* RISCOS */
char *special = "DDC:D96:68E";
#endif /* ?RISCOS */
char *key = NULL;       /* Scramble password if scrambling */
char *tempath = NULL;   /* Path for temporary files */
FILE *mesg;             /* stdout by default, stderr for piping */

/* Zip file globals */
char *zipfile;          /* New or existing zip archive (zip file) */
ulg zipbeg;             /* Starting offset of zip structures */
ulg cenbeg;             /* Starting offset of central directory */
struct zlist far *zfiles = NULL;  /* Pointer to list of files in zip file */
extent zcount;          /* Number of files in zip file */
extent zcomlen;         /* Length of zip file comment */
char *zcomment = NULL;  /* Zip file comment (not zero-terminated) */
struct zlist far **zsort;       /* List of files sorted by name */
ulg tempzn;             /* Count of bytes written to output zip file */

/* Files to operate on that are not in zip file */
struct flist far *found = NULL; /* List of names found */
struct flist far * far *fnxt = &found;
                        /* Where to put next name in found list */
extent fcount;          /* Count of files in list */

/* Patterns to be matched */
struct plist *patterns = NULL;  /* List of patterns to be matched */
unsigned pcount = 0;            /* number of patterns */
unsigned icount = 0;            /* number of include only patterns */

#ifdef IZ_CHECK_TZ
int zp_tz_is_valid;     /* signals "timezone info is available" */
#endif
