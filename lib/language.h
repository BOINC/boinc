// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// flags determining what is written to standard out.
// (errors go to stderr)
//
// NOTE: all writes to stdout should have an if (log.*) {} around them.
//

#ifndef _LANGUAGE_H_
#define _LANGUAGE_H_

#ifndef _WIN32
#include <cstdio>
#endif

class LANGUAGE {
    char *language_file_contents;
public:
    LANGUAGE();
    ~LANGUAGE();
    int read_language_file(char *);
    int get_translation(char *, char *, char *, int );
};

#endif

