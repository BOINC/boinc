// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "boinc_db.h"

#define HR_NTYPES 3

struct HR_INFO {
    double *rac_per_class[HR_NTYPES];
    void write_file(const char*);
    void read_file(const char*);
    void scan_db();
};

extern int hr_class(HOST&, int hr_type);
extern bool hr_unknown_platform_type(HOST&, int hr_type);
extern const char* hr_names[];
extern int hr_nclasses[];
