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

// syntax: sign_executable <exectuable_path> <code_sign_file>

#include "crypt.h"
#include "backend_lib.h"
#include <cstdlib>

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
        fprintf(stderr, "syntax: sign_executable <path> <code_sign_file>\n"
                "\n"
                "Outputs to stdout.\n");
        return 1;
    }

    char signature_text[1024];
    if (sign_executable(argv[1], argv[2], signature_text)) {
        return 1;
    }
    printf("%s", signature_text);

    return 0;
}
