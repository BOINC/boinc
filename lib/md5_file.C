#include <stdio.h>

#include "md5.h"
#include "md5_file.h"
#include "error_numbers.h"

int md5_file(char* path, char* output, double& nbytes) {
    unsigned char buf[4096];
    unsigned char binout[16];
    FILE* f;
    md5_state_t state;
    int i, n;
    if(path==NULL) {
        fprintf(stderr, "error: md5_file: unexpected NULL pointer path\n");
        return ERR_NULL;
    }
    if(output==NULL) {
        fprintf(stderr, "error: md5_file: unexpected NULL pointer output\n");
        return ERR_NULL;
    }
    nbytes = 0;
    f = fopen(path, "rb");
    if (!f) {
        fprintf(stdout, "md5_file: can't open %s\n", path);
        perror("md5_file");
        return -1;
    }
    md5_init(&state);
    while (1) {
        n = fread(buf, 1, 4096, f);
        if (n<=0) break;
        nbytes += n;
        md5_append(&state, buf, n);
    }
    md5_finish(&state, binout);
    for (i=0; i<16; i++) {
        sprintf(output+2*i, "%02x", binout[i]);
    }
    output[32] = 0;
    fclose(f);
    return 0;
}

int md5_block(unsigned char* data, int nbytes, char* output) {
    unsigned char binout[16];
    int i;
    if(data==NULL) {
        fprintf(stderr, "error: md5_block: unexpected NULL pointer data\n");
        return ERR_NULL;
    }
    if(nbytes<0) {
        fprintf(stderr, "error: md5_block: negative nbytes\n");
        return ERR_NEG;
    }
    if(output==NULL) {
        fprintf(stderr, "error: md5_block: unexpected NULL pointer output\n");
        return ERR_NULL;
    }
    md5_state_t state;
    md5_init(&state);
    md5_append(&state, data, nbytes);
    md5_finish(&state, binout);
    for (i=0; i<16; i++) {
        sprintf(output+2*i, "%02x", binout[i]);
    }
    output[32] = 0;
    return 0;
}
