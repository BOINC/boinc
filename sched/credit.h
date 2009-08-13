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

#include "boinc_db.h"

// Historical note: named after Jeff Cobb
//
#define COBBLESTONE_FACTOR 100.0

extern void compute_credit_rating(HOST&);
extern double credit_multiplier(int, time_t);
extern double fpops_to_credit(double fpops, double intops);
    // credit that should be granted for a given number of
    // floating-point and integer ops
extern int update_credit_per_cpu_sec(
    double credit, double cpu_time, double& credit_per_cpu_sec
);
extern int grant_credit(
    DB_HOST& host, double start_time, double cpu_time, double credit
);
