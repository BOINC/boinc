// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// syntax: sign_executable data_file private_key_file

#include <stdio.h>

#include "config.h"
#include "crypt.h"

int sign_executable(char* path, char* code_sign_keyfile, char* signature_text) {
    DATA_BLOCK signature;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    R_RSA_PRIVATE_KEY code_sign_key;
    int retval = read_key_file(code_sign_keyfile, code_sign_key);
    if (retval) {
        fprintf(stderr, "add: can't read key\n");
        exit(1);
    }
    signature.data = signature_buf;
    sign_file(path, code_sign_key, signature);
    sprint_hex_data(signature_text, signature);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "syntax: sign_executable data_file private_key_file\n"
            "\n"
            "Writes signature to stdout.\n"
        );
        return 1;
    }

    char signature_text[1024];
    if (sign_executable(argv[1], argv[2], signature_text)) {
        return 1;
    }
    printf("%s", signature_text);

    return 0;
}
