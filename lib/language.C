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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstring>
#include <cstdlib>
#endif

#include "util.h"
#include "parse.h"

#include "language.h"

LANGUAGE::LANGUAGE() {
    language_file_contents = NULL;
}

LANGUAGE::~LANGUAGE() {
    if (language_file_contents) free(language_file_contents);
    language_file_contents = NULL;
}

int LANGUAGE::read_language_file(char *file_name) {
    int retval;

    // TODO: put in a size limitation here?

    if (language_file_contents) free(language_file_contents);
    language_file_contents = NULL;

    retval = read_file_malloc(file_name, language_file_contents);
    if (retval) return retval;

    return 0;
}

int LANGUAGE::get_translation(char *section_name, char *entry_name,
                              char *translation, int trans_size) {
    char buf[256], *sec_ptr, buf2[256];

    // If we never opened the language file, return the default value
    if (!language_file_contents) goto return_default;

    // Find the specified section
    sprintf(buf, "[%s]", section_name);
    sec_ptr = strstr(language_file_contents,buf);
    if (!sec_ptr) goto return_default;

    // Find the translation in the section
    // skip the header line
    sec_ptr += strlen(buf);
    sprintf(buf, "%s=", entry_name);
    while (sgets(buf2, 256, sec_ptr)) {
        if (!strncmp(buf, buf2, strlen(buf))) {
            safe_strncpy(translation,buf2+strlen(buf),trans_size);
            return trans_size;
        } else if (!strncmp("[", buf2, 1)) {
            goto return_default;
        }
    }

return_default:
    safe_strncpy(translation,entry_name,trans_size);
    return trans_size;
}


const char *BOINC_RCSID_c4c9d87372 = "$Id$";
