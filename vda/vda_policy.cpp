// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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
#include <string.h>

#include "vda_policy.h"

int POLICY::parse(const char* filename) {
    int n;
    char buf[256];

    strcpy(description, "");

    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "No policy file %s\n", filename);
        return -1;
    }
    n = fscanf(f, "%d", &replication);
    if (n != 1) {
        fprintf(stderr, "parse error in %s\n", filename);
        fclose(f);
        return -1;
    }
    n = fscanf(f, "%d", &coding_levels);
    if (n != 1) {
        fprintf(stderr, "parse error in %s\n", filename);
        fclose(f);
        return -1;
    }
    for (int i=0; i<coding_levels; i++) {
        CODING& c = codings[i];
        n = fscanf(f, "%d %d %d", &c.n, &c.k, &c.n_upload);
        if (n != 3) {
            fprintf(stderr, "parse error in %s\n", filename);
            fclose(f);
            return -1;
        }
        c.m = c.n + c.k;

        sprintf(buf, "(%d %d %d) ", c.n, c.k, c.n_upload);
        strcat(description, buf);
    }
    fclose(f);
    sprintf(buf, "X%d", replication);
    strcat(description, buf);
    return 0;
}
