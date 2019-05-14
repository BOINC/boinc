// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// burn up to one CPU second

#include "config.h"
#include <cstdio>
#include <ctime>

int main() {
    int now = time(0), i;
    double x=0;

    while (1) {
        for (i=0; i<1000000; i++) x += 1;
        if (time(0) != now) break;
    }

    FILE* f = fopen("out", "w");
    fprintf(f, "done\n");
    fclose(f);
}
