// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#include <stdio.h>
#include <boinc/boinc_api.h>

int main(int argc, char **argv) {
    int res = boinc_init();
    if (res) {
        fprintf(stderr, "Error initializing BOINC options: %d\n", res);
        return res;
    }

    printf("Running test...\n");
    boinc_fraction_done(0.0);
    for(int i = 1; i <= 10; ++i) {
        printf("%d/10\n", i);
        boinc_fraction_done(i / 10.0);
    }
    boinc_fraction_done(1.0);
    printf("Test completed.\n");
    boinc_finish(0);
    return 0;
}