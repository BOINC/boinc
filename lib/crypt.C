#include <stdio.h>
#include <stdlib.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "md5_file.h"
#include "crypt.h"
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
    if(f==NULL) {
        fprintf(stderr, "error: print_hex_data: unexpected NULL pointer f\n");
        return ERR_NULL;
    }
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
    if(p==NULL) {
        fprintf(stderr, "error: sprint_hex_data: unexpected NULL pointer p\n");
        return ERR_NULL;
    }
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
    if(f==NULL) {
        fprintf(stderr, "error: scan_hex_data: unexpected NULL pointer f\n");
        return ERR_NULL;
    }
#if _USING_FCGI_
    char *p, buf[256];
    int i, j;
    while (1) {
        p = fgets(buf, 256, f);
	if (!p) return -1;
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
        n = fscanf(f, "%2x", x.data+x.len);
        if (n <= 0) break;
        x.len++;
    }
#endif
    return 0;
}

// same, but read from buffer
//
int sscan_hex_data(char* p, DATA_BLOCK& x) {
    if(p==NULL) {
        fprintf(stderr, "error: sscan_hex_data: unexpected NULL pointer p\n");
        return ERR_NULL;
    }
    if(x.len<0) {
        fprintf(stderr, "error: sscan_hex_data: negative x.len\n");
        return ERR_NEG;
    }
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
    int len;
    DATA_BLOCK x;
    if(f==NULL) {
        fprintf(stderr, "error: print_key_hex: unexpected NULL pointer f\n");
        return ERR_NULL;
    }
    if(key==NULL) {
        fprintf(stderr, "error: print_key_hex: unexpected NULL pointer key\n");
        return ERR_NULL;
    }
    if(size<0) {
        fprintf(stderr, "error: print_key_hex: negative size\n");
        return ERR_NEG;
    }
    fprintf(f, "%d\n", key->bits);
    len = size - sizeof(key->bits);
    x.data = key->data;
    x.len = len;
    return print_hex_data(f, x);
}

int scan_key_hex(FILE* f, KEY* key, int size) {
    int len, i, n;
    int num_bits;
    if(f==NULL) {
        fprintf(stderr, "error: scan_key_hex: unexpected NULL pointer f\n");
        return ERR_NULL;
    }
    if(key==NULL) {
        fprintf(stderr, "error: scan_key_hex: unexpected NULL pointer key\n");
        return ERR_NULL;
    }
    if(size<=0) {
        fprintf(stderr, "error: scan_key_hex: size = %d\n", size);
        return ERR_NEG;
    }
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
	if (!p) return -1;
        n = strlen(p)/2;
        if (n == 0) break;
        for (i=0; i<n; i++) {
            sscanf(buf+i*2, "%2x", &b);
            if (j >= len) return -1;
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
int sscan_key_hex(char* buf, KEY* key, int size) {
    int n, retval,num_bits;
    DATA_BLOCK db;

    if(buf==NULL) {
        fprintf(stderr, "error: sscan_key_hex: unexpected NULL pointer buf\n");
        return ERR_NULL;
    }
    if(key==NULL) {
        fprintf(stderr, "error: sscan_key_hex: unexpected NULL pointer key\n");
        return ERR_NULL;
    }
    if(size<=0) {
        fprintf(stderr, "error: sscan_key_hex: size = %d\n", size);
        return ERR_NEG;
    }
    //fprintf(stderr, "buf = %s\n", buf);
    n = sscanf(buf, "%d", &num_bits);
    key->bits = num_bits; //key->bits is a short
    //fprintf(stderr, "key->bits = %d\n", key->bits);

    if (n != 1) return -1;
    buf = strchr(buf, '\n');
    if (!buf) return -1;
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
    retval = RSAPrivateEncrypt(out.data, &out.len, in.data, n, &key);
    if (retval ) return retval;
    nbytes_encrypted = retval;
    return 0;
}

int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out) {
    if(key.bits<=0) {
        fprintf(stderr, "error: decrypt_public: key.bits = %d\n", key.bits);
        return ERR_NEG;
    }
    return RSAPublicDecrypt(out.data, &out.len, in.data, in.len, &key);
}

int sign_file(char* path, R_RSA_PRIVATE_KEY& key, DATA_BLOCK& signature) {
    char md5_buf[MD5_LEN];
    double file_length;
    DATA_BLOCK in_block;
    int retval, n;
    if(path==NULL) {
        fprintf(stderr, "error: sign_file: unexpected NULL pointer path\n");
        return ERR_NULL;
    }
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
    if(path==NULL) {
        fprintf(stderr, "error: verify_file: unexpected NULL pointer path\n");
        return ERR_NULL;
    }
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
    char* path, char* signature_text, char* key_text, bool& answer
) {
    R_RSA_PUBLIC_KEY key;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    int retval;
    DATA_BLOCK signature;
    if(path==NULL) {
        fprintf(stderr, "error: verify_file2: unexpected NULL pointer path\n");
        return ERR_NULL;
    }
    if(signature_text==NULL) {
        fprintf(stderr, "error: verify_file2: unexpected NULL pointer signature_text\n");
        return ERR_NULL;
    }
    if(key_text==NULL) {
        fprintf(stderr, "error: verify_file2: unexpected NULL pointer key_text\n");
        return ERR_NULL;
    }
    retval = sscan_key_hex(key_text, (KEY*)&key, sizeof(key));
    if (retval) {
        fprintf(stderr, "error: verify_file2: sscan_key_hex did not work\n");
        return retval;
    }
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
    if(text==NULL) {
        fprintf(stderr, "error: verify_string: unexpected NULL pointer text\n");
        return ERR_NULL;
    }
    if(signature_text==NULL) {
        fprintf(stderr, "error: verify_string: unexpected NULL pointer signature_text\n");
	return ERR_NULL;
    }
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
