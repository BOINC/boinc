// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//
//  mac_branding.h
//

#ifndef mac_branding_h
#define mac_branding_h

#define NUMBRANDS 5

extern char *appName[NUMBRANDS];
extern char *appPath[NUMBRANDS];
extern char *brandName[NUMBRANDS];
extern char *saverName[NUMBRANDS];
extern char *receiptName[NUMBRANDS];
extern char *skinName[NUMBRANDS];
extern char *logoFile[NUMBRANDS];

// The strings for these arrays are initialized in mac_branding.cpp, not here to avoid
// duplicate definition errors when mac_branding.h is included from multiple source files.
//

extern bool check_branding_arrays(char* badArrayNames, int len);

#endif /* mac_branding_h */
