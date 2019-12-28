// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#endif

#ifdef _WIN32
#include <wincrypt.h>
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#ifdef ANDROID
#include <stdlib.h>
#endif

#include "error_numbers.h"
#include "md5.h"

#include "md5_file.h"

int md5_file(const char* path, char* output, double& nbytes, bool is_gzip) {
    unsigned char buf[4096];
    unsigned char binout[16];
    md5_state_t state;
    int i, n;

    nbytes = 0;
#ifndef _USING_FCGI_
    FILE *f = fopen(path, "rb");
#else
    FILE *f = FCGI::fopen(path, "rb");
#endif
    if (!f) {
        fprintf(stderr, "md5_file: can't open %s\n", path);
#ifndef _USING_FCGI_
        std::perror("md5_file");
#else
        FCGI::perror("md5_file");
#endif

        return ERR_FOPEN;
    }
    md5_init(&state);

    // check and skip gzip header if needed
    //
    if (is_gzip) {
        n = (int)fread(buf, 1, 10, f);
        if (n != 10) {
            fclose(f);
            return ERR_BAD_FORMAT;
        }
        if (buf[0] != 0x1f || buf[1] != 0x8b || buf[2] != 0x08) {
            fclose(f);
            return ERR_BAD_FORMAT;
        } 
        nbytes = 10;
    }

    while (1) {
        n = (int)fread(buf, 1, 4096, f);
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

int md5_block(const unsigned char* data, int nbytes, char* output,
    const unsigned char* data2, int nbytes2     // optional 2nd block
) {
    unsigned char binout[16];
    int i;

    md5_state_t state;
    md5_init(&state);
    md5_append(&state, data, nbytes);
    if (data2) {
        md5_append(&state, data2, nbytes2);
    }
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

// make a secure (i.e. hard to guess)
// 32-char string using OS-supplied random bits
//
int make_secure_random_string_os(char* out) {
    char buf[256];
#ifdef _WIN32
    HCRYPTPROV hCryptProv;
        
    if(! CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) {
        return -1;
    }
    
    if(! CryptGenRandom(hCryptProv, (DWORD) 32, (BYTE *) buf)) {
        CryptReleaseContext(hCryptProv, 0);
        return -1;
    }
        
    CryptReleaseContext(hCryptProv, 0);
#elif defined ANDROID
    return -1;
#else
#ifndef _USING_FCGI_
    FILE* f = fopen("/dev/random", "r");
#else
    FILE* f = FCGI::fopen("/dev/random", "r");
#endif
    if (!f) {
        return -1;
    }
    size_t n = fread(buf, 32, 1, f);
    fclose(f);
    if (n != 1) return -1;
#endif
    md5_block((const unsigned char*)buf, 32, out);
    return 0;
}

