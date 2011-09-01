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

// The BOINC client calls the project application with the arguments:
//   --gpu_type TYPE --device N
// where TYPE is ATI or NVIDIA, and N is the GPU number of that type
// For example, for ATI GPU number 0, the arguments will be:
//   --gpu_type ATI --device 0
//
// To get the cl_device_id and cl_platform_id for the OpenCL GPU 
// asigned to your application call this function:
//
// NOTE: You should compile and link this function as part of your 
// application; it is not included in the standard BOINC libraries.
//

int boinc_get_opencl_ids(int argc, char** argv, cl_device_id*, cl_platform_id*);
