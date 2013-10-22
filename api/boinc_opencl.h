// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// BOINC API for OpenCL apps

// Get the cl_device_id and cl_platform_id for the OpenCL GPU 
// assigned to your job.
//
// NOTE: Compile and link this function with your application;
// it is not included in the standard BOINC libraries.

#include "cl_boinc.h"

int boinc_get_opencl_ids(
    int argc, char** argv, int type,
    cl_device_id* device, cl_platform_id* platform
);

// doesn't work w/ pre-7 clients; use the above
//
int boinc_get_opencl_ids(cl_device_id* device, cl_platform_id* platform);
