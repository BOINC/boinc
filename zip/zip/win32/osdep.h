/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/* Automatic setting of the common Microsoft C idenfifier MSC.
 * NOTE: Watcom also defines M_I*86 !
 */
#if defined(_MSC_VER) || (defined(M_I86) && !defined(__WATCOMC__))
#  ifndef MSC
#    define MSC                 /* This should work for older MSC, too!  */
#  endif
#endif

#if defined(__WATCOMC__) && defined(__386__)
#  define WATCOMC_386
#endif

#if (defined(__CYGWIN32__) && !defined(__CYGWIN__))
#  define __CYGWIN__            /* compatibility for CygWin B19 and older */
#endif

/* enable multibyte character set support by default */
#ifndef _MBCS
#  define _MBCS
#endif
#if defined(__CYGWIN__)
#  undef _MBCS
#endif

#ifndef MSDOS
/*
 * Windows 95 (and Windows NT) file systems are (to some extend)
 * extensions of MSDOS. Common features include for example:
 *      FAT or (FAT like) file systems,
 *      '\\' as directory separator in paths,
 *      "\r\n" as record (line) terminator in text files, ...
 */
#  define MSDOS
/* inherit MS-DOS file system etc. stuff */
#endif

#define USE_CASE_MAP
#define PROCNAME(n) (action == ADD || action == UPDATE ? wild(n) : \
                     procname(n, 1))
#define BROKEN_FSEEK
#ifndef __RSXNT__
#  define HAVE_FSEEKABLE
#endif

/* File operations--use "b" for binary if allowed or fixed length 512 on VMS
 *                  use "S" for sequential access on NT to prevent the NT
 *                  file cache eating up memory with large .zip files
 */
#define FOPR "rb"
#define FOPM "r+b"
#define FOPW "wbS"

#if (defined(__CYGWIN__) && !defined(NO_MKTIME))
#  define NO_MKTIME             /* Cygnus' mktime() implementation is buggy */
#endif
#if (!defined(NT_TZBUG_WORKAROUND) && !defined(NO_NT_TZBUG_WORKAROUND))
#  define NT_TZBUG_WORKAROUND
#endif
#if (defined(UTIL) && defined(NT_TZBUG_WORKAROUND))
#  undef NT_TZBUG_WORKAROUND    /* the Zip utilities do not use time-stamps */
#endif
#if !defined(NO_EF_UT_TIME) && !defined(USE_EF_UT_TIME)
#  define USE_EF_UT_TIME
#endif
#if (!defined(NO_NTSD_EAS) && !defined(NTSD_EAS))
#  define NTSD_EAS
#endif

#if (defined(NTSD_EAS) && !defined(ZP_NEED_MEMCOMPR))
#  define ZP_NEED_MEMCOMPR
#endif

#ifdef WINDLL
# ifndef NO_ASM
#   define NO_ASM
# endif
# ifndef MSWIN
#   define MSWIN
# endif
# ifndef REENTRANT
#   define REENTRANT
# endif
#endif /* WINDLL */

/* Enable use of optimized x86 assembler version of longest_match() for
   MSDOS, WIN32 and OS2 per default.  */
#if !defined(NO_ASM) && !defined(ASMV)
#  define ASMV
#endif

#if !defined(__GO32__) && !defined(__EMX__) && !defined(__CYGWIN__)
#  define NO_UNISTD_H
#endif

/* the following definitions are considered as "obsolete" by Microsoft and
 * might be missing in some versions of <windows.h>
 */
#ifndef AnsiToOem
#  define AnsiToOem CharToOemA
#endif
#ifndef OemToAnsi
#  define OemToAnsi OemToCharA
#endif

#if (defined(__RSXNT__) && defined(__CRTRSXNT__))
#  include <crtrsxnt.h>
#endif

/* Get types and stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#ifdef _MBCS
#  if (!defined(__EMX__) && !defined(__MINGW32__) && !defined(__CYGWIN__))
#    include <stdlib.h>
#    include <mbstring.h>
#  endif
#  if (defined(__MINGW32__) && !defined(MB_CUR_MAX))
#    ifdef __MSVCRT__
       extern int *__p___mb_cur_max(void);
#      define MB_CUR_MAX (*__p___mb_cur_max())
#    else
       extern int *_imp____mb_cur_max_dll;
#      define MB_CUR_MAX (*_imp____mb_cur_max_dll)
#    endif
#  endif
#  if (defined(__LCC__) && !defined(MB_CUR_MAX))
     extern int *_imp____mb_cur_max;
#    define MB_CUR_MAX (*_imp____mb_cur_max)
#  endif
#endif

#ifdef __LCC__
#  include <time.h>
#endif
#if (defined(__RSXNT__) || defined(__EMX__)) && !defined(tzset)
#  define tzset _tzset
#endif
#ifdef __MINGW32__
   extern void _tzset(void);  /* this is missing in <time.h> */
#  ifndef tzset
#    define tzset _tzset
#  endif
#endif

#ifdef MATCH
#  undef MATCH
#endif
#define MATCH dosmatch          /* use DOS style wildcard matching */

#ifdef ZCRYPT_INTERNAL
#  ifdef WINDLL
#    define ZCR_SEED2     (unsigned)3141592654L /* use PI as seed pattern */
#  else
#    include <process.h>        /* getpid() declaration for srand seed */
#  endif
#endif

/* This patch of stat() is useful for at least three compilers.  It is   */
/* difficult to take a stat() of a root directory under Windows95, so  */
/* zstat_zipwin32() detects that case and fills in suitable values.    */
#ifndef __RSXNT__
#  ifndef W32_STATROOT_FIX
#    define W32_STATROOT_FIX
#  endif
#endif /* !__RSXNT__ */

#if (defined(NT_TZBUG_WORKAROUND) || defined(W32_STATROOT_FIX))
#  define W32_STAT_BANDAID
   int zstat_zipwin32(const char *path, struct stat *buf);
#  ifdef SSTAT
#    undef SSTAT
#  endif
#  define SSTAT zstat_zipwin32
#endif /* NT_TZBUG_WORKAROUND || W32_STATROOT_FIX */

int getch_win32(void);

#ifdef __GNUC__
# define IZ_PACKED      __attribute__((packed))
#else
# define IZ_PACKED
#endif

/* for some (all ?) versions of IBM C Set/2 and IBM C Set++ */
#ifndef S_IFMT
#  define S_IFMT 0xF000
#endif /* !S_IFMT */

#ifdef __WATCOMC__
#  include <stdio.h>    /* PATH_MAX is defined here */
#  define NO_MKTEMP

/* Get asm routines to link properly without using "__cdecl": */
#  ifdef __386__
#    ifdef ASMV
#      pragma aux match_init    "_*" parm caller [] modify []
#      pragma aux longest_match "_*" parm caller [] value [eax] \
                                      modify [eax ecx edx]
#    endif
#    if defined(ASM_CRC) && !defined(USE_ZLIB)
#      pragma aux crc32         "_*" parm caller [] value [eax] modify [eax]
#      pragma aux get_crc_table "_*" parm caller [] value [eax] \
                                      modify [eax ecx edx]
#    endif /* ASM_CRC && !USE_ZLIB */
#  endif /* __386__ */
#endif /* __WATCOMC__ */
