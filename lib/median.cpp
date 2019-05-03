// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

#include "median.h"
#include <cstdio>
#include <cmath>
#include "util.h"
#if 0
int main() {
    AVERAGE_VAR avg;
    MEDIAN_VAR med;
    avg.clear();
    med.clear();
    srandom(time(0));

    for (int i=0; i<100000; i++) {
        double x=-log(drand());
        avg.update_var(x,20,0.01,3);
        med.update_var(x,20,0.001,3);
        printf("%d %d %d %f %f %f %f\n", i, (int)avg.n, (int)med.n, avg.get_avg(), med.get_med(), sqrt(avg.get_var()), sqrt(med.get_var()));

    }
}
#endif
