// switcher.C
//
// When run as
// switcher Path X1 ... Xn
// runs program at Path with args X1. ... Xn

#include <unistd.h>
#include <stdio.h>
#include <cerrno>

int main(int argc, char** argv) {

#if 0           // For debugging
    for (int i=0; i<argc; i++) {
        fprintf(stderr, "switcher arg %d: %s\n", i, argv[i]);
    }
    fflush(stderr);
#endif

    execv(argv[1], argv+2);
    
    // If we got here execv failed
    fprintf(stderr, "Process creation (%s) failed: errno=%d\n", argv[1], errno);

}
