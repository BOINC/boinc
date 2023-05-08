///////////////////////////////////////////////////////////////////////////////
// Original Name:        src/common/intl.cpp
// Original Purpose:     Internationalization and localisation for wxWidgets
// Original Author:      Vadim Zeitlin
// Modified by: Charlie Fenton for BOINC  13 June, 2013
// Created:     29/01/98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
//
// This file is part of BOINC.
// http://boinc.berkeley.edu
//
// Simplified string localization code for use in BOINC adapted from
// wxWidgets src/common/intl.cpp.
//
// For sample code to get preferred system language, see
// wxLocale::GetSystemLanguage() in wxWidgets src/common/intl.cpp
//
// Those portions of this code which are taken from wxWidgets are
// Copyright:   (c) 1998 Vadim Zeitlin
// and are covered by the wxWidgets license which can be found here:
// <http://www.wxwidgets.org/about/licence3.txt>
//
// This code assumes all catalogs are encoded UTF-8
// (Note: wxstd.mo files are encoded ISO 8859-1 not UTF-8.)
//
// We recommended that you first call BOINCTranslationAddCatalog()
// for the user's preferred language, then for the user's second
// preferred language and (optionally) then for the user's third
// preferred language.  This will make it more likely that a
// translation will be found in some language useful to the user.
//

#ifndef BOINC_TRANSLATE_H
#define BOINC_TRANSLATE_H

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
// returns a pointer to the UTF-8 encoded localized string.
// Returns a pointer to the original string if no translation
// was found.
//
uint8_t * _(char *src);

void BOINCTranslationCleanup();

#ifdef __cplusplus
} // extern "C" {
#endif

#endif  // BOINC_TRANSLATE_H
