// test sporadic app
//
//  loop
//      wait for NWAIT secs
//      wait for could compute
//      ask to compute (simulate getting a request)
//      when OK, compute for NCOMP secs
//      suspend as needed
//
// computing is embedded in the loop.
// in a real app you'd want to use threads

// by default this uses the BOINC API for communicating sporadic state.
// --wrapped: use files instead (run under wrapper)

#define NWAIT 10
#define NCOMP 10

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdio.h>

#include "boinc_api.h"
#include "util.h"
#include "common_defs.h"

bool wrapped = false;
int ac_fd, ca_fd;

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

void set_ac_state(SPORADIC_AC_STATE ac_state) {
    static SPORADIC_AC_STATE last = AC_NONE;
    if (wrapped) {
        if (ac_state != last) {
            char buf[256];
            sprintf(buf, "%d\n", ac_state);
            lseek(ac_fd, 0, SEEK_SET);
            write(ac_fd, buf, static_cast<unsigned int>(strlen(buf)));
        }
        last = ac_state;
    } else {
        boinc_sporadic_set_ac_state(ac_state);
    }
}

SPORADIC_CA_STATE get_ca_state() {
    if (wrapped) {
        // could check mod time; don't bother
        char buf[256];
        lseek(ca_fd, 0, SEEK_SET);
        read(ca_fd, buf, sizeof(buf));
        int s;
        int n = sscanf(buf, "%d", &s);
        if (n==1) return (SPORADIC_CA_STATE)s;
        fprintf(stderr, "can't read CA state\n");
        exit(1);
    } else {
        return boinc_sporadic_get_ca_state();
    }
}

int main(int argc, char** argv) {
    SPORADIC_CA_STATE ca_state;
    SPORADIC_AC_STATE ac_state;

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--wrapped")) {
            wrapped = true;
        }
    }

    if (wrapped) {
        ca_fd = open("ca", O_RDONLY);
        ac_fd = open("ac", O_WRONLY);
        if (ca_fd<0 || ac_fd<0) {
            fprintf(stderr, "can't open files\n");
            exit(1);
        }
    } else {
        boinc_init();
    }

    fprintf(stderr, "starting\n");
    while (true) {
        // wait for a bit
        ac_state = AC_DONT_WANT_COMPUTE;
        set_ac_state(ac_state);
        for (int i=0; i<NWAIT; i++) {
            fprintf(stderr, "sleep - don't want to compute\n");
            boinc_sleep(1);
        }
        // wait until client says we can possibly compute
        while (1) {
            ca_state = get_ca_state();
            if (ca_state != CA_COULD_COMPUTE) {
                fprintf(stderr, "sleep - waiting for COULD_COMPUTE\n");
                boinc_sleep(1);
            } else {
                break;
            }
        }
        // tell the client we want to compute
        ac_state = AC_WANT_COMPUTE;
        set_ac_state(ac_state);
        int n = NCOMP;
        while (true) {
            // compute only if client says so
            ca_state = get_ca_state();
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
