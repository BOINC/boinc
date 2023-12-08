// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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

#ifndef BOINC_VBOXCHECKPOINT_H
#define BOINC_VBOXCHECKPOINT_H


#define CHECKPOINT_FILENAME "vbox_checkpoint.xml"
#define WEBAPI_FILENAME "vbox_webapi.xml"
#define REMOTEDESKTOP_FILENAME "vbox_remote_desktop.xml"

struct VBOX_CHECKPOINT {
    VBOX_CHECKPOINT();
    ~VBOX_CHECKPOINT();

    void clear();
    int parse();
    int write();
    int update(double elapsed_time, double cpu_time);

    double elapsed_time;
    double cpu_time;
    int webapi_port;
    int remote_desktop_port;
};

#endif
