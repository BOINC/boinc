#ifndef _CRYPT_
#define _CRYPT_
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

#endif
