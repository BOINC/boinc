// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#endif

#ifdef _WIN32
#include <wincrypt.h>
#endif

#ifdef ANDROID
#include <stdlib.h>
#endif

#include "boinc_stdio.h"
#include "error_numbers.h"
#include "md5.h"

#include "md5_file.h"

int md5_file(const char* path, char* output, double& nbytes, bool is_gzip) {
    unsigned char buf[4096];
    unsigned char binout[16];
    md5_state_t state;
    int i, n;

    nbytes = 0;
    FILE *f = boinc::fopen(path, "rb");
    if (!f) {
        boinc::fprintf(stderr, "md5_file: can't open %s\n", path);
        boinc::perror("md5_file");

        return ERR_FOPEN;
    }
    md5_init(&state);

    // check and skip gzip header if needed
    //
    if (is_gzip) {
        n = (int)boinc::fread(buf, 1, 10, f);
        if (n != 10) {
            boinc::fclose(f);
            return ERR_BAD_FORMAT;
        }
        if (buf[0] != 0x1f || buf[1] != 0x8b || buf[2] != 0x08) {
            boinc::fclose(f);
            return ERR_BAD_FORMAT;
        }
        nbytes = 10;
    }

    while (1) {
        n = (int)boinc::fread(buf, 1, 4096, f);
        if (n<=0) break;
        nbytes += n;
        md5_append(&state, buf, n);
    }
    md5_finish(&state, binout);
    for (i=0; i<16; i++) {
        sprintf(output+2*i, "%02x", binout[i]);
    }
    output[32] = 0;
    boinc::fclose(f);
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
        if (GetLastError() == NTE_BAD_KEYSET) {
            if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
                return -1;
            }
        } else {
            return -2;
        }
    }

    if(! CryptGenRandom(hCryptProv, (DWORD) 32, (BYTE *) buf)) {
        CryptReleaseContext(hCryptProv, 0);
        return -3;
    }

    CryptReleaseContext(hCryptProv, 0);
#elif defined ANDROID
    return -1;
#else
    FILE* f = boinc::fopen("/dev/random", "r");
    if (!f) {
        return -1;
    }
    size_t n = boinc::fread(buf, 32, 1, f);
    boinc::fclose(f);
    if (n != 1) return -2;
#endif
    md5_block((const unsigned char*)buf, 32, out);
    return 0;
}

