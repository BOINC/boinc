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
#define NCOMP 30

#include "boinc_api.h"
#include "util.h"
#include "common_defs.h"

void boinc_sporadic_set_ac_state(AC_STATE);
CA_STATE boinc_sporadic_get_ca_state();

void print_state() {
}

void compute_one_sec() {
    double start = dtime();
    while (1) {
        double x = 0;
        for (int i=0; i<1e9; i++) {
            x += 1;
        }
        if (dtime() > start+1) break;
    }
}

int main(int, char**) {
    boinc_init();
    CA_STATE ca_state;
    AC_STATE ac_state;
    while (true) {
        // wait for a bit
        ac_state = AC_DONT_WANT_COMPUTE;
        boinc_sporadic_set_ac_state(ac_state);
        for (int i=0; i<NWAIT; i++) {
            boinc_sleep(1);
        }
        // wait until client says we can possibly compute
        while (1) {
            ca_state = boinc_sporadic_get_ca_state();
            if (ca_state != CA_COULD_COMPUTE) {
                boinc_sleep(1);
            }
        }
        // tell the client we want to compute
        ac_state = AC_WANT_COMPUTE;
        boinc_sporadic_set_ac_state(ac_state);
        int n = NCOMP;
        while (true) {
            // compute only if client says so
            ca_state = boinc_sporadic_get_ca_state();
            if (ca_state == CA_COMPUTING) {
                compute_one_sec();
                n--;
                if (n == 0) break;
            } else {
                boinc_sleep(1);
            }
        }
    }
}
