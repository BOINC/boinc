#include <stdio.h>

#include "md5.h"
#include "md5_file.h"

int md5_file(char* path, char* output, double& nbytes) {
    unsigned char buf[4096];
    unsigned char binout[16];
    FILE* f;
    md5_state_t state;
    int i, n;

    nbytes = 0;
    f = fopen(path, "r");
    if (!f) return -1;
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
