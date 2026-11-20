// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

#ifndef BOINC_OPENCL_BOINC_H
#define BOINC_OPENCL_BOINC_H

#include "cl_boinc.h"

struct MIOFILE;
struct XML_PARSER;

#define MAX_OPENCL_PLATFORMS 16
#define MAX_OPENCL_CPU_PLATFORMS 4

enum COPROC_USAGE {
    COPROC_IGNORED,
    COPROC_UNUSED,
    COPROC_USED
};

// there's some duplication between the values in
// the OPENCL_DEVICE_PROP struct and the NVIDIA/ATI structs
//
struct OPENCL_DEVICE_PROP {
    cl_device_id device_id = nullptr;
    char name[256]; // model name
        // e.g. Intel(R) UHD Graphics 630
    char vendor[256]; // vendor
        // e.g. Intel(R) Corporation
    cl_uint vendor_id = 0; // Vendor's unique ID for this device on this host
    cl_bool available = false; // Is this device available?
    cl_device_fp_config half_fp_config = 0; // Half precision capabilities
    cl_device_fp_config single_fp_config = 0; // Single precision
    cl_device_fp_config double_fp_config = 0; // Double precision
    cl_bool endian_little = false; // TRUE if little-endian
    cl_device_exec_capabilities execution_capabilities = 0;
    char extensions[2048]; // List of device extensions
    cl_ulong global_mem_size = 0; // in bytes (OpenCL can report 4GB Max)
    cl_ulong local_mem_size = 0;
    cl_uint max_clock_frequency = 0; // in MHz
    cl_uint max_compute_units = 0;

    //
    // cl_nv_device_attribute_query
    //
    cl_uint nv_compute_capability_major = 0;
    cl_uint nv_compute_capability_minor = 0;

    //
    // cl_amd_device_attribute_query
    //
    cl_uint amd_simd_per_compute_unit = 0;
    cl_uint amd_simd_width = 0;
    cl_uint amd_simd_instruction_width = 0;

    char opencl_platform_version[64]; // Version of OpenCL supported
                                      // the device's platform
    char opencl_device_version[128]; // OpenCL version supported by device;
                                     // example: "OpenCL 1.1 beta"
    int opencl_device_version_int = 0; // same, encoded as e.g. 101
    int get_device_version_int(); // call this to encode
    int opencl_driver_revision = 0; // OpenCL runtime revision is available
    int get_opencl_driver_revision(); // call this to encode
    char opencl_driver_version[512]; // For example: "CLH 1.0"
    int device_num = 0; // temp used in scan process
    double peak_flops = .0; // temp used in scan process
    COPROC_USAGE is_used = COPROC_IGNORED; // temp used in scan process
    double opencl_available_ram = .0; // temp used in scan process
    int opencl_device_index = 0; // zero-based device number
                                 // within this OpenCL platform
    bool warn_bad_cuda = false; // If true, warn we can't use GPU
                                // due to CUDA version

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, const char* tag, bool temp_file=false);
#endif
    int parse(XML_PARSER&, const char* end_tag);
    void description(char* buf, int buflen, const char* type);
    OPENCL_DEVICE_PROP(){}
    void clear() {
        static const OPENCL_DEVICE_PROP x;
        *this = x;
    }
};

// NOTE: OpenCL has only 32 bits for global_mem_size, so
// it can report a max of only 4GB.
// Get the CPU RAM size from gstate.hostinfo.m_nbytes.
//
struct OPENCL_CPU_PROP {
    char platform_vendor[256];
    OPENCL_DEVICE_PROP opencl_prop;

    OPENCL_CPU_PROP() {
        clear();
    }
    void clear();
    void write_xml(MIOFILE&);
    int parse(XML_PARSER&);
    void description(char* buf, int buflen);
};

#endif
