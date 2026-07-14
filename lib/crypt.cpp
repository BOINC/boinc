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
    fputs(hex_data.c_str(), f);

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
bool print_key_hex(FILE* f, KEY* key, size_t size) {
    if (!f || !key || size == 0) {
        return false;
    }

    fprintf(f, "%d\n", key->bits);
    const size_t len = size - sizeof(key->bits);
    vector<uint8_t> data_vector(key->data, key->data + len);
    return print_hex_data(f, data_vector);
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

vector<uint8_t> scan_key_hex(FILE* f) {
    return sscan_key_hex(reinterpret_cast<char*>(scan_raw_data(f).data()));
}

using unique_PKEY_CTX = std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter<EVP_PKEY_CTX, EVP_PKEY_CTX_free>>;
using unique_BN = std::unique_ptr<BIGNUM, OpenSSLDeleter<BIGNUM, BN_free>>;
using unique_BN_CTX = std::unique_ptr<BN_CTX, OpenSSLDeleter<BN_CTX, BN_CTX_free>>;

unique_EVP_PKEY private_to_openssl(R_RSA_PRIVATE_KEY& priv) {
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

    using unique_BLD = std::unique_ptr<OSSL_PARAM_BLD, OpenSSLDeleter<OSSL_PARAM_BLD, OSSL_PARAM_BLD_free>>;

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

    using unique_PARAMS = std::unique_ptr<OSSL_PARAM, OpenSSLDeleter<OSSL_PARAM, OSSL_PARAM_free>>;

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

unique_EVP_PKEY public_to_openssl(R_RSA_PUBLIC_KEY& pub) {
    unique_BN n(BN_bin2bn(pub.modulus, sizeof(pub.modulus), nullptr));
    unique_BN e(BN_bin2bn(pub.exponent, sizeof(pub.exponent), nullptr));

    if (!n || !e) {
        return nullptr;
    }

    using unique_BLD = std::unique_ptr<OSSL_PARAM_BLD, OpenSSLDeleter<OSSL_PARAM_BLD, OSSL_PARAM_BLD_free>>;

    unique_BLD bld(OSSL_PARAM_BLD_new());
    if (!bld) {
        return nullptr;
    }

    if (!OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_N, n.get()) ||
        !OSSL_PARAM_BLD_push_BN(bld.get(), OSSL_PKEY_PARAM_RSA_E, e.get())) {
        return nullptr;
    }

    using unique_PARAMS = std::unique_ptr<OSSL_PARAM, OpenSSLDeleter<OSSL_PARAM, OSSL_PARAM_free>>;

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
int encrypt_private(R_RSA_PRIVATE_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
    const size_t modulus_len = (key.bits+7)/8;
    size_t n = in.len;
    if (n >= modulus_len-11) {
        n = modulus_len-11;
    }

    unique_EVP_PKEY pkey = private_to_openssl(key);
    if (!pkey) {
        return ERR_CRYPTO;
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx) {
        return ERR_CRYPTO;
    }

    if (EVP_PKEY_sign_init(ctx.get()) <= 0) {
        return ERR_CRYPTO;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        return ERR_CRYPTO;
    }

    size_t outlen = out.len;
    if (EVP_PKEY_sign(ctx.get(), out.data, &outlen, in.data, n) <= 0) {
        return ERR_CRYPTO;
    }

    out.len = static_cast<unsigned int>(outlen);
    return 0;
}

int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
    int retval;

    unique_EVP_PKEY pkey = public_to_openssl(key);

    if (!pkey) {
        return ERR_CRYPTO;
    }

    unique_PKEY_CTX ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx) {
        return ERR_CRYPTO;
    }

    if (EVP_PKEY_verify_recover_init(ctx.get()) <= 0) {
        return ERR_CRYPTO;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        return ERR_CRYPTO;
    }

    size_t outlen = out.len;

    if (EVP_PKEY_verify_recover(ctx.get(), out.data, &outlen, in.data, in.len) <= 0) {
        return ERR_CRYPTO;
    }

    out.len = static_cast<unsigned int>(outlen);
    return 0;
}

int sign_file(const char* path, R_RSA_PRIVATE_KEY& key, DATA_BLOCK& signature) {
    char md5_buf[MD5_LEN];
    double file_length;
    DATA_BLOCK in_block;
    int retval;

    retval = md5_file(path, md5_buf, file_length);
    if (retval) return retval;
    in_block.data = (unsigned char*)md5_buf;
    in_block.len = (unsigned int)strlen(md5_buf);
    retval = encrypt_private(key, in_block, signature);
    if (retval) return retval;
    return 0;
}

int sign_block(DATA_BLOCK& data_block, R_RSA_PRIVATE_KEY& key, DATA_BLOCK& signature) {
    char md5_buf[MD5_LEN];
    int retval;
    DATA_BLOCK in_block;

    md5_block(data_block.data, data_block.len, md5_buf);
    in_block.data = (unsigned char*)md5_buf;
    in_block.len = (unsigned int)strlen(md5_buf);
    retval = encrypt_private(key, in_block, signature);
    if (retval) {
        printf("sign_block: encrypt_private returned %d\n", retval);
        return retval;
    }
    return 0;
}

// compute an XML signature element for some text
//
int generate_signature(
    char* text_to_sign, string& signature_hex, R_RSA_PRIVATE_KEY& key
)  {
    DATA_BLOCK block, signature_data;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    int retval;

    block.data = (unsigned char*)text_to_sign;
    block.len = (unsigned int)strlen(text_to_sign);
    signature_data.data = signature_buf;
    signature_data.len = SIGNATURE_SIZE_BINARY;
    retval = sign_block(block, key, signature_data);
    if (retval) return retval;
    vector<uint8_t> signature_vector(signature_data.data, signature_data.data + signature_data.len);
    signature_hex = sprint_hex_data(signature_vector);
    return 0;
}

// check a file signature
//
int check_file_signature(
    const char* md5_buf, R_RSA_PUBLIC_KEY& key,
    DATA_BLOCK& signature, bool& answer
) {
    char clear_buf[MD5_LEN];
    int n, retval;
    DATA_BLOCK clear_signature;
    clear_buf[0]=0;

    n = (int)strlen(md5_buf);
    clear_signature.data = (unsigned char*)clear_buf;
    clear_signature.len = MD5_LEN;
    retval = decrypt_public(key, signature, clear_signature);
    if (retval) {
        fprintf(stderr,
            "%s: check_file_signature: decrypt_public error %d\n",
            time_to_string(dtime()), retval
        );
        return retval;
    }
    answer = !strncmp(md5_buf, clear_buf, n);
    return 0;
}

// same, signature given as string
//
int check_file_signature2(
    const char* md5, const char* signature_text,
    const char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    DATA_BLOCK signature;

    std::vector<uint8_t> key_data = sscan_key_hex(key_text);
    if (key_data.empty()) {
        fprintf(stderr, "%s: check_file_signature2: sscan_key_hex failed\n",
            time_to_string(dtime())
        );
        return ERR_BAD_HEX_FORMAT;
    }
    memcpy(&key, key_data.data(), sizeof(key));
    signature.data = signature_buf;
    signature.len = sizeof(signature_buf);
    vector<uint8_t> data  = sscan_hex_data(vector<uint8_t>(signature_text, signature_text + strlen(signature_text)));
    if (data.size() != sizeof(signature_buf)) {
        fprintf(stderr, "%s: check_file_signature2: signature size mismatch: expected %zu, got %zu\n",
            time_to_string(dtime()), sizeof(signature_buf), data.size()
        );
        return ERR_BAD_HEX_FORMAT;
    }
    memcpy(signature.data, data.data(), sizeof(signature_buf));
    return check_file_signature(md5, key, signature, answer);
}

// same, both text and signature are char strings
//
int check_string_signature(
    const char* text, const char* signature_text, R_RSA_PUBLIC_KEY& key,
    bool& answer
) {
    char md5_buf[MD5_LEN];
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    char clear_buf[MD5_LEN];
    int retval, n;
    DATA_BLOCK signature, clear_signature;

    retval = md5_block((const unsigned char*)text, (int)strlen(text), md5_buf);
    if (retval) return retval;
    n = (int)strlen(md5_buf);
    signature.data = signature_buf;
    signature.len = sizeof(signature_buf);
    vector<uint8_t> data  = sscan_hex_data(vector<uint8_t>(signature_text, signature_text + strlen(signature_text)));
    signature.len = data.size();
    memcpy(signature.data, data.data(), sizeof(signature_buf));
    clear_signature.data = (unsigned char*)clear_buf;
    clear_signature.len = 256;
    retval = decrypt_public(key, signature, clear_signature);
    if (retval) return retval;
    answer = !strncmp(md5_buf, clear_buf, n);
    return 0;
}

// Same, where public key is also encoded as text
//
int check_string_signature2(
    const char* text, const char* signature_text, const char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    int retval;

    std::vector<uint8_t> key_data = sscan_key_hex(key_text);
    if (key_data.empty()) {
        return 1;
    }
    memcpy(&key, key_data.data(), sizeof(key));
    return check_string_signature(text, signature_text, key, answer);
}

int read_key_file(const char* keyfile, R_RSA_PRIVATE_KEY& key) {
    int retval;
#ifndef _USING_FCGI_
    FILE* fkey = fopen(keyfile, "r");
#else
    FCGI_FILE* fkey = FCGI::fopen(keyfile, "r");
#endif
    if (!fkey) {
        fprintf(stderr,
            "%s: can't open key file (%s)\n",
            time_to_string(dtime()), keyfile
        );
        return ERR_FOPEN;
    }
    vector<uint8_t> result = scan_key_hex(fkey);
    fclose(fkey);
    if (result.empty()) {
        fprintf(stderr, "%s: can't parse key\n", time_to_string(dtime()));
        return ERR_FREAD;
    }
    memcpy(&key, result.data(), sizeof(key));
    return 0;
}

static void bn_to_bin(const BIGNUM* bn, unsigned char* bin, int n) {
    memset(bin, 0, n);
    int m = BN_num_bytes(bn);
    BN_bn2bin(bn, bin+n-m);
}

void openssl_to_keys(
    RSA* rp, int nbits, R_RSA_PRIVATE_KEY& priv, R_RSA_PUBLIC_KEY& pub
) {
    pub.bits = nbits;
#ifdef HAVE_OPAQUE_RSA_DSA_DH
    const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    const BIGNUM *p;
    const BIGNUM *q;
    const BIGNUM *dmp1;
    const BIGNUM *dmq1;
    const BIGNUM *iqmp;
    RSA_get0_key(rp, &n, &e, &d);
    RSA_get0_factors(rp, &p, &q);
    RSA_get0_crt_params(rp, &dmp1, &dmq1, &iqmp);

    if (n)
        bn_to_bin(n, pub.modulus, sizeof(pub.modulus));
    if (e)
        bn_to_bin(e, pub.exponent, sizeof(pub.exponent));
#else
    bn_to_bin(rp->n, pub.modulus, sizeof(pub.modulus));
    bn_to_bin(rp->e, pub.exponent, sizeof(pub.exponent));
#endif

    memset(&priv, 0, sizeof(priv));
    priv.bits = (unsigned short)nbits;
#ifdef HAVE_OPAQUE_RSA_DSA_DH
    if (n)
        bn_to_bin(n, priv.modulus, sizeof(priv.modulus));
    if (e)
        bn_to_bin(e, priv.publicExponent, sizeof(priv.publicExponent));
    if (d)
        bn_to_bin(d, priv.exponent, sizeof(priv.exponent));
    if (p)
        bn_to_bin(p, priv.prime[0], sizeof(priv.prime[0]));
    if (q)
        bn_to_bin(q, priv.prime[1], sizeof(priv.prime[1]));
    if (dmp1)
        bn_to_bin(dmp1, priv.primeExponent[0], sizeof(priv.primeExponent[0]));
    if (dmq1)
        bn_to_bin(dmq1, priv.primeExponent[1], sizeof(priv.primeExponent[1]));
    if (iqmp)
        bn_to_bin(iqmp, priv.coefficient, sizeof(priv.coefficient));
#else
    bn_to_bin(rp->n, priv.modulus, sizeof(priv.modulus));
    bn_to_bin(rp->e, priv.publicExponent, sizeof(priv.publicExponent));
    bn_to_bin(rp->d, priv.exponent, sizeof(priv.exponent));
    bn_to_bin(rp->p, priv.prime[0], sizeof(priv.prime[0]));
    bn_to_bin(rp->q, priv.prime[1], sizeof(priv.prime[1]));
    bn_to_bin(rp->dmp1, priv.primeExponent[0], sizeof(priv.primeExponent[0]));
    bn_to_bin(rp->dmq1, priv.primeExponent[1], sizeof(priv.primeExponent[1]));
    bn_to_bin(rp->iqmp, priv.coefficient, sizeof(priv.coefficient));
#endif
}

static int _bn2bin(const BIGNUM *from, unsigned char *to, int max) {
    int i;
    i=BN_num_bytes(from);
    if (i > max) {
        return(0);
    }
    memset(to,0,(unsigned int)max);
    if (!BN_bn2bin(from,&(to[max-i])))
        return(0);
    return(1);
}

int openssl_to_private(RSA *from, R_RSA_PRIVATE_KEY *to) {
#ifdef HAVE_OPAQUE_RSA_DSA_DH
    const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    const BIGNUM *p;
    const BIGNUM *q;
    const BIGNUM *dmp1;
    const BIGNUM *dmq1;
    const BIGNUM *iqmp;

    RSA_get0_key(from, &n, &e, &d);
    RSA_get0_factors(from, &p, &q);
    RSA_get0_crt_params(from, &dmp1, &dmq1, &iqmp);

    to->bits = (unsigned short)BN_num_bits(n);
    if (!_bn2bin(n,to->modulus,MAX_RSA_MODULUS_LEN))
        return(0);
    if (!_bn2bin(e,to->publicExponent,MAX_RSA_MODULUS_LEN))
        return(0);
    if (!_bn2bin(d,to->exponent,MAX_RSA_MODULUS_LEN))
        return(0);
    if (!_bn2bin(p,to->prime[0],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(q,to->prime[1],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(dmp1,to->primeExponent[0],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(dmq1,to->primeExponent[1],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(iqmp,to->coefficient,MAX_RSA_PRIME_LEN))
        return(0);
#else
    to->bits = BN_num_bits(from->n);
    if (!_bn2bin(from->n,to->modulus,MAX_RSA_MODULUS_LEN))
        return(0);
    if (!_bn2bin(from->e,to->publicExponent,MAX_RSA_MODULUS_LEN))
        return(0);
    if (!_bn2bin(from->d,to->exponent,MAX_RSA_MODULUS_LEN))
        return(0);
    if (!_bn2bin(from->p,to->prime[0],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(from->q,to->prime[1],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(from->dmp1,to->primeExponent[0],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(from->dmq1,to->primeExponent[1],MAX_RSA_PRIME_LEN))
        return(0);
    if (!_bn2bin(from->iqmp,to->coefficient,MAX_RSA_PRIME_LEN))
        return(0);
#endif
    return 1;
}

int check_validity_of_cert(
    const char *cFile, const unsigned char *md5_md, unsigned char *sfileMsg,
    const int sfsize, const char* caPath
) {
    int retval = 0;
    X509 *cert;
    X509_STORE *store;
    X509_LOOKUP *lookup;
    X509_STORE_CTX *ctx = 0;
    EVP_PKEY *pubKey;
    BIO *bio;

    bio = BIO_new(BIO_s_file());
    BIO_read_filename(bio, cFile);
    if (NULL == (cert = PEM_read_bio_X509(bio, NULL, 0, NULL))) {
        BIO_vfree(bio);
        return 0;
    }
    // verify certificate
    store = X509_STORE_new();
    lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
    X509_LOOKUP_add_dir(lookup, (char *)caPath, X509_FILETYPE_PEM);
    if ((ctx = X509_STORE_CTX_new()) != 0) {
        if (X509_STORE_CTX_init(ctx, store, cert, 0) == 1)
            retval = X509_verify_cert(ctx);
        X509_STORE_CTX_free(ctx);
    }
    X509_STORE_free(store);

    if (retval != 1) {
        fprintf(stderr,
            "%s: ERROR: Cannot verify certificate ('%s')\n",
            time_to_string(dtime()), cFile
        );
        return 0;
    }
    pubKey = X509_get_pubkey(cert);
    if (!pubKey) {
        X509_free(cert);
        BIO_vfree(bio);
        return 0;
    }
#ifdef HAVE_OPAQUE_EVP_PKEY
    if (EVP_PKEY_id(pubKey) == EVP_PKEY_RSA) {
#else
    if (pubKey->type == EVP_PKEY_RSA) {
#endif
        BN_CTX *c = BN_CTX_new();
        if (!c) {
            X509_free(cert);
            EVP_PKEY_free(pubKey);
            BIO_vfree(bio);
            return 0;
        }
#ifdef HAVE_OPAQUE_RSA_DSA_DH
        RSA *rsa;
        // CAUTION: In OpenSSL 3.0.0, EVP_PKEY_get0_RSA() now returns a
        // pointer of type "const struct rsa_st*" to an immutable value.
        // Do not try to modify the contents of the returned struct.
        rsa = (rsa_st*)EVP_PKEY_get0_RSA(pubKey);
        if (!RSA_blinding_on(rsa, c)) {
#else
        if (!RSA_blinding_on(pubKey->pkey.rsa, c)) {
#endif
            X509_free(cert);
            EVP_PKEY_free(pubKey);
            BIO_vfree(bio);
            BN_CTX_free(c);
            return 0;
        }
#ifdef HAVE_OPAQUE_RSA_DSA_DH
        retval = RSA_verify(NID_md5, md5_md, MD5_DIGEST_LENGTH, sfileMsg, sfsize, rsa);
        RSA_blinding_off(rsa);
#else
        retval = RSA_verify(NID_md5, md5_md, MD5_DIGEST_LENGTH, sfileMsg, sfsize, pubKey->pkey.rsa);
        RSA_blinding_off(pubKey->pkey.rsa);
#endif
        BN_CTX_free(c);
    }
#ifdef HAVE_OPAQUE_EVP_PKEY
    if (EVP_PKEY_id(pubKey) == EVP_PKEY_DSA) {
#else
    if (pubKey->type == EVP_PKEY_DSA) {
#endif
        fprintf(stderr,
            "%s: ERROR: DSA keys are not supported.\n",
            time_to_string(dtime())
        );
        return 0;
    }
    EVP_PKEY_free(pubKey);
    X509_free(cert);
    BIO_vfree(bio);
    return retval;
}

char *check_validity(
    const char *certPath, const char *origFile, unsigned char *signature,
    char* caPath
) {
    MD5_CTX md5CTX;
    int rbytes;
    unsigned char md5_md[MD5_DIGEST_LENGTH],  rbuf[2048];

// OpenSSL 1.1 does initialization internally. This is default.
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(HAVE_LIBRESSL)
    SSL_load_error_strings();
    SSL_library_init();
#endif

    if (!is_file(origFile)) {
        return NULL;
    }
    FILE* of = boinc_fopen(origFile, "r");
    if (!of) return NULL;
    MD5_Init(&md5CTX);
    while (0 != (rbytes = (int)fread(rbuf, 1, sizeof(rbuf), of))) {
        MD5_Update(&md5CTX, rbuf, rbytes);
    }
    MD5_Final(md5_md, &md5CTX);
    fclose(of);

    DIRREF dir = dir_open(certPath);

    char file[MAXPATHLEN];
    while (!dir_scan(file, dir, sizeof(file))) {
        char fpath[MAXPATHLEN];
        snprintf(fpath, sizeof(fpath), "%.*s/%.*s", DIR_LEN, certPath, FILE_LEN, file);
        // TODO : replace '128'
        if (check_validity_of_cert(fpath, md5_md, signature, 128, caPath)) {
            dir_close(dir);
            return strdup(fpath);
        }
    }

    dir_close(dir);
    return NULL;
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
            verified = check_validity_of_cert(fbuf, md5_md, sig_db.data, 128, trustLocation);
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

