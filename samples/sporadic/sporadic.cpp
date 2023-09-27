// test sporadic app
//
//  loop
//      wait for NWAIT secs
//      wait for could compute
//      ask to compute (simulate getting a request)
//      when OK, compute for NCOMP secs
//      suspend as needed
//
// computing is embedded in loop.
// in a real app you'd want to use threads

#define NWAIT 10
#define NCOMP 10

#include "boinc_api.h"
#include "util.h"
#include "common_defs.h"

void boinc_sporadic_set_ac_state(SPORADIC_AC_STATE);
SPORADIC_CA_STATE boinc_sporadic_get_ca_state();

void print_state() {
}

void compute_one_sec() {
    double start = dtime();
    while (1) {
        double x = 0;
        for (int i=0; i<1e8; i++) {
            x += 1;
        }
        if (dtime() > start+1) break;
    }
}

int main(int, char**) {
    boinc_init();
    SPORADIC_CA_STATE ca_state;
    SPORADIC_AC_STATE ac_state;
    fprintf(stderr, "starting\n");
    while (true) {
        // wait for a bit
        ac_state = AC_DONT_WANT_COMPUTE;
        boinc_sporadic_set_ac_state(ac_state);
        for (int i=0; i<NWAIT; i++) {
            fprintf(stderr, "sleep - don't want to compute\n");
            boinc_sleep(1);
        }
        // wait until client says we can possibly compute
        while (1) {
            ca_state = boinc_sporadic_get_ca_state();
            if (ca_state != CA_COULD_COMPUTE) {
                fprintf(stderr, "sleep - waiting for COULD_COMPUTE\n");
                boinc_sleep(1);
            } else {
                break;
            }
        }
        // tell the client we want to compute
        ac_state = AC_WANT_COMPUTE;
        boinc_sporadic_set_ac_state(ac_state);
        int n = NCOMP;
        while (true) {
            // compute only if client says so
            ca_state = boinc_sporadic_get_ca_state();
            fprintf(stderr, "CA state: %d\n", ca_state);
            if (ca_state == CA_COMPUTING) {
                fprintf(stderr, "computing 1 sec\n");
                compute_one_sec();
                n--;
                if (n == 0) {
                    fprintf(stderr, "done computing\n");
                    break;
                }
            } else {
                fprintf(stderr, "sleep - waiting for COMPUTING\n");
                boinc_sleep(1);
            }
        }
    }
}
