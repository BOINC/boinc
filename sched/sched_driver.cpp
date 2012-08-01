// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


// This program generates a stream of scheduler requests;
// it acts as a "driver" for the scheduler when used as:
// sched_driver | cgi --batch --mark_jobs_done
//
// This was written to test the homogeneous redundancy features
// of the feeder and scheduler,
// but it could be used for a variety of other purposes.
//
// Usage: sched_driver --nrequests N --reqs_per_second X
//
// Each request asks for a uniformly-distributed random amount of work.
// The OS and CPU info is taken from the successive lines of a file of the form
// | os_name | p_vendor | p_model |
// Generate this file with a SQL query, trimming off the start and end.

// Notes:
// 1) Use sample_trivial_validator and sample_dummy_assimilator
// 2) Edit the following to something in your DB

#define AUTHENTICATOR    "49bcae97f1788385b0f41123acdf5694"
    // authenticator of a user record
#define HOSTID "7"
    // ID of a host belonging to that user

#include <cstdio>
#include <vector>
#include <cmath>
#include <cstring>

#include "util.h"
#include "svn_version.h"

using std::vector;

struct HOST_DESC{
    char os_name[128];
    char p_vendor[128];
    char p_model[128];
};

vector<HOST_DESC> host_descs;
double min_time = 1;
double max_time = 1;

void read_hosts() {
    char buf[256], buf2[256];
    host_descs.clear();
    FILE* f = fopen("host_descs.txt", "r");
    if (!f) {
        fprintf(stderr, "no input file\n");
        exit(1);
    }
    while (fgets(buf, sizeof(buf), f)) {
        HOST_DESC hd;
        strcpy(buf2, buf);
        char* p1 = strtok(buf2, "\t\n");
        strcpy(hd.os_name, p1);
        char* p2 = strtok(0, "\t\n");
        strcpy(hd.p_vendor, p2);
        char* p3 = strtok(0, "\t\n");
        if (!p3) {
            fprintf(stderr, "bad line: %s\n", buf);
            exit(1);
        }
        strcpy(hd.p_model, p3);
        host_descs.push_back(hd);
    }
    fclose(f);
}

inline double req_time() {
    if (max_time == min_time) return min_time;
    return min_time  + drand()*(max_time-min_time);
}

inline double exponential(double mean) {
        return -mean*log(1-drand()); 
}

void make_request(int i) {
    HOST_DESC& hd = host_descs[i%host_descs.size()];
    printf(
        "<scheduler_request>\n"
        "   <authenticator>%s</authenticator>\n"
        "   <hostid>%s</hostid>\n"
        "   <work_req_seconds>%f</work_req_seconds>\n"
        "   <platform_name>windows_intelx86</platform_name>\n"
        "   <host_info>\n"
        "      <os_name>%s</os_name>\n"
        "      <p_vendor>%s</p_vendor>\n"
        "      <p_model>%s</p_model>\n"
        "      <p_fops>1e9</p_fops>\n"
        "      <m_nbytes>1e9</m_nbytes>\n"
        "      <d_total>1e11</d_total>\n"
        "      <d_free>1e11</d_free>\n"
        "   </host_info>\n"
        "</scheduler_request>\n",
        AUTHENTICATOR,
        HOSTID,
        req_time(),
        hd.os_name,
        hd.p_vendor,
        hd.p_model
    );
}

void usage(char *name) {
    fprintf(stderr,
        "This program generates a stream of scheduler requests;\n"
        "it acts as a \"driver\" for the scheduler when used as:\n"
        "%s | cgi --batch --mark_jobs_done\n\n"
        "This was written to test the homogeneous redundancy features\n"
        "of the feeder and scheduler, and to measure server performance,\n"
        "but it could be used for other purposes.\n"
        "\n"
        "Each request asks for a uniformly-distributed random amount of work.\n"
        "The OS and CPU info is taken from the successive lines of a file of the form\n"
        "| os_name | p_vendor | p_model |\n"
        "You can generate this file with a SQL query, trimming off the start and end.\n"
        "\n"
        "Notes:\n"
        "1) Use sample_trivial_validator and sample_dummy_assimilator\n"
        "\n"
        "Usage: %s [OPTION]...\n"
        "\n"
        "Options: \n"
        "  --nrequests N                  Sets the total numberer of requests to N\n"
        "  --reqs_per_second X            Sets the number of requests per second to X\n"
        "  [ -h | --help ]                Show this help text.\n"
        "  [ -v | --version ]             Show version information\n",
        name, name
    );
}

int main(int argc, char** argv) {
    int i, nrequests = 1;
    double reqs_per_second = 1;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--nrequests")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            nrequests = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "--reqs_per_second")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            reqs_per_second = atof(argv[i]);
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(0);
        }
        else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            fprintf(stderr, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }
    read_hosts();
    double t1, t2, x;
    for (i=0; i<nrequests; i++) {
        t1 = dtime();
        make_request(i);
        t2 = dtime();
        x = exponential(1./reqs_per_second);
        if (t2 - t1 < x) {
            boinc_sleep(x - (t2-t1));
        }
    }
}
