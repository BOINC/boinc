// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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

extern bool already_sent_to_different_platform_quick(
    SCHEDULER_REQUEST& sreq, WORKUNIT&, APP&
);

extern bool already_sent_to_different_platform_careful(
    SCHEDULER_REQUEST& sreq, WORK_REQ& wreq, WORKUNIT& workunit, APP&
);

extern bool hr_unknown_platform(HOST&);

extern int hr_class(HOST&, APP&);

// return the HR type to use for this app;
// app-specific HR type overrides global HR type
//
inline int hr_type(APP& app) {
    if (app.homogeneous_redundancy) {
        return app.homogeneous_redundancy;
    }
    return config.homogeneous_redundancy;
}
