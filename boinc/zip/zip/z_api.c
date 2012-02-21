/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*---------------------------------------------------------------------------

  api.c

  This module supplies a Zip dll engine for use directly from C/C++
  programs.

  The entry points are:

    ZpVer *ZpVersion(void);
    int EXPENTRY ZpInit(LPZIPUSERFUNCTIONS lpZipUserFunc);
    BOOL EXPENTRY ZpSetOptions(LPZPOPT Opts);
    ZPOPT EXPENTRY ZpGetOptions(void);
    int EXPENTRY ZpArchive(ZCL C);

  This module is currently only used by the Windows dll, and is not used at
  all by any of the other platforms, although it should be easy enough to
  implement this on most platforms.

  ---------------------------------------------------------------------------*/
#define __API_C

#ifdef WINDLL
#  include <windows.h>
#  include "windll/windll.h"
#endif

#ifdef OS2
#  define  INCL_DOSMEMMGR
#  include <os2.h>
#endif

#ifdef __BORLANDC__
#include <dir.h>
#endif
#include <direct.h>
#include <ctype.h>

#include "z_api.h"                /* this includes zip.h */
//#include "z_crypt.h"

#include "revision.h"

#ifdef USE_ZLIB
#  include "zlib.h"
#endif


#ifdef _WINDLL
DLLPRNT *lpZipPrint;
DLLPASSWORD *lpZipPassword;
extern DLLCOMMENT *lpComment;
#endif

ZIPUSERFUNCTIONS ZipUserFunctions, far * lpZipUserFunctions;

int ZipRet;

/* Local forward declarations */
extern int  zipmain OF((int, char **));
int AllocMemory(int, char *, char *);

ZPOPT Options;
char **argVee;
int argCee;

/*---------------------------------------------------------------------------
    Local functions
  ---------------------------------------------------------------------------*/

int AllocMemory(int i, char *cmd, char *str)
{
int j;
if ((argVee[i] = (char *) malloc( sizeof(char) * strlen(cmd)+1 )) == NULL)
   {
   for (j = 0; j < i; j++)
       {
       free (argVee[j]);
       argVee[j] = NULL;
       }
   free(argVee);
   fprintf(stdout, "Unable to allocate memory in zip library at %s\n", str);
   return ZE_MEM;
   }
strcpy( argVee[i], cmd );
return ZE_OK;
}

/*---------------------------------------------------------------------------
    Documented API entry points
  ---------------------------------------------------------------------------*/

int EXPENTRY ZpInit(LPZIPUSERFUNCTIONS lpZipUserFunc)
{
ZipUserFunctions = *lpZipUserFunc;
lpZipUserFunctions = &ZipUserFunctions;

if (!lpZipUserFunctions->print ||
    !lpZipUserFunctions->comment)
    return FALSE;

return TRUE;
}

BOOL EXPENTRY ZpSetOptions(LPZPOPT Opts)
{
Options = *Opts;
return TRUE;
}

ZPOPT EXPENTRY ZpGetOptions(void)
{
#if CRYPT
Options.fEncryption = TRUE;
#else
Options.fEncryption = FALSE;
#endif
return Options;
}

int EXPENTRY ZpArchive(ZCL C)
/* Add, update, freshen, or delete zip entries in a zip file.  See the
   command help in help() zip.c */
{
int i, k, j, m;
char szOrigDir[PATH_MAX];

argCee = 0;
/* malloc additional 26 to allow for additional command line arguments */
if ((argVee = (char **)malloc((C.argc+26)*sizeof(char *))) == NULL)
   {
   fprintf(stdout, "Unable to allocate memory in zip dll\n");
   return ZE_MEM;
   }
if ((argVee[argCee] = (char *) malloc( sizeof(char) * strlen("wiz.exe")+1 )) == NULL)
   {
   free(argVee);
   fprintf(stdout, "Unable to allocate memory in zip dll\n");
   return ZE_MEM;
   }
strcpy( argVee[argCee], "wiz.exe" );
argCee++;

/* Set compression level efficacy -0...-9 */
if (AllocMemory(argCee, "-0", "Compression") != ZE_OK)
   return ZE_MEM;
argVee[argCee][1] = Options.fLevel;
argCee++;

if (Options.fOffsets)    /* Update offsets for SFX prefix */
   {
   if (AllocMemory(argCee, "-A", "Offsets") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fDeleteEntries)    /* Delete files from zip file -d */
   {
   if (AllocMemory(argCee, "-d", "Delete") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fNoDirEntries) /* Do not add directory entries -D */
   {
   if (AllocMemory(argCee, "-D", "No Dir Entries") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fFreshen) /* Freshen zip file--overwrite only -f */
   {
   if (AllocMemory(argCee, "-f", "Freshen") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fRepair)  /* Fix archive -F or -FF */
   {
   if (Options.fRepair == 1)
      {
      if (AllocMemory(argCee, "-F", "Repair") != ZE_OK)
         return ZE_MEM;
      }
   else
      {
      if (AllocMemory(argCee, "-FF", "Repair") != ZE_OK)
         return ZE_MEM;
      }
   argCee++;
   }
if (Options.fGrow) /* Allow appending to a zip file -g */
   {
   if (AllocMemory(argCee, "-g", "Appending") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fJunkDir) /* Junk directory names -j */
   {
   if (AllocMemory(argCee, "-j", "Junk Dir Names") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fEncrypt) /* encrypt -e */
   {
   if (AllocMemory(argCee, "-e", "Encrypt") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fJunkSFX) /* Junk sfx prefix */
   {
   if (AllocMemory(argCee, "-J", "Junk SFX") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }

if (Options.fForce) /* Make entries using DOS names (k for Katz) -k */
   {
   if (AllocMemory(argCee, "-k", "Force DOS") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }

if (Options.fLF_CRLF) /* Translate LF_CRLF -l */
   {
   if (AllocMemory(argCee, "-l", "LF-CRLF") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fCRLF_LF) /* Translate CR/LF to LF -ll */
   {
   if (AllocMemory(argCee, "-ll", "CRLF-LF") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fMove) /* Delete files added to or updated in zip file -m */
   {
   if (AllocMemory(argCee, "-m", "Move") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }

if (Options.fLatestTime) /* Set zip file time to time of latest file in it -o */
   {
   if (AllocMemory(argCee, "-o", "Time") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }

if (Options.fComment) /* Add archive comment "-z" */
   {
   if (AllocMemory(argCee, "-z", "Comment") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }

if (Options.fQuiet) /* quiet operation -q */
   {
   if (AllocMemory(argCee, "-q", "Quiet") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fRecurse == 1) /* recurse into subdirectories -r */
   {
   if (AllocMemory(argCee, "-r", "Recurse -r") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
else if (Options.fRecurse == 2) /* recurse into subdirectories -R */
   {
   if (AllocMemory(argCee, "-R", "Recurse -R") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fSystem)  /* include system and hidden files -S */
   {
   if (AllocMemory(argCee, "-S", "System") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fExcludeDate)    /* Exclude files newer than specified date -tt */
   {
   if ((Options.Date != NULL) && (Options.Date[0] != '\0'))
      {
      if (AllocMemory(argCee, "-tt", "Date") != ZE_OK)
         return ZE_MEM;
      argCee++;
      if (AllocMemory(argCee, Options.Date, "Date") != ZE_OK)
         return ZE_MEM;
      argCee++;
      }
   }

if (Options.fIncludeDate)    /* include files newer than specified date -t */
   {
   if ((Options.Date != NULL) && (Options.Date[0] != '\0'))
      {
      if (AllocMemory(argCee, "-t", "Date") != ZE_OK)
         return ZE_MEM;
      argCee++;
      if (AllocMemory(argCee, Options.Date, "Date") != ZE_OK)
         return ZE_MEM;
      argCee++;
      }
   }

if (Options.fUpdate) /* Update zip file--overwrite only if newer -u */
   {
   if (AllocMemory(argCee, "-u", "Update") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fVerbose)  /* Mention oddities in zip file structure -v */
   {
   if (AllocMemory(argCee, "-v", "Verbose") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (Options.fVolume)  /* Include volume label -$ */
   {
   if (AllocMemory(argCee, "-$", "Volume") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
#ifdef WIN32
if (Options.fPrivilege)  /* Use privileges -! */
   {
   if (AllocMemory(argCee, "-!", "Privileges") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
#endif
if (Options.fExtra)  /* Exclude extra attributes -X */
   {
   if (AllocMemory(argCee, "-X", "Extra") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if ((Options.szTempDir != NULL) && (Options.szTempDir[0] != '\0')
     && Options.fTemp) /* Use temporary directory -b */
   {
   if (AllocMemory(argCee, "-b", "Temp dir switch command") != ZE_OK)
      return ZE_MEM;
   argCee++;
   if (AllocMemory(argCee, Options.szTempDir, "Temporary directory") != ZE_OK)
      return ZE_MEM;
   argCee++;
   }
if (AllocMemory(argCee, C.lpszZipFN, "Zip file name") != ZE_OK)
   return ZE_MEM;
argCee++;

getcwd(szOrigDir, PATH_MAX); /* Save current drive and directory */

if ((Options.szRootDir != NULL) && (Options.szRootDir[0] != '\0'))
   {
   chdir(Options.szRootDir);
#ifdef __BORLANDC__
   setdisk(toupper(Options.szRootDir[0]) - 'A');
#endif
   lstrcat(Options.szRootDir, "\\"); /* append trailing \\ */
   if (C.FNV != NULL)
      {
      for (k = 0; k < C.argc; k++)
         {
         if (AllocMemory(argCee, C.FNV[k], "Making argv") != ZE_OK)
            return ZE_MEM;
         if ((strncmp(Options.szRootDir, C.FNV[k], lstrlen(Options.szRootDir))) == 0)
            {
            m = 0;
            for (j = lstrlen(Options.szRootDir); j < lstrlen(C.FNV[k]); j++)
               argVee[argCee][m++] = C.FNV[k][j];
            argVee[argCee][m] = '\0';
            }
         argCee++;
         }
      }
   }
else
   if (C.FNV != NULL)
      for (k = 0; k < C.argc; k++)
         {
         if (AllocMemory(argCee, C.FNV[k], "Making argv") != ZE_OK)
            return ZE_MEM;
         argCee++;
         }

argVee[argCee] = NULL;

ZipRet = zipmain(argCee, argVee);

chdir(szOrigDir);
#ifdef __BORLANDC__
setdisk(toupper(szOrigDir[0]) - 'A');
#endif

/* Free the arguments in the array */
for (i = 0; i < argCee; i++)
   {
      free (argVee[i]);
      argVee[i] = NULL;
   }
/* Then free the array itself */
free(argVee);

return ZipRet;
}

#if CRYPT
int encr_passwd(int modeflag, char *pwbuf, int size, const char *zfn)
{
return (*lpZipUserFunctions->password)(pwbuf, size, ((modeflag == ZP_PW_VERIFY) ?
                  "Verify password: " : "Enter password: "),
                  (char *)zfn);
}
#endif /* CRYPT */

void EXPENTRY ZpVersion(ZpVer far * p)   /* should be pointer to const struct */
{
    p->structlen = ZPVER_LEN;

#ifdef BETA
    p->flag = 1;
#else
    p->flag = 0;
#endif
    lstrcpy(p->betalevel, Z_BETALEVEL);
    lstrcpy(p->date, REVDATE);

#ifdef ZLIB_VERSION
    lstrcpy(p->zlib_version, ZLIB_VERSION);
    p->flag |= 2;
#else
    p->zlib_version[0] = '\0';
#endif

    p->zip.major = Z_MAJORVER;
    p->zip.minor = Z_MINORVER;
    p->zip.patchlevel = Z_PATCHLEVEL;


    p->os2dll.major = D2_MAJORVER;
    p->os2dll.minor = D2_MINORVER;
    p->os2dll.patchlevel = D2_PATCHLEVEL;


    p->windll.major = DW_MAJORVER;
    p->windll.minor = DW_MINORVER;
    p->windll.patchlevel = DW_PATCHLEVEL;
}

const char *BOINC_RCSID_e99e3c832e = "$Id: z_api.c 4979 2005-01-02 18:29:53Z ballen $";
