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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cctype>
#include <cstdio>
#include <cstdlib>
#endif

#include "md5_file.h"
#include "error_numbers.h"

#include "crypt.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

// NOTE: the fast CGI I/O library doesn't have fscanf(),
// so some of the following have been modified to use
// fgets() and sscanf() instead

// write some data in hex notation.
// NOTE: since length may not be known to the reader,
// we follow the data with a non-hex character '.'
//
int print_hex_data(FILE* f, DATA_BLOCK& x) {
    unsigned int i;

    for (i=0; i<x.len; i++) {
        fprintf(f, "%02x", x.data[i]);
        if (i%32==31) fprintf(f, "\n");
    }
    if (x.len%32 != 0) fprintf(f, "\n");
    fprintf(f, ".\n");
    return 0;
}

// same, but write to buffer
//
int sprint_hex_data(char* p, DATA_BLOCK& x) {
    unsigned int i;
    char buf[16];

    strcpy(p, "");
    for (i=0; i<x.len; i++) {
        sprintf(buf, "%02x", x.data[i]);
        strcat(p, buf);
        if (i%32==31) strcat(p, "\n");
    }
    if (x.len%32 != 0) strcat(p, "\n");
    strcat(p, ".\n");

    return 0;
}

// scan data in hex notation.
// stop when you reach a non-parsed character.
// NOTE: buffer must be big enough; no checking is done.
//
int scan_hex_data(FILE* f, DATA_BLOCK& x) {
    int n;

    x.len = 0;
#if _USING_FCGI_
    char *p, buf[256];
    int i, j;
    while (1) {
        p = fgets(buf, 256, f);
        if (!p) return ERR_GETS;
        n = strlen(p)/2;
        if (n == 0) break;
        for (i=0; i<n; i++) {
            sscanf(buf+i*2, "%2x", &j);
            x.data[x.len] = j;
            x.len++;
        }
    }
#else
    while (1) {
        int j;
        n = fscanf(f, "%2x", &j);
        if (n <= 0) break;
        x.data[x.len] = j;
        x.len++;
    }
#endif
    return 0;
}

// same, but read from buffer
//
static int sscan_hex_data(const char* p, DATA_BLOCK& x) {
    int m, n, nleft=x.len;

    x.len = 0;
    while (1) {
        if (isspace(*p)) {
            ++p;
            continue;
        }
        n = sscanf(p, "%2x", &m);
        if (n <= 0) break;
        x.data[x.len++] = m;
        nleft--;
        if (nleft<0) {
            fprintf(stderr, "sscan_hex_data: buffer overflow\n");
            return ERR_BAD_HEX_FORMAT;
        }
        p += 2;
    }
    return 0;
}

// print a key in ASCII form
//
int print_key_hex(FILE* f, KEY* key, int size) {
    int len;
    DATA_BLOCK x;

    fprintf(f, "%d\n", key->bits);
    len = size - sizeof(key->bits);
    x.data = key->data;
    x.len = len;
    return print_hex_data(f, x);
}

int scan_key_hex(FILE* f, KEY* key, int size) {
    int len, i, n;
    int num_bits;

#if _USING_FCGI_
#if 0
    char *p, buf[256];
    int j = 0, b;
    fgets(buf, 256, f);
    sscanf(buf, "%d", &num_bits);
    key->bits = num_bits;
    len = size - sizeof(key->bits);
    while (1) {
        p = fgets(buf, 256, f);
        if (!p) return ERR_GETS;
        n = strlen(p)/2;
        if (n == 0) break;
        for (i=0; i<n; i++) {
            sscanf(buf+i*2, "%2x", &b);
            if (j >= len) return ERR_SCANF;
            key->data[j++] = b;
        }
    }
    fgets(buf, size, f);
    sscanf(buf, ".");
#endif
#else
    fscanf(f, "%d", &num_bits);
    key->bits = num_bits;
    len = size - sizeof(key->bits);
    for (i=0; i<len; i++) {
        fscanf(f, "%2x", &n);
        key->data[i] = n;
    }
    fscanf(f, ".");
#endif
    return 0;
}

// parse a text-encoded key from a memory buffer
//
int sscan_key_hex(const char* buf, KEY* key, int size) {
    int n, retval,num_bits;
    DATA_BLOCK db;

    //fprintf(stderr, "buf = %s\n", buf);
    n = sscanf(buf, "%d", &num_bits);
    key->bits = num_bits; //key->bits is a short
    //fprintf(stderr, "key->bits = %d\n", key->bits);

    if (n != 1) return ERR_SCANF;
    buf = strchr(buf, '\n');
    if (!buf) return ERR_STRCHR;
    buf += 1;
    db.data = key->data;
    db.len = size - sizeof(key->bits); //huh???
    retval = sscan_hex_data(buf, db);
    return retval;
}

// encrypt some data.
// The amount encrypted may be less than what's supplied.
// The output buffer must be at least MIN_OUT_BUFFER_SIZE.
// The output block must be decrypted in its entirety.
//
int encrypt_private(
    R_RSA_PRIVATE_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out,
    int& nbytes_encrypted
) {
    int retval, n, modulus_len;

    modulus_len = (key.bits+7)/8;
    n = in.len;
    if (n >= modulus_len-11) {
        n = modulus_len-11;
    }
#ifdef USE_RSAEURO
    retval = RSAPrivateEncrypt(out.data, &out.len, in.data, n, &key);
    if (retval ) return retval;
    nbytes_encrypted = retval;
#endif
#ifdef USE_OPENSSL
    RSA* rp = RSA_new();
    private_to_openssl(key, rp);
    RSA_private_encrypt(n, in.data, out.data, rp, RSA_PKCS1_PADDING);
    out.len = RSA_size(rp);
    RSA_free(rp);
#endif

    return 0;
}

int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
#ifdef USE_RSAEURO
    return RSAPublicDecrypt(out.data, &out.len, in.data, in.len, &key);
#endif
#ifdef USE_OPENSSL
    RSA* rp = RSA_new();
    public_to_openssl(key, rp);
    RSA_public_decrypt(in.len, in.data, out.data, rp, RSA_PKCS1_PADDING);
    out.len = RSA_size(rp);
    return 0;
#endif
}

int sign_file(const char* path, R_RSA_PRIVATE_KEY& key, DATA_BLOCK& signature) {
    char md5_buf[MD5_LEN];
    double file_length;
    DATA_BLOCK in_block;
    int retval, n;

    retval = md5_file(path, md5_buf, file_length);
    if (retval) return retval;
    in_block.data = (unsigned char*)md5_buf;
    in_block.len = strlen(md5_buf);
    retval = encrypt_private(key, in_block, signature, n);
    if (retval) return retval;
    return 0;
}

int sign_block(DATA_BLOCK& data_block, R_RSA_PRIVATE_KEY& key, DATA_BLOCK& signature) {
    char md5_buf[MD5_LEN];
    int retval, n;
    DATA_BLOCK in_block;

    md5_block(data_block.data, data_block.len, md5_buf);
    in_block.data = (unsigned char*)md5_buf;
    in_block.len = strlen(md5_buf);
    retval = encrypt_private(key, in_block, signature, n);
    if (retval) {
        printf("sign_block: encrypt_private returned %d\n", retval);
        return retval;
    }
    return 0;
}

int verify_file(
    const char* path, R_RSA_PUBLIC_KEY& key, DATA_BLOCK& signature, bool& answer
) {
    char md5_buf[MD5_LEN], clear_buf[MD5_LEN];
    double file_length;
    int n, retval;
    DATA_BLOCK clear_signature;

    retval = md5_file(path, md5_buf, file_length);
    if (retval) {
        fprintf(stderr, "error: verify_file: md5_file error %d\n", retval);
        return retval;
    }
    n = strlen(md5_buf);
    clear_signature.data = (unsigned char*)clear_buf;
    clear_signature.len = MD5_LEN;
    retval = decrypt_public(key, signature, clear_signature);
    if (retval) {
        fprintf(stderr, "error: verify_file: decrypt_public error %d\n", retval);
        return retval;
    }
    answer = !strncmp(md5_buf, clear_buf, n);
    return 0;
}

int verify_file2(
    const char* path, const char* signature_text, const char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    int retval;
    DATA_BLOCK signature;

    retval = sscan_key_hex(key_text, (KEY*)&key, sizeof(key));
    if (retval) {
        fprintf(stderr, "error: verify_file2: sscan_key_hex did not work\n");
        return retval;
    }
    signature.data = signature_buf;
    signature.len = sizeof(signature_buf);
    retval = sscan_hex_data(signature_text, signature);
    if (retval) return retval;
    return verify_file(path, key, signature, answer);
}

// verify, where both text and signature are char strings
//
int verify_string(
    const char* text, const char* signature_text, R_RSA_PUBLIC_KEY& key, bool& answer
) {
    char md5_buf[MD5_LEN];
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    char clear_buf[MD5_LEN];
    int retval, n;
    DATA_BLOCK signature, clear_signature;

    retval = md5_block((const unsigned char*)text, strlen(text), md5_buf);
    if (retval) return retval;
    n = strlen(md5_buf);
    signature.data = signature_buf;
    signature.len = sizeof(signature_buf);
    retval = sscan_hex_data(signature_text, signature);
    if (retval) return retval;
    clear_signature.data = (unsigned char*)clear_buf;
    clear_signature.len = 256;
    retval = decrypt_public(key, signature, clear_signature);
    if (retval) return retval;
    answer = !strncmp(md5_buf, clear_buf, n);
    return 0;
}

// Same, where public key is also encoded as text
//
int verify_string2(
    const char* text, const char* signature_text, const char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    int retval;

    retval = sscan_key_hex(key_text, (KEY*)&key, sizeof(key));
    if (retval) return retval;
    return verify_string(text, signature_text, key, answer);
}

int read_key_file(const char* keyfile, R_RSA_PRIVATE_KEY& key) {
    int retval;
    FILE* fkey = fopen(keyfile, "r");
    if (!fkey) {
        fprintf(stderr, "can't open key file (%s)\n", keyfile);
        return ERR_FOPEN;
    }
    retval = scan_key_hex(fkey, (KEY*)&key, sizeof(key));
    fclose(fkey);
    if (retval) {
        fprintf(stderr, "can't parse key\n");
        return retval;
    }
    return 0;
}

#ifdef USE_OPENSSL

static void bn_to_bin(BIGNUM* bn, unsigned char* bin, int n) {
    memset(bin, 0, n);
    int m = BN_num_bytes(bn);
    BN_bn2bin(bn, bin+n-m);
}

void openssl_to_keys(
    RSA* rp, int nbits, R_RSA_PRIVATE_KEY& priv, R_RSA_PUBLIC_KEY& pub
) {
    unsigned char buf[256];

    pub.bits = nbits;
    bn_to_bin(rp->n, pub.modulus, sizeof(pub.modulus));
    bn_to_bin(rp->e, pub.exponent, sizeof(pub.exponent));

    memset(&priv, 0, sizeof(priv));
    priv.bits = nbits;
    bn_to_bin(rp->n, priv.modulus, sizeof(priv.modulus));
    bn_to_bin(rp->e, priv.publicExponent, sizeof(priv.publicExponent));
    bn_to_bin(rp->d, priv.exponent, sizeof(priv.exponent));
    bn_to_bin(rp->p, priv.prime[0], sizeof(priv.prime[0]));
    bn_to_bin(rp->q, priv.prime[1], sizeof(priv.prime[1]));
    bn_to_bin(rp->dmp1, priv.primeExponent[0], sizeof(priv.primeExponent[0]));
    bn_to_bin(rp->dmq1, priv.primeExponent[1], sizeof(priv.primeExponent[1]));
    bn_to_bin(rp->iqmp, priv.coefficient, sizeof(priv.coefficient));
}

void private_to_openssl(R_RSA_PRIVATE_KEY& priv, RSA* rp) {
    rp->n = BN_bin2bn(priv.modulus, sizeof(priv.modulus), 0);
    rp->e = BN_bin2bn(priv.publicExponent, sizeof(priv.publicExponent), 0);
    rp->d = BN_bin2bn(priv.exponent, sizeof(priv.exponent), 0);
    rp->p = BN_bin2bn(priv.prime[0], sizeof(priv.prime[0]), 0);
    rp->q = BN_bin2bn(priv.prime[1], sizeof(priv.prime[1]), 0);
    rp->dmp1 = BN_bin2bn(priv.primeExponent[0], sizeof(priv.primeExponent[0]), 0);
    rp->dmq1 = BN_bin2bn(priv.primeExponent[1], sizeof(priv.primeExponent[1]), 0);
    rp->iqmp = BN_bin2bn(priv.coefficient, sizeof(priv.coefficient), 0);
}

void public_to_openssl(R_RSA_PUBLIC_KEY& pub, RSA* rp) {
    rp->n = BN_bin2bn(pub.modulus, sizeof(pub.modulus), 0);
    rp->e = BN_bin2bn(pub.exponent, sizeof(pub.exponent), 0);
}

#endif
const char *BOINC_RCSID_4f0c2e42ea = "$Id$";
