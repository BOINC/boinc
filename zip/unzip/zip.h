/*
  Copyright (c) 1990-2001 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/* This is a dummy zip.h to allow the source files shared with Zip (crypt.c,
   crc32.c, crctab.c, ttyio.c) to compile for UnZip.  (In case you are looking
   for the Info-ZIP license, please follow the pointers above.)  */

#ifndef __zip_h   /* don't include more than once */
#define __zip_h

#define UNZIP_INTERNAL
#include "unzip.h"

#define local static

#define ZE_MEM         PK_MEM
#define ziperr(c, h)   return

#endif /* !__zip_h */
