// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include <string.h>
#include <stdlib.h>

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

