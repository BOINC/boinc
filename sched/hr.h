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

#ifndef BOINC_HR_H
#define BOINC_HR_H

#include "boinc_db.h"

#define HR_NTYPES 3
    // actually ntypes+1 (0 is reserved for "no HR")

extern int hr_class(HOST&, int hr_type);
extern bool hr_unknown_class(HOST&, int hr_type);
extern const char* hr_names[HR_NTYPES];
extern int hr_nclasses[HR_NTYPES];

#endif
