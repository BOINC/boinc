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

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include <openssl/pem.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>

#include "boinc_stdio.h"
#include "md5_file.h"
#include "filesys.h"
#include "util.h"

#include "crypt.h"

using std::string;
using std::vector;

#define MD5_DIGEST_LENGTH 16

// write some data in hex notation.
// NOTE: since length may not be known to the reader,
// we follow the data with a non-hex character '.'
//
bool print_hex_data(FILE *f, const vector<uint8_t> &x) {
    if (!f) {
        return false;
    }

    bool result = false;
    string hex_data;
    std::tie(result, hex_data) = sprint_hex_data(x);
    if (!result || hex_data.empty()) {
        return false;
    }
    return fputs(hex_data.c_str(), f) >= 0;
}

// same, but write to buffer
//
std::pair<bool, string> sprint_hex_data(const vector<uint8_t> &x) {
    if (x.empty()) {
        return std::make_pair(false, string());
    }
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

    return std::make_pair(true, result);
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

std::pair<bool, vector<uint8_t>> scan_raw_data(FILE *f) {
    vector<uint8_t> data;
    if (!f) {
        return std::make_pair(false, vector<uint8_t>());
    }
    int j;
    while(EOF != (j = fgetc(f))) {
        data.emplace_back(static_cast<uint8_t>(j));
    }
    return std::make_pair(true, data);
}

// scan data in hex notation.
// stop when you reach a non-parsed character.
//
static std::pair<bool, vector<uint8_t>> sscan_hex_data(
    const std::vector<uint8_t>& buffer) {
    if (buffer.empty()) {
        return std::make_pair(false, vector<uint8_t>());
    }
    const char hex[] = "0123456789abcdef";
    vector<uint8_t> data;
    for(size_t i = 0; i < buffer.size(); ) {
        const int c1 = buffer[i++];
        if (c1 == '\n') {
            continue; // skip newlines
        }
        if (!isxdigit(c1)) {
            break;
        }
        if (i >= buffer.size()) {
            break; // no second character
        }
        const int c2 = buffer[i++];
        if (!isxdigit(c2)) {
            break;
        }
        int value =
            (strchr(hex, tolower(c1)) - hex) * 16 +
            (strchr(hex, tolower(c2)) - hex);
        data.emplace_back(static_cast<uint8_t>(value));
    }
    return std::make_pair(true, data);
}

// same, but read from file
//
std::pair<bool, vector<uint8_t>> scan_hex_data(FILE* f) {
    bool result = false;
    vector<uint8_t> data;
    std::tie(result, data) = scan_raw_data(f);
    if (!result || data.empty()) {
        return std::make_pair(false, vector<uint8_t>());
    }
    return sscan_hex_data(data);
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
static std::pair<bool, std::vector<uint8_t>> sscan_key_hex(const char* buf) {
    if (!buf) {
        return std::make_pair(false, std::vector<uint8_t>());
    }

    std::vector<uint8_t> result;
    unsigned short int num_bits = 0;
    while(true) {
        if (!buf) {
            return std::make_pair(false, std::vector<uint8_t>());
        }
        const int c = *buf++;
        if (c == '\n') {
            break;
        }
        if (c == EOF || !isdigit(c)) {
            return std::make_pair(false, std::vector<uint8_t>());
        }
        if (num_bits > 0) {
            num_bits = num_bits * 10 + (c - '0');
        } else {
            num_bits = c - '0';
        }
    }
    if (num_bits <= 0) {
        return std::make_pair(false, std::vector<uint8_t>());
    }

    bool error_flag = false;
    vector<uint8_t> data;
    std::tie(error_flag, data) =
        sscan_hex_data(vector<uint8_t>(buf, buf + strlen(buf)));
    if (!error_flag || data.empty()) {
        return std::make_pair(false, std::vector<uint8_t>());
    }
    result.resize(sizeof(num_bits));
    *reinterpret_cast<short int*>(result.data()) = num_bits;
    result.reserve(result.size() + data.size());
    result.insert(result.end(), data.begin(), data.end());
    return std::make_pair(true, result);
}

template<typename T>
static inline std::pair<bool, T> scan_key_hex(FILE* f) {
    T key;
    memset(&key, 0, sizeof(key));
    bool result = false;
    vector<uint8_t> data;
    std::tie(result, data) = scan_raw_data(f);
    if (!result || data.empty()) {
        return std::make_pair(false, key);
    }
    data.push_back('\0'); // null-terminate for sscan_key_hex
    vector<uint8_t> result_data;
    std::tie(result, result_data) =
        sscan_key_hex(reinterpret_cast<const char*>(data.data()));
    if (!result || result_data.empty() || result_data.size() != sizeof(key)) {
        return std::make_pair(false, key);
    }
    memcpy(&key, result_data.data(), sizeof(key));
    return std::make_pair(true, key);
}

std::pair<bool, R_RSA_PUBLIC_KEY> scan_public_key_hex(FILE *f) {
    return scan_key_hex<R_RSA_PUBLIC_KEY>(f);
}

std::pair<bool, R_RSA_PRIVATE_KEY> scan_private_key_hex(FILE *f) {
    return scan_key_hex<R_RSA_PRIVATE_KEY>(f);
}

using unique_PKEY_CTX = std::unique_ptr<EVP_PKEY_CTX,
    OpenSSLDeleter<EVP_PKEY_CTX, EVP_PKEY_CTX_free>>;
using unique_BLD = std::unique_ptr<OSSL_PARAM_BLD,
    OpenSSLDeleter<OSSL_PARAM_BLD, OSSL_PARAM_BLD_free>>;
using unique_BN = std::unique_ptr<BIGNUM,
    OpenSSLDeleter<BIGNUM, BN_free>>;
using unique_BN_CTX = std::unique_ptr<BN_CTX,
    OpenSSLDeleter<BN_CTX, BN_CTX_free>>;
using unique_PARAMS = std::unique_ptr<OSSL_PARAM,
    OpenSSLDeleter<OSSL_PARAM, OSSL_PARAM_free>>;

std::pair<bool, unique_EVP_PKEY> private_to_openssl(
    const R_RSA_PRIVATE_KEY& priv) {
    unique_BN n(
        BN_bin2bn(priv.modulus, sizeof(priv.modulus), nullptr)
    );
    unique_BN e(
        BN_bin2bn(priv.publicExponent, sizeof(priv.publicExponent), nullptr)
    );
    unique_BN d(
        BN_bin2bn(priv.exponent, sizeof(priv.exponent), nullptr)
    );
    unique_BN p(
        BN_bin2bn(priv.prime[0], sizeof(priv.prime[0]), nullptr)
    );
    unique_BN q(
        BN_bin2bn(priv.prime[1], sizeof(priv.prime[1]), nullptr)
    );
    unique_BN dmp1(
        BN_bin2bn(priv.primeExponent[0], sizeof(priv.primeExponent[0]), nullptr)
    );
    unique_BN dmq1(
        BN_bin2bn(priv.primeExponent[1], sizeof(priv.primeExponent[1]), nullptr)
    );
    unique_BN iqmp(
        BN_bin2bn(priv.coefficient, sizeof(priv.coefficient), nullptr)
    );

    if (!n || !e || !d || !p || !q || !dmp1 || !dmq1 || !iqmp) {
        return std::make_pair(false, nullptr);
    }

    unique_BLD bld(OSSL_PARAM_BLD_new());
    if (!bld) {
        return std::make_pair(false, nullptr);
    }

    if (!OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_N, n.get()
        ) ||
        !OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_E, e.get()
        ) ||
        !OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_D, d.get()
        ) ||
        !OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_FACTOR1, p.get()
        ) ||
        !OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_FACTOR2, q.get()
        ) ||
        !OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_EXPONENT1, dmp1.get()
        ) ||
        !OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_EXPONENT2, dmq1.get()
        ) ||
        !OSSL_PARAM_BLD_push_BN(
            bld.get(), OSSL_PKEY_PARAM_RSA_COEFFICIENT1, iqmp.get())
        ) {
        return std::make_pair(false, nullptr);
    }

    unique_PARAMS params(OSSL_PARAM_BLD_to_param(bld.get()));
    if (!params) {
        return std::make_pair(false, nullptr);
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    if (!ctx) {
        return std::make_pair(false, nullptr);
    }

    if (EVP_PKEY_fromdata_init(ctx.get()) <= 0) {
        return std::make_pair(false, nullptr);
    }

    EVP_PKEY* pkey_raw = nullptr;
    if (EVP_PKEY_fromdata(
            ctx.get(), &pkey_raw, EVP_PKEY_KEYPAIR, params.get()) <= 0) {
        return std::make_pair(false, nullptr);
    }

    unique_EVP_PKEY pkey(pkey_raw);
    return std::make_pair(true, std::move(pkey));
}

std::pair<bool, unique_EVP_PKEY> public_to_openssl(
    const R_RSA_PUBLIC_KEY& pub) {
    unique_BN n(BN_bin2bn(pub.modulus, sizeof(pub.modulus), nullptr));
    unique_BN e(BN_bin2bn(pub.exponent, sizeof(pub.exponent), nullptr));

    if (!n || !e) {
        return std::make_pair(false, nullptr);
    }

    unique_BLD bld(OSSL_PARAM_BLD_new());
    if (!bld) {
        return std::make_pair(false, nullptr);
    }

    if (!OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_N, n.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_E, e.get())) {
        return std::make_pair(false, nullptr);
    }

    unique_PARAMS params(OSSL_PARAM_BLD_to_param(bld.get()));
    if (!params) {
        return std::make_pair(false, nullptr);
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    if (!ctx) {
        return std::make_pair(false, nullptr);
    }

    if (EVP_PKEY_fromdata_init(ctx.get()) <= 0) {
        return std::make_pair(false, nullptr);
    }

    EVP_PKEY* pkey_raw = nullptr;
    if (EVP_PKEY_fromdata(
            ctx.get(), &pkey_raw, EVP_PKEY_PUBLIC_KEY, params.get()) <= 0) {
        return std::make_pair(false, nullptr);
    }
    unique_EVP_PKEY pkey(pkey_raw);

    return std::make_pair(true, std::move(pkey));
}

// encrypt some data.
// The amount encrypted may be less than what's supplied.
// The output block must be decrypted in its entirety.
//
std::pair<bool, vector<uint8_t>> encrypt_private(
    const R_RSA_PRIVATE_KEY& key, const vector<uint8_t>& in
    ) {
    const size_t max_len = ((key.bits + 7) / 8) - 11;
    const size_t n = in.size() < max_len ? in.size() : max_len;

    bool result = false;
    unique_EVP_PKEY pkey;
    std::tie(result, pkey) = private_to_openssl(key);
    if (!result || !pkey) {
        return std::make_pair(false, vector<uint8_t>());
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx) {
        return std::make_pair(false, vector<uint8_t>());
    }

    if (EVP_PKEY_sign_init(ctx.get()) <= 0) {
        return std::make_pair(false, vector<uint8_t>());
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        return std::make_pair(false, vector<uint8_t>());
    }

    size_t outlen = 0;
    if (EVP_PKEY_sign(ctx.get(), nullptr, &outlen, in.data(), n) <= 0) {
        return std::make_pair(false, vector<uint8_t>());
    }

    vector<uint8_t> out;
    out.resize(outlen);

    if (EVP_PKEY_sign(ctx.get(), out.data(), &outlen, in.data(), n) <= 0) {
        out.clear();
        return std::make_pair(false, vector<uint8_t>());
    }

    return std::make_pair(true, out);
}

std::pair<bool, vector<uint8_t>> decrypt_public(
    const R_RSA_PUBLIC_KEY& key, const vector<uint8_t>& in
    ) {
    bool result = false;
    unique_EVP_PKEY pkey;
    std::tie(result, pkey) = public_to_openssl(key);

    if (!result || !pkey) {
        return std::make_pair(false, vector<uint8_t>());
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx) {
        return std::make_pair(false, vector<uint8_t>());
    }

    if (EVP_PKEY_verify_recover_init(ctx.get()) <= 0) {
        return std::make_pair(false, vector<uint8_t>());
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        return std::make_pair(false, vector<uint8_t>());
    }

    size_t outlen = 0;
    if (EVP_PKEY_verify_recover(
        ctx.get(), nullptr, &outlen, in.data(), in.size()) <= 0) {
        return std::make_pair(false, vector<uint8_t>());
    }

    vector<uint8_t> out;
    out.resize(outlen);

    if (EVP_PKEY_verify_recover(
        ctx.get(), out.data(), &outlen, in.data(), in.size()) <= 0) {
        out.clear();
        return std::make_pair(false, vector<uint8_t>());
    }

    out.resize(outlen);
    return std::make_pair(true, out);
}

//TODO: md5 requires further refactoring
std::pair<bool, vector<uint8_t>> sign_file(
    const string& path, const R_RSA_PRIVATE_KEY& key) {
    char md5_buf[MD5_LEN];
    double file_length;

    const int retval = md5_file(path.data(), md5_buf, file_length);
    if (retval) {
        return std::make_pair(false, vector<uint8_t>());
    }

    vector<uint8_t> md5_vector(md5_buf, md5_buf + strlen(md5_buf));
    return encrypt_private(key, md5_vector);
}

//TODO: md5 requires further refactoring
std::pair<bool, vector<uint8_t>> sign_block(
    const vector<uint8_t>& data_block, const R_RSA_PRIVATE_KEY& key
    ) {
    char md5_buf[MD5_LEN];

    const int retval = md5_block(data_block.data(), data_block.size(), md5_buf);
    if (retval) {
        return std::make_pair(false, vector<uint8_t>());
    }

    vector<uint8_t> md5_vector(md5_buf, md5_buf + strlen(md5_buf));
    return encrypt_private(key, md5_vector);
}

// compute an XML signature element for some text
//
std::pair<bool, string> generate_signature(
    const string& text_to_sign, const R_RSA_PRIVATE_KEY& key
    ) {
    vector<uint8_t> data_to_sign(text_to_sign.begin(), text_to_sign.end());
    bool result = false;
    vector<uint8_t> signature;
    std::tie(result, signature) = sign_block(data_to_sign, key);
    if (!result || signature.empty()) {
        return std::make_pair(false, string());
    }
    return sprint_hex_data(signature);
}

// check a file signature
//
std::pair<int, bool> check_file_signature(
    const string& md5_buf,
    const R_RSA_PUBLIC_KEY& key,
    const vector<uint8_t>& signature
    ) {
    bool result = false;
    vector<uint8_t> decrypted;
    std::tie(result, decrypted) = decrypt_public(key, signature);
    if (!result || decrypted.empty()) {
        fprintf(stderr,
            "%s: check_file_signature: decrypt_public error\n",
            time_to_string(dtime()));
        return std::make_pair(ERR_CRYPTO, false);
    }
    vector<uint8_t> md5_vector(md5_buf.begin(), md5_buf.end());
    const bool answer = (decrypted == md5_vector);
    return std::make_pair(0, answer);
}

// same, signature given as string
//
std::pair<int, bool> check_file_signature(
    const string& md5, const string& signature_text, const string& key_text
    ) {
    bool result = false;
    std::vector<uint8_t> key_data;
    std::tie(result, key_data) = sscan_key_hex(key_text.data());
    if (!result || key_data.empty()) {
        return std::make_pair(ERR_BAD_HEX_FORMAT, false);
    }

    R_RSA_PUBLIC_KEY key;
    memcpy(&key, key_data.data(), sizeof(key));

    vector<uint8_t> signature;
    std::tie(result, signature) = sscan_hex_data(
        vector<uint8_t>(signature_text.begin(), signature_text.end()));
    if (!result || signature.empty()) {
        return std::make_pair(ERR_BAD_HEX_FORMAT, false);
    }
    return check_file_signature(md5, key, signature);
}

//TODO: this requires further refactoring after MD5 is refactored
// same, both text and signature are char strings
//
std::pair<int, bool> check_string_signature(
    const string& text,
    const string& signature_text,
    const R_RSA_PUBLIC_KEY& key
    ) {
    char md5_buf[MD5_LEN];

    int retval = md5_block(reinterpret_cast<const unsigned char*>(
        text.data()), text.size(), md5_buf);
    if (retval) {
        return std::make_pair(retval, false);
    }
    bool result = false;
    vector<uint8_t> signature;
    std::tie(result, signature) = sscan_hex_data(vector<uint8_t>(
        signature_text.begin(), signature_text.end()));
    if (!result || signature.empty()) {
        return std::make_pair(ERR_BAD_HEX_FORMAT, false);
    }
    vector<uint8_t> decrypted;
    std::tie(result, decrypted) = decrypt_public(key, signature);
    if (!result || decrypted.empty()) {
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
std::pair<int, bool> check_string_signature(
    const string& text,
    const string& signature_text,
    const string& key_text
    ) {
    R_RSA_PUBLIC_KEY key;

    bool result = false;
    std::vector<uint8_t> key_data;
    std::tie(result, key_data) = sscan_key_hex(key_text.data());
    if (!result || key_data.empty()) {
        return std::make_pair(ERR_BAD_HEX_FORMAT, false);
    }
    memcpy(&key, key_data.data(), sizeof(key));
    return check_string_signature(text, signature_text, key);
}

struct file_closer { void operator()(FILE* f) const { if (f) fclose(f); } };
using unique_FILE = std::unique_ptr<FILE, file_closer>;

std::pair<int, R_RSA_PRIVATE_KEY> read_key_file(const string& keyfile) {
    R_RSA_PRIVATE_KEY key;
    memset(&key, 0, sizeof(key));
    unique_FILE fkey(boinc::fopen(keyfile.data(), "r"));
    if (!fkey) {
        fprintf(stderr,
            "%s: can't open key file (%s)\n",
            time_to_string(dtime()), keyfile.data()
        );
        return std::make_pair(ERR_FOPEN, key);
    }
    bool result = false;
    std::tie(result, key) = scan_private_key_hex(fkey.get());
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

std::pair<int, R_RSA_PUBLIC_KEY> openssl_to_public(
    const unique_EVP_PKEY& pkey
    ) {
    R_RSA_PUBLIC_KEY pub;
    memset(&pub, 0, sizeof(pub));

    BIGNUM *n = nullptr, *e = nullptr;
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_N, &n);
    unique_BN n_unique(n);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_E, &e);
    unique_BN e_unique(e);
    if (!n || !e) {
        return std::make_pair(ERR_CRYPTO, pub);
    }

    if (!bn2bin(n, pub.modulus, sizeof(pub.modulus)) ||
        !bn2bin(e, pub.exponent, sizeof(pub.exponent))) {
        return std::make_pair(ERR_CRYPTO, pub);
    }
    pub.bits = EVP_PKEY_bits(pkey.get());

    return std::make_pair(0, pub);
}

std::pair<int, R_RSA_PRIVATE_KEY> openssl_to_private(
    const unique_EVP_PKEY& pkey
    ) {
    R_RSA_PRIVATE_KEY priv;
    memset(&priv, 0, sizeof(priv));

    BIGNUM *n = nullptr, *e = nullptr, *d = nullptr;
    BIGNUM *p = nullptr, *q = nullptr;
    BIGNUM *dmp1 = nullptr, *dmq1 = nullptr, *iqmp = nullptr;

    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_N, &n);
    unique_BN n_unique(n);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_E, &e);
    unique_BN e_unique(e);
    if (!n || !e) {
        return std::make_pair(ERR_CRYPTO, priv);
    }
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_D, &d);
    unique_BN d_unique(d);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_FACTOR1, &p);
    unique_BN p_unique(p);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_FACTOR2, &q);
    unique_BN q_unique(q);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_EXPONENT1, &dmp1);
    unique_BN dmp1_unique(dmp1);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_EXPONENT2, &dmq1);
    unique_BN dmq1_unique(dmq1);
    EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_COEFFICIENT1, &iqmp);
    unique_BN iqmp_unique(iqmp);

    if (!n || !e) {
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (!bn2bin(n, priv.modulus, sizeof(priv.modulus)) ||
        !bn2bin(e, priv.publicExponent, sizeof(priv.publicExponent))) {
            memset(&priv, 0, sizeof(priv));
            return std::make_pair(ERR_CRYPTO, priv);
    }
    if (d &&
        !bn2bin(d, priv.exponent, sizeof(priv.exponent))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (p &&
        !bn2bin(p, priv.prime[0], sizeof(priv.prime[0]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (q &&
        !bn2bin(q, priv.prime[1], sizeof(priv.prime[1]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (dmp1 &&
        !bn2bin(dmp1, priv.primeExponent[0], sizeof(priv.primeExponent[0]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (dmq1 &&
        !bn2bin(dmq1, priv.primeExponent[1], sizeof(priv.primeExponent[1]))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    if (iqmp &&
        !bn2bin(iqmp, priv.coefficient, sizeof(priv.coefficient))) {
        memset(&priv, 0, sizeof(priv));
        return std::make_pair(ERR_CRYPTO, priv);
    }
    priv.bits = EVP_PKEY_bits(pkey.get());
    return std::make_pair(0, priv);
}

std::tuple<int, R_RSA_PRIVATE_KEY, R_RSA_PUBLIC_KEY> openssl_to_keys(
    const unique_EVP_PKEY& pkey
    ) {
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

static bool check_validity_of_cert(
    const string &cFile,
    const string &md5_md,
    const vector<uint8_t> &sfileMsg,
    const string &caPath
    ) {
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
            pkey_ctx.get(), nullptr, &recovered_len, sfileMsg.data(),
            sfileMsg.size()
        ) <= 0) {
        return false;
    }
    vector<uint8_t> recovered(recovered_len);
    if (EVP_PKEY_verify_recover(
            pkey_ctx.get(), recovered.data(), &recovered_len, sfileMsg.data(),
            sfileMsg.size()
        ) <= 0) {
        return false;
    }
    recovered.resize(recovered_len);
    return (
        recovered_len == (MD5_DIGEST_LENGTH * 2) &&
        !memcmp(recovered.data(), md5_md.data(), recovered_len)
    );
}

std::pair<bool, string> check_validity(
    const string &certPath,
    const string &origFile,
    const vector<uint8_t> &signature,
    const string &caPath
    ) {
    if (!is_file(origFile.data())) {
        return std::make_pair(false, std::string());
    }

    char md5_md[MD5_DIGEST_LENGTH * 2 + 1];
    double bytes = 0;
    if (md5_file(origFile.data(), md5_md, bytes, false) != 0) {
        return std::make_pair(false, std::string());
    }

    DIRREF dir = dir_open(certPath.data());

    char file[MAXPATHLEN];
    while (!dir_scan(file, dir, sizeof(file))) {
        char fpath[MAXPATHLEN];
        snprintf(fpath, sizeof(fpath), "%.*s/%.*s", DIR_LEN, certPath.data(),
            FILE_LEN, file);
        if (check_validity_of_cert(
                fpath, md5_md, signature, caPath
            )) {
            dir_close(dir);
            return std::make_pair(true, string(fpath));
        }
    }

    dir_close(dir);
    return std::make_pair(false, std::string());
}

bool cert_verify_file(CERT_SIGS* signatures, const string &origFile,
    const string &trustLocation) {
    if (signatures == nullptr) {
        return false;
    }

    if (signatures->signatures.size() == 0) {
        printf("No signatures available for file ('%s').\n", origFile.data());
        fflush(stdout);
        return false;
    }

    if (!is_file(origFile.data())) {
        return false;
    }

    char md5_md[MD5_DIGEST_LENGTH * 2 + 1];
    double bytes = 0;
    if (md5_file(origFile.data(), md5_md, bytes, false) != 0) {
        return false;
    }

    for(const CERT_SIG &cert_sig : signatures->signatures) {
        bool result = false;
        vector<uint8_t> sig_vector;
        std::tie(result, sig_vector) = sscan_hex_data(vector<uint8_t>(
            cert_sig.signature,
            cert_sig.signature + strlen(cert_sig.signature))
        );
        if (!result || sig_vector.size() != 128) {
            printf("Signature size mismatch: expected 128, got %zu\n",
            sig_vector.size());
            return false;
        }
        size_t file_counter = 0;
        while (1) {
            char fbuf[MAXPATHLEN];
            snprintf(fbuf, MAXPATHLEN, "%s/%s.%zu", trustLocation.data(),
            cert_sig.hash, file_counter);
            unique_FILE f(boinc::fopen(fbuf, "r"));
            if (!f) {
                break;
            }
            unique_BIO bio(BIO_new(BIO_s_file()));
            BIO_read_filename(bio.get(), fbuf);
            unique_X509 cert(PEM_read_bio_X509(bio.get(), nullptr, 0, nullptr));
            if (!cert) {
                printf("Cannot read certificate ('%s')\n", fbuf);
                ++file_counter;
                continue;
            }
            fflush(stdout);
            X509_NAME *subj = X509_get_subject_name(cert.get());

            char buf[256];
            X509_NAME_oneline(subj, buf, 256);
            if (strcmp(buf, cert_sig.subject)) {
                printf("Subject does not match ('%s' <-> '%s')\n", buf,
                    cert_sig.subject);
                ++file_counter;
                continue;
            }
            if (check_validity_of_cert(
                fbuf, md5_md, sig_vector, trustLocation)) {
                return true;
            }
            ++file_counter;
        }
    }
    return false;
}
