/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  zip.c by Mark Adler.
 */
#define __ZIP_C

#include "zip.h"
#include <time.h>       /* for tzset() declaration */

#include <setjmp.h>

#ifdef WINDLL
#  include <windows.h>
//#  include <setjmp.h>
#  include "windll/windll.h"
#endif
#define DEFCPYRT        /* main module: enable copyright string defines! */
#include "revision.h"
//#include "z_crypt.h"
#include "z_ttyio.h"
#ifdef VMS
#  include "vms/vmsmunch.h"
#endif

#ifdef MACOS
#  include "macglob.h"
   extern MacZipGlobals MacZip;
   extern int error_level;
#endif

#if (defined(MSDOS) && !defined(__GO32__)) || defined(__human68k__)
#  include <process.h>
#  if (!defined(P_WAIT) && defined(_P_WAIT))
#    define P_WAIT _P_WAIT
#  endif
#endif


#define MAXCOM 256      /* Maximum one-line comment size */


/* Local option flags */
#ifndef DELETE
#define DELETE  0
#endif
#define ADD     1
#define UPDATE  2
#define FRESHEN 3
local int action = ADD; /* one of ADD, UPDATE, FRESHEN, or DELETE */
local int comadd = 0;   /* 1=add comments for new files */
local int zipedit = 0;  /* 1=edit zip comment and all file comments */
local int latest = 0;   /* 1=set zip file time to time of latest file */
local ulg before = 0;   /* 0=ignore, else exclude files before this time */
local ulg after = 0;    /* 0=ignore, else exclude files newer than this time */
local int test = 0;     /* 1=test zip file with unzip -t */
local int tempdir = 0;  /* 1=use temp directory (-b) */
local int junk_sfx = 0; /* 1=junk the sfx prefix */
#if defined(AMIGA) || defined(MACOS)
local int filenotes = 0; /* 1=take comments from AmigaDOS/MACOS filenotes */
#endif

#ifdef EBCDIC
int aflag = __EBCDIC;   /* Convert EBCDIC to ASCII or stay EBCDIC ? */
#endif
#ifdef CMS_MVS
int bflag = 0;          /* Use text mode as default */
#endif

#ifdef QDOS
char _version[] = VERSION;
#endif

//#ifdef WINDLL
jmp_buf zipdll_error_return;
//#endif

/* Temporary zip file name and file pointer */
#ifndef MACOS
local char *tempzip;
local FILE *tempzf;
#else
char *tempzip;
FILE *tempzf;
#endif

#if CRYPT
/* Pointer to crc_table, needed in crypt.c */
# if (!defined(USE_ZLIB) || defined(USE_OWN_CRCTAB))
ZCONST ulg near *crc_32_tab;
# else
ZCONST uLongf *crc_32_tab;
# endif
#endif /* CRYPT */

/* Local functions */

local void freeup  OF((void));
local int  finish  OF((int));
#if (!defined(MACOS) && !defined(WINDLL))
local void handler OF((int));
local void license OF((void));
#ifndef VMSCLI
local void help    OF((void));
#endif /* !VMSCLI */
#endif /* !MACOS && !WINDLL */
local int get_filters OF((int argc, char **argv));
#if (!defined(MACOS) && !defined(WINDLL))
local void check_zipfile OF((char *zipname, char *zippath));
local void version_info OF((void));
local void zipstdout OF((void));
#endif /* !MACOS && !WINDLL */

local void freeup()
/* Free all allocations in the 'found' list, the 'zfiles' list and
   the 'patterns' list. */
{
  struct flist far *f;  /* steps through found list */
  struct zlist far *z;  /* pointer to next entry in zfiles list */

  for (f = found; f != NULL; f = fexpel(f))
    ;
  while (zfiles != NULL)
  {
    z = zfiles->nxt;
    if (zfiles->zname && zfiles->zname != zfiles->name)
      free((zvoid *)(zfiles->zname));
    if (zfiles->name)
      free((zvoid *)(zfiles->name));
    if (zfiles->iname)
      free((zvoid *)(zfiles->iname));
    if (zfiles->cext && zfiles->cextra && zfiles->cextra != zfiles->extra)
      free((zvoid *)(zfiles->cextra));
    if (zfiles->ext && zfiles->extra)
      free((zvoid *)(zfiles->extra));
    if (zfiles->com && zfiles->comment)
      free((zvoid *)(zfiles->comment));
    farfree((zvoid far *)zfiles);
    zfiles = z;
    zcount--;
  }

  if (patterns != NULL) {
    while (pcount-- > 0) {
      if (patterns[pcount].zname != NULL)
        free((zvoid *)(patterns[pcount].zname));
    }
    free((zvoid *)patterns);
    patterns = NULL;
  }
}

local int finish(e)
int e;                  /* exit code */
/* Process -o and -m options (if specified), free up malloc'ed stuff, and
   exit with the code e. */
{
  int r;                /* return value from trash() */
  ulg t;                /* latest time in zip file */
  struct zlist far *z;  /* pointer into zfile list */

  /* If latest, set time to zip file to latest file in zip file */
  if (latest && zipfile && strcmp(zipfile, "-"))
  {
    diag("changing time of zip file to time of latest file in it");
    /* find latest time in zip file */
    if (zfiles == NULL)
       zipwarn("zip file is empty, can't make it as old as latest entry", "");
    else {
      t = 0;
      for (z = zfiles; z != NULL; z = z->nxt)
        /* Ignore directories in time comparisons */
#ifdef USE_EF_UT_TIME
        if (z->iname[z->nam-1] != (char)0x2f)   /* ascii '/' */
        {
          iztimes z_utim;
          ulg z_tim;

          z_tim = ((get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
                   unix2dostime(&z_utim.mtime) : z->tim);
          if (t < z_tim)
            t = z_tim;
        }
#else /* !USE_EF_UT_TIME */
        if (z->iname[z->nam-1] != (char)0x2f    /* ascii '/' */
            && t < z->tim)
          t = z->tim;
#endif /* ?USE_EF_UT_TIME */
      /* set modified time of zip file to that time */
      if (t != 0)
        stamp(zipfile, t);
      else
        zipwarn(
         "zip file has only directories, can't make it as old as latest entry",
         "");
    }
  }
  if (tempath != NULL)
  {
    free((zvoid *)tempath);
    tempath = NULL;
  }
  if (zipfile != NULL)
  {
    free((zvoid *)zipfile);
    zipfile = NULL;
  }
  if (zcomment != NULL)
  {
    free((zvoid *)zcomment);
    zcomment = NULL;
  }


  /* If dispose, delete all files in the zfiles list that are marked */
  if (dispose)
  {
    diag("deleting files that were added to zip file");
    if ((r = trash()) != ZE_OK)
      ZIPERR(r, "was deleting moved files and directories");
  }


  /* Done! */
  freeup();
  return e;
}

int ziperr(c, h)
int c;                  /* error code from the ZE_ class */
char *h;                /* message about how it happened */
/* Issue a message for the error, clean up files and memory, and exit. */
{
#ifndef WINDLL
#ifndef MACOS
  static int error_level = 0;
#endif

  if (error_level++ > 0)
     return 0;  /* avoid recursive ziperr() */
#endif /* !WINDLL */

  if (h != NULL) {
    if (PERR(c))
      perror("zip I/O error");
    fflush(mesg);
    fprintf(stderr, "\nzip error: %s (%s)\n", errors[c-1], h);
  }
  if (tempzip != NULL)
  {
    if (tempzip != zipfile) {
      if (tempzf != NULL)
        fclose(tempzf);
#ifndef DEBUG
      destroy(tempzip);
#endif
      free((zvoid *)tempzip);
    } else {
      /* -g option, attempt to restore the old file */
      int k = 0;                        /* keep count for end header */
      ulg cb = cenbeg;                  /* get start of central */
      struct zlist far *z;  /* steps through zfiles linked list */

      fprintf(stderr, "attempting to restore %s to its previous state\n",
         zipfile);
      fseek(tempzf, cenbeg, SEEK_SET);
      tempzn = cenbeg;
      for (z = zfiles; z != NULL; z = z->nxt)
      {
        putcentral(z, tempzf);
        tempzn += 4 + CENHEAD + z->nam + z->cext + z->com;
        k++;
      }
      putend(k, tempzn - cb, cb, zcomlen, zcomment, tempzf);
      fclose(tempzf);
      tempzf = NULL;
    }
  }
  if (key != NULL) {
    free((zvoid *)key);
    key = NULL;
  }
  if (tempath != NULL) {
    free((zvoid *)tempath);
    tempath = NULL;
  }
  if (zipfile != NULL) {
    free((zvoid *)zipfile);
    zipfile = NULL;
  }
  if (zcomment != NULL) {
    free((zvoid *)zcomment);
    zcomment = NULL;
  }
  freeup();
//#ifndef WINDLL
//  EXIT(c);
//#else
   //return c;  /* avoid recursive ziperr() */
  longjmp(zipdll_error_return, c);
//#endif
}


void error(h)
  char *h;
/* Internal error, should never happen */
{
  ziperr(ZE_LOGIC, h);
}

#if (!defined(MACOS) && !defined(WINDLL))
local void handler(s)
int s;                  /* signal number (ignored) */
/* Upon getting a user interrupt, turn echo back on for tty and abort
   cleanly using ziperr(). */
{
#if defined(AMIGA) && defined(__SASC)
   _abort();
#else
#if !defined(MSDOS) && !defined(__human68k__) && !defined(RISCOS)
  echon();
  putc('\n', stderr);
#endif /* !MSDOS */
#endif /* AMIGA && __SASC */
  ziperr(ZE_ABORT, "aborting");
  s++;                                  /* keep some compilers happy */
}
#endif /* !MACOS && !WINDLL */

void zipwarn(a, b)
char *a, *b;            /* message strings juxtaposed in output */
/* Print a warning message to stderr and return. */
{
  if (noisy) fprintf(stderr, "\tzip warning: %s%s\n", a, b);
}

#ifndef WINDLL
local void license()
/* Print license information to stdout. */
{
  extent i;             /* counter for copyright array */

#if 0
  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++) {
    printf(copyright[i], "zip");
    putchar('\n');
  }
#endif
  for (i = 0; i < sizeof(swlicense)/sizeof(char *); i++)
    puts(swlicense[i]);
}

#ifdef VMSCLI
void help()
#else
local void help()
#endif
/* Print help (along with license info) to stdout. */
{
  extent i;             /* counter for help array */

  /* help array */
  static ZCONST char *text[] = {
#ifdef VMS
"Zip %s (%s). Usage: zip==\"$disk:[dir]zip.exe\"",
#else
"Zip %s (%s). Usage:",
#endif
#ifdef MACOS
"zip [-options] [-b fm] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]",
"  The default action is to add or replace zipfile entries from list.",
" ",
"  -f   freshen: only changed files  -u   update: only changed or new files",
"  -d   delete entries in zipfile    -m   move into zipfile (delete files)",
"  -r   recurse into directories     -j   junk (don't record) directory names",
"  -0   store only                   -l   convert LF to CR LF (-ll CR LF to LF)",
"  -1   compress faster              -9   compress better",
"  -q   quiet operation              -v   verbose operation/print version info",
"  -c   add one-line comments        -z   add zipfile comment",
"                                    -o   make zipfile as old as latest entry",
"  -F   fix zipfile (-FF try harder) -D   do not add directory entries",
"  -T   test zipfile integrity       -X   eXclude eXtra file attributes",
#  if CRYPT
"  -e   encrypt                      -n   don't compress these suffixes"
#  else
"  -h   show this help               -n   don't compress these suffixes"
#  endif
," ",
"  Macintosh specific:",
"  -jj  record Fullpath (+ Volname)  -N store finder-comments as comments",
"  -df  zip only datafork of a file  -S include finder invisible/system files"
#else /* !MACOS */
#ifdef VM_CMS
"zip [-options] [-b fm] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]",
#else  /* !VM_CMS */
"zip [-options] [-b path] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]",
#endif /* ?VM_CMS */
"  The default action is to add or replace zipfile entries from list, which",
"  can include the special name - to compress standard input.",
"  If zipfile and list are omitted, zip compresses stdin to stdout.",
"  -f   freshen: only changed files  -u   update: only changed or new files",
"  -d   delete entries in zipfile    -m   move into zipfile (delete files)",
"  -r   recurse into directories     -j   junk (don't record) directory names",
#ifdef THEOS
"  -0   store only                   -l   convert CR to CR LF (-ll CR LF to CR)",
#else
"  -0   store only                   -l   convert LF to CR LF (-ll CR LF to LF)",
#endif
"  -1   compress faster              -9   compress better",
"  -q   quiet operation              -v   verbose operation/print version info",
"  -c   add one-line comments        -z   add zipfile comment",
"  -@   read names from stdin        -o   make zipfile as old as latest entry",
"  -x   exclude the following names  -i   include only the following names",
#ifdef EBCDIC
#ifdef CMS_MVS
"  -a   translate to ASCII           -B   force binary read (text is default)",
#else  /* !CMS_MVS */
"  -a   translate to ASCII",
#endif /* ?CMS_MVS */
#endif /* EBCDIC */
#ifdef TANDEM
"                                    -Bn  set Enscribe formatting options",
#endif
#ifdef VMS
" \"-F\"  fix zipfile(\"-FF\" try harder) \"-D\"  do not add directory entries",
" \"-A\"  adjust self-extracting exe  \"-J\"  junk zipfile prefix (unzipsfx)",
" \"-T\"  test zipfile integrity      \"-X\"  eXclude eXtra file attributes",
" \"-V\"  save VMS file attributes     -w   append version number to stored name",
#else /* !VMS */
"  -F   fix zipfile (-FF try harder) -D   do not add directory entries",
"  -A   adjust self-extracting exe   -J   junk zipfile prefix (unzipsfx)",
"  -T   test zipfile integrity       -X   eXclude eXtra file attributes",
#endif /* ?VMS */
#ifdef WIN32
"  -!   use privileges (if granted) to obtain all aspects of WinNT security",
#endif /* WIN32 */
#ifdef OS2
"  -E   use the .LONGNAME Extended attribute (if found) as filename",
#endif /* OS2 */
#ifdef S_IFLNK
"  -y   store symbolic links as the link instead of the referenced file",
#endif /* !S_IFLNK */
"  -R   PKZIP recursion (see manual)",
#if defined(MSDOS) || defined(OS2)
"  -$   include volume label         -S   include system and hidden files",
#endif
#ifdef AMIGA
#  if CRYPT
"  -N   store filenotes as comments  -e   encrypt",
"  -h   show this help               -n   don't compress these suffixes"
#  else
"  -N   store filenotes as comments  -n   don't compress these suffixes"
#  endif
#else /* !AMIGA */
#  if CRYPT
"  -e   encrypt                      -n   don't compress these suffixes"
#  else
"  -h   show this help               -n   don't compress these suffixes"
#  endif
#endif /* ?AMIGA */
#ifdef RISCOS
,"  -I   don't scan through Image files"
#endif
#endif /* ?MACOS */
  };

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++)
  {
    printf(copyright[i], "zip");
    putchar('\n');
  }
  for (i = 0; i < sizeof(text)/sizeof(char *); i++)
  {
    printf(text[i], VERSION, REVDATE);
    putchar('\n');
  }
}

/*
 * XXX version_info() in a separate file
 */
local void version_info()
/* Print verbose info about program version and compile time options
   to stdout. */
{
  extent i;             /* counter in text arrays */
  char *envptr;

  /* Options info array */
  static ZCONST char *comp_opts[] = {
#ifdef ASM_CRC
    "ASM_CRC",
#endif
#ifdef ASMV
    "ASMV",
#endif
#ifdef DYN_ALLOC
    "DYN_ALLOC",
#endif
#ifdef MMAP
    "MMAP",
#endif
#ifdef BIG_MEM
    "BIG_MEM",
#endif
#ifdef MEDIUM_MEM
    "MEDIUM_MEM",
#endif
#ifdef SMALL_MEM
    "SMALL_MEM",
#endif
#ifdef DEBUG
    "DEBUG",
#endif
#ifdef USE_EF_UT_TIME
    "USE_EF_UT_TIME",
#endif
#ifdef NTSD_EAS
    "NTSD_EAS",
#endif
#ifdef VMS
#ifdef VMSCLI
    "VMSCLI",
#endif
#ifdef VMS_IM_EXTRA
    "VMS_IM_EXTRA",
#endif
#ifdef VMS_PK_EXTRA
    "VMS_PK_EXTRA",
#endif
#endif /* VMS */
#ifdef WILD_STOP_AT_DIR
    "WILD_STOP_AT_DIR",
#endif
#ifdef USE_ZLIB
    "USE_ZLIB",
#endif
#if CRYPT && defined(PASSWD_FROM_STDIN)
    "PASSWD_FROM_STDIN",
#endif /* CRYPT & PASSWD_FROM_STDIN */
    NULL
  };

  static ZCONST char *zipenv_names[] = {
#ifndef VMS
#  ifndef RISCOS
    "ZIP"
#  else /* RISCOS */
    "Zip$Options"
#  endif /* ?RISCOS */
#else /* VMS */
    "ZIP_OPTS"
#endif /* ?VMS */
    ,"ZIPOPT"
#ifdef AZTEC_C
    ,     /* extremely lame compiler bug workaround */
#endif
#ifndef __RSXNT__
# ifdef __EMX__
    ,"EMX"
    ,"EMXOPT"
# endif
# if (defined(__GO32__) && (!defined(__DJGPP__) || __DJGPP__ < 2))
    ,"GO32"
    ,"GO32TMP"
# endif
# if (defined(__DJGPP__) && __DJGPP__ >= 2)
    ,"TMPDIR"
# endif
#endif /* !__RSXNT__ */
#ifdef RISCOS
    ,"Zip$Exts"
#endif
  };

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++)
  {
    printf(copyright[i], "zip");
    putchar('\n');
  }

  for (i = 0; i < sizeof(versinfolines)/sizeof(char *); i++)
  {
    printf(versinfolines[i], "Zip", VERSION, REVDATE);
    putchar('\n');
  }

  version_local();

  puts("Zip special compilation options:");
#if WSIZE != 0x8000
  printf("\tWSIZE=%u\n", WSIZE);
#endif
  for (i = 0; (int)i < (int)(sizeof(comp_opts)/sizeof(char *) - 1); i++)
  {
    printf("\t%s\n",comp_opts[i]);
  }
#if CRYPT
  printf("\t[encryption, version %d.%d%s of %s]\n",
            CR_MAJORVER, CR_MINORVER, CR_BETA_VER, CR_VERSION_DATE);
  ++i;
#endif /* CRYPT */
  if (i == 0)
      puts("\t[none]");

  puts("\nZip environment options:");
  for (i = 0; i < sizeof(zipenv_names)/sizeof(char *); i++)
  {
    envptr = getenv(zipenv_names[i]);
    printf("%16s:  %s\n", zipenv_names[i],
           ((envptr == (char *)NULL || *envptr == 0) ? "[none]" : envptr));
  }
}
#endif /* !WINDLL */


#ifndef PROCNAME
#  define PROCNAME(n) procname(n, (action == DELETE || action == FRESHEN))
#endif /* PROCNAME */

#ifndef WINDLL
#ifndef MACOS
local void zipstdout()
/* setup for writing zip file on stdout */
{
  int r;
  mesg = stderr;
  if (isatty(1))
    ziperr(ZE_PARMS, "cannot write zip file to terminal");
  if ((zipfile = malloc(4)) == NULL)
    ziperr(ZE_MEM, "was processing arguments");
  strcpy(zipfile, "-");
  if ((r = readzipfile()) != ZE_OK)
    ziperr(r, zipfile);
}
#endif /* !MACOS */

local void check_zipfile(zipname, zippath)
  char *zipname;
  char *zippath;
  /* Invoke unzip -t on the given zip file */
{
#if (defined(MSDOS) && !defined(__GO32__)) || defined(__human68k__)
   int status, len;
   char *path, *p;

   status = spawnlp(P_WAIT, "unzip", "unzip", verbose ? "-t" : "-tqq",
                    zipname, NULL);
#ifdef __human68k__
   if (status == -1)
     perror("unzip");
#else
/*
 * unzip isn't in PATH range, assume an absolute path to zip in argv[0]
 * and hope that unzip is in the same directory.
 */
   if (status == -1) {
     p = MBSRCHR(zippath, '\\');
     path = MBSRCHR((p == NULL ? zippath : p), '/');
     if (path != NULL)
       p = path;
     if (p != NULL) {
       len = (int)(p - zippath) + 1;
       if ((path = malloc(len + sizeof("unzip.exe"))) == NULL)
         ziperr(ZE_MEM, "was creating unzip path");
       memcpy(path, zippath, len);
       strcpy(&path[len], "unzip.exe");
       status = spawnlp(P_WAIT, path, "unzip", verbose ? "-t" : "-tqq",
                        zipname, NULL);
       free(path);
     }
     if (status == -1)
       perror("unzip");
   }
#endif /* ?__human68k__ */
   if (status != 0) {
#else /* (MSDOS && !__GO32__) || __human68k__ */
   char cmd[FNMAX+16];

   /* Tell picky compilers to shut up about unused variables */
   zippath = zippath;

   strcpy(cmd, "unzip -t ");
#ifdef QDOS
   strcat(cmd, "-Q4 ");
#endif
   if (!verbose) strcat(cmd, "-qq ");
   if ((int)strlen(zipname) > FNMAX) {
     error("zip filename too long");
   }
# ifdef UNIX
   strcat(cmd, "'");    /* accept space or $ in name */
   strcat(cmd, zipname);
   strcat(cmd, "'");
# else
   strcat(cmd, zipname);
# endif
# ifdef VMS
   if (!system(cmd)) {
# else
   if (system(cmd)) {
# endif
#endif /* ?((MSDOS && !__GO32__) || __human68k__) */
     fprintf(mesg, "test of %s FAILED\n", zipfile);
     ziperr(ZE_TEST, "original files unmodified");
   }
   if (noisy)
     fprintf(mesg, "test of %s OK\n", zipfile);
}
#endif /* !WINDLL */

local int get_filters(argc, argv)
  int argc;               /* number of tokens in command line */
  char **argv;            /* command line tokens */
/* Counts number of -i or -x patterns, sets patterns and pcount */
  {
  int i;
  int flag = 0, archive_seen = 0;
  char *iname, *p = NULL;
  FILE *fp;

  pcount = 0;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      p = argv[i];
      while (*(++p) != '\0') {
        if (*p == 'i' || *p == 'x')
          break;
      }
      if (*p != '\0') {
        flag = *p;
        p = p[1] == '@' ? p + 2 : NULL;
        if (p != NULL && patterns == NULL) {
          fp = fopen(p, "r");
          if (fp == NULL) {
            ZIPERR(ZE_OPEN, p);
          }
          while (fgets(errbuf, FNMAX, fp) != NULL)
            pcount++;
          fclose(fp);
        }
      } else if (MBSRCHR(argv[i], 'R') != NULL) {
        p = NULL;
        flag = 'R';
      } else if (flag != 'R') {
        flag = 0;
      }
    }
    if (flag && (archive_seen || p != NULL)) {
      if (patterns != NULL) {
        if (p != NULL) {
          fp = fopen(p, "r");
          if (fp == NULL) {
            ZIPERR(ZE_OPEN, p);
          }
          while ((p = getnam(errbuf, fp)) != NULL) {
            iname = ex2in(p, 0, (int *)NULL);
            if (iname != NULL) {
              patterns[pcount].zname = in2ex(iname);
              free(iname);
            } else {
              patterns[pcount].zname = NULL;
            }
            patterns[pcount].select = flag;
            if (flag != 'x')
              icount++;
            pcount++;
          }
          fclose(fp);
          flag = 0;
          p = NULL;
        }
        else if (argv[i][0] != '-') {
          iname = ex2in(argv[i], 0, (int *)NULL);
          patterns[pcount].zname = (iname != NULL ? in2ex(iname) : NULL);
          if (iname != NULL)
            free(iname);
          patterns[pcount].select = flag;
          if (flag != 'x')
            icount++;
          pcount++;
        }
      }
      else if (p == NULL)
        pcount++;
      else
        flag = 0;
    } else {
      if (flag != 'R')
        flag = 0;               /* only 'R' is allowed before zipfile arg */
      archive_seen = 1;         /* first non-flag arg is archive name */
    }
  }
  if (pcount == 0 || patterns != NULL) return ZE_OK;
  patterns = (struct plist*) malloc(pcount * sizeof(struct plist));
  if (patterns == NULL) {
    ZIPERR(ZE_MEM, "was creating pattern list");
    }
  return get_filters(argc, argv);
}

#if CRYPT
#ifndef WINDLL
int encr_passwd(modeflag, pwbuf, size, zfn)
int modeflag;
char *pwbuf;
int size;
ZCONST char *zfn;
{
    char *prompt;

    /* Tell picky compilers to shut up about unused variables */
    zfn = zfn;

    prompt = (modeflag == ZP_PW_VERIFY) ?
              "Verify password: " : "Enter password: ";

    if (getp(prompt, pwbuf, size) == NULL) {
      ziperr(ZE_PARMS, "stderr is not a tty");
    }
    return IZ_PW_ENTERED;
}
#endif /* !WINDLL */
#else /* !CRYPT */
int encr_passwd(modeflag, pwbuf, size, zfn)
int modeflag;
char *pwbuf;
int size;
ZCONST char *zfn;
{
    /* Tell picky compilers to shut up about unused variables */
    modeflag = modeflag; pwbuf = pwbuf; size = size; zfn = zfn;

    return ZE_LOGIC;    /* This function should never be called! */
}
#endif /* CRYPT */

//#ifndef USE_ZIPMAIN
//int main(argc, argv)
//#else
int zip_main(int argc, char** argv)
/* Add, update, freshen, or delete zip entries in a zip file.  See the
   command help in help() above. */
{
  int a;                /* attributes of zip file */
  ulg c;                /* start of central directory */
  int d;                /* true if just adding to a zip file */
  char *e;              /* malloc'd comment buffer */
  struct flist far *f;  /* steps through found linked list */
  int i;                /* arg counter, root directory flag */
  int k;                /* next argument type, marked counter,
                           comment size, entry count */
  ulg n;                /* total of entry len's */
  int o;                /* true if there were any ZE_OPEN errors */
  char *p;              /* steps through option arguments */
  char *pp;             /* temporary pointer */
  ulg *cmptime = NULL;  /* pointer to 'before' or 'after' */
  int r;                /* temporary variable */
  int s;                /* flag to read names from stdin */
  ulg t;                /* file time, length of central directory */
  int first_listarg = 0;/* index of first arg of "process these files" list */
  struct zlist far *v;  /* temporary variable */
  struct zlist far * far *w;    /* pointer to last link in zfiles list */
  FILE *x, *y;          /* input and output zip files */
  struct zlist far *z;  /* steps through zfiles linked list */
//#ifdef WINDLL
  int retcode;          /* return code for dll */
//#endif
#if (!defined(VMS) && !defined(CMS_MVS))
  char *zipbuf;         /* stdio buffer for the zip file */
#endif /* !VMS && !CMS_MVS */
  FILE *comment_stream; /* set to stderr if anything is read from stdin */

#ifdef THEOS
  /* the argument expansion from the standard library is full of bugs */
  /* use mine instead */
  _setargv(&argc, &argv);
  setlocale(LC_CTYPE,"I");
#else
  SETLOCALE(LC_CTYPE,"");
#endif

#if defined(__IBMC__) && defined(__DEBUG_ALLOC__)
  {
    extern void DebugMalloc(void);
    atexit(DebugMalloc);
  }
#endif
#ifdef QDOS
  {
    extern void QDOSexit(void);
    atexit(QDOSexit);
  }
#endif

#ifdef RISCOS
  set_prefix();
#endif

#ifdef __human68k__
  fflush(stderr);
  setbuf(stderr, NULL);
#endif

/* Re-initialize global variables to make the zip dll re-entrant. It is
 * possible that we could get away with not re-initializing all of these
 * but better safe than sorry.
 */
//#ifndef MACOS // should work on MACOSX which is all we need
  retcode = setjmp(zipdll_error_return);
  if (retcode) {
    return retcode;
  }
//#endif /* !MACOS */
//#if defined(MACOS) || defined(WINDLL)
  action = ADD; /* one of ADD, UPDATE, FRESHEN, or DELETE */
  comadd = 0;   /* 1=add comments for new files */
  zipedit = 0;  /* 1=edit zip comment and all file comments */
  latest = 0;   /* 1=set zip file time to time of latest file */
  before = 0;   /* 0=ignore, else exclude files before this time */
  after = 0;    /* 0=ignore, else exclude files newer than this time */
  test = 0;     /* 1=test zip file with unzip -t */
  tempdir = 0;  /* 1=use temp directory (-b) */
  junk_sfx = 0; /* 1=junk the sfx prefix */
#if defined(AMIGA) || defined(MACOS)
  filenotes = 0;/* 1=take comments from AmigaDOS/MACOS filenotes */
#endif
  //zipstate = -1;
  tempzip = NULL;
  fcount = 0;
  recurse = 0;         /* 1=recurse into directories; 2=match filenames */
  dispose = 0;         /* 1=remove files after put in zip file */
  pathput = 1;         /* 1=store path with name */
  method = BEST;       /* one of BEST, DEFLATE (only), or STORE (only) */
  dosify = 0;          /* 1=make new entries look like MSDOS */
  verbose = 0;         /* 1=report oddities in zip file structure */
  fix = 0;             /* 1=fix the zip file */
  adjust = 0;          /* 1=adjust offsets for sfx'd file (keep preamble) */
  level = 6;           /* 0=fastest compression, 9=best compression */
  translate_eol = 0;   /* Translate end-of-line LF -> CR LF */
#ifdef WIN32
  use_longname_ea = 0; /* 1=use the .LONGNAME EA as the file's name */
  use_privileges = 0;  /* 1=use security privileges overrides */
#endif
  hidden_files = 0;    /* process hidden and system files */
  volume_label = 0;    /* add volume label */
  dirnames = 1;        /* include directory entries by default */
  linkput = 0;         /* 1=store symbolic links as such */
  noisy = 1;           /* 0=quiet operation */
  extra_fields = 1;    /* 0=do not create extra fields */
  special = ".Z:.zip:.zoo:.arc:.lzh:.arj"; /* List of special suffixes */
  key = NULL;          /* Scramble password if scrambling */
  tempath = NULL;      /* Path for temporary files */
  found = NULL;        /* where in found, or new found entry */
  fnxt = &found;
  patterns = NULL;     /* List of patterns to be matched */
  pcount = 0;          /* number of patterns */
  icount = 0;          /* number of include only patterns */

//#endif /* MACOS || WINDLL */

  mesg = (FILE *) stdout; /* cannot be made at link time for VMS */
  comment_stream = (FILE *)stdin;

  init_upper();           /* build case map table */

#if (defined(IZ_CHECK_TZ) && defined(USE_EF_UT_TIME))
#  ifndef VALID_TIMEZONE
#     define VALID_TIMEZONE(tmp) \
             (((tmp = getenv("TZ")) != NULL) && (*tmp != '\0'))
#  endif
  zp_tz_is_valid = VALID_TIMEZONE(p);
#if (defined(AMIGA) || defined(DOS))
  if (!zp_tz_is_valid)
    extra_fields = 0;     /* disable storing "UT" time stamps */
#endif /* AMIGA || DOS */
#endif /* IZ_CHECK_TZ && USE_EF_UT_TIME */

/* For systems that do not have tzset() but supply this function using another
   name (_tzset() or something similar), an appropiate "#define tzset ..."
   should be added to the system specifc configuration section.  */
#if (!defined(TOPS20) && !defined(VMS))
#if (!defined(RISCOS) && !defined(MACOS) && !defined(QDOS))
#if (!defined(BSD) && !defined(MTS) && !defined(CMS_MVS) && !defined(TANDEM))
  tzset();
#endif
#endif
#endif

#ifdef VMSCLI
    {
        ulg status = vms_zip_cmdline(&argc, &argv);
        if (!(status & 1))
            return status;
    }
#endif /* VMSCLI */

  /* extract extended argument list from environment */
  expand_args(&argc, &argv);

//printf("\nArgs = %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);

#ifndef WINDLL
  /* Process arguments */
  diag("processing arguments");
  /* First, check if just the help or version screen should be displayed */
  if (isatty(1)) {              /* output screen is available */
    if (argc == 1)
    {                           /* show help screen */
#ifdef VMSCLI
      VMSCLI_help();
#else
      help();
#endif
      EXIT(0, "");
    }
    else if (argc == 2 && strcmp(argv[1], "-v") == 0)
    {                           /* show diagnostic version info */
      version_info();
      EXIT(0, "");
    }
  }
#ifndef VMS
#  ifndef RISCOS
  envargs(&argc, &argv, "ZIPOPT", "ZIP");  /* get options from environment */
#  else /* RISCOS */
  envargs(&argc, &argv, "ZIPOPT", "Zip$Options");  /* get options from environment */
  getRISCOSexts("Zip$Exts");        /* get the extensions to swap from environment */
#  endif /* ? RISCOS */
#else /* VMS */
  envargs(&argc, &argv, "ZIPOPT", "ZIP_OPTS");  /* 4th arg for unzip compat. */
#endif /* ?VMS */
#endif /* !WINDLL */

  zipfile = tempzip = NULL;
  tempzf = NULL;
  d = 0;                        /* disallow adding to a zip file */
  k = 0;                        /* Next non-option argument type */
  s = 0;                        /* set by -@ if -@ is early */

  r = get_filters(argc, argv);      /* scan first the -x and -i patterns */
#ifdef WINDLL
  if (r != ZE_OK)
     return r;
#endif

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      if (argv[i][1])
        for (p = argv[i]+1; *p; p++)
          switch (*p)
          {
#ifdef EBCDIC
            case 'a':
              aflag = ASCII;
              printf("Translating to ASCII...\n");
              break;
#endif /* EBCDIC */
#ifdef CMS_MVS
            case 'B':
              bflag = 1;
              printf("Using binary mode...\n");
              break;
#endif /* CMS_MVS */
#ifdef TANDEM
            case 'B':
              nskformatopt(&p);
              break;
#endif
            case '0':
              method = STORE; level = 0; break;
            case '1':  case '2':  case '3':  case '4':
            case '5':  case '6':  case '7':  case '8':  case '9':
                        /* Set the compression efficacy */
              level = *p - '0';  break;
            case 'A':   /* Adjust unzipsfx'd zipfile:  adjust offsets only */
              adjust = 1; break;
            case 'b':   /* Specify path for temporary file */
              tempdir = 1;
              if (k != 0) {
                ZIPERR(ZE_PARMS, "use -b before zip file name");
              }
              else
                k = 1;          /* Next non-option is path */
              break;
            case 'c':   /* Add comments for new files in zip file */
              comadd = 1;  break;
            case 'd':   /* Delete files from zip file */
#ifdef MACOS
              if (p[1] == 'f') {
                ++p;
                MacZip.DataForkOnly = true;
              } else
#endif /* MACOS */
              {
                if (action != ADD) {
                  ZIPERR(ZE_PARMS, "specify just one action");
                }
                action = DELETE;
                break;
              }
            case 'D':   /* Do not add directory entries */
              dirnames = 0; break;
            case 'e':   /* Encrypt */
#if !CRYPT
              ZIPERR(ZE_PARMS, "encryption not supported");
#else /* CRYPT */
              if (key == NULL) {
                if ((key = malloc(IZ_PWLEN+1)) == NULL) {
                  ZIPERR(ZE_MEM, "was getting encryption password");
                }
                r = encr_passwd(ZP_PW_ENTER, key, IZ_PWLEN+1, zipfile);
                if (r != IZ_PW_ENTERED) {
                  if (r < IZ_PW_ENTERED)
                    r = ZE_PARMS;
                  ZIPERR(r, "was getting encryption password");
                }
                if (*key == '\0') {
                  ZIPERR(ZE_PARMS, "zero length password not allowed");
                }
                if ((e = malloc(IZ_PWLEN+1)) == NULL) {
                  ZIPERR(ZE_MEM, "was verifying encryption password");
                }
                r = encr_passwd(ZP_PW_VERIFY, e, IZ_PWLEN+1, zipfile);
                if (r != IZ_PW_ENTERED && r != IZ_PW_SKIPVERIFY) {
                  free((zvoid *)e);
                  if (r < ZE_OK) r = ZE_PARMS;
                  ZIPERR(r, "was verifying encryption password");
                }
                r = ((r == IZ_PW_SKIPVERIFY) ? 0 : strcmp(key, e));
                free((zvoid *)e);
                if (r) {
                  ZIPERR(ZE_PARMS, "password verification failed");
                }
              }
#endif /* !CRYPT */
              break;
            case 'F':   /* fix the zip file */
              fix++; break;
            case 'f':   /* Freshen zip file--overwrite only */
              if (action != ADD) {
                ZIPERR(ZE_PARMS, "specify just one action");
              }
              action = FRESHEN;
              break;
            case 'g':   /* Allow appending to a zip file */
              d = 1;  break;
#ifndef WINDLL
            case 'h': case 'H': case '?':  /* Help */
#ifdef VMSCLI
              VMSCLI_help();
#else
              help();
#endif
              RETURN(finish(ZE_OK));
#endif /* !WINDLL */

#ifdef RISCOS
            case 'I':   /* Don't scan through Image files */
              scanimage = 0;
              break;
#endif
#ifdef MACOS
            case 'j':   /* Junk path / Store absolute path */
              if (p[1] == 'j') {    /* store absolute path including volname */
                ++p;
                MacZip.StoreFullPath = true;
              } else {              /* junk directory names */
                pathput = 0;  break;
              }
#else /* !MACOS */
            case 'j':   /* Junk directory names */
              pathput = 0;  break;
#endif /* ?MACOS */
            case 'J':   /* Junk sfx prefix */
              junk_sfx = 1;  break;
            case 'k':   /* Make entries using DOS names (k for Katz) */
              dosify = 1;  break;
            case 'l':   /* Translate end-of-line */
              translate_eol++; break;
#ifndef WINDLL
            case 'L':   /* Show license */
              license();
              RETURN(finish(ZE_OK));
#endif
            case 'm':   /* Delete files added or updated in zip file */
              dispose = 1;  break;
            case 'n':   /* Don't compress files with a special suffix */
              special = NULL; /* will be set at next argument */
              break;
#if defined(AMIGA) || defined(MACOS)
            case 'N':   /* Get zipfile comments from AmigaDOS/MACOS filenotes */
              filenotes = 1; break;
#endif
            case 'o':   /* Set zip file time to time of latest file in it */
              latest = 1;  break;
            case 'p':   /* Store path with name */
              break;            /* (do nothing as annoyance avoidance) */
            case 'P':   /* password for encryption */
              if (k != 0) {
                ZIPERR(ZE_PARMS, "use -P before zip file name");
              }
              if (key != NULL) {
                ZIPERR(ZE_PARMS, "can only have one -P");
              }
#if CRYPT
              k = 7;
#else
              ZIPERR(ZE_PARMS, "encryption not supported");
#endif /* CRYPT */
              break;
#if defined(QDOS) || defined(QLZIP)
            case 'Q':
              qlflag  = strtol((p+1), &p, 10);
              if (qlflag == 0) qlflag = 4;
              p--;
              break;
#endif
            case 'q':   /* Quiet operation */
              noisy = 0;
#ifdef MACOS
              MacZip.MacZip_Noisy = false;
#endif  /* MACOS */
              if (verbose) verbose--;
              break;
            case 'r':   /* Recurse into subdirectories, match full path */
              if (recurse == 2) {
                ZIPERR(ZE_PARMS, "do not specify both -r and -R");
              }
              recurse = 1;  break;
            case 'R':   /* Recurse into subdirectories, match filename */
              if (recurse == 1) {
                ZIPERR(ZE_PARMS, "do not specify both -r and -R");
              }
              recurse = 2;  break;
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(ATARI)
            case 'S':
              hidden_files = 1; break;
#endif /* MSDOS || OS2 || WIN32 || ATARI */
#ifdef MACOS
            case 'S':
              MacZip.IncludeInvisible = true; break;
#endif /* MACOS */
            case 't':   /* Exclude files earlier than specified date */
              if (p[1] == 't') {
                ++p;
                cmptime = &after;
              } else {
                cmptime = &before;
              }
              if (*cmptime) {
                ZIPERR(ZE_PARMS, (cmptime == &after ?
                       "can only have one -tt" : "can only have one -t"));
              }
              k = 2;  break;
            case 'T':   /* test zip file */
              test = 1; break;
            case 'u':   /* Update zip file--overwrite only if newer */
              if (action != ADD) {
                ZIPERR(ZE_PARMS, "specify just one action");
              }
              action = UPDATE;
              break;
            case 'v':   /* Mention oddities in zip file structure */
              noisy = 1;
              verbose++;
              break;
#ifdef VMS
            case 'V':   /* Store in VMS format */
              vms_native = 1; break;
            case 'w':   /* Append the VMS version number */
              vmsver = 1;  break;
#endif /* VMS */
            case 'i':   /* Include only the following files */
            case 'x':   /* Exclude following files */
              if (p[1] == '@' && p[2] != '\0') {
                goto nextarg;
              }
              if (zipfile == NULL) {
                ZIPERR(ZE_PARMS, "use -x or -i after name of zipfile");
              }
              k = 5;
              break;
#ifdef S_IFLNK
            case 'y':   /* Store symbolic links as such */
              linkput = 1;  break;
#endif /* S_IFLNK */
            case 'z':   /* Edit zip file comment */
              zipedit = 1;  break;
#if defined(MSDOS) || defined(OS2)
            case '$':   /* Include volume label */
              volume_label = 1; break;
#endif
#ifndef MACOS
            case '@':   /* read file names from stdin */
              comment_stream = NULL;
              if (k < 3)        /* zip file not read yet */
                s = 1;          /* defer -@ until after zipfile read */
              else if (strcmp(zipfile, "-") == 0) {
                ZIPERR(ZE_PARMS, "can't use - and -@ together");
              }
              else              /* zip file read--do it now */
                while ((pp = getnam(errbuf, stdin)) != NULL)
                {
                  k = 4;
                  if ((r = PROCNAME(pp)) != ZE_OK)
                  {
                    if (r == ZE_MISS)
                      zipwarn("name not matched: ", pp);
                    else {
                      ZIPERR(r, pp);
                    }
                  }
                }
              break;
#endif /* !MACOS */
            case 'X':
              extra_fields = 0;
              break;
#ifdef OS2
            case 'E':
              /* use the .LONGNAME EA (if any) as the file's name. */
              use_longname_ea = 1;
              break;
#endif
#ifdef WIN32
            case '!':
              /* use security privilege overrides */
              use_privileges = 1;
              break;
#endif
            default:
            {
              sprintf(errbuf, "no such option: %c", *p);
              ZIPERR(ZE_PARMS, errbuf);
            }
          }
      else              /* just a dash */
        switch (k)
        {
#if (!defined(MACOS) && !defined(WINDLL))
        case 0:
          zipstdout();
          k = 3;
          if (s) {
            ZIPERR(ZE_PARMS, "can't use - and -@ together");
          }
          break;
#endif /* !MACOS && !WINDLL */
        case 1:
          ZIPERR(ZE_PARMS, "invalid path");
          /* not reached */
        case 2:
          ZIPERR(ZE_PARMS, "invalid time");
          /* not reached */
        case 3:  case 4:
          comment_stream = NULL;
          if ((r = PROCNAME(argv[i])) != ZE_OK) {
            if (r == ZE_MISS)
              zipwarn("name not matched: ", argv[i]);
            else {
              ZIPERR(r, argv[i]);
            }
          }
          if (k == 3) {
            first_listarg = i;
            k = 4;
          }
        }
    }
    else                /* not an option */
    {
      if (special == NULL)
        special = argv[i];
      else if (k == 5)
        break; /* -i and -x arguments already scanned */
      else if (k == 6) {
#ifdef AMIGA
        if ((r = PROCNAME("")) != ZE_OK) {
#else
        if ((r = PROCNAME(".")) != ZE_OK) {
#endif
          if (r == ZE_MISS)
            zipwarn("name not matched: ", argv[i]);
          else {
            ZIPERR(r, argv[i]);
          }
        }
        if (first_listarg == 0)
          first_listarg = i;
        break;
      }
      else switch (k)
      {
        case 0:
          if ((zipfile = ziptyp(argv[i])) == NULL) {
            ZIPERR(ZE_MEM, "was processing arguments");
          }
          if ((r = readzipfile()) != ZE_OK) {
            ZIPERR(r, zipfile);
          }
          k = 3;
          if (s)
          {
            while ((pp = getnam(errbuf, stdin)) != NULL)
            {
              k = 4;
              if ((r = PROCNAME(pp)) != ZE_OK) {
                if (r == ZE_MISS)
                  zipwarn("name not matched: ", pp);
                else {
                  ZIPERR(r, pp);
                }
              }
            }
            s = 0;
          }
          if (recurse == 2)
            k = 6;
          break;
        case 1:
          if ((tempath = malloc(strlen(argv[i]) + 1)) == NULL) {
            ZIPERR(ZE_MEM, "was processing arguments");
          }
          strcpy(tempath, argv[i]);
          k = (zipfile != NULL ? (first_listarg > 0 ? 4 : 3) : 0);
          break;
        case 2:
        {
          int yyyy, mm, dd;       /* results of sscanf() */

          /* Support ISO 8601 & American dates */
          if ((sscanf(argv[i], "%4d-%2d-%2d", &yyyy, &mm, &dd) != 3 &&
               sscanf(argv[i], "%2d%2d%4d", &mm, &dd, &yyyy) != 3) ||
              mm < 1 || mm > 12 || dd < 1 || dd > 31) {
            ZIPERR(ZE_PARMS, (cmptime == &after ?
                   "invalid date entered for -tt option" :
                   "invalid date entered for -t option"));
          }
          *cmptime = dostime(yyyy, mm, dd, 0, 0, 0);
          k = (zipfile != NULL ? (first_listarg > 0 ? 4 : 3) : 0);
          break;
        }
        case 3:  case 4:
//CMC
//printf("k=3/4, PROCNAME=%s\n", argv[i]);
          if ((r = PROCNAME(argv[i])) != ZE_OK) {
            if (r == ZE_MISS) {
//printf("ZE_MISS\n");
              zipwarn("name not matched: ", argv[i]); 
            }
            else {
//printf("ZIPERR\n");
              ZIPERR(r, argv[i]);
            }
          }
          if (k == 3) {
            first_listarg = i;
            k = 4;
          }
          break;
        case 7:
          if ((key = malloc(strlen(argv[i]) + 1)) == NULL) {
            ZIPERR(ZE_MEM, "was processing arguments");
          }
          strcpy(key, argv[i]);
          k = (zipfile != NULL ? (first_listarg > 0 ? 4 : 3) : 0);
      }
    }
nextarg: ;
  }
  if (k == 7 || k == 1) {
    ZIPERR(ZE_PARMS, "missing argument for -b or -P");
  }

#if (defined(MSDOS) || defined(OS2)) && !defined(WIN32)
  if ((k == 3 || k == 4) && volume_label == 1) {
    PROCNAME(NULL);
    k = 4;
  }
#endif

  if (pcount && first_listarg == 0 &&
      (k < 3 || (action != UPDATE && action != FRESHEN))) {
    ZIPERR(ZE_PARMS, "nothing to select from");
  }

#if (!defined(MACOS) && !defined(WINDLL))
  if (k < 3) {               /* zip used as filter */
    zipstdout();
    comment_stream = NULL;
    if ((r = procname("-", 0)) != ZE_OK) {
      if (r == ZE_MISS)
        zipwarn("name not matched: ", "-");
      else {
        ZIPERR(r, "-");
      }
    }
    k = 4;
    if (s) {
      ZIPERR(ZE_PARMS, "can't use - and -@ together");
    }
  }
#endif /* !MACOS && !WINDLL */

  /* Clean up selections ("3 <= k <= 5" now) */
  if (k != 4 && first_listarg == 0 &&
      (action == UPDATE || action == FRESHEN)) {
    /* if -u or -f with no args, do all, but, when present, apply filters */
    for (z = zfiles; z != NULL; z = z->nxt) {
      z->mark = pcount ? filter(z->zname, 0) : 1;
    }
  }
  if ((r = check_dup()) != ZE_OK) {     /* remove duplicates in found list */
    if (r == ZE_PARMS) {
      ZIPERR(r, "cannot repeat names in zip file");
    }
    else {
      ZIPERR(r, "was processing list of files");
    }
  }

  if (zcount)
    free((zvoid *)zsort);

  /* Check option combinations */
  if (special == NULL) {
    ZIPERR(ZE_PARMS, "missing suffix list");
  }
  if (level == 9 || !strcmp(special, ";") || !strcmp(special, ":"))
    special = NULL; /* compress everything */

  if (action == DELETE && (method != BEST || dispose || recurse ||
      key != NULL || comadd || zipedit)) {
    zipwarn("invalid option(s) used with -d; ignored.","");
    /* reset flags - needed? */
    method  = BEST;
    dispose = 0;
    recurse = 0;
    if (key != NULL)
      free((zvoid *)key);
    key     = NULL;
    comadd  = 0;
    zipedit = 0;
  }
  if (linkput && dosify)
    {
      zipwarn("can't use -y with -k, -y ignored", "");
      linkput = 0;
    }
  if (fix && adjust)
    {
      zipwarn("can't use -F with -A, -F ignored", "");
    }
  if (test && !strcmp(zipfile, "-")) {
    test = 0;
    zipwarn("can't use -T on stdout, -T ignored", "");
  }
  if ((action != ADD || d) && !strcmp(zipfile, "-")) {
    ZIPERR(ZE_PARMS, "can't use -d,-f,-u or -g on stdout\n");
  }
#if defined(EBCDIC)  && !defined(OS390)
  if (aflag==ASCII && !translate_eol) {
    /* Translation to ASCII implies EOL translation!
     * (on OS390, consistent EOL translation is controlled separately)
     * The default translation mode is "UNIX" mode (single LF terminators).
     */
    translate_eol = 2;
  }
#endif
#ifdef CMS_MVS
  if (aflag==ASCII && bflag)
    ZIPERR(ZE_PARMS, "can't use -a with -B");
#endif
#ifdef VMS
  if (!extra_fields && vms_native)
    {
      zipwarn("can't use -V with -X, -V ignored", "");
      vms_native = 0;
    }
  if (vms_native && translate_eol)
    ZIPERR(ZE_PARMS, "can't use -V with -l or -ll");
#endif
  if (zcount == 0 && (action != ADD || d)) {
    zipwarn(zipfile, " not found or empty");
  }

/*
 * XXX make some kind of mktemppath() function for each OS.
 */

#ifndef VM_CMS
/* For CMS, leave tempath NULL.  A-disk will be used as default. */
  /* If -b not specified, make temporary path the same as the zip file */
#if defined(MSDOS) || defined(__human68k__) || defined(AMIGA)
  if (tempath == NULL && ((p = MBSRCHR(zipfile, '/')) != NULL ||
#  ifdef MSDOS
                          (p = MBSRCHR(zipfile, '\\')) != NULL ||
#  endif /* MSDOS */
                          (p = MBSRCHR(zipfile, ':')) != NULL))
  {
    if (*p == ':')
      p++;
#else
#ifdef RISCOS
  if (tempath == NULL && (p = MBSRCHR(zipfile, '.')) != NULL)
  {
#else
#ifdef QDOS
  if (tempath == NULL && (p = LastDir(zipfile)) != NULL)
  {
#else
  if (tempath == NULL && (p = MBSRCHR(zipfile, '/')) != NULL)
  {
#endif /* QDOS */
#endif /* RISCOS */
#endif /* MSDOS || __human68k__ || AMIGA */
    if ((tempath = malloc((int)(p - zipfile) + 1)) == NULL) {
      ZIPERR(ZE_MEM, "was processing arguments");
    }
    r = *p;  *p = 0;
    strcpy(tempath, zipfile);
    *p = (char)r;
  }
#endif /* VM_CMS */

#if (defined(IZ_CHECK_TZ) && defined(USE_EF_UT_TIME))
  if (!zp_tz_is_valid && action != DELETE) {
    zipwarn("TZ environment variable not found, cannot use UTC times!!","");
  }
#endif /* IZ_CHECK_TZ && USE_EF_UT_TIME */

  /* For each marked entry, if not deleting, check if it exists, and if
     updating or freshening, compare date with entry in old zip file.
     Unmark if it doesn't exist or is too old, else update marked count. */
#ifdef MACOS
  PrintStatProgress("Getting file information ...");
#endif
  diag("stating marked entries");
  k = 0;                        /* Initialize marked count */
  for (z = zfiles; z != NULL; z = z->nxt)
    if (z->mark) {
#ifdef USE_EF_UT_TIME
      iztimes f_utim, z_utim;
#endif /* USE_EF_UT_TIME */
      Trace((stderr, "zip diagnostics: marked file=%s\n", z->zname));

      if (action != DELETE &&
#ifdef USE_EF_UT_TIME
          ((t = filetime(z->name, (ulg *)NULL, (long *)NULL, &f_utim))
#else /* !USE_EF_UT_TIME */
          ((t = filetime(z->name, (ulg *)NULL, (long *)NULL, (iztimes *)NULL))
#endif /* ?USE_EF_UT_TIME */
              == 0 ||
           t < before || (after && t >= after) ||
           ((action == UPDATE || action == FRESHEN) &&
#ifdef USE_EF_UT_TIME
            ((get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
             f_utim.mtime <= ROUNDED_TIME(z_utim.mtime) : t <= z->tim)
#else /* !USE_EF_UT_TIME */
            t <= z->tim
#endif /* ?USE_EF_UT_TIME */
           )))
      {
        z->mark = comadd ? 2 : 0;
        z->trash = t && t >= before &&
                   (after == 0 || t < after);   /* delete if -um or -fm */
        if (verbose) {
          fprintf(mesg, "zip diagnostic: %s %s\n", z->zname,
                 z->trash ? "up to date" : "missing or early");
        }
      }
      else
        k++;
    }

  /* Remove entries from found list that do not exist or are too old */
  diag("stating new entries");
  Trace((stderr, "zip diagnostic: fcount=%u\n", (unsigned)fcount));
  for (f = found; f != NULL;) {
    Trace((stderr, "zip diagnostic: new file=%s\n", f->zname));
    if (action == DELETE || action == FRESHEN ||
        (t = filetime(f->name, (ulg *)NULL, (long *)NULL, (iztimes *)NULL))
           == 0 ||
        t < before || (after && t >= after) ||
        (namecmp(f->zname, zipfile) == 0 && strcmp(zipfile, "-")))
      f = fexpel(f);
    else
      f = f->nxt;
  }
#ifdef MACOS
  PrintStatProgress("done");
#endif

  /* Make sure there's something left to do */
  if (k == 0 && found == NULL &&
      !(zfiles != NULL &&
        (latest || fix || adjust || junk_sfx || comadd || zipedit))) {
    if (test && (zfiles != NULL || zipbeg != 0)) {
#ifndef WINDLL
      check_zipfile(zipfile, argv[0]);
#endif
      RETURN(finish(ZE_OK));
    }
    if (action == UPDATE || action == FRESHEN) {
      RETURN(finish(ZE_NONE));
    }
    else if (zfiles == NULL && (latest || fix || adjust || junk_sfx)) {
      ZIPERR(ZE_NAME, zipfile);
    }
#ifndef WINDLL
    else if (recurse && (pcount == 0) && (first_listarg > 0)) {
#ifdef VMS
      strcpy(errbuf, "try: zip \"");
      for (i = 1; i < (first_listarg - 1); i++)
        strcat(strcat(errbuf, argv[i]), "\" ");
      strcat(strcat(errbuf, argv[i]), " *.* -i");
#else /* !VMS */
      strcpy(errbuf, "try: zip");
      for (i = 1; i < first_listarg; i++)
        strcat(strcat(errbuf, " "), argv[i]);
#  ifdef AMIGA
      strcat(errbuf, " \"\" -i");
#  else
      strcat(errbuf, " . -i");
#  endif
#endif /* ?VMS */
      for (i = first_listarg; i < argc; i++)
        strcat(strcat(errbuf, " "), argv[i]);
      ZIPERR(ZE_NONE, errbuf);
    }
#endif /* !WINDLL */
    else {
      ZIPERR(ZE_NONE, zipfile);
    }
  }
  d = (d && k == 0 && (zipbeg || zfiles != NULL)); /* d true if appending */

#if CRYPT
  /* Initialize the crc_32_tab pointer, when encryption was requested. */
  if (key != NULL)
    crc_32_tab = get_crc_table_boinc();
#endif /* CRYPT */

  /* Before we get carried away, make sure zip file is writeable. This
   * has the undesired side effect of leaving one empty junk file on a WORM,
   * so when the zipfile does not exist already and when -b is specified,
   * the writability check is made in replace().
   */
  if (strcmp(zipfile, "-"))
  {
    if (tempdir && zfiles == NULL && zipbeg == 0) {
      a = 0;
    } else {
       x = zfiles == NULL && zipbeg == 0 ? fopen(zipfile, FOPW) :
                                           fopen(zipfile, FOPM);
      /* Note: FOPW and FOPM expand to several parameters for VMS */
      if (x == NULL) {
        ZIPERR(ZE_CREAT, zipfile);
      }
      fclose(x);
      a = getfileattr(zipfile);
      if (zfiles == NULL && zipbeg == 0)
        destroy(zipfile);
    }
  }
  else
    a = 0;

  /* Throw away the garbage in front of the zip file for -J */
  if (junk_sfx) zipbeg = 0;

  /* Open zip file and temporary output file */
  diag("opening zip file and creating temporary zip file");
  x = NULL;
  tempzn = 0;
  if (strcmp(zipfile, "-") == 0)
  {
#ifdef MSDOS
    /* It is nonsense to emit the binary data stream of a zipfile to
     * the (text mode) console.  This case should already have been caught
     * in a call to zipstdout() far above.  Therefore, if the following
     * failsafe check detects a console attached to stdout, zip is stopped
     * with an "internal logic error"!  */
    if (isatty(fileno(stdout)))
      ZIPERR(ZE_LOGIC, "tried to write binary zipfile data to console!");
    /* Set stdout mode to binary for MSDOS systems */
#  ifdef __HIGHC__
    setmode(stdout, _BINARY);
#  else
    setmode(fileno(stdout), O_BINARY);
#  endif
    tempzf = y = fdopen(fileno(stdout), FOPW);
#else
    tempzf = y = stdout;
#endif
    /* tempzip must be malloced so a later free won't barf */
    tempzip = malloc(4);
    if (tempzip == NULL) {
      ZIPERR(ZE_MEM, "allocating temp filename");
    }
    strcpy(tempzip, "-");
  }
  else if (d) /* d true if just appending (-g) */
  {
    if ((y = fopen(zipfile, FOPM)) == NULL) {
      ZIPERR(ZE_NAME, zipfile);
    }
    tempzip = zipfile;
    tempzf = y;
    if (fseek(y, cenbeg, SEEK_SET)) {
      ZIPERR(ferror(y) ? ZE_READ : ZE_EOF, zipfile);
    }
    tempzn = cenbeg;
  }
  else
  {
    if ((zfiles != NULL || zipbeg) && (x = fopen(zipfile, FOPR_EX)) == NULL) {
      ZIPERR(ZE_NAME, zipfile);
    }
    if ((tempzip = tempname(zipfile)) == NULL) {
      ZIPERR(ZE_MEM, "allocating temp filename");
    }
    if ((tempzf = y = fopen(tempzip, FOPW_TMP)) == NULL) {
      ZIPERR(ZE_TEMP, tempzip);
    }
  }

#if (!defined(VMS) && !defined(CMS_MVS))
  /* Use large buffer to speed up stdio: */
#if (defined(_IOFBF) || !defined(BUFSIZ))
  zipbuf = (char *)malloc(ZBSZ);
#else
  zipbuf = (char *)malloc(BUFSIZ);
#endif
  if (zipbuf == NULL) {
    ZIPERR(ZE_MEM, tempzip);
  }
# ifdef _IOFBF
  setvbuf(y, zipbuf, _IOFBF, ZBSZ);
# else
  setbuf(y, zipbuf);
# endif /* _IOBUF */
#endif /* !VMS  && !CMS_MVS */

  if (strcmp(zipfile, "-") != 0 && !d)  /* this must go *after* set[v]buf */
  {
    if (zipbeg && (r = fcopy(x, y, zipbeg)) != ZE_OK) {
      ZIPERR(r, r == ZE_TEMP ? tempzip : zipfile);
      }
    tempzn = zipbeg;
  }

  o = 0;                                /* no ZE_OPEN errors yet */


  /* Process zip file, updating marked files */
#ifdef DEBUG
  if (zfiles != NULL)
    diag("going through old zip file");
#endif
  w = &zfiles;
  while ((z = *w) != NULL) {
    if (z->mark == 1)
    {
      /* if not deleting, zip it up */
      if (action != DELETE)
      {
        if (noisy)
        {
          if (action == FRESHEN)
             fprintf(mesg, "freshening: %s", z->zname);
          else
             fprintf(mesg, "updating: %s", z->zname);
          fflush(mesg);
        }
        if ((r = zipup(z, y)) != ZE_OK && r != ZE_OPEN && r != ZE_MISS)
        {
          if (noisy)
          {
#if (!defined(MACOS) && !defined(WINDLL))
            putc('\n', mesg);
            fflush(mesg);
#else
            fprintf(stdout, "\n");
#endif
          }
          sprintf(errbuf, "was zipping %s", z->name);
          ZIPERR(r, errbuf);
        }
        if (r == ZE_OPEN || r == ZE_MISS)
        {
          o = 1;
          if (noisy)
          {
#if (!defined(MACOS) && !defined(WINDLL))
            putc('\n', mesg);
            fflush(mesg);
#else
            fprintf(stdout, "\n");
#endif
          }
          if (r == ZE_OPEN) {
            perror(z->zname);
            zipwarn("could not open for reading: ", z->zname);
          } else {
            zipwarn("file and directory with the same name: ", z->zname);
          }
          zipwarn("will just copy entry over: ", z->zname);
          if ((r = zipcopy(z, x, y)) != ZE_OK)
          {
            sprintf(errbuf, "was copying %s", z->zname);
            ZIPERR(r, errbuf);
          }
          z->mark = 0;
        }
        w = &z->nxt;
      }
      else
      {
        if (noisy)
        {
          fprintf(mesg, "deleting: %s\n", z->zname);
          fflush(mesg);
        }
#ifdef WINDLL
        if (lpZipUserFunctions->ServiceApplication != NULL) {
          if ((*lpZipUserFunctions->ServiceApplication)(z->zname, 0))
            ZIPERR(ZE_ABORT, "User terminated operation");
        }
#endif
        v = z->nxt;                     /* delete entry from list */
        free((zvoid *)(z->iname));
        free((zvoid *)(z->zname));
        if (z->ext)
          free((zvoid *)(z->extra));
        if (z->cext && z->cextra != z->extra)
          free((zvoid *)(z->cextra));
        if (z->com)
          free((zvoid *)(z->comment));
        farfree((zvoid far *)z);
        *w = v;
        zcount--;
      }
    }
    else
    {
      /* copy the original entry verbatim */
      if (!d && (r = zipcopy(z, x, y)) != ZE_OK)
      {
        sprintf(errbuf, "was copying %s", z->zname);
        ZIPERR(r, errbuf);
      }
      w = &z->nxt;
    }
  }


  /* Process the edited found list, adding them to the zip file */
  diag("zipping up new entries, if any");
  Trace((stderr, "zip diagnostic: fcount=%u\n", (unsigned)fcount));
  for (f = found; f != NULL; f = fexpel(f))
  {
    //printf("found = %s\n", f->name);

    /* add a new zfiles entry and set the name */
    if ((z = (struct zlist far *)farmalloc(sizeof(struct zlist))) == NULL) {
      ZIPERR(ZE_MEM, "was adding files to zip file");
    }
    z->nxt = NULL;
    z->name = f->name;
    f->name = NULL;
    z->iname = f->iname;
    f->iname = NULL;
    z->zname = f->zname;
    f->zname = NULL;
    z->ext = z->cext = z->com = 0;
    z->extra = z->cextra = NULL;
    z->mark = 1;
    z->dosflag = f->dosflag;
    /* zip it up */
    if (noisy)
    {
      fprintf(mesg, "  adding: %s", z->zname);
      fflush(mesg);
    }
    if ((r = zipup(z, y)) != ZE_OK  && r != ZE_OPEN && r != ZE_MISS)
    {
      if (noisy)
      {
#if (!defined(MACOS) && !defined(WINDLL))
        putc('\n', mesg);
        fflush(mesg);
#else
        fprintf(stdout, "\n");
#endif
      }
      sprintf(errbuf, "was zipping %s", z->zname);
      ZIPERR(r, errbuf);
    }
    if (r == ZE_OPEN || r == ZE_MISS)
    {
      o = 1;
      if (noisy)
      {
#if (!defined(MACOS) && !defined(WINDLL))
        putc('\n', mesg);
        fflush(mesg);
#else
        fprintf(stdout, "\n");
#endif
      }
      if (r == ZE_OPEN) {
        perror("zip warning");
        zipwarn("could not open for reading: ", z->zname);
      } else {
        zipwarn("file and directory with the same name: ", z->zname);
      }
      free((zvoid *)(z->name));
      free((zvoid *)(z->iname));
      free((zvoid *)(z->zname));
      farfree((zvoid far *)z);
    }
    else
    {
      *w = z;
      w = &z->nxt;
      zcount++;
    }
  }
  if (key != NULL)
  {
    free((zvoid *)key);
    key = NULL;
  }


  /* Get one line comment for each new entry */
#if defined(AMIGA) || defined(MACOS)
  if (comadd || filenotes)
  {
    if (comadd)
#else
  if (comadd)
  {
#endif
    {
      if (comment_stream == NULL) {
#ifndef RISCOS
        comment_stream = (FILE*)fdopen(fileno(stderr), "r");
#else
        comment_stream = stderr;
#endif
      }
      if ((e = malloc(MAXCOM + 1)) == NULL) {
        ZIPERR(ZE_MEM, "was reading comment lines");
      }
    }
#ifdef __human68k__
    setmode(fileno(comment_stream), O_TEXT);
#endif
#ifdef MACOS
    if (noisy) fprintf(mesg, "\nStart commenting files ...\n");
#endif
    for (z = zfiles; z != NULL; z = z->nxt)
      if (z->mark)
#if defined(AMIGA) || defined(MACOS)
        if (filenotes && (p = GetComment(z->zname)))
        {
          if (z->comment = malloc(k = strlen(p)+1))
          {
            z->com = k;
            strcpy(z->comment, p);
          }
          else
          {
            free((zvoid *)e);
            ZIPERR(ZE_MEM, "was reading filenotes");
          }
        }
        else if (comadd)
#endif /* AMIGA || MACOS */
        {
          if (noisy)
            fprintf(mesg, "Enter comment for %s:\n", z->zname);
          if (fgets(e, MAXCOM+1, comment_stream) != NULL)
          {
            if ((p = malloc((k = strlen(e))+1)) == NULL)
            {
              free((zvoid *)e);
              ZIPERR(ZE_MEM, "was reading comment lines");
            }
            strcpy(p, e);
            if (p[k-1] == '\n')
              p[--k] = 0;
            z->comment = p;
            z->com = k;
          }
        }
#ifdef MACOS
    if (noisy) fprintf(mesg, "\n...done");
#endif
#if defined(AMIGA) || defined(MACOS)
    if (comadd)
      free((zvoid *)e);
    GetComment(NULL);           /* makes it free its internal storage */
#else
    free((zvoid *)e);
#endif
  }

  /* Get multi-line comment for the zip file */
  if (zipedit)
  {
#ifndef WINDLL
    if (comment_stream == NULL) {
#ifndef RISCOS
      comment_stream = (FILE*)fdopen(fileno(stderr), "r");
#else
      comment_stream = stderr;
#endif
    }
    if ((e = malloc(MAXCOM + 1)) == NULL) {
      ZIPERR(ZE_MEM, "was reading comment lines");
    }
    if (noisy && zcomlen)
    {
      fputs("current zip file comment is:\n", mesg);
      fwrite(zcomment, 1, zcomlen, mesg);
      if (zcomment[zcomlen-1] != '\n')
        putc('\n', mesg);
      free((zvoid *)zcomment);
    }
    if ((zcomment = malloc(1)) == NULL)
      ZIPERR(ZE_MEM, "was setting comments to null");
    zcomment[0] = '\0';
    if (noisy)
      fputs("enter new zip file comment (end with .):\n", mesg);
#if (defined(AMIGA) && (defined(LATTICE)||defined(__SASC)))
    flushall();  /* tty input/output is out of sync here */
#endif
#ifdef __human68k__
    setmode(fileno(comment_stream), O_TEXT);
#endif
#ifdef MACOS
    printf("\n enter new zip file comment \n");
    if (fgets(e, MAXCOM+1, comment_stream) != NULL) {
        if ((p = malloc((k = strlen(e))+1)) == NULL) {
            free((zvoid *)e);
            ZIPERR(ZE_MEM, "was reading comment lines");
        }
        strcpy(p, e);
        if (p[k-1] == '\n') p[--k] = 0;
        zcomment = p;
    }
#else /* !MACOS */
    while (fgets(e, MAXCOM+1, comment_stream) != NULL && strcmp(e, ".\n"))
    {
      if (e[(r = strlen(e)) - 1] == '\n')
        e[--r] = 0;
      if ((p = malloc((*zcomment ? strlen(zcomment) + 3 : 1) + r)) == NULL)
      {
        free((zvoid *)e);
        ZIPERR(ZE_MEM, "was reading comment lines");
      }
      if (*zcomment)
        strcat(strcat(strcpy(p, zcomment), "\r\n"), e);
      else
        strcpy(p, *e ? e : "\r\n");
      free((zvoid *)zcomment);
      zcomment = p;
    }
#endif /* ?MACOS */
    free((zvoid *)e);
#else /* WINDLL */
    comment(zcomlen);
    if ((p = malloc(strlen(szCommentBuf)+1)) == NULL) {
      ZIPERR(ZE_MEM, "was setting comments to null");
    }
    if (szCommentBuf[0] != '\0')
       lstrcpy(p, szCommentBuf);
    else
       p[0] = '\0';
    free((zvoid *)zcomment);
    GlobalUnlock(hStr);
    GlobalFree(hStr);
    zcomment = p;
#endif /* WINDLL */
    zcomlen = strlen(zcomment);
  }


  /* Write central directory and end header to temporary zip */
  diag("writing central directory");
  k = 0;                        /* keep count for end header */
  c = tempzn;                   /* get start of central */
  n = t = 0;
  for (z = zfiles; z != NULL; z = z->nxt)
  {
    if ((r = putcentral(z, y)) != ZE_OK) {
      ZIPERR(r, tempzip);
    }
    tempzn += 4 + CENHEAD + z->nam + z->cext + z->com;
    n += z->len;
    t += z->siz;
    k++;
  }
  if (k == 0)
    zipwarn("zip file empty", "");
  if (verbose)
    fprintf(mesg, "total bytes=%lu, compressed=%lu -> %d%% savings\n",
           n, t, percent(n, t));
  t = tempzn - c;               /* compute length of central */
  diag("writing end of central directory");
  if ((r = putend(k, t, c, zcomlen, zcomment, y)) != ZE_OK) {
    ZIPERR(r, tempzip);
  }
  tempzf = NULL;
  if (fclose(y)) {
    ZIPERR(d ? ZE_WRITE : ZE_TEMP, tempzip);
  }
  if (x != NULL)
    fclose(x);

  /* Free some memory before spawning unzip */
#ifdef USE_ZLIB
  zl_deflate_free();
#else
  lm_free();
#endif

#ifndef WINDLL
  /* Test new zip file before overwriting old one or removing input files */
  if (test)
    check_zipfile(tempzip, argv[0]);
#endif
  /* Replace old zip file with new zip file, leaving only the new one */
  if (strcmp(zipfile, "-") && !d)
  {
    diag("replacing old zip file with new zip file");
    if ((r = replace(zipfile, tempzip)) != ZE_OK)
    {
      zipwarn("new zip file left as: ", tempzip);
      free((zvoid *)tempzip);
      tempzip = NULL;
      ZIPERR(r, "was replacing the original zip file");
    }
    free((zvoid *)tempzip);
  }
  tempzip = NULL;
  if (a && strcmp(zipfile, "-")) {
    setfileattr(zipfile, a);
#ifdef VMS
    /* If the zip file existed previously, restore its record format: */
    if (x != NULL)
      (void)VMSmunch(zipfile, RESTORE_RTYPE, NULL);
#endif
  }

#ifdef __BEOS__
  /* Set the filetype of the zipfile to "application/zip" */
  setfiletype( zipfile, "application/zip" );
#endif

#ifdef MACOS
  /* Set the Creator/Type of the zipfile to 'IZip' and 'ZIP ' */
  setfiletype(zipfile, 'IZip', 'ZIP ');
#endif

#ifdef RISCOS
  /* Set the filetype of the zipfile to &DDC */
  setfiletype(zipfile, 0xDDC);
#endif

  /* Finish up (process -o, -m, clean up).  Exit code depends on o. */
#if (!defined(VMS) && !defined(CMS_MVS))
  free((zvoid *) zipbuf);
#endif /* !VMS && !CMS_MVS */
  RETURN(finish(o ? ZE_OPEN : ZE_OK));
}


const char *BOINC_RCSID_32cef20f4f = "$Id: zip.c 7481 2005-08-25 21:33:28Z davea $";
