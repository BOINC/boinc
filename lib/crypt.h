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

#ifndef H_CRYPT
#define H_CRYPT
// some interface functions for RSAEuro

#include <stdio.h>

#include "rsaeuro.h"
extern "C" {
#include "rsa.h"
}

struct KEY {
    unsigned short int bits;
    unsigned char data[1];
};

struct DATA_BLOCK {
    unsigned char* data;
    unsigned int len;
};

#define MIN_OUT_BUFFER_SIZE MAX_RSA_MODULUS_LEN+1

// the size of a binary signature (encrypted MD5)
//
#define SIGNATURE_SIZE_BINARY MIN_OUT_BUFFER_SIZE

// size of text-encoded signature
#define SIGNATURE_SIZE_TEXT (SIGNATURE_SIZE_BINARY*2+20)

int print_hex_data(FILE* f, DATA_BLOCK&);
int sprint_hex_data(char* p, DATA_BLOCK&);
int scan_hex_data(FILE* f, DATA_BLOCK&);
int sscan_hex_data(char* p, DATA_BLOCK&);
int print_key_hex(FILE*, KEY* key, int len);
int scan_key_hex(FILE*, KEY* key, int len);
int sscan_key_hex(char*, KEY* key, int len);
int encrypt_private(
    R_RSA_PRIVATE_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out, int&
);
int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out);
int sign_file(char* path, R_RSA_PRIVATE_KEY&, DATA_BLOCK& signature);
int sign_block(DATA_BLOCK& data, R_RSA_PRIVATE_KEY&, DATA_BLOCK& signature);
int verify_file(char* path, R_RSA_PUBLIC_KEY&, DATA_BLOCK& signature, bool&);
int verify_file2(char* path, char* signature, char* key, bool&);
int verify_string(char* text, char* signature, R_RSA_PUBLIC_KEY&, bool&);
int verify_string2(char* text, char* signature, char* key, bool&);

int read_key_file(char* keyfile, R_RSA_PRIVATE_KEY& key);

#endif
