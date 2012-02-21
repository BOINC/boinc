/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
   crypt.h (dummy version) by Info-ZIP.      Last revised: 15 Aug 98

   This is a non-functional version of Info-ZIP's crypt.h encryption/
   decryption header file for Zip, ZipCloak, UnZip and fUnZip.  This
   file is not copyrighted and may be distributed without restriction.
   See the "WHERE" file for sites from which to obtain the full crypt
   sources (zcrypt28.zip or later).
 */

#ifndef __crypt_h   /* don't include more than once */
#define __crypt_h

#ifdef CRYPT
#  undef CRYPT
#endif
#define CRYPT  0    /* dummy version */

#define zencode
#define zdecode

#define zfwrite  fwrite

#endif /* !__crypt_h */
