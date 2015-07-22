// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// test program for (some) remote job submission functions

#include "remote_submit.h"

const char* project_url = "http://isaac.ssl.berkeley.edu/test/";
const char* authenticator = "157f96a018b0b2f2b466e2ce3c7f54db";

void test_query_batches() {
    string error_msg;
    vector<BATCH_STATUS> batches;
    int retval = query_batches(
        project_url, authenticator, batches, error_msg
    );
    if (retval) {
        printf("Error: %d (%s)\n", retval, error_msg.c_str());
        return;
    }
    for (unsigned int i=0; i<batches.size(); i++) {
        BATCH_STATUS& bs = batches[i];
        bs.print();
    }
}

void test_query_batch() {
    string error_msg;
    vector<JOB_STATE> jobs;
    int batch_id = 207;
    const char* batch_name = "";
    int retval = query_batch(
        project_url, authenticator, batch_id, batch_name, jobs, error_msg
    );
    if (retval) {
        printf("Error: %d (%s)\n", retval, error_msg.c_str());
        return;
    }
    for (unsigned int i=0; i<jobs.size(); i++) {
        JOB_STATE& js = jobs[i];
        js.print();
    }
}

int main(int, char**) {
    //test_query_batches();
    test_query_batch();
}
