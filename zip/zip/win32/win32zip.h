/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
#ifndef _WIN32ZIP_H
#define _WIN32ZIP_H

/*
 * NT specific functions for ZIP.
 */

int GetFileMode(char *name);
long GetTheFileTime(char *name, iztimes *z_times);

int IsFileNameValid(char *name);
int IsFileSystemOldFAT(char *dir);
void ChangeNameForFAT(char *name);

char *getVolumeLabel(int drive, ulg *vtime, ulg *vmode, time_t *vutim);

#if 0 /* never used ?? */
char *StringLower(char *);
#endif

char *GetLongPathEA(char *name);

#endif /* _WIN32ZIP_H */
