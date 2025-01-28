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

#ifndef BOINC_GPU_DETECT_H
#define BOINC_GPU_DETECT_H

#include <vector>
#include <string>

#include "coproc.h"

extern std::vector<COPROC_ATI> ati_gpus;
extern std::vector<COPROC_NVIDIA> nvidia_gpus;
extern std::vector<COPROC_INTEL> intel_gpus;
extern std::vector<COPROC_APPLE> apple_gpus;
extern std::vector<OPENCL_DEVICE_PROP> nvidia_opencls;
extern std::vector<OPENCL_DEVICE_PROP> ati_opencls;
extern std::vector<OPENCL_DEVICE_PROP> intel_gpu_opencls;
extern std::vector<OPENCL_DEVICE_PROP> apple_gpu_opencls;
extern std::vector<OPENCL_DEVICE_PROP> other_opencls;
extern std::vector<OPENCL_CPU_PROP> cpu_opencls;

extern void gpu_warning(std::vector<std::string> &warnings, const char* msg);

#endif
