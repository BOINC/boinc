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

std::string md5_string(const unsigned char* data, int nbytes) {
    char output[MD5_LEN];
    md5_block(data, nbytes, output);
    return std::string(output);
}


const char *BOINC_RCSID_5a0dc438fe = "$Id$";
