#include <stdio.h>

#include "md5_file.h"

main(int, char** argv) {
    char out[33];
    double nbytes;

    md5_file(argv[1], out, nbytes);
    printf("%s\n%f bytes\n", out, nbytes);
}
