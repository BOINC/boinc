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


// utility program for encryption.
//
// -genkey n private_keyfile public_keyfile
//                  create a key pair with n bits (512 <= n <= 1024)
//                  write it in hex notation
// -sign file private_keyfile
//                  create a signature for a given file
//                  write it in hex notation
// -sign_string string private_keyfile
//                  create a signature for a given string
//                  write it in hex notation
// -verify file signature_file public_keyfile
//                  verify a signature
// -test_crypt private_keyfile public_keyfile
//                  test encrypt/decrypt

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "crypt.h"

void die(const char* p) {
    fprintf(stderr, "Error: %s\n", p);
    exit(1);
}

unsigned int random_int() {
    unsigned int n;
    FILE* f = fopen("/dev/random", "r");
    if (!f) {
        fprintf(stderr, "can't open /dev/random\n");
        exit(1);
    }
    fread(&n, sizeof(n), 1, f);
    return n;
}

int main(int argc, char** argv) {
    R_RSA_PUBLIC_KEY public_key;
    R_RSA_PRIVATE_KEY private_key;
    int n, retval;
    bool is_valid;
    DATA_BLOCK signature, in, out;
    unsigned char signature_buf[256], buf[256], buf2[256];
    FILE *f, *fpriv, *fpub;
    char cbuf[256];

    if (argc == 1) {
        printf("missing command\n");
        exit(1);
    }
    if (!strcmp(argv[1], "-genkey")) {
        if (argc < 5) {
            fprintf(stderr, "missing cmdline args\n");
            exit(1);
        }
        printf("creating keys in %s and %s\n", argv[3], argv[4]);
        n = atoi(argv[2]);

        srand(random_int());
        RSA* rp = RSA_generate_key(n,  65537, 0, 0);
        openssl_to_keys(rp, n, private_key, public_key);
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
    } else if (!strcmp(argv[1], "-sign_string")) {
        fpriv = fopen(argv[3], "r");
        if (!fpriv) die("fopen");
        retval = scan_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        if (retval) die("scan_key_hex\n");
        generate_signature(argv[2], cbuf, private_key);
        puts(cbuf);
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
        strcpy((char*)buf2, "foobar");
        in.data = buf2;
        in.len = strlen((char*)in.data);
        out.data = buf;
        encrypt_private(private_key, in, out);
        in = out;
        out.data = buf2;
        decrypt_public(public_key, in, out);
        printf("out: %s\n", out.data);
    } else {
        printf("unrecognized command\n");
    }

    return 0;
}

const char *BOINC_RCSID_6633b596b9 = "$Id$";
