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

#ifndef BOINC_CRYPT_H
#define BOINC_CRYPT_H

// We're set up to use either RSAEuro or the OpenSSL crypto library.
// We use our own data structures (R_RSA_PUBLIC_KEY and R_RSA_PRIVATE_KEY)
// to store keys in either case.

#include <cstdio>

#include <openssl/rsa.h>

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) /* OpenSSL 1.1.0+ */
#define HAVE_OPAQUE_EVP_PKEY 1 /* since 1.1.0 -pre3 */
#define HAVE_OPAQUE_RSA_DSA_DH 1 /* since 1.1.0 -pre5 */
#endif

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
extern int openssl_to_private(RSA *from, R_RSA_PRIVATE_KEY *to);

struct KEY {
    unsigned short int bits;
    unsigned char data[1];
};

struct DATA_BLOCK {
    unsigned char* data;
    unsigned int len;
};

#define MIN_OUT_BUFFER_SIZE (MAX_RSA_MODULUS_LEN+1)

// the size of a binary signature (encrypted MD5)
//
#define SIGNATURE_SIZE_BINARY MIN_OUT_BUFFER_SIZE

// size of text-encoded signature
#define SIGNATURE_SIZE_TEXT (SIGNATURE_SIZE_BINARY*2+20)
extern int sprint_hex_data(char* p, DATA_BLOCK&);
#ifdef _USING_FCGI_
#undef FILE
#endif
extern int print_hex_data(FILE* f, DATA_BLOCK&);
extern int scan_hex_data(FILE* f, DATA_BLOCK&);
extern int print_key_hex(FILE*, KEY* key, int len);
extern int scan_key_hex(FILE*, KEY* key, int len);
#ifdef _USING_FCGI_
#define FILE FCGI_FILE
#endif
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
extern int check_file_signature(
    const char* md5, R_RSA_PUBLIC_KEY&, DATA_BLOCK& signature, bool&
);
extern int check_file_signature2(
    const char* md5, const char* signature, const char* key, bool&
);
extern int check_string_signature(
    const char* text, const char* signature, R_RSA_PUBLIC_KEY&, bool&
);
extern int check_string_signature2(
    const char* text, const char* signature, const char* key, bool&
);
extern int print_raw_data(FILE* f, DATA_BLOCK& x);
extern int scan_raw_data(FILE *f, DATA_BLOCK& x);
extern int read_key_file(const char* keyfile, R_RSA_PRIVATE_KEY& key);
extern int generate_signature(
    char* text_to_sign, char* signature_hex, R_RSA_PRIVATE_KEY& key
);

//   Check if sfileMsg (of length sfsize) has been created from sha1_md using the
//   private key belonging to the public key file cFile
//   Return:
//    1: YES
//    0: NO or error
extern int check_validity_of_cert(
    const char *cFile, const unsigned char *sha1_md,
    unsigned char *sfileMsg, const int sfsize, const char* caPath
);

extern char *check_validity(const char *certPath, const char *origFile,
    unsigned char *signature, char* caPath
);

struct CERT_SIGS;

int cert_verify_file(
    CERT_SIGS* signatures, const char* origFile, const char* trustLocation
);
#endif
