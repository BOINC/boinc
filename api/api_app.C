// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

// test program for MFILE class

#include <cstdlib>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "boinc_api.h"

int recover(char* file, unsigned long int* i);
int timer(int secs, int usecs);
int parse_args(int argc, char **argv, int& secs, int& usecs);

int main(int argc, char **argv) {
    MFILE mf, check;
    unsigned long int i = 0;
    int temp=0, secs, usecs;
    APP_IN ai;
    APP_OUT ao;
    boinc_init(ai);
    mf.open("foobar", "w");
    mf.printf("blah %d %f\n", 17, 34.5);
    mf.printf("foo\n");
    if(!recover("counter", &i)) {
	check.open("counter", "w");
	check.printf("%d", 0);
	check.flush();
	check.close();
    }
    if(parse_args(argc, argv, secs, usecs)) {
        fprintf(stderr, "error: could not parse arguments\n");
        return 1;
    }
    if(timer(secs, usecs)) {
        fprintf(stderr, "error: could not initialize timer\n");
        return 1;
    }
    for(; i<100000000; i++) {
	if(time_to_checkpoint()) {
            check.open("counter", "w");
            check.printf("%d", i);
            check.flush();
            check.close();
            ao.percent_done = ((double)i)/100000000.0;
	    checkpoint_completed(ao);
	}
	temp++;
    }
    mf.close();
    ao.percent_done = 1;
    app_completed(ao);
    return 0;
}

int recover(char* file, unsigned long int* i) {
    FILE* f = fopen(file, "r");
    if(f==NULL) {
        *i=0;
        return 0;
    }
    fscanf(f, "%lu", i);
    if(fclose(f)) {
        fprintf(stderr, "error: could not close file %s\n", file);
        exit(-1);
    }
    return *i;
}

int timer(int secs, int usecs) {
    int retval=0;
#ifdef HAVE_SYS_TIME_H
    itimerval value;
    value.it_value.tv_sec=secs;
    value.it_value.tv_usec=usecs;
    value.it_interval.tv_sec=0;
    value.it_interval.tv_usec=0;
    retval = setitimer(ITIMER_REAL, &value, NULL);
#endif
    return retval;
}

int parse_args(int argc, char **argv, int& secs, int& usecs) {
    if(argc != 3) {
        fprintf(stderr, "error: incorrect number of arguments %d\n", argc);
        return 1;
    }
    secs = atoi(argv[1]);
    usecs = atoi(argv[2]);
    return 0;
}

const char *BOINC_RCSID_bccd17d4ec = "$Id$";
