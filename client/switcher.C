// switcher.C
//
// When run as
// switcher P X1 ... Xn
// runs program P with args X1. ... Xn

#include <unistd.h>

int main(int, char** argv) {
    execv(argv[1], argv+1);
}
