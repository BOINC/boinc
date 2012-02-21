/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/* crc32.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* $Id: z_crc32.c 7481 2005-08-25 21:33:28Z davea $ */

#define __CRC32_C       /* identifies this source module */

#include "zip.h"

#ifndef USE_ZLIB
#ifndef ASM_CRC

#ifndef ZCONST
#  define ZCONST const
#endif

#ifdef CRC32
#  undef CRC32
#endif
#define CRC32(c, b) (crc_table[((int)(c) ^ (b)) & 0xff] ^ ((c) >> 8))
#define DO1(buf)  crc = CRC32(crc, *buf++)
#define DO2(buf)  DO1(buf); DO1(buf)
#define DO4(buf)  DO2(buf); DO2(buf)
#define DO8(buf)  DO4(buf); DO4(buf)

/* ========================================================================= */
ulg crc32(crc, buf, len)
    register ulg crc;           /* crc shift register */
    register ZCONST uch *buf;   /* pointer to bytes to pump through */
    extent len;                 /* number of bytes in buf[] */
/* Run a set of bytes through the crc shift register.  If buf is a NULL
   pointer, then initialize the crc shift register contents instead.
   Return the current crc in either case. */
{
  register ZCONST ulg near *crc_table;

  if (buf == NULL) return 0L;

  crc_table = get_crc_table_boinc();

  crc = crc ^ 0xffffffffL;
#ifndef NO_UNROLLED_LOOPS
  while (len >= 8) {
    DO8(buf);
    len -= 8;
  }
#endif
  if (len) do {
    DO1(buf);
  } while (--len);
  return crc ^ 0xffffffffL;     /* (instead of ~c for 64-bit machines) */
}
#endif /* !ASM_CRC */
#endif /* !USE_ZLIB */

const char *BOINC_RCSID_49357bb004 = "$Id: z_crc32.c 7481 2005-08-25 21:33:28Z davea $";
