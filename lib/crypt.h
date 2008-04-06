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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef H_CRYPT
#define H_CRYPT

// We're set up to use either RSAEuro or the OpenSSL crypto library.
// We use our own data structures (R_RSA_PUBLIC_KEY and R_RSA_PRIVATE_KEY)
// to store keys in either case.

// Only define these here if they haven't been defined elsewhere
#if !(defined(USE_OPENSSL) || defined(USE_RSAEURO))
#define USE_OPENSSL 1
//#define USE_RSAEURO 1
#endif

#include <stdio.h>
#include <string.h>

#ifdef USE_RSAEURO
#include "rsaeuro.h"
extern "C" {
#include "rsa.h"
}

#endif

#ifdef USE_OPENSSL
#include <openssl/rsa.h>

#define MAX_RSA_MODULUS_BITS 1024
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)

typedef struct {
  unsigned short int bits;                     /* length in bits of modulus */
  unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
  unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* public exponent */
} R_RSA_PUBLIC_KEY;

typedef struct {
  unsigned short int bits;                     /* length in bits of modulus */
  unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
  unsigned char publicExponent[MAX_RSA_MODULUS_LEN];     /* public exponent */
  unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* private exponent */
  unsigned char prime[2][MAX_RSA_PRIME_LEN];   /* prime factors */
  unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];     /* exponents for CRT */
  unsigned char coefficient[MAX_RSA_PRIME_LEN];          /* CRT coefficient */
} R_RSA_PRIVATE_KEY;

// functions to convert between OpenSSL's keys (using BIGNUMs)
// and our binary format

extern void openssl_to_keys(
    RSA* rp, int nbits, R_RSA_PRIVATE_KEY& priv, R_RSA_PUBLIC_KEY& pub
);
extern void private_to_openssl(R_RSA_PRIVATE_KEY& priv, RSA* rp);
extern void public_to_openssl(R_RSA_PUBLIC_KEY& pub, RSA* rp);

#endif

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

extern int print_hex_data(FILE* f, DATA_BLOCK&);
extern int sprint_hex_data(char* p, DATA_BLOCK&);
extern int scan_hex_data(FILE* f, DATA_BLOCK&);
extern int print_key_hex(FILE*, KEY* key, int len);
extern int scan_key_hex(FILE*, KEY* key, int len);
extern int sscan_key_hex(const char*, KEY* key, int len);
extern int encrypt_private(
    R_RSA_PRIVATE_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out
);
extern int decrypt_public(
    R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out
);
extern int sign_file(
    const char* path, R_RSA_PRIVATE_KEY&, DATA_BLOCK& signature
);
extern int sign_block(
    DATA_BLOCK& data, R_RSA_PRIVATE_KEY&, DATA_BLOCK& signature
);
extern int verify_file(
    const char* path, R_RSA_PUBLIC_KEY&, DATA_BLOCK& signature, bool&
);
extern int verify_file2(
    const char* path, const char* signature, const char* key, bool&
);
extern int verify_string(
    const char* text, const char* signature, R_RSA_PUBLIC_KEY&, bool&
);
extern int verify_string2(
    const char* text, const char* signature, const char* key, bool&
);
extern int read_key_file(const char* keyfile, R_RSA_PRIVATE_KEY& key);
extern int generate_signature(
    char* text_to_sign, char* signature_hex, R_RSA_PRIVATE_KEY& key
);

#endif
