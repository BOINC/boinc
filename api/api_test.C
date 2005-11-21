// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "config.h"

#include <cstdio>
#include <cstdlib>

#include "boinc_api.h"

int get_run_info(double& time, unsigned long int& counter);
void run_api_test(char* args);
int print_results(double time1, double time2,
    unsigned long int counter1, unsigned long int counter2
);
int initialize_api();

int main() {
    double time1, time2;
    unsigned long int counter1, counter2;
    if(initialize_api()) {
        fprintf(stderr, "error: could not initialize api\n");
        return 1;
    }
    run_api_test("2 100");
    get_run_info(time1, counter1);
    if(initialize_api()) {
        fprintf(stderr, "error: could not initialize api\n");
        return 1;
    }
    run_api_test("0 0");
    get_run_info(time2, counter2);
    print_results(time1, time2, counter1, counter2);
}

int get_run_info(double& time, unsigned long int& counter) {
    APP_OUT ao;
    FILE* f=fopen(APP_TO_CORE_FILE, "r");
    if(f == NULL) {
        fprintf(stderr, "error: could not open %s\n", APP_TO_CORE_FILE);
        return 1;
    }
    parse_app_file(f, ao);
    time = ao.cpu_time_at_checkpoint;
    if(fclose(f)) {
        fprintf(stderr, "error: could not close %s\n", APP_TO_CORE_FILE);
        return 1;
    }
    f=fopen("counter", "r");
    fscanf(f, "%lu", &counter);
    if(fclose(f)) {
        fprintf(stderr, "error: could not close counter\n");
        return 1;
    }
    return 0;
}

void run_api_test(char* args) {
    char buf[256];
    sprintf(buf, "../api/api_app %s", args);
    system(buf);
}

int initialize_api() {
    APP_IN ai;
    FILE* f;
    ai.graphics.xsize=0;
    ai.graphics.ysize=0;
    ai.graphics.refresh_period=0;
    ai.checkpoint_period = 1.0;
    ai.poll_period = 0;
    ai.cpu_time = 0;
    f = fopen(CORE_TO_APP_FILE, "w");
    write_core_file(f, ai);
    if(fclose(f)) {
        fprintf(stderr, "error: could not close %s\n", CORE_TO_APP_FILE);
        return 1;
    }
    return 0;
}

int print_results(double time1, double time2, unsigned long int counter1,
    unsigned long int counter2
) {
    if(counter2 < counter1) printf("api test counter did not work properly\n");
    if(time1 == 0) printf("api test did not work first time\n");
    if(time2 == 0) printf("api test did not work second time\n");
    if(counter1 == counter2) printf("api test did not resume from restart\n");
    printf("total run time for api test was %f\n", time1+time2);
    return 0;
}

const char *BOINC_RCSID_b94ec48d11 = "$Id$";
