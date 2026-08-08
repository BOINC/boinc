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

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <openssl/ssl.h>
#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>

#include "boinc_stdio.h"
#include "md5_file.h"
#include "cert_sig.h"
#include "filesys.h"
#include "error_numbers.h"
#include "util.h"

#include "crypt.h"

using std::string;
using std::vector;

// NOTE: the fast CGI I/O library doesn't have fscanf(),
// so some of the following have been modified to use
// fgets() and sscanf() instead

// write some data in hex notation.
// NOTE: since length may not be known to the reader,
// we follow the data with a non-hex character '.'
//
bool print_hex_data(FILE *f, const vector<uint8_t> &x) {
    if (!f) {
        return false;
    }

    string hex_data = sprint_hex_data(x);
    return fputs(hex_data.c_str(), f) >= 0;

    return true;
}

// same, but write to buffer
//
string sprint_hex_data(const vector<uint8_t> &x) {
    const char hex[] = "0123456789abcdef";
    string result;

    for (size_t i = 0; i < x.size(); ++i) {
        result.push_back(hex[x[i] / 16]);
        result.push_back(hex[x[i] % 16]);
        if (i % 32 == 31) {
            result.push_back('\n');
        }
    }
    if (x.size() % 32 != 0) {
        result.push_back('\n');
    }
    result += ".\n";

    return result;
}

bool print_raw_data(FILE *f, const vector<uint8_t> &x) {
    if (!f) {
        return false;
    }
    for (uint8_t xc : x) {
        fprintf(f, "%c", xc);
    }
    return true;
}

vector<uint8_t> scan_raw_data(FILE *f) {
    vector<uint8_t> data;
    if (!f) {
        return data;
    }
    int j;
    while(EOF != (j = fgetc(f))) {
        data.emplace_back(static_cast<uint8_t>(j));
    }
    return data;
}

// scan data in hex notation.
// stop when you reach a non-parsed character.
//
static vector<uint8_t> sscan_hex_data(const std::vector<uint8_t>& buffer) {
    vector<uint8_t> data;
    if (buffer.empty()) {
        return data;
    }
    const char hex[] = "0123456789abcdef";
    for(size_t i = 0; i < buffer.size(); ) {
        int c1 = buffer[i++];
        if (c1 == '\n') {
            continue; // skip newlines
        }
        if (!isxdigit(c1)) {
            break;
        }
        if (i >= buffer.size()) {
            break; // no second character
        }
        int c2 = buffer[i++];
        if (!isxdigit(c2)) {
            break;
        }
        int value =
            (strchr(hex, tolower(c1)) - hex) * 16 +
            (strchr(hex, tolower(c2)) - hex);
        data.emplace_back(static_cast<uint8_t>(value));
    }
    return data;
}

// same, but read from file
//
vector<uint8_t> scan_hex_data(FILE* f) {
    return sscan_hex_data(scan_raw_data(f));
}

// print a key in ASCII form
//
bool print_key_hex(FILE* f, const KEY* key, size_t size) {
    if (!f || !key || size == 0) {
        return false;
    }

    fprintf(f, "%d\n", key->bits);
    const size_t len = size - sizeof(key->bits);
    vector<uint8_t> data_vector(key->data, key->data + len);
    return print_hex_data(f, data_vector);
}

bool print_private_key_hex(FILE* f, const R_RSA_PRIVATE_KEY& key) {
    return print_key_hex(f, reinterpret_cast<const KEY*>(&key), sizeof(key));
}

bool print_public_key_hex(FILE* f, const R_RSA_PUBLIC_KEY& key) {
    return print_key_hex(f, reinterpret_cast<const KEY*>(&key), sizeof(key));
}

// parse a text-encoded key from a memory buffer
//
static std::vector<uint8_t> sscan_key_hex(const char* buf) {
    std::vector<uint8_t> result;
    if (!buf) {
        return result;
    }

    unsigned short int num_bits = 0;
    while(true) {
        if (!buf) {
            return result;
        }
        int c = *buf++;
        if (c == '\n') {
            break;
        }
        if (c == EOF || !isdigit(c)) {
            return result;
        }
        if (num_bits > 0) {
            num_bits = num_bits * 10 + (c - '0');
        } else {
            num_bits = c - '0';
        }
    }
    if (num_bits <= 0) {
        return result;
    }

    vector<uint8_t> data =
        sscan_hex_data(vector<uint8_t>(buf, buf + strlen(buf)));
    if (data.empty()) {
        return result;
    }
    result.resize(sizeof(num_bits));
    *reinterpret_cast<short int*>(result.data()) = num_bits;
    result.reserve(result.size() + data.size());
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

template<typename T>
static inline std::pair<bool, T> scan_key_hex(FILE* f) {
    T key;
    memset(&key, 0, sizeof(key));
    vector<uint8_t> result =
        sscan_key_hex(reinterpret_cast<char*>(scan_raw_data(f).data()));
    if (result.empty() || result.size() != sizeof(key)) {
        return std::make_pair(false, key);
    }
    memcpy(&key, result.data(), sizeof(key));
    return std::make_pair(true, key);
}

std::pair<bool, R_RSA_PUBLIC_KEY> scan_public_key_hex(FILE *f) {
    return scan_key_hex<R_RSA_PUBLIC_KEY>(f);
}

std::pair<bool, R_RSA_PRIVATE_KEY> scan_private_key_hex(FILE *f) {
    return scan_key_hex<R_RSA_PRIVATE_KEY>(f);
}

using unique_PKEY_CTX = std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter<EVP_PKEY_CTX, EVP_PKEY_CTX_free>>;
using unique_BLD = std::unique_ptr<OSSL_PARAM_BLD, OpenSSLDeleter<OSSL_PARAM_BLD, OSSL_PARAM_BLD_free>>;
using unique_BN = std::unique_ptr<BIGNUM, OpenSSLDeleter<BIGNUM, BN_free>>;
using unique_BN_CTX = std::unique_ptr<BN_CTX, OpenSSLDeleter<BN_CTX, BN_CTX_free>>;
using unique_PARAMS = std::unique_ptr<OSSL_PARAM, OpenSSLDeleter<OSSL_PARAM, OSSL_PARAM_free>>;

unique_EVP_PKEY private_to_openssl(const R_RSA_PRIVATE_KEY& priv) {
    unique_BN n(BN_bin2bn(priv.modulus, sizeof(priv.modulus), nullptr));
    unique_BN e(BN_bin2bn(priv.publicExponent, sizeof(priv.publicExponent), nullptr));
    unique_BN d(BN_bin2bn(priv.exponent, sizeof(priv.exponent), nullptr));
    unique_BN p(BN_bin2bn(priv.prime[0], sizeof(priv.prime[0]), nullptr));
    unique_BN q(BN_bin2bn(priv.prime[1], sizeof(priv.prime[1]), nullptr));
    unique_BN dmp1(BN_bin2bn(priv.primeExponent[0], sizeof(priv.primeExponent[0]), nullptr));
    unique_BN dmq1(BN_bin2bn(priv.primeExponent[1], sizeof(priv.primeExponent[1]), nullptr));
    unique_BN iqmp(BN_bin2bn(priv.coefficient, sizeof(priv.coefficient), nullptr));

    if (!n || !e || !d || !p || !q || !dmp1 || !dmq1 || !iqmp) {
        return nullptr;
    }

    unique_BLD bld(OSSL_PARAM_BLD_new());
    if (!bld) {
        return nullptr;
    }

    if (!OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_N, n.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_E, e.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_D, d.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_FACTOR1, p.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_FACTOR2, q.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_EXPONENT1, dmp1.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_EXPONENT2, dmq1.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_COEFFICIENT1, iqmp.get())) {
        return nullptr;
    }

    unique_PARAMS params(OSSL_PARAM_BLD_to_param(bld.get()));
    if (!params) {
        return nullptr;
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    if (!ctx) {
        return nullptr;
    }

    if (EVP_PKEY_fromdata_init(ctx.get()) <= 0) {
        return nullptr;
    }

    EVP_PKEY* pkey_raw = nullptr;
    if (EVP_PKEY_fromdata(ctx.get(), &pkey_raw, EVP_PKEY_KEYPAIR, params.get()) <= 0) {
        return nullptr;
    }

    unique_EVP_PKEY pkey(pkey_raw);
    return pkey;
}

unique_EVP_PKEY public_to_openssl(const R_RSA_PUBLIC_KEY& pub) {
    unique_BN n(BN_bin2bn(pub.modulus, sizeof(pub.modulus), nullptr));
    unique_BN e(BN_bin2bn(pub.exponent, sizeof(pub.exponent), nullptr));

    if (!n || !e) {
        return nullptr;
    }

    unique_BLD bld(OSSL_PARAM_BLD_new());
    if (!bld) {
        return nullptr;
    }

    if (!OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_N, n.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_E, e.get())) {
        return nullptr;
    }

    unique_PARAMS params(OSSL_PARAM_BLD_to_param(bld.get()));
    if (!params) {
        return nullptr;
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    if (!ctx) {
        return nullptr;
    }

    if (EVP_PKEY_fromdata_init(ctx.get()) <= 0) {
        return nullptr;
    }

    EVP_PKEY* pkey_raw = nullptr;
    if (EVP_PKEY_fromdata(ctx.get(), &pkey_raw, EVP_PKEY_PUBLIC_KEY, params.get()) <= 0) {
        return nullptr;
    }
    unique_EVP_PKEY pkey(pkey_raw);

    return pkey;
}

// encrypt some data.
// The amount encrypted may be less than what's supplied.
// The output buffer must be at least MIN_OUT_BUFFER_SIZE.
// The output block must be decrypted in its entirety.
//
vector<uint8_t> encrypt_private(const R_RSA_PRIVATE_KEY& key, const vector<uint8_t>& in) {
    const size_t max_len = ((key.bits + 7) / 8) - 11;
    const size_t n = in.size() < max_len ? in.size() : max_len;

    vector<uint8_t> out;

    unique_EVP_PKEY pkey = private_to_openssl(key);
    if (!pkey) {
        return out;
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx) {
        return out;
    }

    if (EVP_PKEY_sign_init(ctx.get()) <= 0) {
        return out;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        return out;
    }

    size_t outlen = 0;
    if (EVP_PKEY_sign(ctx.get(), nullptr, &outlen, in.data(), n) <= 0) {
        return out;
    }

    out.resize(outlen);

    if (EVP_PKEY_sign(ctx.get(), out.data(), &outlen, in.data(), n) <= 0) {
        out.clear();
        return out;
    }

    return out;
}

vector<uint8_t> decrypt_public(const R_RSA_PUBLIC_KEY& key, const vector<uint8_t>& in) {
    vector<uint8_t> out;
    unique_EVP_PKEY pkey = public_to_openssl(key);

    if (!pkey) {
        return out;
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx) {
        return out;
    }

    if (EVP_PKEY_verify_recover_init(ctx.get()) <= 0) {
        return out;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        return out;
    }

    size_t outlen = 0;
    if (EVP_PKEY_verify_recover(ctx.get(), nullptr, &outlen, in.data(), in.size()) <= 0) {
        return out;
    }

    out.resize(outlen);

    if (EVP_PKEY_verify_recover(ctx.get(), out.data(), &outlen, in.data(), in.size()) <= 0) {
        out.clear();
        return out;
    }

    out.resize(outlen);
    return out;
}

//TODO: md5 requires further refactoring
vector<uint8_t> sign_file(const string& path, const R_RSA_PRIVATE_KEY& key) {
    char md5_buf[MD5_LEN];
    double file_length;

    int retval = md5_file(path.data(), md5_buf, file_length);
    if (retval) {
        return vector<uint8_t>();
    }

    vector<uint8_t> md5_vector(md5_buf, md5_buf + strlen(md5_buf));
    return encrypt_private(key, md5_vector);
}

//TODO: md5 requires further refactoring
vector<uint8_t> sign_block(const vector<uint8_t>& data_block, const R_RSA_PRIVATE_KEY& key) {
    char md5_buf[MD5_LEN];

    int retval = md5_block(data_block.data(), data_block.size(), md5_buf);
    if (retval) {
        return vector<uint8_t>();
    }

    vector<uint8_t> md5_vector(md5_buf, md5_buf + strlen(md5_buf));
    return encrypt_private(key, md5_vector);
}

// compute an XML signature element for some text
//
string generate_signature(const string& text_to_sign, const R_RSA_PRIVATE_KEY& key) {
    vector<uint8_t> data_to_sign(text_to_sign.begin(), text_to_sign.end());
    vector<uint8_t> signature = sign_block(data_to_sign, key);
    if (signature.empty()) {
        return string();
    }
    return sprint_hex_data(signature);
}

// check a file signature
//
std::pair<int, bool> check_file_signature(const string& md5_buf, const R_RSA_PUBLIC_KEY& key, const vector<uint8_t>& signature) {
    vector<uint8_t> decrypted = decrypt_public(key, signature);
    if (decrypted.empty()) {
        fprintf(stderr,
            "%s: check_file_signature: decrypt_public error\n",
            time_to_string(dtime()));
        return std::make_pair(ERR_CRYPTO, false);
    }
    // convert string md5_buf to vector<uint8_t>
    vector<uint8_t> md5_vector(md5_buf.begin(), md5_buf.end());
    const bool answer = (decrypted == md5_vector);
    return std::make_pair(0, answer);
}

// same, signature given as string
//
std::pair<int, bool> check_file_signature(const string& md5, const string& signature_text, const string& key_text) {
    std::vector<uint8_t> key_data = sscan_key_hex(key_text.data());
    if (key_data.empty()) {
        return std::make_pair(ERR_BAD_HEX_FORMAT, false);
    }

    R_RSA_PUBLIC_KEY key;
    memcpy(&key, key_data.data(), sizeof(key));

    vector<uint8_t> signature  = sscan_hex_data(vector<uint8_t>(signature_text.begin(), signature_text.end()));
    return check_file_signature(md5, key, signature);
}

//TODO: this requires further refactoring after MD5 is refactored
// same, both text and signature are char strings
//
std::pair<int, bool> check_string_signature(const string& text, const string& signature_text, const R_RSA_PUBLIC_KEY& key) {
    char md5_buf[MD5_LEN];

    int retval = md5_block(reinterpret_cast<const unsigned char*>(text.data()), text.size(), md5_buf);
    if (retval) {
        return std::make_pair(retval, false);
    }
    vector<uint8_t> signature  = sscan_hex_data(vector<uint8_t>(signature_text.begin(), signature_text.end()));
    vector<uint8_t> decrypted = decrypt_public(key, signature);
    if (decrypted.empty()) {
        fprintf(stderr,
            "%s: check_string_signature: decrypt_public error\n",
            time_to_string(dtime()));
        return std::make_pair(ERR_CRYPTO, false);
    }
    vector<uint8_t> md5_vector(md5_buf, md5_buf + strlen(md5_buf));
    const bool answer = (decrypted == md5_vector);
    return std::make_pair(0, answer);
}

// Same, where public key is also encoded as text
//
std::pair<int, bool> check_string_signature(const string& text, const string& signature_text, const string& key_text) {
    R_RSA_PUBLIC_KEY key;

    std::vector<uint8_t> key_data = sscan_key_hex(key_text.data());
    if (key_data.empty()) {
        return std::make_pair(ERR_BAD_HEX_FORMAT, false);
    }
    memcpy(&key, key_data.data(), sizeof(key));
    return check_string_signature(text, signature_text, key);
}

std::pair<int, R_RSA_PRIVATE_KEY> read_key_file(const string& keyfile) {
    R_RSA_PRIVATE_KEY key;
    memset(&key, 0, sizeof(key));
    FILE* fkey = boinc::fopen(keyfile.data(), "r");
    if (!fkey) {
        fprintf(stderr,
            "%s: can't open key file (%s)\n",
            time_to_string(dtime()), keyfile.data()
        );
        return std::make_pair(ERR_FOPEN, key);
    }
    bool result = false;
    std::tie(result, key) = scan_private_key_hex(fkey);
    boinc::fclose(fkey);
    if (!result) {
        fprintf(stderr, "%s: can't parse key\n", time_to_string(dtime()));
        return std::make_pair(ERR_FREAD, key);
    }
    return std::make_pair(0, key);
}

static bool bn2bin(const BIGNUM *from, unsigned char *to, size_t max) {
    int i = BN_num_bytes(from);
    if (i > static_cast<int>(max)) {
        return false;
    }
    memset(to,0,max);
    if (!BN_bn2bin(from,&(to[max-i])))
        return false;
    return true;
}

std::pair<int, R_RSA_PUBLIC_KEY> openssl_to_public(const unique_EVP_PKEY& pkey) {
    R_RSA_PUBLIC_KEY pub;
    memset(&pub, 0, sizeof(pub));

    BIGNUM *n = nullptr, *e = nullptr;
    if (!EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_N, &n) ||
        !EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_E, &e)) {
        return std::make_pair(ERR_CRYPTO, pub);
    }

    if (!bn2bin(n, pub.modulus, sizeof(pub.modulus)) ||
        !bn2bin(e, pub.exponent, sizeof(pub.exponent))) {
        return std::make_pair(ERR_CRYPTO, pub);
    }
    pub.bits = EVP_PKEY_bits(pkey.get());

    return std::make_pair(0, pub);
}

std::pair<int, R_RSA_PRIVATE_KEY> openssl_to_private(const unique_EVP_PKEY& pkey) {
    R_RSA_PRIVATE_KEY priv;
    memset(&priv, 0, sizeof(priv));

    BIGNUM *n = nullptr, *e = nullptr, *d = nullptr;
    BIGNUM *p = nullptr, *q = nullptr;
    BIGNUM *dmp1 = nullptr, *dmq1 = nullptr, *iqmp = nullptr;

    if (!EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_N, &n) ||
        !EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_E, &e)) {
        return std::make_pair(ERR_CRYPTO, priv);
    }
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_D, &d);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_FACTOR1, &p);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_FACTOR2, &q);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_EXPONENT1, &dmp1);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_EXPONENT2, &dmq1);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_COEFFICIENT1, &iqmp);

    if (!n || !e) {
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (!bn2bin(n, priv.modulus, sizeof(priv.modulus)) ||
        !bn2bin(e, priv.publicExponent, sizeof(priv.publicExponent))) {
            memset(&priv, 0, sizeof(priv));
            return std::make_pair(ERR_CRYPTO, priv);
    }
    if (d && !bn2bin(d, priv.exponent, sizeof(priv.exponent))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (p && !bn2bin(p, priv.prime[0], sizeof(priv.prime[0]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (q && !bn2bin(q, priv.prime[1], sizeof(priv.prime[1]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (dmp1 && !bn2bin(dmp1, priv.primeExponent[0], sizeof(priv.primeExponent[0]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (dmq1 && !bn2bin(dmq1, priv.primeExponent[1], sizeof(priv.primeExponent[1]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (iqmp && !bn2bin(iqmp, priv.coefficient, sizeof(priv.coefficient))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    priv.bits = EVP_PKEY_bits(pkey.get());
    return std::make_pair(0, priv);
}

std::tuple<int, R_RSA_PRIVATE_KEY, R_RSA_PUBLIC_KEY> openssl_to_keys(const unique_EVP_PKEY& pkey) {
    int ret = 0;
    R_RSA_PRIVATE_KEY priv;
    R_RSA_PUBLIC_KEY pub;
    memset(&priv, 0, sizeof(priv));
    memset(&pub, 0, sizeof(pub));
    std::tie(ret, priv) = openssl_to_private(pkey);
    if (ret) {
        return std::make_tuple(ret, priv, pub);
    }
    std::tie(ret, pub) = openssl_to_public(pkey);
    if (ret) {
        return std::make_tuple(ret, priv, pub);
    }
    return std::make_tuple(0, priv, pub);
}

using unique_BIO = std::unique_ptr<BIO, OpenSSLDeleter<BIO, BIO_vfree>>;
using unique_X509 = std::unique_ptr<X509, OpenSSLDeleter<X509, X509_free>>;

static bool check_validity_of_cert(const string &cFile, const vector<uint8_t> &md5_md,
    const vector<uint8_t> &sfileMsg, const string &caPath) {
    unique_BIO bio(BIO_new(BIO_s_file()));
    BIO_read_filename(bio.get(), cFile.data());
    unique_X509 cert(PEM_read_bio_X509(bio.get(), nullptr, 0, nullptr));
    if (!cert) {
        return false;
    }

    // verify certificate
    X509_STORE *store = X509_STORE_new();
    X509_LOOKUP *lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
    X509_LOOKUP_add_dir(lookup, caPath.data(), X509_FILETYPE_PEM);
    int retval = 0;
    X509_STORE_CTX *ctx = X509_STORE_CTX_new();
    if (ctx != nullptr) {
        if (X509_STORE_CTX_init(ctx, store, cert.get(), 0) == 1)
            retval = X509_verify_cert(ctx);
        X509_STORE_CTX_free(ctx);
    }
    X509_STORE_free(store);

    if (retval != 1) {
        fprintf(stderr,
            "%s: ERROR: Cannot verify certificate ('%s')\n",
            time_to_string(dtime()), cFile.data()
        );
        return false;
    }
    unique_EVP_PKEY pubKey(X509_get_pubkey(cert.get()));
    if (!pubKey) {
        return false;
    }

    if (EVP_PKEY_id(pubKey.get()) != EVP_PKEY_RSA) {
        fprintf(stderr,
            "%s: ERROR: only RSA keys are supported.\n",
            time_to_string(dtime())
        );
        return false;
    }

    unique_PKEY_CTX pkey_ctx(EVP_PKEY_CTX_new(pubKey.get(), nullptr));
    if (!pkey_ctx) {
        return false;
    }
    if (EVP_PKEY_verify_recover_init(pkey_ctx.get()) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(pkey_ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        return false;
    }

    size_t recovered_len = 0;
    if (EVP_PKEY_verify_recover(
            pkey_ctx.get(), nullptr, &recovered_len, sfileMsg.data(), sfileMsg.size()
        ) <= 0) {
        return false;
    }
    vector<uint8_t> recovered(recovered_len);
    if (EVP_PKEY_verify_recover(
            pkey_ctx.get(), recovered.data(), &recovered_len, sfileMsg.data(), sfileMsg.size()
        ) <= 0) {
        return false;
    }
    recovered.resize(recovered_len);
    char md5_hex[MD5_DIGEST_LENGTH * 2 + 1];
    for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        snprintf(md5_hex + (i * 2), 3, "%02x", md5_md[i]);
    }
    md5_hex[MD5_DIGEST_LENGTH * 2] = '\0';
    return (
        recovered_len == (MD5_DIGEST_LENGTH * 2) &&
        !memcmp(recovered.data(), md5_hex, recovered_len)
    );
}

std::pair<bool, string> check_validity(const string &certPath, const string &origFile,
    const vector<uint8_t> &signature, const string &caPath) {
    vector<uint8_t> md5_md(MD5_DIGEST_LENGTH);
    unsigned char rbuf[2048];

    if (!is_file(origFile.data())) {
        return std::make_pair(false, std::string());
    }
    FILE* of = boinc_fopen(origFile.data(), "r");
    if (!of) {
        return std::make_pair(false, std::string());
    }

    MD5_CTX md5CTX;
    MD5_Init(&md5CTX);
    size_t rbytes;
    while (0 != (rbytes = fread(rbuf, 1, sizeof(rbuf), of))) {
        MD5_Update(&md5CTX, rbuf, rbytes);
    }
    MD5_Final(md5_md.data(), &md5CTX);
    fclose(of);

    DIRREF dir = dir_open(certPath.data());

    char file[MAXPATHLEN];
    while (!dir_scan(file, dir, sizeof(file))) {
        char fpath[MAXPATHLEN];
        snprintf(fpath, sizeof(fpath), "%.*s/%.*s", DIR_LEN, certPath.data(), FILE_LEN, file);
        if (check_validity_of_cert(
                fpath, md5_md, signature, caPath
            )) {
            dir_close(dir);
            return std::make_pair(true, strdup(fpath));
        }
    }

    dir_close(dir);
    return std::make_pair(false, std::string());
}

int cert_verify_file(
    CERT_SIGS* signatures, const char* origFile, const char* trustLocation
) {
    MD5_CTX md5CTX;
    int rbytes;
    unsigned char md5_md[MD5_DIGEST_LENGTH],  rbuf[2048];
    char buf[256];
    char fbuf[MAXPATHLEN];
    int verified = false;
    int file_counter = 0;
    DATA_BLOCK sig_db;
    BIO *bio;
    X509 *cert;
    X509_NAME *subj;

    if (signatures->signatures.size() == 0) {
        printf("No signatures available for file ('%s').\n", origFile);
        fflush(stdout);
        return false;
    }
// OpenSSL 1.1 does initialization internally. This is default.
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(HAVE_LIBRESSL)
    SSL_library_init();
#endif
    if (!is_file(origFile)) return false;
    FILE* of = boinc_fopen(origFile, "r");
    if (!of) return false;
    MD5_Init(&md5CTX);
    while (0 != (rbytes = (int)fread(rbuf, 1, sizeof(rbuf), of))) {
        MD5_Update(&md5CTX, rbuf, rbytes);
    }
    MD5_Final(md5_md, &md5CTX);
    fclose(of);
    for(unsigned int i=0;i < signatures->signatures.size(); i++) {
        sig_db.data = (unsigned char*)calloc(128, sizeof(char));
        if (sig_db.data == NULL) {
            printf("Cannot allocate 128 bytes for signature buffer\n");
            return false;
        }
        sig_db.len=128;
        vector<uint8_t> sig_vector = sscan_hex_data(vector<uint8_t>(signatures->signatures.at(i).signature, signatures->signatures.at(i).signature + strlen(signatures->signatures.at(i).signature)));
        if (sig_vector.size() != 128) {
            printf("Signature size mismatch: expected 128, got %zu\n", sig_vector.size());
            free(sig_db.data);
            return false;
        }
        memcpy(sig_db.data, sig_vector.data(), 128);
        file_counter = 0;
        while (1) {
            snprintf(fbuf, MAXPATHLEN, "%s/%s.%d", trustLocation, signatures->signatures.at(i).hash,
                file_counter);
#ifndef _USING_FCGI_
            FILE *f = fopen(fbuf, "r");
#else
            FCGI_FILE *f = FCGI::fopen(fbuf, "r");
#endif
            if (f==NULL)
                break;
            fclose(f);
            bio = BIO_new(BIO_s_file());
            BIO_read_filename(bio, fbuf);
            if (NULL == (cert = PEM_read_bio_X509(bio, NULL, 0, NULL))) {
                BIO_vfree(bio);
                printf("Cannot read certificate ('%s')\n", fbuf);
                file_counter++;
                continue;
            }
            fflush(stdout);
            subj = X509_get_subject_name(cert);
            X509_NAME_oneline(subj, buf, 256);
            // ???
            //X509_NAME_free(subj);
            X509_free(cert);
            BIO_vfree(bio);
            if (strcmp(buf, signatures->signatures.at(i).subject)) {
                printf("Subject does not match ('%s' <-> '%s')\n", buf, signatures->signatures.at(i).subject);
                file_counter++;
                continue;
            }
            //TODO: temp
            vector<uint8_t> md5_vector(md5_md, md5_md + MD5_DIGEST_LENGTH);
            vector<uint8_t> sig_vector(sig_db.data, sig_db.data + 128);
            verified = check_validity_of_cert(fbuf, md5_vector, sig_vector, trustLocation);
            if (verified)
                break;
            file_counter++;
        }
        free(sig_db.data);
        if (!verified)
            return false;
    }
    return verified;
}
