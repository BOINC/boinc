static volatile const char *BOINCrcsid="$Id$";
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstdio>
#endif

#include "md5.h"
#include "md5_file.h"
#include "error_numbers.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

int md5_file(const char* path, char* output, double& nbytes) {
    unsigned char buf[4096];
    unsigned char binout[16];
    FILE* f;
    md5_state_t state;
    int i, n;

    nbytes = 0;
    f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "md5_file: can't open %s\n", path);
        perror("md5_file");
        return ERR_FOPEN;
    }
    md5_init(&state);
    while (1) {
        n = fread(buf, 1, 4096, f);
        if (n<=0) break;
        nbytes += n;
        md5_append(&state, buf, n);
    }
    md5_finish(&state, binout);
    for (i=0; i<16; i++) {
        sprintf(output+2*i, "%02x", binout[i]);
    }
    output[32] = 0;
    fclose(f);
    return 0;
}

int md5_block(const unsigned char* data, int nbytes, char* output) {
    unsigned char binout[16];
    int i;

    md5_state_t state;
    md5_init(&state);
    md5_append(&state, data, nbytes);
    md5_finish(&state, binout);
    for (i=0; i<16; i++) {
        sprintf(output+2*i, "%02x", binout[i]);
    }
    output[32] = 0;
    return 0;
}

std::string md5_string(const unsigned char* data, int nbytes)
{
    char output[32];
    md5_block(data, nbytes, output);
    return std::string(output);
}

