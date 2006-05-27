#include "util.h"
#include "boinc_api.h"

int main() {
    boinc_init();
    while (1) {
        boinc_sleep(1);
    }
}
