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

#include "crypt.h"

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
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

#ifdef _MSC_VER
#define snprintf    _snprintf
#define strdup      _strdup
#endif

#include <openssl/ssl.h>
#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/err.h>

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "md5_file.h"
#include "cert_sig.h"
#include "filesys.h"
#include "error_numbers.h"

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
int sprint_hex_data(char* out_buf, DATA_BLOCK& x) {
    unsigned int i;
    const char hex[] = "0123456789abcdef";
    char* p = out_buf;

    for (i=0; i<x.len; i++) {
        *p++ = hex[x.data[i]/16];
        *p++ = hex[x.data[i]%16];
        if (i%32==31) *p++ = '\n';
    }
    if (x.len%32 != 0) *p++ = '\n';
    strcpy(p, ".\n");

    return 0;
}

int print_raw_data(FILE* f, DATA_BLOCK& x) {
    unsigned int i;
    for (i=0; i<x.len; i++) {
        //printf("%x ", x.data[i]);
        fprintf(f, "%c", x.data[i]);
    }
    return 0;
}

// NOTE: buffer must be big enough; no checking is done.
int scan_raw_data(FILE *f, DATA_BLOCK& x) {
    int i=0,j;
    while(EOF!=(j=fgetc(f))) {
        x.data[i]=j;
        i++;
    }
    x.len = i;    
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
        if (nleft<=0) {
            fprintf(stderr, "sscan_hex_data: buffer overflow\n");
            return ERR_BAD_HEX_FORMAT;
        }
        x.data[x.len++] = m;
        nleft--;
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
    char *p, buf[256];
    int j = 0, b;
    fgets(buf, 256, f);
    sscanf(buf, "%d", &num_bits);
    key->bits = num_bits;
    len = size - sizeof(key->bits);
    while (1) {
        p = fgets(buf, 256, f);
        if (!p) break;
        n = (strlen(p)-1)/2;
        if (n == 0) break;
        for (i=0; i<n; i++) {
            sscanf(buf+i*2, "%2x", &b);
            if (j == len) break;
            key->data[j++] = b;
        }
    }
    if (j != len) return ERR_NULL;
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

    if (n != 1) return ERR_XML_PARSE;
    buf = strchr(buf, '\n');
    if (!buf) return ERR_XML_PARSE;
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
int encrypt_private(R_RSA_PRIVATE_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
    int n, modulus_len, retval;

    modulus_len = (key.bits+7)/8;
    n = in.len;
    if (n >= modulus_len-11) {
        n = modulus_len-11;
    }
    RSA* rp = RSA_new();
    private_to_openssl(key, rp);
    retval = RSA_private_encrypt(n, in.data, out.data, rp, RSA_PKCS1_PADDING);
    if (retval < 0) {
        RSA_free(rp);
        return ERR_CRYPTO;
    }
    out.len = RSA_size(rp);
    RSA_free(rp);
    return 0;
}

int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
    int retval;
    RSA* rp = RSA_new();
    public_to_openssl(key, rp);
    retval = RSA_public_decrypt(in.len, in.data, out.data, rp, RSA_PKCS1_PADDING);
    if (retval < 0) {
        RSA_free(rp);
        return ERR_CRYPTO;
    }
    out.len = RSA_size(rp);
    RSA_free(rp);
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
    char* text_to_sign, char* signature_hex, R_RSA_PRIVATE_KEY& key
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
    sprint_hex_data(signature_hex, signature_data);
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

    n = (int)strlen(md5_buf);
    clear_signature.data = (unsigned char*)clear_buf;
    clear_signature.len = MD5_LEN;
    retval = decrypt_public(key, signature, clear_signature);
    if (retval) {
        fprintf(stderr, "check_file_signature: decrypt_public error %d\n", retval);
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
    int retval;
    DATA_BLOCK signature;

    retval = sscan_key_hex(key_text, (KEY*)&key, sizeof(key));
    if (retval) {
        fprintf(stderr, "check_file_signature2: sscan_key_hex failed\n");
        return retval;
    }
    signature.data = signature_buf;
    signature.len = sizeof(signature_buf);
    retval = sscan_hex_data(signature_text, signature);
    if (retval) return retval;
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
int check_string_signature2(
    const char* text, const char* signature_text, const char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    int retval;

    retval = sscan_key_hex(key_text, (KEY*)&key, sizeof(key));
    if (retval) return retval;
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

static void bn_to_bin(BIGNUM* bn, unsigned char* bin, int n) {
    memset(bin, 0, n);
    int m = BN_num_bytes(bn);
    BN_bn2bin(bn, bin+n-m);
}

void openssl_to_keys(
    RSA* rp, int nbits, R_RSA_PRIVATE_KEY& priv, R_RSA_PUBLIC_KEY& pub
) {
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

static int _bn2bin(BIGNUM *from, unsigned char *to, int max) {
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
        fprintf(stderr,"ERROR: Cannot verify certificate ('%s')\n", cFile);
        return 0;
    }        
    pubKey = X509_get_pubkey(cert);
    if (!pubKey) {
        X509_free(cert);
        BIO_vfree(bio);
        return 0;
    }
    if (pubKey->type == EVP_PKEY_RSA) {
        BN_CTX *c = BN_CTX_new();
        if (!c) {
	        X509_free(cert);
	        EVP_PKEY_free(pubKey);
	        BIO_vfree(bio);
	        return 0;
	    }
	    if (!RSA_blinding_on(pubKey->pkey.rsa, c)) {
	        X509_free(cert);
	        EVP_PKEY_free(pubKey);
	        BIO_vfree(bio);
	        BN_CTX_free(c);
	        return 0;
	    }
	    retval = RSA_verify(NID_md5, md5_md, MD5_DIGEST_LENGTH, sfileMsg, sfsize, pubKey->pkey.rsa);
	    RSA_blinding_off(pubKey->pkey.rsa);
	    BN_CTX_free(c);
    }
    if (pubKey->type == EVP_PKEY_DSA) {
        fprintf(stderr, "ERROR: DSA keys are not supported.\n");
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

    SSL_load_error_strings();
    SSL_library_init();

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

    char file[256];
    while (dir_scan(file, dir, sizeof(file))) {
        char fpath[MAXPATHLEN];
	    snprintf(fpath, sizeof(fpath), "%s/%s", certPath, file);
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
    SSL_library_init();
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
        sscan_hex_data(signatures->signatures.at(i).signature, sig_db);
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

