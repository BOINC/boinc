/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  zipfile.c by Mark Adler.
 */
#define __ZIPFILE_C

#include "zip.h"
#include "revision.h"

#ifdef VMS
#  include <rms.h>
#  include <starlet.h>
#  include "vms/vmsmunch.h"
#  include "vms/vmsdefs.h"
#endif

#ifdef __RSXNT__
#  include <windows.h>
#endif

/*
 * XXX start of zipfile.h
 */
#ifdef THEOS
/* Macros cause stack overflow in compiler */
ush SH(uch* p) { return ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8)); }
ulg LG(uch* p) { return ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16)); }
#else /* !THEOS */
/* Macros for converting integers in little-endian to machine format */
#define SH(a) ((ush)(((ush)(uch)(a)[0]) | (((ush)(uch)(a)[1]) << 8)))
#define LG(a) ((ulg)SH(a) | ((ulg)SH((a)+2) << 16))
#endif /* ?THEOS */

/* Macros for writing machine integers to little-endian format */
#define PUTSH(a,f) {putc((char)((a) & 0xff),(f)); putc((char)((a) >> 8),(f));}
#define PUTLG(a,f) {PUTSH((a) & 0xffff,(f)) PUTSH((a) >> 16,(f))}


/* -- Structure of a ZIP file -- */

/* Signatures for zip file information headers */
#define LOCSIG     0x04034b50L
#define CENSIG     0x02014b50L
#define ENDSIG     0x06054b50L
#define EXTLOCSIG  0x08074b50L

/* Offsets of values in headers */
#define LOCVER  0               /* version needed to extract */
#define LOCFLG  2               /* encrypt, deflate flags */
#define LOCHOW  4               /* compression method */
#define LOCTIM  6               /* last modified file time, DOS format */
#define LOCDAT  8               /* last modified file date, DOS format */
#define LOCCRC  10              /* uncompressed crc-32 for file */
#define LOCSIZ  14              /* compressed size in zip file */
#define LOCLEN  18              /* uncompressed size */
#define LOCNAM  22              /* length of filename */
#define LOCEXT  24              /* length of extra field */

#define EXTCRC  0               /* uncompressed crc-32 for file */
#define EXTSIZ  4               /* compressed size in zip file */
#define EXTLEN  8               /* uncompressed size */

#define CENVEM  0               /* version made by */
#define CENVER  2               /* version needed to extract */
#define CENFLG  4               /* encrypt, deflate flags */
#define CENHOW  6               /* compression method */
#define CENTIM  8               /* last modified file time, DOS format */
#define CENDAT  10              /* last modified file date, DOS format */
#define CENCRC  12              /* uncompressed crc-32 for file */
#define CENSIZ  16              /* compressed size in zip file */
#define CENLEN  20              /* uncompressed size */
#define CENNAM  24              /* length of filename */
#define CENEXT  26              /* length of extra field */
#define CENCOM  28              /* file comment length */
#define CENDSK  30              /* disk number start */
#define CENATT  32              /* internal file attributes */
#define CENATX  34              /* external file attributes */
#define CENOFF  38              /* relative offset of local header */

#define ENDDSK  0               /* number of this disk */
#define ENDBEG  2               /* number of the starting disk */
#define ENDSUB  4               /* entries on this disk */
#define ENDTOT  6               /* total number of entries */
#define ENDSIZ  8               /* size of entire central directory */
#define ENDOFF  12              /* offset of central on starting disk */
#define ENDCOM  16              /* length of zip file comment */



/* Local functions */

local int zqcmp OF((ZCONST zvoid *, ZCONST zvoid *));
local int scanzipf_reg OF((FILE *f));
#ifndef UTIL
   local int rqcmp OF((ZCONST zvoid *, ZCONST zvoid *));
   local int zbcmp OF((ZCONST zvoid *, ZCONST zvoid far *));
   local void zipoddities OF((struct zlist far *));
   local int scanzipf_fix OF((FILE *f));
#  ifdef USE_EF_UT_TIME
     local int ef_scan_ut_time OF((char *ef_buf, extent ef_len, int ef_is_cent,
                                   iztimes *z_utim));
#  endif /* USE_EF_UT_TIME */
   local void cutpath OF((char *p, int delim));
#endif /* !UTIL */

/*
 * XXX end of zipfile.h
 */

/* Local data */

#ifdef HANDLE_AMIGA_SFX
   ulg amiga_sfx_offset;        /* place where size field needs updating */
#endif


local int zqcmp(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to zip entries */
/* Used by qsort() to compare entries in the zfile list.
 * Compares the internal names z->iname */
{
  return namecmp((*(struct zlist far **)a)->iname,
                 (*(struct zlist far **)b)->iname);
}

#ifndef UTIL

local int rqcmp(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to zip entries */
/* Used by qsort() to compare entries in the zfile list.
 * Compare the internal names z->iname, but in reverse order. */
{
  return namecmp((*(struct zlist far **)b)->iname,
                 (*(struct zlist far **)a)->iname);
}


local int zbcmp(n, z)
ZCONST zvoid *n;        /* string to search for */
ZCONST zvoid far *z;    /* pointer to a pointer to a zip entry */
/* Used by search() to compare a target to an entry in the zfile list. */
{
  return namecmp((char *)n, ((struct zlist far *)z)->zname);
}


struct zlist far *zsearch(n)
ZCONST char *n;         /* name to find */
/* Return a pointer to the entry in zfile with the name n, or NULL if
   not found. */
{
  zvoid far **p;        /* result of search() */

  if (zcount &&
      (p = search(n, (ZCONST zvoid far **)zsort, zcount, zbcmp)) != NULL)
    return *(struct zlist far **)p;
  else
    return NULL;
}

#endif /* !UTIL */

#ifndef VMS
#  ifndef PATHCUT
#    define PATHCUT '/'
#  endif

char *ziptyp(s)
char *s;                /* file name to force to zip */
/* If the file name *s has a dot (other than the first char), or if
   the -A option is used (adjust self-extracting file) then return
   the name, otherwise append .zip to the name.  Allocate the space for
   the name in either case.  Return a pointer to the new name, or NULL
   if malloc() fails. */
{
  char *q;              /* temporary pointer */
  char *t;              /* pointer to malloc'ed string */
#ifdef THEOS
  char *r;              /* temporary pointer */
  char *disk;
#endif

  if ((t = malloc(strlen(s) + 5)) == NULL)
    return NULL;
  strcpy(t, s);
#ifdef __human68k__
  _toslash(t);
#endif
#ifdef MSDOS
  for (q = t; *q; INCSTR(q))
    if (*q == '\\')
      *q = '/';
#endif /* MSDOS */
#ifdef __RSXNT__   /* RSXNT/EMX C rtl uses OEM charset */
  AnsiToOem(t, t);
#endif
  if (adjust) return t;
#ifndef RISCOS
# ifndef QDOS
#  ifdef AMIGA
  if ((q = MBSRCHR(t, '/')) == NULL)
    q = MBSRCHR(t, ':');
  if (MBSRCHR((q ? q + 1 : t), '.') == NULL)
#  else /* !AMIGA */
#    ifdef THEOS
  /* the argument expansion add a dot to the end of file names when
   * there is no extension and at least one of a argument has wild cards.
   * So check for at least one character in the extension if there is a dot
   * in file name */
  if ((q = MBSRCHR((q = MBSRCHR(t, PATHCUT)) == NULL ? t : q + 1, '.')) == NULL
    || q[1] == '\0') {
#    else /* !THEOS */
#      ifdef TANDEM
  if (MBSRCHR((q = MBSRCHR(t, '.')) == NULL ? t : q + 1, ' ') == NULL)
#      else /* !TANDEM */
  if (MBSRCHR((q = MBSRCHR(t, PATHCUT)) == NULL ? t : q + 1, '.') == NULL)
#      endif /* ?TANDEM */
#    endif /* ?THEOS */
#  endif /* ?AMIGA */
#  ifdef CMS_MVS
    if (strncmp(t,"dd:",3) != 0 && strncmp(t,"DD:",3) != 0)
#  endif /* CMS_MVS */
#  ifdef THEOS
    /* insert .zip extension before disk name */
    if ((r = MBSRCHR(t, ':')) != NULL) {
        /* save disk name */
        if ((disk = strdup(r)) == NULL)
            return NULL;
        strcpy(r[-1] == '.' ? r - 1 : r, ".zip");
        strcat(t, disk);
        free(disk);
    } else {
        if (q != NULL && *q == '.')
          strcpy(q, ".zip");
        else
          strcat(t, ".zip");
    }
  }
#  else /* !THEOS */
#    ifdef TANDEM     /*  Tandem can't cope with extensions */
    strcat(t, " ZIP");
#    else /* !TANDEM */
    strcat(t, ".zip");
#    endif /* ?TANDEM */
#  endif /* ?THEOS */
# else /* QDOS */
  q = LastDir(t);
  if(MBSRCHR(q, '_') == NULL && MBSRCHR(q, '.') == NULL)
  {
      strcat(t, "_zip");
  }
# endif /* QDOS */
#endif /* !RISCOS */
  return t;
}

#else /* VMS */

# define PATHCUT ']'

char *ziptyp(s)
char *s;
{   int status;
    struct FAB fab;
    struct NAM nam;
    static char zero=0;
    char result[NAM$C_MAXRSS+1],exp[NAM$C_MAXRSS+1];
    char *p;

    fab = cc$rms_fab;
    nam = cc$rms_nam;

    fab.fab$l_fna = s;
    fab.fab$b_fns = strlen(fab.fab$l_fna);

    fab.fab$l_dna = "sys$disk:[].zip";          /* Default fspec */
    fab.fab$b_dns = strlen(fab.fab$l_dna);

    fab.fab$l_nam = &nam;

    nam.nam$l_rsa = result;                     /* Put resultant name of */
    nam.nam$b_rss = sizeof(result)-1;           /* existing zipfile here */

    nam.nam$l_esa = exp;                        /* For full spec of */
    nam.nam$b_ess = sizeof(exp)-1;              /* file to create */

    status = sys$parse(&fab);
    if( (status & 1) == 0 )
        return &zero;

    status = sys$search(&fab);
    if( status & 1 )
    {               /* Existing ZIP file */
        int l;
        if( (p=malloc( (l=nam.nam$b_rsl) + 1 )) != NULL )
        {       result[l] = 0;
                strcpy(p,result);
        }
    }
    else
    {               /* New ZIP file */
        int l;
        if( (p=malloc( (l=nam.nam$b_esl) + 1 )) != NULL )
        {       exp[l] = 0;
                strcpy(p,exp);
        }
    }
    return p;
}

#endif  /* VMS */

#ifndef UTIL

local void zipoddities(z)
struct zlist far *z;
{
    if ((z->vem >> 8) >= NUM_HOSTS)
    {
        sprintf(errbuf, "made by version %d.%d on system type %d: ",
                (ush)(z->vem & 0xff) / (ush)10, (ush)(z->vem & 0xff) % (ush)10,
                z->vem >> 8);
        zipwarn(errbuf, z->zname);
    }
    if (z->ver != 10 && z->ver != 11 && z->ver != 20)
    {
        sprintf(errbuf, "needs unzip %d.%d on system type %d: ",
                (ush)(z->ver & 0xff) / (ush)10,
                (ush)(z->ver & 0xff) % (ush)10, z->ver >> 8);
        zipwarn(errbuf, z->zname);
    }
    if (z->flg != z->lflg)
    {
        sprintf(errbuf, "local flags = 0x%04x, central = 0x%04x: ",
                z->lflg, z->flg);
        zipwarn(errbuf, z->zname);
    }
    else if (z->flg & ~0xf)
    {
        sprintf(errbuf, "undefined bits used in flags = 0x%04x: ", z->flg);
        zipwarn(errbuf, z->zname);
    }
    if (z->how > DEFLATE)
    {
        sprintf(errbuf, "unknown compression method %u: ", z->how);
        zipwarn(errbuf, z->zname);
    }
    if (z->dsk)
    {
        sprintf(errbuf, "starts on disk %u: ", z->dsk);
        zipwarn(errbuf, z->zname);
    }
    if (z->att!=ASCII && z->att!=BINARY && z->att!=__EBCDIC)
    {
        sprintf(errbuf, "unknown internal attributes = 0x%04x: ", z->att);
        zipwarn(errbuf, z->zname);
    }
#if 0
/* This test is ridiculous, it produces an error message for almost every */
/* platform of origin other than MS-DOS, Unix, VMS, and Acorn!  Perhaps   */
/* we could test "if (z->dosflag && z->atx & ~0xffL)", but what for?      */
    if (((n = z->vem >> 8) != 3) && n != 2 && n != 13 && z->atx & ~0xffL)
    {
        sprintf(errbuf, "unknown external attributes = 0x%08lx: ", z->atx);
        zipwarn(errbuf, z->zname);
    }
#endif
    if (z->ext || z->cext)
    {
        if (z->ext && z->cext && z->extra != z->cextra)
        {
          sprintf(errbuf,
                  "local extra (%ld bytes) != central extra (%ld bytes): ",
                  (ulg)z->ext, (ulg)z->cext);
          if (noisy) fprintf(stderr, "\tzip info: %s%s\n", errbuf, z->zname);
        }
#if (!defined(RISCOS) && !defined(CMS_MVS))
        /* in noisy mode, extra field sizes are always reported */
        else if (noisy)
#else /* RISCOS || CMS_MVS */
/* avoid warnings for zipfiles created on the same type of OS system! */
/* or, was this warning really intended (eg. OS/2)? */
        /* Only give info if extra bytes were added by another system */
        else if (noisy && ((z->vem >> 8) != (OS_CODE >> 8)))
#endif /* ?(RISCOS || CMS_MVS) */
        {
            fprintf(stderr, "zip info: %s has %ld bytes of %sextra data\n",
                    z->zname, z->ext ? (ulg)z->ext : (ulg)z->cext,
                    z->ext ? (z->cext ? "" : "local ") : "central ");
        }
    }
}

/*
 * scanzipf_fix is called with zip -F or zip -FF
 * read the file from front to back and pick up the pieces
 * NOTE: there are still checks missing to see if the header
 *       that was found is *VALID*
 */
local int scanzipf_fix(f)
  FILE *f;                      /* zip file */
/*
   The name of the zip file is pointed to by the global "zipfile".  The globals
   zipbeg, cenbeg, zfiles, zcount, zcomlen, zcomment, and zsort are filled in.
   Return an error code in the ZE_ class.
*/
{
    ulg a = 0L;                 /* attributes returned by filetime() */
    char b[CENHEAD];            /* buffer for central headers */
    ush flg;                    /* general purpose bit flag */
    int m;                      /* mismatch flag */
    extent n;                   /* length of name */
    ulg p;                      /* current file offset */
    ulg s;                      /* size of data, start of central */
    struct zlist far * far *x;  /* pointer last entry's link */
    struct zlist far *z;        /* current zip entry structure */

    /* Get any file attribute valid for this OS, to set in the central
     * directory when fixing the archive:
     */
#ifndef UTIL
    filetime(zipfile, &a, (long*)&s, NULL);
#endif
    x = &zfiles;                        /* first link */
    p = 0;                              /* starting file offset */
#ifdef HANDLE_AMIGA_SFX
    amiga_sfx_offset = 0L;
#endif

    /* Find start of zip structures */
    for (;;) {
      while ((m = getc(f)) != EOF && m != 0x50)    /* 0x50 == 'P' */
      {
#ifdef HANDLE_AMIGA_SFX
        if (p == 0 && m == 0)
          amiga_sfx_offset = 1L;
        else if (amiga_sfx_offset) {
          if ((p == 1 && m != 0) || (p == 2 && m != 3)
                                 || (p == 3 && (uch) m != 0xF3))
            amiga_sfx_offset = 0L;
        }
#endif /* HANDLE_AMIGA_SFX */
        p++;
      }
      b[0] = (char) m;
      if (fread(b+1, 3, 1, f) != 1 || (s = LG(b)) == LOCSIG || s == ENDSIG)
        break;
      if (fseek(f, -3L, SEEK_CUR))
        return ferror(f) ? ZE_READ : ZE_EOF;
      p++;
    }
    zipbeg = p;
#ifdef HANDLE_AMIGA_SFX
    if (amiga_sfx_offset && zipbeg >= 12 && (zipbeg & 3) == 0
        && fseek(f, -12L, SEEK_CUR) == 0 && fread(b, 12, 1, f) == 1
        && LG(b + 4) == 0xF1030000 /* 1009 in Motorola byte order */)
      amiga_sfx_offset = zipbeg - 4;
    else
      amiga_sfx_offset = 0L;
#endif /* HANDLE_AMIGA_SFX */

    /* Read local headers */
    while (LG(b) == LOCSIG)
    {
      if ((z = (struct zlist far *)farmalloc(sizeof(struct zlist))) == NULL ||
          zcount + 1 < zcount)
        return ZE_MEM;
      if (fread(b, LOCHEAD, 1, f) != 1) {
          farfree((zvoid far *)z);
          break;
      }

      z->ver = SH(LOCVER + b);
      z->vem = (ush)(dosify ? 20 : OS_CODE + Z_MAJORVER * 10 + Z_MINORVER);
      z->dosflag = dosify;
      flg = z->flg = z->lflg = SH(LOCFLG + b);
      z->how = SH(LOCHOW + b);
      z->tim = LG(LOCTIM + b);          /* time and date into one long */
      z->crc = LG(LOCCRC + b);
      z->siz = LG(LOCSIZ + b);
      z->len = LG(LOCLEN + b);
      n = z->nam = SH(LOCNAM + b);
      z->cext = z->ext = SH(LOCEXT + b);

      z->com = 0;
      z->dsk = 0;
      z->att = 0;
      z->atx = dosify ? a & 0xff : a;     /* Attributes from filetime() */
      z->mark = 0;
      z->trash = 0;

      s = fix > 1 ? 0L : z->siz; /* discard compressed size with -FF */

      /* Initialize all fields pointing to malloced data to NULL */
      z->zname = z->name = z->iname = z->extra = z->cextra = z->comment = NULL;

      /* Link into list */
      *x = z;
      z->nxt = NULL;
      x = &z->nxt;

      /* Read file name and extra field and skip data */
      if (n == 0)
      {
        sprintf(errbuf, "%lu", (ulg)zcount + 1);
        zipwarn("zero-length name for entry #", errbuf);
#ifndef DEBUG
        return ZE_FORM;
#endif
      }
      if ((z->iname = malloc(n+1)) ==  NULL ||
          (z->ext && (z->extra = malloc(z->ext)) == NULL))
        return ZE_MEM;
      if (fread(z->iname, n, 1, f) != 1 ||
          (z->ext && fread(z->extra, z->ext, 1, f) != 1) ||
          (s && fseek(f, (long)s, SEEK_CUR)))
        return ferror(f) ? ZE_READ : ZE_EOF;
      /* If there is an extended local header, s is either 0 or
       * the correct compressed size.
       */
      z->iname[n] = '\0';               /* terminate name */
      z->zname = in2ex(z->iname);       /* convert to external name */
      if (z->zname == NULL)
        return ZE_MEM;
      z->name = z->zname;
      z->cextra = z->extra;
      if (noisy) fprintf(mesg, "zip: reading %s\n", z->zname);

      /* Save offset, update for next header */
      z->off = p;
      p += 4 + LOCHEAD + n + z->ext + s;
      zcount++;

      /* Skip extended local header if there is one */
      if ((flg & 8) != 0) {
        /* Skip the compressed data if compressed size is unknown.
         * For safety, we should use the central directory.
         */
        if (s == 0) {
          for (;;) {
            while ((m = getc(f)) != EOF && m != 0x50) ;  /* 0x50 == 'P' */
            b[0] = (char) m;
            if (fread(b+1, 15, 1, f) != 1 || LG(b) == EXTLOCSIG)
              break;
            if (fseek(f, -15L, SEEK_CUR))
              return ferror(f) ? ZE_READ : ZE_EOF;
          }
          s = LG(4 + EXTSIZ + b);
          p += s;
          if ((ulg) ftell(f) != p+16L) {
            zipwarn("bad extended local header for ", z->zname);
            return ZE_FORM;
          }
        } else {
          /* compressed size non-zero, assume that it is valid: */
          Assert(p == ftell(f), "bad compressed size with extended header");

          if (fseek(f, p, SEEK_SET) || fread(b, 16, 1, f) != 1)
            return ferror(f) ? ZE_READ : ZE_EOF;
          if (LG(b) != EXTLOCSIG) {
            zipwarn("extended local header not found for ", z->zname);
            return ZE_FORM;
          }
        }
        /* overwrite the unknown values of the local header: */

        /* already in host format */
        z->crc = LG(4 + EXTCRC + b);
        z->siz = s;
        z->len = LG(4 + EXTLEN + b);

        p += 16L;
      }
      else if (fix > 1) {
        /* Don't trust the compressed size */
        for (;;) {
          while ((m = getc(f)) != EOF && m != 0x50) p++; /* 0x50 == 'P' */
          b[0] = (char) m;
          if (fread(b+1, 3, 1, f) != 1 || (s = LG(b)) == LOCSIG || s == CENSIG)
            break;
          if (fseek(f, -3L, SEEK_CUR))
            return ferror(f) ? ZE_READ : ZE_EOF;
          p++;
        }
        s = p - (z->off + 4 + LOCHEAD + n + z->ext);
        if (s != z->siz) {
          fprintf(mesg, " compressed size %ld, actual size %ld for %s\n",
                  z->siz, s, z->zname);
          z->siz = s;
        }
        /* next LOCSIG already read at this point, don't read it again: */
        continue;
      }

      /* Read next signature */
      if (fread(b, 4, 1, f) != 1)
          break;
    }

    s = p;                              /* save start of central */

    if (LG(b) != CENSIG && noisy) {
      fprintf(mesg, "zip warning: %s %s truncated.\n", zipfile,
              fix > 1 ? "has been" : "would be");

      if (fix == 1) {
        fprintf(mesg,
   "Retry with option -qF to truncate, with -FF to attempt full recovery\n");
        ZIPERR(ZE_FORM, NULL);
      }
    }

    cenbeg = s;

    if (zipbeg && noisy)
      fprintf(mesg, "%s: adjusting offsets for a preamble of %lu bytes\n",
              zipfile, zipbeg);

    return ZE_OK;
}

#endif /* !UTIL */

/*
 * scanzipf_reg starts searching for the End Signature at the end of the file
 * The End Signature points to the Central Directory Signature which points
 * to the Local Directory Signature
 * XXX probably some more consistency checks are needed
 */
local int scanzipf_reg(f)
  FILE *f;                      /* zip file */
/*
   The name of the zip file is pointed to by the global "zipfile".  The globals
   zipbeg, cenbeg, zfiles, zcount, zcomlen, zcomment, and zsort are filled in.
   Return an error code in the ZE_ class.
*/
{
    char b[CENHEAD];            /* buffer for central headers */
    ush flg;                    /* general purpose bit flag */
    int m;                      /* mismatch flag */
    extent n;                   /* length of name */
    struct zlist far * far *x;  /* pointer last entry's link */
    struct zlist far *z;        /* current zip entry structure */
    char *t;                    /* temporary pointer */
    char far *u;                /* temporary variable */
    int found;
    char *buf;                  /* temp buffer for reading zipfile */
    long deltaoff;

    buf = malloc(4096 + 4);
    if (buf == NULL)
      return ZE_MEM;

#ifdef HANDLE_AMIGA_SFX
    amiga_sfx_offset = (fread(buf, 1, 4, f) == 4 && LG(buf) == 0xF3030000);
    /* == 1 if this file is an Amiga executable (presumably UnZipSFX) */
#endif
    found = 0;
    t = &buf[4096];
    t[1] = '\0';
    t[2] = '\0';
    t[3] = '\0';
    if (fseek(f, -4096L, SEEK_END) == 0) {
      zipbeg = (ulg) (ftell(f) + 4096L);
      while (!found && zipbeg >= 4096) {
        zipbeg -= 4096L;
        buf[4096] = t[1];
        buf[4097] = t[2];
        buf[4098] = t[3];
/*
 * XXX error check ??
 */
        fread(buf, 1, 4096, f);
        fseek(f, -8192L, SEEK_CUR);
        t = &buf[4095];
/*
 * XXX far pointer arithmetic in DOS
 */
        while (t >= buf) {
          /* Check for ENDSIG ("PK\5\6" in ASCII) */
          if (LG(t) == ENDSIG) {
            found = 1;
/*
 * XXX error check ??
 * XXX far pointer arithmetic in DOS
 */
            zipbeg += (ulg) (t - buf);
            fseek(f, (long) zipbeg + 4L, SEEK_SET);
            break;
          }
          --t;
        }
      }
    }
    else
       zipbeg = 4096L;
/*
 * XXX warn: garbage at the end of the file ignored
 */
    if (!found && zipbeg > 0) {
      size_t s;

      fseek(f, 0L, SEEK_SET);
      clearerr(f);
      s = fread(buf, 1, (size_t) zipbeg, f);
      buf[s] = t[1];
      buf[s + 1] = t[2];
      buf[s + 2] = t[3];
      t = &buf[s - 1];
/*
 * XXX far pointer comparison in DOS
 */
      while (t >= buf) {
        /* Check for ENDSIG ("PK\5\6" in ASCII) */
        if (LG(t) == ENDSIG) {
          found = 1;
/*
 * XXX far pointer arithmetic in DOS
 */
          zipbeg = (ulg) (t - buf);
          fseek(f, (long) zipbeg + 4L, SEEK_SET);
          break;
        }
        --t;
      }
    }
    free(buf);
    if (!found) {
      zipwarn("missing end signature--probably not a zip file (did you", "");
      zipwarn("remember to use binary mode when you transferred it?)", "");
      return ZE_FORM;
    }
    /* Read end header */
    if (fread(b, ENDHEAD, 1, f) != 1)
      return ferror(f) ? ZE_READ : ZE_EOF;
    if (SH(ENDDSK + b) || SH(ENDBEG + b) ||
        SH(ENDSUB + b) != SH(ENDTOT + b))
      zipwarn("multiple disk information ignored", "");
    zcomlen = SH(ENDCOM + b);
    if (zcomlen)
    {
      if ((zcomment = malloc(zcomlen)) == NULL)
        return ZE_MEM;
      if (fread(zcomment, zcomlen, 1, f) != 1)
      {
        free((zvoid *)zcomment);
        zcomment = NULL;
        return ferror(f) ? ZE_READ : ZE_EOF;
      }
#ifdef EBCDIC
      if (zcomment)
         memtoebc(zcomment, zcomment, zcomlen);
#endif /* EBCDIC */
    }
/*
 * XXX assumes central header immediately precedes end header
 */
    cenbeg = zipbeg - LG(ENDSIZ + b);
    deltaoff = adjust ? cenbeg - LG(b + ENDOFF) : 0L;
    if (fseek(f, cenbeg, SEEK_SET) != 0) {
        perror("fseek");
        return ZE_FORM; /* XXX */
    }

    x = &zfiles;                        /* first link */

    if (fread(b, 4, 1, f) != 1)
      return ferror(f) ? ZE_READ : ZE_EOF;

    while (LG(b) == CENSIG) {
      /* Read central header. The portion of the central header that should
         be in common with local header is read raw, for later comparison.
         (this requires that the offset of ext in the zlist structure
         be greater than or equal to LOCHEAD) */
      if (fread(b, CENHEAD, 1, f) != 1)
        return ferror(f) ? ZE_READ : ZE_EOF;
      if ((z = (struct zlist far *)farmalloc(sizeof(struct zlist))) == NULL)
        return ZE_MEM;
      z->vem = SH(CENVEM + b);
      for (u = (char far *)(&(z->ver)), n = 0; n < (CENNAM-CENVER); n++)
        u[n] = b[CENVER + n];
      z->nam = SH(CENNAM + b);          /* used before comparing cen vs. loc */
      z->cext = SH(CENEXT + b);         /* may be different from z->ext */
      z->com = SH(CENCOM + b);
      z->dsk = SH(CENDSK + b);
      z->att = SH(CENATT + b);
      z->atx = LG(CENATX + b);
      z->off = LG(CENOFF + b) + deltaoff;
      z->dosflag = (z->vem & 0xff00) == 0;

      /* Initialize all fields pointing to malloced data to NULL */
      z->zname = z->name = z->iname = z->extra = z->cextra = z->comment = NULL;

      /* Link into list */
      *x = z;
      z->nxt = NULL;
      x = &z->nxt;

      /* Read file name, extra field and comment field */
      if (z->nam == 0)
      {
        sprintf(errbuf, "%lu", (ulg)zcount + 1);
        zipwarn("zero-length name for entry #", errbuf);
#ifndef DEBUG
        farfree((zvoid far *)z);
        return ZE_FORM;
#endif
      }
      if ((z->iname = malloc(z->nam+1)) ==  NULL ||
          (z->cext && (z->cextra = malloc(z->cext)) == NULL) ||
          (z->com && (z->comment = malloc(z->com)) == NULL))
        return ZE_MEM;
      if (fread(z->iname, z->nam, 1, f) != 1 ||
          (z->cext && fread(z->cextra, z->cext, 1, f) != 1) ||
          (z->com && fread(z->comment, z->com, 1, f) != 1))
        return ferror(f) ? ZE_READ : ZE_EOF;
      z->iname[z->nam] = '\0';                  /* terminate name */
#ifdef EBCDIC
      if (z->com)
         memtoebc(z->comment, z->comment, z->com);
#endif /* EBCDIC */
      /* Update zipbeg offset, prepare for next header */
      if (z->off < zipbeg)
         zipbeg = z->off;
      zcount++;
      /* Read next signature */
      if (fread(b, 4, 1, f) != 1)
          return ferror(f) ? ZE_READ : ZE_EOF;
    }

    /* Point to start of header list and read local headers */
    z = zfiles;
    while (z != NULL) {
      /* Read next signature */
      if (fseek(f, z->off, SEEK_SET) != 0 || fread(b, 4, 1, f) != 1)
        return ferror(f) ? ZE_READ : ZE_EOF;
      if (LG(b) == LOCSIG) {
        if (fread(b, LOCHEAD, 1, f) != 1)
            return ferror(f) ? ZE_READ : ZE_EOF;
        z->lflg = SH(LOCFLG + b);
        n = SH(LOCNAM + b);
        z->ext = SH(LOCEXT + b);

        /* Compare name and extra fields */
        if (n != z->nam)
        {
#ifdef EBCDIC
          strtoebc(z->iname, z->iname);
#endif
          zipwarn("name lengths in local and central differ for ", z->iname);
          return ZE_FORM;
        }
        if ((t = malloc(z->nam)) == NULL)
          return ZE_MEM;
        if (fread(t, z->nam, 1, f) != 1)
        {
          free((zvoid *)t);
          return ferror(f) ? ZE_READ : ZE_EOF;
        }
        if (memcmp(t, z->iname, z->nam))
        {
          free((zvoid *)t);
#ifdef EBCDIC
          strtoebc(z->iname, z->iname);
#endif
          zipwarn("names in local and central differ for ", z->iname);
          return ZE_FORM;
        }
        free((zvoid *)t);
        if (z->ext)
        {
          if ((z->extra = malloc(z->ext)) == NULL)
            return ZE_MEM;
          if (fread(z->extra, z->ext, 1, f) != 1)
          {
            free((zvoid *)(z->extra));
            return ferror(f) ? ZE_READ : ZE_EOF;
          }
          if (z->ext == z->cext && memcmp(z->extra, z->cextra, z->ext) == 0)
          {
            free((zvoid *)(z->extra));
            z->extra = z->cextra;
          }
        }

        /* Check extended local header if there is one */
        if ((z->lflg & 8) != 0)
        {
          char buf2[16];
          ulg s;                        /* size of compressed data */

          s = LG(LOCSIZ + b);
          if (s == 0)
            s = LG((CENSIZ-CENVER) + (char far *)(&(z->ver)));
          if (fseek(f, (z->off + (4+LOCHEAD) + z->nam + z->ext + s), SEEK_SET)
              || (fread(buf2, 16, 1, f) != 1))
            return ferror(f) ? ZE_READ : ZE_EOF;
          if (LG(buf2) != EXTLOCSIG)
          {
#ifdef EBCDIC
            strtoebc(z->iname, z->iname);
#endif
            zipwarn("extended local header not found for ", z->iname);
            return ZE_FORM;
          }
          /* overwrite the unknown values of the local header: */
          for (n = 0; n < 12; n++)
            b[LOCCRC+n] = buf2[4+n];
        }

        /* Compare local header with that part of central header (except
           for the reserved bits in the general purpose flags and except
           for the already checked entry name length */
        u = (char far *)(&(z->ver));
        flg = SH((CENFLG-CENVER) + u);          /* Save central flags word */
        u[CENFLG-CENVER+1] &= 0x1f;             /* Mask reserved flag bits */
        b[LOCFLG+1] &= 0x1f;
        for (m = 0, n = 0; n < LOCNAM; n++)
          if (b[n] != u[n])
          {
            if (!m)
            {
              zipwarn("local and central headers differ for ", z->zname);
              m = 1;
            }
            if (noisy)
            {
              sprintf(errbuf, " offset %u--local = %02x, central = %02x",
                      (unsigned)n, (uch)b[n], (uch)u[n]);
              zipwarn(errbuf, "");
            }
          }
        if (m && !adjust)
          return ZE_FORM;

        /* Complete the setup of the zlist entry by translating the remaining
         * central header fields in memory, starting with the fields with
         * highest offset. This order of the conversion commands takes into
         * account potential buffer overlaps caused by structure padding.
         */
        z->len = LG((CENLEN-CENVER) + u);
        z->siz = LG((CENSIZ-CENVER) + u);
        z->crc = LG((CENCRC-CENVER) + u);
        z->tim = LG((CENTIM-CENVER) + u);   /* time and date into one long */
        z->how = SH((CENHOW-CENVER) + u);
        z->flg = flg;                       /* may be different from z->lflg */
        z->ver = SH((CENVER-CENVER) + u);

        /* Clear actions */
        z->mark = 0;
        z->trash = 0;
#ifdef UTIL
/* We only need z->iname in the utils */
        z->name = z->iname;
#ifdef EBCDIC
/* z->zname is used for printing and must be coded in native charset */
        if ((z->zname = malloc(z->nam+1)) ==  NULL)
          return ZE_MEM;
        strtoebc(z->zname, z->iname);
#else
        z->zname = z->iname;
#endif
#else /* !UTIL */
        z->zname = in2ex(z->iname);       /* convert to external name */
        if (z->zname == NULL)
          return ZE_MEM;
        z->name = z->zname;
#endif /* ?UTIL */
      }
      else {
#ifdef EBCDIC
        strtoebc(z->iname, z->iname);
#endif
        zipwarn("local header not found for ", z->iname);
        return ZE_FORM;
      }
#ifndef UTIL
      if (verbose)
        zipoddities(z);
#endif
      z = z->nxt;
    }

    if (zipbeg && noisy)
      fprintf(mesg, "%s: %s a preamble of %lu bytes\n",
              zipfile, adjust ? "adjusting offsets for" : "found", zipbeg);

#ifdef HANDLE_AMIGA_SFX
    if (zipbeg < 12 || (zipbeg & 3) != 0 /* must be longword aligned */)
      amiga_sfx_offset = 0;
    else if (amiga_sfx_offset) {
      char buf2[16];
      if (!fseek(f, zipbeg - 12, SEEK_SET) && fread(buf2, 12, 1, f) == 1) {
        if (LG(buf2 + 4) == 0xF1030000 /* 1009 in Motorola byte order */)
          /* could also check if LG(buf2) == 0xF2030000... no for now */
          amiga_sfx_offset = zipbeg - 4;
        else
          amiga_sfx_offset = 0L;
      }
    }
#endif /* HANDLE_AMIGA_SFX */
    return ZE_OK;
}


/*
 * readzipfile initializes the global variables that hold the zipfile
 * directory info and opens the zipfile. For the actual zipfile scan,
 * the subroutine scanzipf_reg() or scanzipf_fix() is called,
 * depending on the mode of operation (regular processing, or zipfix mode).
 */
int readzipfile()
/*
   The name of the zip file is pointed to by the global "zipfile".
   The globals zipbeg, zfiles, zcount, and zcomlen are initialized.
   Return an error code in the ZE_ class.
*/
{
  FILE *f;              /* zip file */
  int retval;           /* return code */
  int readable;         /* 1 if zipfile exists and is readable */

  /* Initialize zip file info */
  zipbeg = 0;
  zfiles = NULL;                        /* Points to first header */
  zcount = 0;                           /* number of files */
  zcomlen = 0;                          /* zip file comment length */
  retval = ZE_OK;
  f = NULL;                             /* shut up some compilers */

  /* If zip file exists, read headers and check structure */
#ifdef VMS
  if (zipfile == NULL || !(*zipfile) || !strcmp(zipfile, "-"))
    return ZE_OK;
  {
    int rtype;

    if ((VMSmunch(zipfile, GET_RTYPE, (char *)&rtype) == RMS$_NORMAL) &&
        (rtype == FAT$C_VARIABLE)) {
      fprintf(stderr,
     "\n     Error:  zipfile is in variable-length record format.  Please\n\
     run \"bilf b %s\" to convert the zipfile to fixed-length\n\
     record format.\n\n", zipfile);
      return ZE_FORM;
    }
  }
  readable = ((f = fopen(zipfile, FOPR)) != NULL);
#else /* !VMS */
  readable = (zipfile != NULL && *zipfile && strcmp(zipfile, "-") &&
              (f = fopen(zipfile, FOPR)) != NULL);
#endif /* ?VMS */
#ifdef MVS
  /* Very nasty special case for MVS.  Just because the zipfile has been
   * opened for reading does not mean that we can actually read the data.
   * Typical JCL to create a zipfile is
   *
   * //ZIPFILE  DD  DISP=(NEW,CATLG),DSN=prefix.ZIP,
   * //             SPACE=(CYL,(10,10))
   *
   * That creates a VTOC entry with an end of file marker (DS1LSTAR) of zero.
   * Alas the VTOC end of file marker is only used when the file is opened in
   * append mode.  When a file is opened in read mode, the "other" end of file
   * marker is used, a zero length data block signals end of file when reading.
   * With a brand new file which has not been written to yet, it is undefined
   * what you read off the disk.  In fact you read whatever data was in the same
   * disk tracks before the zipfile was allocated.  You would be amazed at the
   * number of application programmers who still do not understand this.  Makes
   * for interesting and semi-random errors, GIGO.
   *
   * Newer versions of SMS will automatically write a zero length block when a
   * file is allocated.  However not all sites run SMS or they run older levels
   * so we cannot rely on that.  The only safe thing to do is close the file,
   * open in append mode (we already know that the file exists), close it again,
   * reopen in read mode and try to read a data block.  Opening and closing in
   * append mode will write a zero length block where DS1LSTAR points, making
   * sure that the VTOC and internal end of file markers are in sync.  Then it
   * is safe to read data.  If we cannot read one byte of data after all that,
   * it is a brand new zipfile and must not be read.
   */
  if (readable)
  {
    char c;
    fclose(f);
    /* append mode */
    if ((f = fopen(zipfile, "ab")) == NULL) {
      ZIPERR(ZE_OPEN, zipfile);
    }
    fclose(f);
    /* read mode again */
    if ((f = fopen(zipfile, FOPR)) == NULL) {
      ZIPERR(ZE_OPEN, zipfile);
    }
    if (fread(&c, 1, 1, f) != 1) {
      /* no actual data */
      readable = 0;
      fclose(f);
    }
    else{
      fseek(f, 0, SEEK_SET);  /* at least one byte in zipfile, back to the start */
    }
  }
#endif /* MVS */
  if (readable)
  {
#ifndef UTIL
    retval = (fix && !adjust) ? scanzipf_fix(f) : scanzipf_reg(f);
#else
    retval = scanzipf_reg(f);
#endif

    /* Done with zip file for now */
    fclose(f);

    /* If one or more files, sort by name */
    if (zcount)
    {
      struct zlist far * far *x;    /* pointer into zsort array */
      struct zlist far *z;          /* pointer into zfiles linked list */
      extent zl_size = zcount * sizeof(struct zlist far *);

      if (zl_size / sizeof(struct zlist far *) != zcount ||
          (x = zsort = (struct zlist far **)malloc(zl_size)) == NULL)
        return ZE_MEM;
      for (z = zfiles; z != NULL; z = z->nxt)
        *x++ = z;
      qsort((char *)zsort, zcount, sizeof(struct zlist far *), zqcmp);
    }
  }
  return retval;
}


int putlocal(z, f)
struct zlist far *z;    /* zip entry to write local header for */
FILE *f;                /* file to write to */
/* Write a local header described by *z to file *f.  Return an error code
   in the ZE_ class. */
{
  PUTLG(LOCSIG, f);
  PUTSH(z->ver, f);
  PUTSH(z->lflg, f);
  PUTSH(z->how, f);
  PUTLG(z->tim, f);
  PUTLG(z->crc, f);
  PUTLG(z->siz, f);
  PUTLG(z->len, f);
  PUTSH(z->nam, f);
  PUTSH(z->ext, f);
  if (fwrite(z->iname, 1, z->nam, f) != z->nam ||
      (z->ext && fwrite(z->extra, 1, z->ext, f) != z->ext))
    return ZE_TEMP;
  return ZE_OK;
}

int putextended(z, f)
struct zlist far *z;    /* zip entry to write local header for */
FILE *f;                /* file to write to */
/* Write an extended local header described by *z to file *f.
 * Return an error code in the ZE_ class. */
{
  PUTLG(EXTLOCSIG, f);
  PUTLG(z->crc, f);
  PUTLG(z->siz, f);
  PUTLG(z->len, f);
  return ZE_OK;
}

int putcentral(z, f)
struct zlist far *z;    /* zip entry to write central header for */
FILE *f;                /* file to write to */
/* Write a central header described by *z to file *f.  Return an error code
   in the ZE_ class. */
{
  PUTLG(CENSIG, f);
  PUTSH(z->vem, f);
  PUTSH(z->ver, f);
  PUTSH(z->flg, f);
  PUTSH(z->how, f);
  PUTLG(z->tim, f);
  PUTLG(z->crc, f);
  PUTLG(z->siz, f);
  PUTLG(z->len, f);
  PUTSH(z->nam, f);
  PUTSH(z->cext, f);
  PUTSH(z->com, f);
  PUTSH(z->dsk, f);
  PUTSH(z->att, f);
  PUTLG(z->atx, f);
  PUTLG(z->off, f);
#ifdef EBCDIC
  if (z->com)
    memtoasc(z->comment, z->comment, z->com);
#endif /* EBCDIC */
  if (fwrite(z->iname, 1, z->nam, f) != z->nam ||
      (z->cext && fwrite(z->cextra, 1, z->cext, f) != z->cext) ||
      (z->com && fwrite(z->comment, 1, z->com, f) != z->com))
    return ZE_TEMP;
  return ZE_OK;
}


int putend(n, s, c, m, z, f)
int n;                  /* number of entries in central directory */
ulg s;                  /* size of central directory */
ulg c;                  /* offset of central directory */
extent m;               /* length of zip file comment (0 if none) */
char *z;                /* zip file comment if m != 0 */
FILE *f;                /* file to write to */
/* Write the end of central directory data to file *f.  Return an error code
   in the ZE_ class. */
{
  PUTLG(ENDSIG, f);
  PUTSH(0, f);
  PUTSH(0, f);
  PUTSH(n, f);
  PUTSH(n, f);
  PUTLG(s, f);
  PUTLG(c, f);
  PUTSH(m, f);
/* Write the comment, if any */
#ifdef EBCDIC
  memtoasc(z, z, m);
#endif
  if (m && fwrite(z, 1, m, f) != m)
    return ZE_TEMP;

#ifdef HANDLE_AMIGA_SFX
  if (amiga_sfx_offset && zipbeg /* -J zeroes this */) {
    s = ftell(f);
    while (s & 3) s++, putc(0, f);   /* final marker must be longword aligned */
    PUTLG(0xF2030000 /* 1010 in Motorola byte order */, f);
    c = (s - amiga_sfx_offset - 4) / 4;  /* size of archive part in longwords */
    if (fseek(f, amiga_sfx_offset, SEEK_SET) != 0)
      return ZE_TEMP;
    c = ((c >> 24) & 0xFF) | ((c >> 8) & 0xFF00)
         | ((c & 0xFF00) << 8) | ((c & 0xFF) << 24);     /* invert byte order */
    PUTLG(c, f);
    fseek(f, 0, SEEK_END);                                    /* just in case */
  }
#endif
  return ZE_OK;
}


/* Note: a zip "entry" includes a local header (which includes the file
   name), an encryption header if encrypting, the compressed data
   and possibly an extended local header. */

int zipcopy(z, x, y)
struct zlist far *z;    /* zip entry to copy */
FILE *x, *y;            /* source and destination files */
/* Copy the zip entry described by *z from file *x to file *y.  Return an
   error code in the ZE_ class.  Also update tempzn by the number of bytes
   copied. */
{
  ulg n;                /* holds local header offset */

  Trace((stderr, "zipcopy %s\n", z->zname));
  n = (ulg)(4 + LOCHEAD) + (ulg)z->nam + (ulg)z->ext;

  if (fix > 1) {
    if (fseek(x, z->off + n, SEEK_SET)) /* seek to compressed data */
      return ferror(x) ? ZE_READ : ZE_EOF;

    if (fix > 2) {
      /* Update length of entry's name, it may have been changed.  This is
         needed to support the ZipNote ability to rename archive entries. */
      z->nam = strlen(z->iname);
      n = (ulg)(4 + LOCHEAD) + (ulg)z->nam + (ulg)z->ext;
    }

    /* do not trust the old compressed size */
    if (putlocal(z, y) != ZE_OK)
      return ZE_TEMP;

    z->off = tempzn;
    tempzn += n;
    n = z->siz;
  } else {
    if (fseek(x, z->off, SEEK_SET))     /* seek to local header */
      return ferror(x) ? ZE_READ : ZE_EOF;

    z->off = tempzn;
    n += z->siz;
  }
  /* copy the compressed data and the extended local header if there is one */
  if (z->lflg & 8) n += 16;
  tempzn += n;
  return fcopy(x, y, n);
}


#ifndef UTIL

#ifdef USE_EF_UT_TIME

local int ef_scan_ut_time(ef_buf, ef_len, ef_is_cent, z_utim)
char *ef_buf;                   /* buffer containing extra field */
extent ef_len;                  /* total length of extra field */
int ef_is_cent;                 /* flag indicating "is central extra field" */
iztimes *z_utim;                /* return storage: atime, mtime, ctime */
/* This function scans the extra field for EF_TIME or EF_IZUNIX blocks
 * containing Unix style time_t (GMT) values for the entry's access, creation
 * and modification time.
 * If a valid block is found, all time stamps are copied to the iztimes
 * structure.
 * The presence of an EF_TIME or EF_IZUNIX2 block results in ignoring
 * all data from probably present obsolete EF_IZUNIX blocks.
 * If multiple blocks of the same type are found, only the information from
 * the last block is used.
 * The return value is the EF_TIME Flags field (simulated in case of an
 * EF_IZUNIX block) or 0 in case of failure.
 */
{
  int flags = 0;
  unsigned eb_id;
  extent eb_len;
  int have_new_type_eb = FALSE;

  if (ef_len == 0 || ef_buf == NULL)
    return 0;

  Trace((stderr,"\nef_scan_ut_time: scanning extra field of length %u\n",
         ef_len));
  while (ef_len >= EB_HEADSIZE) {
    eb_id = SH(EB_ID + ef_buf);
    eb_len = SH(EB_LEN + ef_buf);

    if (eb_len > (ef_len - EB_HEADSIZE)) {
      /* Discovered some extra field inconsistency! */
      Trace((stderr,"ef_scan_ut_time: block length %u > rest ef_size %u\n",
             eb_len, ef_len - EB_HEADSIZE));
      break;
    }

    switch (eb_id) {
      case EF_TIME:
        flags &= ~0x00ff;       /* ignore previous IZUNIX or EF_TIME fields */
        have_new_type_eb = TRUE;
        if ( eb_len >= EB_UT_MINLEN && z_utim != NULL) {
           unsigned eb_idx = EB_UT_TIME1;
           Trace((stderr,"ef_scan_ut_time: Found TIME extra field\n"));
           flags |= (ef_buf[EB_HEADSIZE+EB_UT_FLAGS] & 0x00ff);
           if ((flags & EB_UT_FL_MTIME)) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->mtime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 eb_idx += 4;
                 Trace((stderr,"  Unix EF modtime = %ld\n", z_utim->mtime));
              } else {
                 flags &= ~EB_UT_FL_MTIME;
                 Trace((stderr,"  Unix EF truncated, no modtime\n"));
              }
           }
           if (ef_is_cent) {
              break;            /* central version of TIME field ends here */
           }
           if (flags & EB_UT_FL_ATIME) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->atime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 eb_idx += 4;
                 Trace((stderr,"  Unix EF acctime = %ld\n", z_utim->atime));
              } else {
                 flags &= ~EB_UT_FL_ATIME;
              }
           }
           if (flags & EB_UT_FL_CTIME) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->ctime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 /* eb_idx += 4; */  /* superfluous for now ... */
                 Trace((stderr,"  Unix EF cretime = %ld\n", z_utim->ctime));
              } else {
                 flags &= ~EB_UT_FL_CTIME;
              }
           }
        }
        break;

      case EF_IZUNIX2:
        if (!have_new_type_eb) {
           flags &= ~0x00ff;    /* ignore any previous IZUNIX field */
           have_new_type_eb = TRUE;
        }
        break;

      case EF_IZUNIX:
        if (eb_len >= EB_UX_MINLEN) {
           Trace((stderr,"ef_scan_ut_time: Found IZUNIX extra field\n"));
           if (have_new_type_eb) {
              break;            /* Ignore IZUNIX extra field block ! */
           }
           z_utim->atime = LG((EB_HEADSIZE+EB_UX_ATIME) + ef_buf);
           z_utim->mtime = LG((EB_HEADSIZE+EB_UX_MTIME) + ef_buf);
           Trace((stderr,"  Unix EF access time = %ld\n",z_utim->atime));
           Trace((stderr,"  Unix EF modif. time = %ld\n",z_utim->mtime));
           flags |= (EB_UT_FL_MTIME | EB_UT_FL_ATIME);  /* signal success */
        }
        break;

      case EF_THEOS:
/*      printf("Not implemented yet\n"); */
        break;

      default:
        break;
    }
    /* Skip this extra field block */
    ef_buf += (eb_len + EB_HEADSIZE);
    ef_len -= (eb_len + EB_HEADSIZE);
  }

  return flags;
}

int get_ef_ut_ztime(z, z_utim)
struct zlist far *z;
iztimes *z_utim;
{
  int r;

#ifdef IZ_CHECK_TZ
  if (!zp_tz_is_valid) return 0;
#endif

  /* First, scan local extra field. */
  r = ef_scan_ut_time(z->extra, z->ext, FALSE, z_utim);

  /* If this was not successful, try central extra field, but only if
     it is really different. */
  if (!r && z->cext > 0 && z->cextra != z->extra)
    r = ef_scan_ut_time(z->cextra, z->cext, TRUE, z_utim);

  return r;
}

#endif /* USE_EF_UT_TIME */


local void cutpath(p, delim)
char *p;                /* path string */
int delim;              /* path component separator char */
/* Cut the last path component off the name *p in place.
 * This should work on both internal and external names.
 */
{
  char *r;              /* pointer to last path delimiter */

#ifdef VMS                      /* change [w.x.y]z to [w.x]y.DIR */
  if ((r = MBSRCHR(p, ']')) != NULL)
  {
    *r = 0;
    if ((r = MBSRCHR(p, '.')) != NULL)
    {
      *r = ']';
      strcat(r, ".DIR;1");     /* this assumes a little padding--see PAD */
    } else {
      *p = 0;
    }
  } else {
    if ((r = MBSRCHR(p, delim)) != NULL)
      *r = 0;
    else
      *p = 0;
  }
#else /* !VMS */
  if ((r = MBSRCHR(p, delim)) != NULL)
    *r = 0;
  else
    *p = 0;
#endif /* ?VMS */
}

int trash()
/* Delete the compressed files and the directories that contained the deleted
   files, if empty.  Return an error code in the ZE_ class.  Failure of
   destroy() or deletedir() is ignored. */
{
  extent i;             /* counter on deleted names */
  extent n;             /* number of directories to delete */
  struct zlist far **s; /* table of zip entries to handle, sorted */
  struct zlist far *z;  /* current zip entry */

  /* Delete marked names and count directories */
  n = 0;
  for (z = zfiles; z != NULL; z = z->nxt)
    if (z->mark == 1 || z->trash)
    {
      z->mark = 1;
      if (z->iname[z->nam - 1] != (char)0x2f) { /* don't unlink directory */
        if (verbose)
          fprintf(mesg, "zip diagnostic: deleting file %s\n", z->name);
        if (destroy(z->name)) {
          zipwarn("error deleting ", z->name);
        }
        /* Try to delete all paths that lead up to marked names. This is
         * necessary only with the -D option.
         */
        if (!dirnames) {
          cutpath(z->name, '/');  /* XXX wrong ??? */
          cutpath(z->iname, 0x2f); /* 0x2f = ascii['/'] */
          z->nam = strlen(z->iname);
          if (z->nam > 0) {
            z->iname[z->nam - 1] = (char)0x2f;
            z->iname[z->nam++] = '\0';
          }
          if (z->nam > 0) n++;
        }
      } else {
        n++;
      }
    }

  /* Construct the list of all marked directories. Some may be duplicated
   * if -D was used.
   */
  if (n)
  {
    if ((s = (struct zlist far **)malloc(n*sizeof(struct zlist far *))) ==
        NULL)
      return ZE_MEM;
    n = 0;
    for (z = zfiles; z != NULL; z = z->nxt) {
      if (z->mark && z->nam > 0 && z->iname[z->nam - 1] == (char)0x2f /* '/' */
          && (n == 0 || strcmp(z->name, s[n-1]->name) != 0)) {
        s[n++] = z;
      }
    }
    /* Sort the files in reverse order to get subdirectories first.
     * To avoid problems with strange naming conventions as in VMS,
     * we sort on the internal names, so x/y/z will always be removed
     * before x/y. On VMS, x/y/z > x/y but [x.y.z] < [x.y]
     */
    qsort((char *)s, n, sizeof(struct zlist far *), rqcmp);

    for (i = 0; i < n; i++) {
      char *p = s[i]->name;
      if (*p == '\0') continue;
      if (p[strlen(p) - 1] == '/') { /* keep VMS [x.y]z.dir;1 intact */
        p[strlen(p) - 1] = '\0';
      }
      if (i == 0 || strcmp(s[i]->name, s[i-1]->name) != 0) {
        if (verbose) {
          fprintf(mesg, "deleting directory %s (if empty)                \n",
                  s[i]->name);
        }
        deletedir(s[i]->name);
      }
    }
    free((zvoid *)s);
  }
  return ZE_OK;
}

#endif /* !UTIL */
