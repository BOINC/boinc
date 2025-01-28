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
#include "common_defs.h"
#include "miofile.h"
#include "parse.h"

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
    cl_device_id device_id;
    char name[256];                     // Device name
    char vendor[256];                   // Device vendor (NVIDIA, ATI, AMD, etc.)
    cl_uint vendor_id;                  // Vendor's unique ID for this device on this host
    cl_bool available;                  // Is this device available?
    cl_device_fp_config half_fp_config; // Half precision capabilities
    cl_device_fp_config single_fp_config;   // Single precision
    cl_device_fp_config double_fp_config;   // Double precision
    cl_bool endian_little;              // TRUE if little-endian
    cl_device_exec_capabilities execution_capabilities;
    char extensions[2048];              // List of device extensions
    cl_ulong global_mem_size;           // in bytes (OpenCL can report 4GB Max)
    cl_ulong local_mem_size;
    cl_uint max_clock_frequency;        // in MHz
    cl_uint max_compute_units;

    //
    // cl_nv_device_attribute_query
    //
    cl_uint nv_compute_capability_major;
    cl_uint nv_compute_capability_minor;

    //
    // cl_amd_device_attribute_query
    //
    cl_uint amd_simd_per_compute_unit;
    cl_uint amd_simd_width;
    cl_uint amd_simd_instruction_width;

    char opencl_platform_version[64];   // Version of OpenCL supported
                                        // the device's platform
    char opencl_device_version[128];     // OpenCL version supported by device;
                                        // example: "OpenCL 1.1 beta"
    int opencl_device_version_int;      // same, encoded as e.g. 101
    int get_device_version_int();       // call this to encode
    int opencl_driver_revision;         // OpenCL runtime revision is available
    int get_opencl_driver_revision();   // call this to encode
    char opencl_driver_version[512];    // For example: "CLH 1.0"
    int device_num;                     // temp used in scan process
    double peak_flops;                  // temp used in scan process
    COPROC_USAGE is_used;               // temp used in scan process
    double opencl_available_ram;        // temp used in scan process
    int opencl_device_index;            // zero-based device number within this OpenCL platform
    bool warn_bad_cuda;                 // If true, warn we can't use GPU due to CUDA version

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
