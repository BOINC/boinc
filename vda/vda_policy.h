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

#ifndef _VDA_POLICY_
#define _VDA_POLICY_

// parameters of 1 level of coding
//

struct CODING {
    int n;
    int k;
    int m;  // n + k
    int n_upload;
};

// description of overall coding
// (possibly w/ multiple coding levels and replication)
//
struct POLICY {
    int replication;
    int coding_levels;
    CODING codings[10];
    double chunk_sizes[10];
    double chunk_size() {
        return chunk_sizes[coding_levels-1];
    }

    char description[256];  // derived from the above

    int parse(const char*);
};

#endif
