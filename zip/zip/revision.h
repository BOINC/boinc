/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  revision.h by Mark Adler.
 */

#ifndef __revision_h
#define __revision_h 1

/* For api version checking */
#define Z_MAJORVER   2
#define Z_MINORVER   3
#define Z_PATCHLEVEL 0
#define Z_BETALEVEL ""

#define VERSION "2.3"
#define REVDATE "November 29th 1999"

#define DW_MAJORVER    Z_MAJORVER
#define DW_MINORVER    Z_MINORVER
#define DW_PATCHLEVEL  Z_PATCHLEVEL

#ifndef WINDLL
/* Copyright notice for binary executables--this notice only applies to
 * those (zip, zipcloak, zipsplit, and zipnote), not to this file
 * (revision.h).
 */

#ifndef DEFCPYRT                     /* copyright[] gets defined only once ! */
extern ZCONST char *copyright[2];    /* keep array sizes in sync with number */
extern ZCONST char *swlicense[40];   /*  of text line in definition below !! */
extern ZCONST char *versinfolines[7];

#else /* DEFCPYRT */

ZCONST char *copyright[] = {
"Copyright (C) 1990-1999 Info-ZIP",
"Type '%s \"-L\"' for software license."
/* XXX still necessary ???? */
#ifdef AZTEC_C
,        /* extremely lame compiler bug workaround */
#endif
};

ZCONST char *versinfolines[] = {
"This is %s %s (%s), by Info-ZIP.",
"Currently maintained by Onno van der Linden. Please send bug reports to",
"the authors at Zip-Bugs@lists.wku.edu; see README for details.",
"",
"Latest sources and executables are at ftp://ftp.cdrom.com/pub/infozip, as of",
"above date; see http://www.cdrom.com/pub/infozip/Zip.html for other sites.",
""
};

ZCONST char *swlicense[] = {
"Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.",
"",
"For the purposes of this copyright and license, \"Info-ZIP\" is defined as",
"the following set of individuals:",
"",
"   Mark Adler, John Bush, Karl Davis, Harald Denker, Jean-Michel Dubois,",
"   Jean-loup Gailly, Hunter Goatley, Ian Gorman, Chris Herborth, Dirk Haase,",
"   Greg Hartwig, Robert Heath, Jonathan Hudson, Paul Kienitz, David Kirschbaum,",
"   Johnny Lee, Onno van der Linden, Igor Mandrichenko, Steve P. Miller,",
"   Sergio Monesi, Keith Owens, George Petrov, Greg Roelofs, Kai Uwe Rommel,",
"   Steve Salisbury, Dave Smith, Christian Spieler, Antoine Verheijen,",
"   Paul von Behren, Rich Wales, Mike White",
"",
"This software is provided \"as is,\" without warranty of any kind, express",
"or implied.  In no event shall Info-ZIP or its contributors be held liable",
"for any direct, indirect, incidental, special or consequential damages",
"arising out of the use of or inability to use this software.",
"",
"Permission is granted to anyone to use this software for any purpose,",
"including commercial applications, and to alter it and redistribute it",
"freely, subject to the following restrictions:",
"",
"    1. Redistributions of source code must retain the above copyright notice,",
"       definition, disclaimer, and this list of conditions.",
"",
"    2. Redistributions in binary form must reproduce the above copyright",
"       notice, definition, disclaimer, and this list of conditions in",
"       documentation and/or other materials provided with the distribution.",
"",
"    3. Altered versions--including, but not limited to, ports to new operating",
"       systems, existing ports with new graphical interfaces, and dynamic,",
"       shared, or static library versions--must be plainly marked as such",
"       and must not be misrepresented as being the original source.  Such",
"       altered versions also must not be misrepresented as being Info-ZIP",
"       releases--including, but not limited to, labeling of the altered",
"       versions with the names \"Info-ZIP\" (or any variation thereof, including,",
"       but not limited to, different capitalizations), \"Pocket UnZip,\" \"WiZ\"",
"       or \"MacZip\" without the explicit permission of Info-ZIP.  Such altered",
"       versions are further prohibited from misrepresentative use of the",
"       Zip-Bugs or Info-ZIP e-mail addresses or of the Info-ZIP URL(s).",
};
#endif /* DEFCPYRT */
#endif /* !WINDLL */
#endif /* !__revision_h */
