// utility program for encryption.
//
// -genkey n private_keyfile public_keyfile
//                  create a key pair with n bits (512 <= n <= 1024)
//                  write it in hex notation
// -sign file private_keyfile
//                  create a signature for a given file
//                  write it in hex notation
// -verify file signature_file public_keyfile
//                  verify a signature
// -test_crypt private_keyfile public_keyfile
//                  test encrypt/decrypt

#include <stdio.h>
#include <stdlib.h>

#include "rsaeuro.h"

#include "crypt.h"

void die(char* p) {
    fprintf(stderr, "Error: %s\n", p);
    exit(1);
}

main(int argc, char** argv) {
    R_RANDOM_STRUCT randomStruct;
    R_RSA_PUBLIC_KEY public_key;
    R_RSA_PRIVATE_KEY private_key;
    R_RSA_PROTO_KEY protoKey;
    int n, retval;
    bool is_valid;
    DATA_BLOCK signature, in, out;
    unsigned char signature_buf[256], buf[256], buf2[256];
    FILE *f, *fpriv, *fpub;

    if (argc == 1) {
        printf("missing command\n");
        exit(1);
    }
    if (!strcmp(argv[1], "-genkey")) {
        n = atoi(argv[2]);

        R_RandomCreate(&randomStruct);

        protoKey.bits = n;
        protoKey.useFermat4 = 1;
        retval = R_GeneratePEMKeys(
            &public_key, &private_key, &protoKey, &randomStruct
        );
        if (retval) die("R_GeneratePEMKeys\n");

        printf("creating keys in %s and %s\n", argv[3], argv[4]);
        fpriv = fopen(argv[3], "w");
        if (!fpriv) die("fopen");
        fpub = fopen(argv[4], "w");
        if (!fpub) die("fopen");
        print_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        print_key_hex(fpub, (KEY*)&public_key, sizeof(public_key));
    } else if (!strcmp(argv[1], "-sign")) {
        fpriv = fopen(argv[3], "r");
        if (!fpriv) die("fopen");
        retval = scan_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        if (retval) die("scan_key_hex\n");
        signature.data = signature_buf;
        signature.len = 256;
        retval = sign_file(argv[2], private_key, signature);
        print_hex_data(stdout, signature);
    } else if (!strcmp(argv[1], "-verify")) {
        fpub = fopen(argv[4], "r");
        if (!fpub) die("fopen");
        retval = scan_key_hex(fpub, (KEY*)&public_key, sizeof(public_key));
        if (retval) die("read_public_key");
        f = fopen(argv[3], "r");
        signature.data = signature_buf;
        signature.len = 256;
        retval = scan_hex_data(f, signature);
        if (retval) die("scan_hex_data");
        retval = verify_file(argv[2], public_key, signature, is_valid);
        if (retval) die("verify_file");
        printf("file is %s\n", is_valid?"valid":"invalid");
    } else if (!strcmp(argv[1], "-test_crypt")) {
        fpriv = fopen(argv[2], "r");
        if (!fpriv) die("fopen");
        retval = scan_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        if (retval) die("scan_key_hex\n");
        fpub = fopen(argv[3], "r");
        if (!fpub) die("fopen");
        retval = scan_key_hex(fpub, (KEY*)&public_key, sizeof(public_key));
        if (retval) die("read_public_key");
        in.data = (unsigned char*) "foobar";
        in.len = strlen((char*)in.data);
        out.data = buf;
        encrypt_private(private_key, in, out, n);
        in = out;
        out.data = buf2;
        decrypt_public(public_key, in, out);
    } else {
        printf("unrecognized command\n");
    }
}
