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

#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>   // For stat()

#include "translate.h"

static static const uint32_t MSGCATALOG_MAGIC    = 0x950412de;
const uint32_t MSGCATALOG_MAGIC_SW = 0xde120495;

#define MAXCATALOGS 5

// an entry in the string table
struct MsgTableEntry {
    uint32_t    nLen;           // length of the string
    uint32_t    ofsString;      // pointer to the string
};

// header of a .mo file
struct MsgCatalogHeader {
    uint32_t    magic,          // offset +00:  magic id
                revision,       //        +04:  revision
                numStrings;     //        +08:  number of strings in the file
    uint32_t    ofsOrigTable,   //        +0C:  start of original string table
                ofsTransTable;  //        +10:  start of translated string table
    uint32_t    nHashSize,      //        +14:  hash table size
                ofsHashTable;   //        +18:  offset of hash table start
};


struct MsgCatalogData {
    uint8_t         *pData;         // Pointer to catalog data
    uint32_t        nSize;          // Amount of memory pointed to by pData.
    uint32_t        NumStrings;     // number of strings in this domain
    bool            bSwapped;       // wrong endianness?
    MsgTableEntry   *pOrigTable;    // pointer to original strings
    MsgTableEntry   *pTransTable;   // pointer to translated strings
    bool            bIsEnglish;     // true if language code == "en"
};

static struct MsgCatalogData theCatalogData[MAXCATALOGS];

static uint32_t     numLoadedCatalogs = 0;

// swap the 2 halves of 32 bit integer if needed
static uint32_t Swap(uint32_t ui, bool bSwapped) {
      return bSwapped ? (ui << 24) | ((ui & 0xff00) << 8) |
                          ((ui >> 8) & 0xff00) | (ui >> 24)
                        : ui;
}


// load a catalog from disk
// open disk file and read in it's contents
//
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
bool LoadCatalog(const char * catalogsDir,
                const char *languageCode,
                const char *catalogName
                ) {
    char searchPath[MAXPATHLEN];
    struct stat sbuf;
    char temp[32];
    char *underscore;
    FILE * f;
    uint8_t *pData;
    uint32_t nSize;
    uint32_t NumStrings;
    bool  bSwapped;
    MsgTableEntry *pOrigTable;
    MsgTableEntry *pTransTable;
    MsgCatalogData *pCatalog;

    if (!strcmp("en", languageCode)) {
        theCatalogData[numLoadedCatalogs++].bIsEnglish = true;
        return true;
    }
  
    strlcpy(searchPath, catalogsDir, sizeof(searchPath));
    strlcat(searchPath, languageCode, sizeof(searchPath));
    strlcat(searchPath, "/", sizeof(searchPath));
    strlcat(searchPath, catalogName, sizeof(searchPath));
    strlcat(searchPath, ".mo", sizeof(searchPath));
    if (stat(searchPath, &sbuf) != 0) {
        // Try just base locale name: for things like "fr_BE" (belgium
        // french) we should use "fr" if no belgium specific message 
        // catalogs exist
        strlcpy(temp, languageCode, sizeof(temp));
        underscore = strchr(temp, (int)'_');
        if (underscore == NULL) return false;
        *underscore = '\0';
        strlcpy(searchPath, catalogsDir, sizeof(searchPath));
        strlcat(searchPath, temp, sizeof(searchPath));
        strlcat(searchPath, "/", sizeof(searchPath));
        strlcat(searchPath, catalogName, sizeof(searchPath));
        strlcat(searchPath, ".mo", sizeof(searchPath));
        if (stat(searchPath, &sbuf) != 0) return false;
    }
    

    pData = (uint8_t*)malloc(sbuf.st_size);
    if (pData == NULL) return false;
    f = fopen(searchPath, "r");
    if (f == NULL) {
        free(pData);
        pData = NULL;
        return false;
    }
    nSize = fread(pData, 1, sbuf.st_size, f);
    fclose(f);
    if (nSize != sbuf.st_size) {
        free(pData);
        pData = NULL;
        return false;
    }
    
    // examine header
    bool bValid = nSize + (size_t)0 > sizeof(MsgCatalogHeader);

    MsgCatalogHeader *pHeader = (MsgCatalogHeader *)pData;
    if ( bValid ) {
    // we'll have to swap all the integers if it's true
    bSwapped = pHeader->magic == MSGCATALOG_MAGIC_SW;

    // check the magic number
    bValid = bSwapped || pHeader->magic == MSGCATALOG_MAGIC;
    }

    if ( !bValid ) {
        // it's either too short or has incorrect magic number
        free(pData);
        pData = NULL;
        return false;
    }

    // initialize
    NumStrings  = Swap(pHeader->numStrings, bSwapped);
    pOrigTable  = (MsgTableEntry *)(pData +
                   Swap(pHeader->ofsOrigTable, bSwapped));
    pTransTable = (MsgTableEntry *)(pData +
                   Swap(pHeader->ofsTransTable, bSwapped));


    // now parse catalog's header and try to extract catalog charset
    uint32_t ofsString = Swap(pOrigTable->ofsString, bSwapped);
    // this check could fail for a corrupt message catalog
    if ( ofsString + Swap(pOrigTable->nLen, bSwapped) <= nSize) {
        char *begin = strstr((char *)pTransTable,
                            "Content-Type: text/plain; charset="
                            );
        if (begin != NULL) {
            if (strncasecmp(begin, "utf-8", 5)) {
                fprintf(stderr, "File %s is not utf-8!\n", searchPath);
                free(pData);
                pData = NULL;
                return false;
            }
        }
    }

    pCatalog = &(theCatalogData[numLoadedCatalogs++]);
    pCatalog->pData = pData;
    pCatalog->nSize = nSize;
    pCatalog->NumStrings = NumStrings;
    pCatalog->bSwapped = bSwapped;
    pCatalog->pOrigTable = pOrigTable;
    pCatalog->pTransTable = pTransTable;
    pCatalog->bIsEnglish = false;
    
    return true;
}


// Searches through the catalogs in the order they were added
// until it finds a translation for the src string, and
// returns a ponter to the UTF-8 encoded localized string.
// Returns a pointer to the original string if no translation
// was found.
//
uint8_t * _(char *src) {
    int i, j;
    MsgCatalogData *pCatalog;
    
    for (j=0; j<numLoadedCatalogs; ++j) {
        pCatalog = &(theCatalogData[j]);
        
        // Since the original strings are English, no 
        // translation is needed for language en.
        if (pCatalog->bIsEnglish) {
            return (uint8_t *)src;
        }
        
        if (pCatalog->pData == NULL) continue;    // Should never happen
        for (i=0; i< pCatalog->NumStrings; ++i) {
            if (!strcmp((char *)pCatalog->pData + pCatalog->pOrigTable[i].ofsString, src)) {
                return (pCatalog->pData + pCatalog->pTransTable[i].ofsString);
            }
        }
    }

    return (uint8_t *)src;
}


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
                                ) {
    if (numLoadedCatalogs >= (MAXCATALOGS)) {
        fprintf(stderr, "Trying to load too many catalogs\n");
        return false;
    }

    if (!LoadCatalog(catalogsDir, languageCode, catalogName)) {
        fprintf(stderr,
                "could not load catalog %s for langage %s\n",
                catalogName, languageCode
                );
        return false;
    }
    
    return true;
}


void BOINCTranslationInit() {
    int i;
    MsgCatalogData *pCatalog;
    
    numLoadedCatalogs = 0;
    
    for (i=0; i<MAXCATALOGS; ++i) {
        pCatalog = &(theCatalogData[i]);
        pCatalog->pData = NULL;
        pCatalog->nSize = 0;
        pCatalog->NumStrings = 0;
        pCatalog->bSwapped = false;
        pCatalog->pOrigTable = NULL;
        pCatalog->pTransTable = NULL;
        pCatalog->bIsEnglish = false;
    }
}


void BOINCTranslationCleanup() {
    int i;
    MsgCatalogData *pCatalog;
    
    for (i=0; i<MAXCATALOGS; ++i) {
        pCatalog = &(theCatalogData[i]);
        if (pCatalog->pData) free(pCatalog->pData);
        pCatalog->pData = NULL;
        pCatalog->nSize = 0;
        pCatalog->NumStrings = 0;
        pCatalog->bSwapped = false;
        pCatalog->pOrigTable = NULL;
        pCatalog->pTransTable = NULL;
        pCatalog->bIsEnglish = false;
    }

    numLoadedCatalogs = 0;
}
