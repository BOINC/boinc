// some interface functions for RSAEuro

struct KEY {
    unsigned short int bits;
    unsigned char data[1];
};

struct DATA_BLOCK {
    unsigned char* data;
    unsigned int len;
};

#define MIN_OUT_BUFFER_SIZE MAX_RSA_MODULUS_LEN+1

int print_hex_data(FILE* f, DATA_BLOCK&);
int scan_hex_data(FILE* f, DATA_BLOCK&);
int print_key_hex(FILE*, KEY* key, int len);
int scan_key_hex(FILE*, KEY* key, int len);
int encrypt_private(
    R_RSA_PRIVATE_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out, int&
);
int decrypt_public(R_RSA_PUBLIC_KEY& key, DATA_BLOCK& in, DATA_BLOCK& out);
int sign_file(char* path, R_RSA_PRIVATE_KEY&, DATA_BLOCK& signature);
int verify_file(char* path, R_RSA_PUBLIC_KEY&, DATA_BLOCK& signature, bool&);
