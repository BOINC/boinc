#include <stdio.h>
#include <malloc.h>

#include "md5_file.h"
#include "crypt.h"

// write some data in hex notation.
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

// same, but write to buffer
//
int sprint_hex_data(char* p, DATA_BLOCK& x) {
    int i;
    char buf[16];

    strcpy(p, "");
    for (i=0; i<x.len; i++) {
        sprintf(buf, "%02x", x.data[i]);
        strcat(p, buf);
        if (i%32==31) strcat(p, "\n");
    }
    if (x.len%32 != 0) strcat(p, "\n");
    strcat(p, ".\n");
}

// scan data in hex notation.
// stop when you reach a non-parsed character.
// NOTE: buffer must be big enough.
//
int scan_hex_data(FILE* f, DATA_BLOCK& x) {
    int n;
    x.len = 0;
    char *retval, buf[3];
    while (1) {
        retval = fgets(buf, 2, f);
	if(retval == NULL) break;
        sscanf(buf, "%2x", x.data+x.len);
        //n = fscanf(f, "%2x", x.data+x.len);
        //if (n <= 0) break;
        x.len++;
    }
    return 0;
}

// same, but read from buffer
//
int sscan_hex_data(char* p, DATA_BLOCK& x) {
    int m, n, nleft=x.len;
    x.len = 0;
    while (1) {
        n = sscanf(p, "%2x", &m);
        if (n <= 0) break;
        x.data[x.len++] = m;
        nleft--;
        if (nleft<0) {
            fprintf(stderr, "sscan_hex_data: buffer overflow\n");
            exit(1);
        }
        p += 2;
        if (*p == '\n') p++;
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
    int len, i, n;
    int num_bits,nbytes_read=0;
    char buf[size+1];

    //fscanf(f, "%d", &num_bits);
    fgets(buf, size, f);
    sscanf(buf, "%d", &num_bits);
    key->bits = num_bits;
    len = size - sizeof(key->bits);
    for (i=0; i<len; i++) {
        if( !nbytes_read ) {
            fgets(buf, size, f);
            nbytes_read = strlen(buf);
        }
        //fscanf(f, "%2x", &n);
        sscanf(buf, "%2x", &n);
        key->data[i] = n;
        nbytes_read -= 2;
    }
    fgets(buf, size, f);
    //fscanf(f, ".");
    sscanf(buf, ".");
    return 0;
}

// parse a text-encoded key from a memory buffer
//
int sscan_key_hex(char* buf, KEY* key, int size) {
    int n, retval;
    DATA_BLOCK db;

    n = sscanf(buf, "%d", &key->bits);
    if (n != 1) return -1;
    buf = strchr(buf, '\n');
    if (!buf) return -1;
    buf += 1;
    db.data = key->data;
    db.len = size - sizeof(key->bits);
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
    retval = RSAPrivateEncrypt(out.data, &out.len, in.data, n, &key);
    if (retval ) return retval;
    nbytes_encrypted = retval;
    return 0;
}

int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
    return RSAPublicDecrypt(out.data, &out.len, in.data, in.len, &key);
}

int sign_file(char* path, R_RSA_PRIVATE_KEY& key, DATA_BLOCK& signature) {
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
    char* path, R_RSA_PUBLIC_KEY& key, DATA_BLOCK& signature, bool& answer
) {
    char md5_buf[MD5_LEN], clear_buf[MD5_LEN];
    double file_length;
    int n, retval;
    DATA_BLOCK clear_signature;

    retval = md5_file(path, md5_buf, file_length);
    if (retval) return retval;
    n = strlen(md5_buf);
    clear_signature.data = (unsigned char*)clear_buf;
    clear_signature.len = MD5_LEN;
    retval = decrypt_public(key, signature, clear_signature);
    if (retval) return retval;
    answer = !strncmp(md5_buf, clear_buf, n);
    return 0;
}

int verify_file2(
    char* path, char* signature_text, char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    int retval;
    DATA_BLOCK signature;

    retval = sscan_key_hex(key_text, (KEY*)&key, sizeof(key));
    if (retval) return retval;
    signature.data = signature_buf;
    signature.len = sizeof(signature_buf);
    sscan_hex_data(signature_text, signature);
    return verify_file(path, key, signature, answer);
}

// verify, where both text and signature are char strings
//
int verify_string(
    char* text, char* signature_text, R_RSA_PUBLIC_KEY& key, bool& answer
) {
    char md5_buf[MD5_LEN];
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    char clear_buf[MD5_LEN];
    int retval, n;
    DATA_BLOCK signature, clear_signature;

    retval = md5_block((unsigned char*)text, strlen(text), md5_buf);
    if (retval) return retval;
    n = strlen(md5_buf);
    signature.data = signature_buf;
    signature.len = sizeof(signature_buf);
    sscan_hex_data(signature_text, signature);
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
    char* text, char* signature_text, char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    int retval;

    retval = sscan_key_hex(key_text, (KEY*)&key, sizeof(key));
    if (retval) return retval;
    return verify_string(text, signature_text, key, answer);
}
