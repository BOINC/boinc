#include <stdio.h>
#include <malloc.h>

#include "rsaeuro.h"
extern "C" {
#include "rsa.h"
}
#include "md5_file.h"
#include "crypt.h"

// print some data in hex notation.
// NOTE: since length may not be known to the reader,
// we follow the data with a non-hex character '.'
//
int print_hex_data(FILE* f, DATA_BLOCK& x) {
    int i;

    for (i=0; i<x.len; i++) {
        fprintf(f, "%02x", x.data[i]);
        if (i%32==31) fprintf(f, "\n");
    }
    if (x.len%32 != 0) fprintf(f, "\n");
    fprintf(f, ".\n");
}

// scan data in hex notation.
// stop when you reach a non-parsed character.
// NOTE: buffer must be big enough.
//
int scan_hex_data(FILE* f, DATA_BLOCK& x) {
    int n;
    x.len = 0;
    while (1) {
        n = fscanf(f, "%2x", x.data+x.len);
        if (n <= 0) break;
        x.len++;
    }
    return 0;
}

// print a key in ASCII form
//
int print_key_hex(FILE* f, KEY* key, int size) {
    int len, i;
    DATA_BLOCK x;

    fprintf(f, "%d\n", key->bits);
    len = size - sizeof(key->bits);
    x.data = key->data;
    x.len = len;
    return print_hex_data(f, x);
}

int scan_key_hex(FILE* f, KEY* key, int size) {
    int len, i;

    fscanf(f, "%d", &key->bits);
    len = size - sizeof(key->bits);
    for (i=0; i<len; i++) {
        fscanf(f, "%2x", key->data+i);
    }
    fscanf(f, ".");
    return 0;
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
    int retval, n;
    n = in.len;
    if (n >= key.bits-11) {
        n = key.bits-11;
    }
    retval = RSAPrivateEncrypt(out.data, &out.len, in.data, n, &key);
    if (retval) return retval;
    nbytes_encrypted = n;
    return 0;
}

int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
    RSAPublicDecrypt(out.data, &out.len, in.data, in.len, &key);
}

int sign_file(char* path, R_RSA_PRIVATE_KEY& key, DATA_BLOCK& signature) {
    char md5_buf[64];
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

int verify_file(
    char* path, R_RSA_PUBLIC_KEY& key, DATA_BLOCK& signature, bool& answer
) {
    char md5_buf[64], clear_buf[256];
    double file_length;
    int n, retval;
    DATA_BLOCK clear_signature;

    retval = md5_file(path, md5_buf, file_length);
    if (retval) return retval;
    n = strlen(md5_buf);
    clear_signature.data = (unsigned char*)clear_buf;
    clear_signature.len = 256;
    retval = decrypt_public(key, signature, clear_signature);
    if (retval) return retval;
    answer = !strncmp(md5_buf, clear_buf, n);
    return 0;
}
