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

#ifndef H_CRYPT
#define H_CRYPT
// some interface functions for RSAEuro

#include <cstdio>

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
