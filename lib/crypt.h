// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include <vector>
#include <string>
#include <memory>

#include <openssl/rsa.h>
#include <openssl/evp.h>

#include "cert_sig.h"

template <typename T, void (*FreeFunc)(T*)>
struct OpenSSLDeleter {
    void operator()(T* ptr) const noexcept{
        if (ptr) {
            FreeFunc(ptr);
        }
    }
};

using unique_EVP_PKEY =
    std::unique_ptr<EVP_PKEY, OpenSSLDeleter<EVP_PKEY, EVP_PKEY_free>>;
using unique_PKEY_CTX =
    std::unique_ptr<EVP_PKEY_CTX,
    OpenSSLDeleter<EVP_PKEY_CTX, EVP_PKEY_CTX_free>>;

#define MAX_RSA_MODULUS_BITS 1024
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)

// We're set up to use OpenSSL crypto library.
// We use our own data structures (R_RSA_PUBLIC_KEY and R_RSA_PRIVATE_KEY)
// to store keys.

typedef struct {
    unsigned short int bits;
    unsigned char modulus[MAX_RSA_MODULUS_LEN];
    unsigned char exponent[MAX_RSA_MODULUS_LEN];
} R_RSA_PUBLIC_KEY;

typedef struct {
    unsigned short int bits;
    unsigned char modulus[MAX_RSA_MODULUS_LEN];
    unsigned char publicExponent[MAX_RSA_MODULUS_LEN];
    unsigned char exponent[MAX_RSA_MODULUS_LEN];
    unsigned char prime[2][MAX_RSA_PRIME_LEN];
    unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];
    unsigned char coefficient[MAX_RSA_PRIME_LEN];
} R_RSA_PRIVATE_KEY;

// functions to convert between OpenSSL's keys (using BIGNUMs)
// and our binary format

// function returns a tuple of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'R_RSA_PRIVATE_KEY' value is the private key
// converted from OpenSSL's EVP_PKEY
// third 'R_RSA_PUBLIC_KEY' value is the public key
// converted from OpenSSL's EVP_PKEY
// if the first value is not 0, the second and third values are invalid
extern std::tuple<int, R_RSA_PRIVATE_KEY, R_RSA_PUBLIC_KEY> openssl_to_keys(
    const unique_EVP_PKEY& pkey
);
extern unique_EVP_PKEY private_to_openssl(const R_RSA_PRIVATE_KEY& priv);
extern unique_EVP_PKEY public_to_openssl(const R_RSA_PUBLIC_KEY& pub);
// function returns a pair of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'R_RSA_PUBLIC_KEY' value is the public key
// converted from OpenSSL's EVP_PKEY
// if the first value is not 0, the second value is invalid
extern std::pair<int, R_RSA_PUBLIC_KEY> openssl_to_public(
    const unique_EVP_PKEY& pkey
);
// function returns a pair of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'R_RSA_PRIVATE_KEY' value is the private key
// converted from OpenSSL's EVP_PKEY
// if the first value is not 0, the second value is invalid
extern std::pair<int, R_RSA_PRIVATE_KEY> openssl_to_private(
    const unique_EVP_PKEY& pkey
);

struct KEY {
    unsigned short int bits;
    unsigned char data[1];
};

extern std::string sprint_hex_data(const std::vector<uint8_t> &data);
#ifdef _USING_FCGI_
#undef FILE
#endif
extern bool print_hex_data(FILE *f, const std::vector<uint8_t> &data);
extern std::vector<uint8_t> scan_hex_data(FILE *f);
extern bool print_private_key_hex(FILE *f, const R_RSA_PRIVATE_KEY& key);
extern bool print_public_key_hex(FILE *f, const R_RSA_PUBLIC_KEY& key);
// return a pair of values:
// first 'bool' value indicates if the key was read successfully
// (true - success, false - failure)
// second 'R_RSA_PUBLIC_KEY' value is the public key read from the file
// if the first value is false, the second value is invalid
extern std::pair<bool, R_RSA_PUBLIC_KEY> scan_public_key_hex(FILE *f);
// return a pair of values:
// first 'bool' value indicates if the key was read successfully
// (true - success, false - failure)
// second 'R_RSA_PRIVATE_KEY' value is the private key read from the file
// if the first value is false, the second value is invalid
extern std::pair<bool, R_RSA_PRIVATE_KEY> scan_private_key_hex(FILE *f);
#ifdef _USING_FCGI_
#define FILE FCGI_FILE
#endif
extern std::vector<uint8_t> encrypt_private(const R_RSA_PRIVATE_KEY& key,
    const std::vector<uint8_t>& in);
extern std::vector<uint8_t> decrypt_public(const R_RSA_PUBLIC_KEY& key,
    const std::vector<uint8_t>& in);
extern std::vector<uint8_t> sign_file(
    const std::string& path, const R_RSA_PRIVATE_KEY& key
);
extern std::vector<uint8_t> sign_block(
    const std::vector<uint8_t>& data, const R_RSA_PRIVATE_KEY& key
);
// return a pair of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'bool' value indicates if the signature is valid
// (true - valid, false - invalid)
extern std::pair<int, bool> check_file_signature(
    const std::string& md5,
    const R_RSA_PUBLIC_KEY& key,
    const std::vector<uint8_t>& signature
);
// return a pair of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'bool' value indicates if the signature is valid
// (true - valid, false - invalid)
extern std::pair<int, bool> check_file_signature(
    const std::string& md5,
    const std::string& signature,
    const std::string& key
);
// return a pair of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'bool' value indicates if the signature is valid
// (true - valid, false - invalid)
extern std::pair<int, bool> check_string_signature(
    const std::string& text,
    const std::string& signature,
    const R_RSA_PUBLIC_KEY& key
);
// return a pair of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'bool' value indicates if the signature is valid
// (true - valid, false - invalid)
extern std::pair<int, bool> check_string_signature(
    const std::string& text,
    const std::string& signature,
    const std::string& key
);
extern bool print_raw_data(FILE *f, const std::vector<uint8_t> &x);
extern std::vector<uint8_t> scan_raw_data(FILE *f);
// return a pair of values:
// first 'int' value indicates and error code
// (0 - success, everything else - error)
// second 'R_RSA_PRIVATE_KEY' value is the private key read from the file
// if the first value is not 0, the second value is invalid
extern std::pair<int, R_RSA_PRIVATE_KEY> read_key_file(
    const std::string& keyfile
);
extern std::string generate_signature(
    const std::string& text_to_sign,
    const R_RSA_PRIVATE_KEY& key
);

// return a pair of values:
// first 'bool' value indicates if the signature is valid
// second 'string' value return a path to the certificate
// that was used for signing
// if the first value is false, the second value is invalid
extern std::pair<bool, std::string> check_validity(
    const std::string &certPath,
    const std::string &origFile,
    const std::vector<uint8_t> &signature,
    const std::string &caPath
);

bool cert_verify_file(CERT_SIGS *signatures, const std::string &origFile,
    const std::string &trustLocation);
#endif
