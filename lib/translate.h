// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// Simplified string localization code adapted from
// wxWidgets src/common/intl.cpp.
// Assumes all catalogs are encoded UTF-8
//
// For sample code to get preferred system language, see
// wxLocale::GetSystemLanguage() in wxWidgets src/common/intl.cpp

#ifndef _BOINC_TRANSLATE_H
#define _BOINC_TRANSLATE_H

// We use only C APIs for maximum portability
#ifdef __cplusplus
extern "C" {
#endif

void BOINCTranslationInit();

// catalogsDir is the path to the locale directory in
// the BOINC Data directory (ending in "locale/").
//
// language code is the standad language code such
// as "fr" or "zh_CN".
//
// catalogName is the domain (the file name of the
// *.mo file without the .mo such as "BOINC-Manager").
//
// Returns true if successful.
//
bool BOINCTranslationAddCatalog(const char * catalogsDir,
                                const char *languageCode,
                                const char * catalogName
                                );

// Searches through the catalogs in the order they were added
// until it finds a translation for the src string, and
// returns a ponter to the UTF-8 encoded localized string.
// Returns a pointer to the original string if no translation
// was found.
//
uint8_t * _(char *src);

void BOINCTranslationCleanup();

#ifdef __cplusplus
} // extern "C" {
#endif

#endif  // _BOINC_TRANSLATE_H
