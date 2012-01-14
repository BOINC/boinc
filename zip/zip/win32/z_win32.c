/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 * WIN32 specific functions for ZIP.
 *
 * The WIN32 version of ZIP heavily relies on the MSDOS and OS2 versions,
 * since we have to do similar things to switch between NTFS, HPFS and FAT.
 */


#include "../zip.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>
#ifdef __RSXNT__
#  include <alloca.h>
#  include "../win32/rsxntwin.h"
#endif
#include "../win32/win32zip.h"

#define A_RONLY    0x01
#define A_HIDDEN   0x02
#define A_SYSTEM   0x04
#define A_LABEL    0x08
#define A_DIR      0x10
#define A_ARCHIVE  0x20


#define EAID     0x0009


#ifndef UTIL

extern int noisy;

#ifdef NT_TZBUG_WORKAROUND
local int FSusesLocalTime(const char *path);
#endif
#if (defined(USE_EF_UT_TIME) || defined(NT_TZBUG_WORKAROUND))
local int FileTime2utime(FILETIME *pft, time_t *ut);
#endif
#if (defined(NT_TZBUG_WORKAROUND) && defined(W32_STAT_BANDAID))
local int VFatFileTime2utime(const FILETIME *pft, time_t *ut);
#endif


/* FAT / HPFS detection */

int IsFileSystemOldFAT(char *dir)
{
  char root[4];
  char vname[128];
  DWORD vnamesize = sizeof(vname);
  DWORD vserial;
  DWORD vfnsize;
  DWORD vfsflags;
  char vfsname[128];
  DWORD vfsnamesize = sizeof(vfsname);

    /*
     * We separate FAT and HPFS+other file systems here.
     * I consider other systems to be similar to HPFS/NTFS, i.e.
     * support for long file names and being case sensitive to some extent.
     */

    strncpy(root, dir, 3);
    if ( isalpha((uch)root[0]) && (root[1] == ':') ) {
      root[0] = to_up(dir[0]);
      root[2] = '\\';
      root[3] = 0;
    }
    else {
      root[0] = '\\';
      root[1] = 0;
    }

    if ( !GetVolumeInformation(root, vname, vnamesize,
                         &vserial, &vfnsize, &vfsflags,
                         vfsname, vfsnamesize)) {
        fprintf(mesg, "zip diagnostic: GetVolumeInformation failed\n");
        return(FALSE);
    }

    return vfnsize <= 12;
}


/* access mode bits and time stamp */

int GetFileMode(char *name)
{
DWORD dwAttr;
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
  char *ansi_name = (char *)alloca(strlen(name) + 1);

  OemToAnsi(name, ansi_name);
  name = ansi_name;
#endif

  dwAttr = GetFileAttributes(name);
  if ( dwAttr == 0xFFFFFFFF ) {
    fprintf(mesg, "zip diagnostic: GetFileAttributes failed\n");
    return(0x20); /* the most likely, though why the error? security? */
  }
  return(
          (dwAttr&FILE_ATTRIBUTE_READONLY  ? A_RONLY   :0)
        | (dwAttr&FILE_ATTRIBUTE_HIDDEN    ? A_HIDDEN  :0)
        | (dwAttr&FILE_ATTRIBUTE_SYSTEM    ? A_SYSTEM  :0)
        | (dwAttr&FILE_ATTRIBUTE_DIRECTORY ? A_DIR     :0)
        | (dwAttr&FILE_ATTRIBUTE_ARCHIVE   ? A_ARCHIVE :0));
}


#ifdef NT_TZBUG_WORKAROUND
local int FSusesLocalTime(const char *path)
{
    char     *tmp0;
    char      rootPathName[4];
    char      tmp1[MAX_PATH], tmp2[MAX_PATH];
    unsigned  volSerNo, maxCompLen, fileSysFlags;
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
    char *ansi_path = (char *)alloca(strlen(path) + 1);

    OemToAnsi(path, ansi_path);
    path = ansi_path;
#endif

    if (isalpha((uch)path[0]) && (path[1] == ':'))
        tmp0 = (char *)path;
    else
    {
        GetFullPathName(path, MAX_PATH, tmp1, &tmp0);
        tmp0 = &tmp1[0];
    }
    strncpy(rootPathName, tmp0, 3);   /* Build the root path name, */
    rootPathName[3] = '\0';           /* e.g. "A:/"                */

    GetVolumeInformation((LPCTSTR)rootPathName, (LPTSTR)tmp1, (DWORD)MAX_PATH,
                         (LPDWORD)&volSerNo, (LPDWORD)&maxCompLen,
                         (LPDWORD)&fileSysFlags, (LPTSTR)tmp2, (DWORD)MAX_PATH);

    /* Volumes in (V)FAT and (OS/2) HPFS format store file timestamps in
     * local time!
     */
    return !strncmp(strupr(tmp2), "FAT", 3) ||
           !strncmp(tmp2, "VFAT", 4) ||
           !strncmp(tmp2, "HPFS", 4);

} /* end function FSusesLocalTime() */
#endif /* NT_TZBUG_WORKAROUND */


#if (defined(USE_EF_UT_TIME) || defined(NT_TZBUG_WORKAROUND))

#if (defined(__GNUC__) || defined(ULONG_LONG_MAX))
   typedef long long            LLONG64;
   typedef unsigned long long   ULLNG64;
#elif (defined(__WATCOMC__) && (__WATCOMC__ >= 1100))
   typedef __int64              LLONG64;
   typedef unsigned __int64     ULLNG64;
#elif (defined(_MSC_VER) && (_MSC_VER >= 1100))
   typedef __int64              LLONG64;
   typedef unsigned __int64     ULLNG64;
#elif (defined(__IBMC__) && (__IBMC__ >= 350))
   typedef __int64              LLONG64;
   typedef unsigned __int64     ULLNG64;
#else
#  define NO_INT64
#endif

#  define UNIX_TIME_ZERO_HI  0x019DB1DEUL
#  define UNIX_TIME_ZERO_LO  0xD53E8000UL
#  define NT_QUANTA_PER_UNIX 10000000L
#  define FTQUANTA_PER_UT_L  (NT_QUANTA_PER_UNIX & 0xFFFF)
#  define FTQUANTA_PER_UT_H  (NT_QUANTA_PER_UNIX >> 16)
#  define UNIX_TIME_UMAX_HI  0x0236485EUL
#  define UNIX_TIME_UMAX_LO  0xD4A5E980UL
#  define UNIX_TIME_SMIN_HI  0x0151669EUL
#  define UNIX_TIME_SMIN_LO  0xD53E8000UL
#  define UNIX_TIME_SMAX_HI  0x01E9FD1EUL
#  define UNIX_TIME_SMAX_LO  0xD4A5E980UL

local int FileTime2utime(FILETIME *pft, time_t *ut)
{
#ifndef NO_INT64
    ULLNG64 NTtime;

    NTtime = ((ULLNG64)pft->dwLowDateTime +
              ((ULLNG64)pft->dwHighDateTime << 32));

    /* underflow and overflow handling */
#ifdef CHECK_UTIME_SIGNED_UNSIGNED
    if ((time_t)0x80000000L < (time_t)0L)
    {
        if (NTtime < ((ULLNG64)UNIX_TIME_SMIN_LO +
                      ((ULLNG64)UNIX_TIME_SMIN_HI << 32))) {
            *ut = (time_t)LONG_MIN;
            return FALSE;
        }
        if (NTtime > ((ULLNG64)UNIX_TIME_SMAX_LO +
                      ((ULLNG64)UNIX_TIME_SMAX_HI << 32))) {
            *ut = (time_t)LONG_MAX;
            return FALSE;
        }
    }
    else
#endif /* CHECK_UTIME_SIGNED_UNSIGNED */
    {
        if (NTtime < ((ULLNG64)UNIX_TIME_ZERO_LO +
                      ((ULLNG64)UNIX_TIME_ZERO_HI << 32))) {
            *ut = (time_t)0;
            return FALSE;
        }
        if (NTtime > ((ULLNG64)UNIX_TIME_UMAX_LO +
                      ((ULLNG64)UNIX_TIME_UMAX_HI << 32))) {
            *ut = (time_t)ULONG_MAX;
            return FALSE;
        }
    }

    NTtime -= ((ULLNG64)UNIX_TIME_ZERO_LO +
               ((ULLNG64)UNIX_TIME_ZERO_HI << 32));
    *ut = (time_t)(NTtime / (unsigned long)NT_QUANTA_PER_UNIX);
    return TRUE;
#else /* NO_INT64 (64-bit integer arithmetics may not be supported) */
    /* nonzero if `y' is a leap year, else zero */
#   define leap(y) (((y)%4 == 0 && (y)%100 != 0) || (y)%400 == 0)
    /* number of leap years from 1970 to `y' (not including `y' itself) */
#   define nleap(y) (((y)-1969)/4 - ((y)-1901)/100 + ((y)-1601)/400)
    /* daycount at the end of month[m-1] */
    static ZCONST ush ydays[] =
      { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

    time_t days;
    SYSTEMTIME w32tm;

    /* underflow and overflow handling */
#ifdef CHECK_UTIME_SIGNED_UNSIGNED
    if ((time_t)0x80000000L < (time_t)0L)
    {
        if ((pft->dwHighDateTime < UNIX_TIME_SMIN_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_SMIN_HI) &&
             (pft->dwLowDateTime < UNIX_TIME_SMIN_LO))) {
            *ut = (time_t)LONG_MIN;
            return FALSE;
        if ((pft->dwHighDateTime > UNIX_TIME_SMAX_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_SMAX_HI) &&
             (pft->dwLowDateTime > UNIX_TIME_SMAX_LO))) {
            *ut = (time_t)LONG_MAX;
            return FALSE;
        }
    }
    else
#endif /* CHECK_UTIME_SIGNED_UNSIGNED */
    {
        if ((pft->dwHighDateTime < UNIX_TIME_ZERO_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_ZERO_HI) &&
             (pft->dwLowDateTime < UNIX_TIME_ZERO_LO))) {
            *ut = (time_t)0;
            return FALSE;
        }
        if ((pft->dwHighDateTime > UNIX_TIME_UMAX_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_UMAX_HI) &&
             (pft->dwLowDateTime > UNIX_TIME_UMAX_LO))) {
            *ut = (time_t)ULONG_MAX;
            return FALSE;
        }
    }

    FileTimeToSystemTime(pft, &w32tm);

    /* set `days' to the number of days into the year */
    days = w32tm.wDay - 1 + ydays[w32tm.wMonth-1] +
           (w32tm.wMonth > 2 && leap (w32tm.wYear));

    /* now set `days' to the number of days since 1 Jan 1970 */
    days += 365 * (time_t)(w32tm.wYear - 1970) +
            (time_t)(nleap(w32tm.wYear));

    *ut = (time_t)(86400L * days + 3600L * (time_t)w32tm.wHour +
                   (time_t)(60 * w32tm.wMinute + w32tm.wSecond));
    return TRUE;
#endif /* ?NO_INT64 */
} /* end function FileTime2utime() */
#endif /* USE_EF_UT_TIME || NT_TZBUG_WORKAROUND */


#if (defined(NT_TZBUG_WORKAROUND) && defined(W32_STAT_BANDAID))

local int VFatFileTime2utime(const FILETIME *pft, time_t *ut)
{
    FILETIME lft;
    SYSTEMTIME w32tm;
    struct tm ltm;

    FileTimeToLocalFileTime(pft, &lft);
    FileTimeToSystemTime(&lft, &w32tm);
    /* underflow and overflow handling */
    /* TODO: The range checks are not accurate, the actual limits may
     *       be off by one daylight-saving-time shift (typically 1 hour),
     *       depending on the current state of "is_dst".
     */
#ifdef CHECK_UTIME_SIGNED_UNSIGNED
    if ((time_t)0x80000000L < (time_t)0L)
    {
        if ((pft->dwHighDateTime < UNIX_TIME_SMIN_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_SMIN_HI) &&
             (pft->dwLowDateTime < UNIX_TIME_SMIN_LO))) {
            *ut = (time_t)LONG_MIN;
            return FALSE;
        if ((pft->dwHighDateTime > UNIX_TIME_SMAX_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_SMAX_HI) &&
             (pft->dwLowDateTime > UNIX_TIME_SMAX_LO))) {
            *ut = (time_t)LONG_MAX;
            return FALSE;
        }
    }
    else
#endif /* CHECK_UTIME_SIGNED_UNSIGNED */
    {
        if ((pft->dwHighDateTime < UNIX_TIME_ZERO_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_ZERO_HI) &&
             (pft->dwLowDateTime < UNIX_TIME_ZERO_LO))) {
            *ut = (time_t)0;
            return FALSE;
        }
        if ((pft->dwHighDateTime > UNIX_TIME_UMAX_HI) ||
            ((pft->dwHighDateTime == UNIX_TIME_UMAX_HI) &&
             (pft->dwLowDateTime > UNIX_TIME_UMAX_LO))) {
            *ut = (time_t)ULONG_MAX;
            return FALSE;
        }
    }
    ltm.tm_year = w32tm.wYear - 1900;
    ltm.tm_mon = w32tm.wMonth - 1;
    ltm.tm_mday = w32tm.wDay;
    ltm.tm_hour = w32tm.wHour;
    ltm.tm_min = w32tm.wMinute;
    ltm.tm_sec = w32tm.wSecond;
    ltm.tm_isdst = -1;  /* let mktime determine if DST is in effect */
    *ut = mktime(&ltm);

    /* a cheap error check: mktime returns "(time_t)-1L" on conversion errors.
     * Normally, we would have to apply a consistency check because "-1"
     * could also be a valid time. But, it is quite unlikely to read back odd
     * time numbers from file systems that store time stamps in DOS format.
     * (The only known exception is creation time on VFAT partitions.)
     */
    return (*ut != (time_t)-1L);

} /* end function VFatFileTime2utime() */
#endif /* NT_TZBUG_WORKAROUND && W32_STAT_BANDAID */


#if 0           /* Currently, this is not used at all */

long GetTheFileTime(char *name, iztimes *z_ut)
{
HANDLE h;
FILETIME Modft, Accft, Creft, lft;
WORD dh, dl;
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
  char *ansi_name = (char *)alloca(strlen(name) + 1);

  OemToAnsi(name, ansi_name);
  name = ansi_name;
#endif

  h = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if ( h != INVALID_HANDLE_VALUE ) {
    BOOL ftOK = GetFileTime(h, &Creft, &Accft, &Modft);
    CloseHandle(h);
#ifdef USE_EF_UT_TIME
    if (ftOK && (z_ut != NULL)) {
      FileTime2utime(&Modft, &(z_ut->mtime));
      if (Accft.dwLowDateTime != 0 || Accft.dwHighDateTime != 0)
          FileTime2utime(&Accft, &(z_ut->atime));
      else
          z_ut->atime = z_ut->mtime;
      if (Creft.dwLowDateTime != 0 || Creft.dwHighDateTime != 0)
          FileTime2utime(&Creft, &(z_ut->ctime));
      else
          z_ut->ctime = z_ut->mtime;
    }
#endif
    FileTimeToLocalFileTime(&ft, &lft);
    FileTimeToDosDateTime(&lft, &dh, &dl);
    return(dh<<16) | dl;
  }
  else
    return 0L;
}

#endif /* never */


void ChangeNameForFAT(char *name)
{
  char *src, *dst, *next, *ptr, *dot, *start;
  static char invalid[] = ":;,=+\"[]<>| \t";

  if ( isalpha((uch)name[0]) && (name[1] == ':') )
    start = name + 2;
  else
    start = name;

  src = dst = start;
  if ( (*src == '/') || (*src == '\\') )
    src++, dst++;

  while ( *src )
  {
    for ( next = src; *next && (*next != '/') && (*next != '\\'); next++ );

    for ( ptr = src, dot = NULL; ptr < next; ptr++ )
      if ( *ptr == '.' )
      {
        dot = ptr; /* remember last dot */
        *ptr = '_';
      }

    if ( dot == NULL )
      for ( ptr = src; ptr < next; ptr++ )
        if ( *ptr == '_' )
          dot = ptr; /* remember last _ as if it were a dot */

    if ( dot && (dot > src) &&
         ((next - dot <= 4) ||
          ((next - src > 8) && (dot - src > 3))) )
    {
      if ( dot )
        *dot = '.';

      for ( ptr = src; (ptr < dot) && ((ptr - src) < 8); ptr++ )
        *dst++ = *ptr;

      for ( ptr = dot; (ptr < next) && ((ptr - dot) < 4); ptr++ )
        *dst++ = *ptr;
    }
    else
    {
      if ( dot && (next - src == 1) )
        *dot = '.';           /* special case: "." as a path component */

      for ( ptr = src; (ptr < next) && ((ptr - src) < 8); ptr++ )
        *dst++ = *ptr;
    }

    *dst++ = *next; /* either '/' or 0 */

    if ( *next )
    {
      src = next + 1;

      if ( *src == 0 ) /* handle trailing '/' on dirs ! */
        *dst = 0;
    }
    else
      break;
  }

  for ( src = start; *src != 0; ++src )
    if ( (strchr(invalid, *src) != NULL) || (*src == ' ') )
      *src = '_';
}

char *GetLongPathEA(char *name)
{
    return(NULL); /* volunteers ? */
}

int IsFileNameValid(x)
char *x;
{
    WIN32_FIND_DATA fd;
    HANDLE h;
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
    char *ansi_name = (char *)alloca(strlen(x) + 1);

    OemToAnsi(x, ansi_name);
    x = ansi_name;
#endif

    if ((h = FindFirstFile(x, &fd)) == INVALID_HANDLE_VALUE)
        return FALSE;
    FindClose(h);
    return TRUE;
}

char *getVolumeLabel(drive, vtime, vmode, vutim)
  int drive;    /* drive name: 'A' .. 'Z' or '\0' for current drive */
  ulg *vtime;   /* volume label creation time (DOS format) */
  ulg *vmode;   /* volume label file mode */
  time_t *vutim;/* volume label creationtime (UNIX format) */

/* If a volume label exists for the given drive, return its name and
   pretend to set its time and mode. The returned name is static data. */
{
  char rootpath[4];
  static char vol[14];
  DWORD fnlen, flags;

  *vmode = A_ARCHIVE | A_LABEL;           /* this is what msdos returns */
  *vtime = dostime(1980, 1, 1, 0, 0, 0);  /* no true date info available */
  *vutim = dos2unixtime(*vtime);
  strcpy(rootpath, "x:\\");
  rootpath[0] = (char)drive;
  if (GetVolumeInformation(drive ? rootpath : NULL, vol, 13, NULL,
                           &fnlen, &flags, NULL, 0))
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
    return (AnsiToOem(vol, vol), vol);
#else
    return vol;
#endif
  else
    return NULL;
}

#endif /* !UTIL */



int ZipIsWinNT(void)    /* returns TRUE if real NT, FALSE if Win95 or Win32s */
{
    static DWORD g_PlatformId = 0xFFFFFFFF; /* saved platform indicator */

    if (g_PlatformId == 0xFFFFFFFF) {
        /* note: GetVersionEx() doesn't exist on WinNT 3.1 */
        if (GetVersion() < 0x80000000)
            g_PlatformId = TRUE;
        else
            g_PlatformId = FALSE;
    }
    return (int)g_PlatformId;
}


#ifndef UTIL
#ifdef __WATCOMC__
#  include <io.h>
#  define _get_osfhandle _os_handle
/* gaah -- Watcom's docs claim that _get_osfhandle exists, but it doesn't.  */
#endif

#ifdef HAVE_FSEEKABLE
/*
 * return TRUE if file is seekable
 */
int fseekable(fp)
FILE *fp;
{
    return GetFileType((HANDLE)_get_osfhandle(fileno(fp))) == FILE_TYPE_DISK;
}
#endif /* HAVE_FSEEKABLE */
#endif /* !UTIL */


#if 0 /* seems to be never used; try it out... */
char *StringLower(char *szArg)
{
  char *szPtr;
/*  unsigned char *szPtr; */
  for ( szPtr = szArg; *szPtr; szPtr++ )
    *szPtr = lower[*szPtr];
  return szArg;
}
#endif /* never */



#ifdef W32_STAT_BANDAID

/* All currently known variants of WIN32 operating systems (Windows 95/98,
 * WinNT 3.x, 4.0, 5.0) have a nasty bug in the OS kernel concerning
 * conversions between UTC and local time: In the time conversion functions
 * of the Win32 API, the timezone offset (including seasonal daylight saving
 * shift) between UTC and local time evaluation is erratically based on the
 * current system time. The correct evaluation must determine the offset
 * value as it {was/is/will be} for the actual time to be converted.
 *
 * The C runtime lib's stat() returns utc time-stamps so that
 * localtime(timestamp) corresponds to the (potentially false) local
 * time shown by the OS' system programs (Explorer, command shell dir, etc.)
 *
 * For the NTFS file system (and other filesystems that store time-stamps
 * as UTC values), this results in st_mtime (, st_{c|a}time) fields which
 * are not stable but vary according to the seasonal change of "daylight
 * saving time in effect / not in effect".
 *
 * To achieve timestamp consistency of UTC (UT extra field) values in
 * Zip archives, the Info-ZIP programs require work-around code for
 * proper time handling in stat() (and other time handling routines).
 */
/* stat() functions under Windows95 tend to fail for root directories.   *
 * Watcom and Borland, at least, are affected by this bug.  Watcom made  *
 * a partial fix for 11.0 but still missed some cases.  This substitute  *
 * detects the case and fills in reasonable values.  Otherwise we get    *
 * effects like failure to extract to a root dir because it's not found. */

int zstat_zipwin32(const char *path, struct stat *buf)
{
    if (!stat(path, buf))
    {
#ifdef NT_TZBUG_WORKAROUND
        /* stat was successful, now redo the time-stamp fetches */
        int fs_uses_loctime = FSusesLocalTime(path);
        HANDLE h;
        FILETIME Modft, Accft, Creft;
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
        char *ansi_path = (char *)alloca(strlen(path) + 1);

        OemToAnsi(path, ansi_path);
#       define Ansi_Path  ansi_path
#else
#       define Ansi_Path  path
#endif

        Trace((stdout, "stat(%s) finds modtime %08lx\n", path, buf->st_mtime));
        h = CreateFile(Ansi_Path, GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            BOOL ftOK = GetFileTime(h, &Creft, &Accft, &Modft);
            CloseHandle(h);

            if (ftOK) {
                if (!fs_uses_loctime) {
                    /*  On a filesystem that stores UTC timestamps, we refill
                     *  the time fields of the struct stat buffer by directly
                     *  using the UTC values as returned by the Win32
                     *  GetFileTime() API call.
                     */
                    FileTime2utime(&Modft, &(buf->st_mtime));
                    if (Accft.dwLowDateTime != 0 || Accft.dwHighDateTime != 0)
                        FileTime2utime(&Accft, &(buf->st_atime));
                    else
                        buf->st_atime = buf->st_mtime;
                    if (Creft.dwLowDateTime != 0 || Creft.dwHighDateTime != 0)
                        FileTime2utime(&Creft, &(buf->st_ctime));
                    else
                        buf->st_ctime = buf->st_mtime;
                    Tracev((stdout,"NTFS, recalculated modtime %08lx\n",
                            buf->st_mtime));
                } else {
                    /*  On VFAT and FAT-like filesystems, the FILETIME values
                     *  are converted back to the stable local time before
                     *  converting them to UTC unix time-stamps.
                     */
                    VFatFileTime2utime(&Modft, &(buf->st_mtime));
                    if (Accft.dwLowDateTime != 0 || Accft.dwHighDateTime != 0)
                        VFatFileTime2utime(&Accft, &(buf->st_atime));
                    else
                        buf->st_atime = buf->st_mtime;
                    if (Creft.dwLowDateTime != 0 || Creft.dwHighDateTime != 0)
                        VFatFileTime2utime(&Creft, &(buf->st_ctime));
                    else
                        buf->st_ctime = buf->st_mtime;
                    Tracev((stdout, "VFAT, recalculated modtime %08lx\n",
                            buf->st_mtime));
                }
            }
        }
#       undef Ansi_Path
#endif /* NT_TZBUG_WORKAROUND */
        return 0;
    }
#ifdef W32_STATROOT_FIX
    else
    {
        DWORD flags;
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
        char *ansi_path = (char *)alloca(strlen(path) + 1);

        OemToAnsi(path, ansi_path);
#       define Ansi_Path  ansi_path
#else
#       define Ansi_Path  path
#endif

        flags = GetFileAttributes(Ansi_Path);
        if (flags != 0xFFFFFFFF && flags & FILE_ATTRIBUTE_DIRECTORY) {
            Trace((stderr, "\nstat(\"%s\",...) failed on existing directory\n",
                   path));
            memset(buf, 0, sizeof(struct stat));
            buf->st_atime = buf->st_ctime = buf->st_mtime =
              dos2unixtime(DOSTIME_MINIMUM);
            /* !!!   you MUST NOT add a cast to the type of "st_mode" here;
             * !!!   different compilers do not agree on the "st_mode" size!
             * !!!   (And, some compiler may not declare the "mode_t" type
             * !!!   identifier, so you cannot use it, either.)
             */
            buf->st_mode = S_IFDIR | S_IREAD |
                           ((flags & FILE_ATTRIBUTE_READONLY) ? 0 : S_IWRITE);
            return 0;
        } /* assumes: stat() won't fail on non-dirs without good reason */
#       undef Ansi_Path
    }
#endif /* W32_STATROOT_FIX */
    return -1;
}

#endif /* W32_STAT_BANDAID */



#ifndef WINDLL
/* This replacement getch() function was originally created for Watcom C
 * and then additionally used with CYGWIN. Since UnZip 5.4, all other Win32
 * ports apply this replacement rather that their supplied getch() (or
 * alike) function.  There are problems with unabsorbed LF characters left
 * over in the keyboard buffer under Win95 (and 98) when ENTER was pressed.
 * (Under Win95, ENTER returns two(!!) characters: CR-LF.)  This problem
 * does not appear when run on a WinNT console prompt!
 */

/* Watcom 10.6's getch() does not handle Alt+<digit><digit><digit>. */
/* Note that if PASSWD_FROM_STDIN is defined, the file containing   */
/* the password must have a carriage return after the word, not a   */
/* Unix-style newline (linefeed only).  This discards linefeeds.    */



/******************************/
/*  Function version_local()  */
/******************************/

void version_local()
{
    static ZCONST char CompiledWith[] = "Compiled with %s%s for %s%s%s%s.\n\n";
#if (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__DJGPP__))
    char buf[80];
#if (defined(_MSC_VER) && (_MSC_VER > 900))
    char buf2[80];
#endif
#endif

    printf(CompiledWith,

#if defined(_MSC_VER)  /* MSC == MSVC++, including the SDK compiler */
      (sprintf(buf, "Microsoft C %d.%02d ", _MSC_VER/100, _MSC_VER%100), buf),
#  if (_MSC_VER == 800)
        "(Visual C++ v1.1)",
#  elif (_MSC_VER == 850)
        "(Windows NT v3.5 SDK)",
#  elif (_MSC_VER == 900)
        "(Visual C++ v2.x)",
#  elif (_MSC_VER > 900)
        (sprintf(buf2, "(Visual C++ v%d.%d)", _MSC_VER/100 - 6,
                 _MSC_VER%100/10), buf2),
#  else
        "(bad version)",
#  endif
#elif defined(__WATCOMC__)
#  if (__WATCOMC__ % 10 > 0)
/* We do this silly test because __WATCOMC__ gives two digits for the  */
/* minor version, but Watcom packaging prefers to show only one digit. */
        (sprintf(buf, "Watcom C/C++ %d.%02d", __WATCOMC__ / 100,
                 __WATCOMC__ % 100), buf), "",
#  else
        (sprintf(buf, "Watcom C/C++ %d.%d", __WATCOMC__ / 100,
                 (__WATCOMC__ % 100) / 10), buf), "",
#  endif /* __WATCOMC__ % 10 > 0 */
#elif defined(__TURBOC__)
#  ifdef __BORLANDC__
     "Borland C++",
#    if (__BORLANDC__ == 0x0452)   /* __BCPLUSPLUS__ = 0x0320 */
        " 4.0 or 4.02",
#    elif (__BORLANDC__ == 0x0460)   /* __BCPLUSPLUS__ = 0x0340 */
        " 4.5",
#    elif (__BORLANDC__ == 0x0500)   /* __TURBOC__ = 0x0500 */
        " 5.0",
#    elif (__BORLANDC__ == 0x0520)   /* __TURBOC__ = 0x0520 */
        " 5.2 (C++ Builder)",
#    else
        " later than 5.2",
#    endif
#  else /* !__BORLANDC__ */
     "Turbo C",
#    if (__TURBOC__ >= 0x0400)     /* Kevin:  3.0 -> 0x0401 */
        "++ 3.0 or later",
#    elif (__TURBOC__ == 0x0295)     /* [661] vfy'd by Kevin */
        "++ 1.0",
#    endif
#  endif /* __BORLANDC__ */
#elif defined(__GNUC__)
#  ifdef __RSXNT__
#    if (defined(__DJGPP__) && !defined(__EMX__))
      (sprintf(buf, "rsxnt(djgpp v%d.%02d) / gcc ",
        __DJGPP__, __DJGPP_MINOR__), buf),
#    elif defined(__DJGPP__)
      (sprintf(buf, "rsxnt(emx+djgpp v%d.%02d) / gcc ",
        __DJGPP__, __DJGPP_MINOR__), buf),
#    elif (defined(__GO32__) && !defined(__EMX__))
      "rsxnt(djgpp v1.x) / gcc ",
#    elif defined(__GO32__)
      "rsxnt(emx + djgpp v1.x) / gcc ",
#    elif defined(__EMX__)
      "rsxnt(emx)+gcc ",
#    else
      "rsxnt(unknown) / gcc ",
#    endif
#  elif defined(__CYGWIN__)
      "Cygnus win32 / gcc ",
#  elif defined(__MINGW32__)
      "mingw32 / gcc ",
#  else
      "gcc ",
#  endif
      __VERSION__,
#elif defined(__LCC__)
      "LCC-Win32", "",
#else
      "unknown compiler (SDK?)", "",
#endif

      "\nWindows 9x / Windows NT", " (32-bit)",

#ifdef __DATE__
      " on ", __DATE__
#else
      "", ""
#endif
    );

    return;

} /* end function version_local() */
#endif /* !WINDLL */

const char *BOINC_RCSID_1fddd51a9a = "$Id: z_win32.c 4979 2005-01-02 18:29:53Z ballen $";
