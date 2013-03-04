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

// Structures representing coprocessors (e.g. GPUs);
// used in both client and server.
//
// Notes:
//
// 1) The use of "CUDA" is misleading; it really means "NVIDIA GPU".
// 2) The design treats each resource type as a pool of identical devices;
//  for example, there is a single "CUDA long-term debt" per project,
//  and a scheduler request contains a request (#instances, instance-seconds)
//  for CUDA jobs.
//  In reality, the instances of a resource type can have different properties:
//  In the case of CUDA, "compute capability", driver version, RAM, speed, etc.
//  How to resolve this discrepancy?
//
//  Prior to 21 Apr 09 we identified the fastest instance
//  and pretended that the others were identical to it.
//  This approach has a serious flaw:
//  suppose that the fastest instance has characteristics
//  (version, RAM etc.) that satisfy the project's requirements,
//  but other instances to not.
//  Then BOINC executes jobs on GPUs that can't handle them,
//  the jobs fail, the host is punished, etc.
//
//  We could treat each GPU has a separate resource,
//  with its own set of debts, backoffs, etc.
//  However, this would imply tying jobs to instances,
//  which is undesirable from a scheduling viewpoint.
//  It would also be a big code change in both client and server.
//
//  Instead, (as of 21 Apr 09) our approach is to identify a
//  "most capable" instance, which in the case of CUDA is based on
//  a) compute capability
//  b) driver version
//  c) RAM size
//  d) est. FLOPS
//  (in decreasing priority).
//  We ignore and don't use any instances that are less capable
//  on any of these axes.
//
//  This design avoids running coprocessor apps on instances
//  that are incapable of handling them, and it involves no server changes.
//  Its drawback is that, on systems with multiple and differing GPUs,
//  it may not use some GPUs that actually could be used.

#ifndef _COPROC_
#define _COPROC_

#include <vector>
#include <string>

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "miofile.h"
#include "error_numbers.h"
#include "parse.h"
#include "cal_boinc.h"
#include "cl_boinc.h"

#define DEFER_ON_GPU_AVAIL_RAM  0

#define MAX_COPROC_INSTANCES 64
#define MAX_RSC 8
    // max # of processing resources types

#define MAX_OPENCL_PLATFORMS 16

#define PROC_TYPE_CPU        0
#define PROC_TYPE_NVIDIA_GPU 1
#define PROC_TYPE_AMD_GPU    2
#define PROC_TYPE_INTEL_GPU  3
#define NPROC_TYPES          4

extern const char* proc_type_name(int);
    // user-readable name
extern const char* proc_type_name_xml(int);
    // name used in XML and COPROC::type

// deprecated, but keep for simplicity
#define GPU_TYPE_NVIDIA proc_type_name_xml(PROC_TYPE_NVIDIA_GPU)
#define GPU_TYPE_ATI proc_type_name_xml(PROC_TYPE_AMD_GPU)
#define GPU_TYPE_INTEL proc_type_name_xml(PROC_TYPE_INTEL_GPU)

enum COPROC_USAGE {
    COPROC_IGNORED,
    COPROC_UNUSED,
    COPROC_USED
};
    

// represents a requirement for a coproc.
// This is a parsed version of the <coproc> elements in an <app_version>
// (used in client only)
//
struct COPROC_REQ {
    char type[256];     // must be unique
    double count;
    int parse(XML_PARSER&);
};

struct PCI_INFO {
    bool present;
    int bus_id;
    int device_id;
    int domain_id;

    void write(MIOFILE&);
    int parse(XML_PARSER&);
};

// there's some duplication between the values in 
// the OPENCL_DEVICE_PROP struct and the NVIDIA/ATI structs
//
struct OPENCL_DEVICE_PROP {
    cl_device_id device_id;
    char name[256];                     // Device name
    char vendor[256];                   // Device vendor (NVIDIA, ATI, AMD, etc.)
    cl_uint vendor_id;                  // OpenCL ID of device vendor
    cl_bool available;                  // Is this device available?
    cl_device_fp_config half_fp_config; // Half precision capabilities
    cl_device_fp_config single_fp_config;   // Single precision
    cl_device_fp_config double_fp_config;   // Double precision
    cl_bool endian_little;              // TRUE if little-endian
    cl_device_exec_capabilities execution_capabilities;
    char extensions[1024];              // List of device extensions
    cl_ulong global_mem_size;           // in bytes
    cl_ulong local_mem_size;
    cl_uint max_clock_frequency;        // in MHz
    cl_uint max_compute_units;
    char opencl_platform_version[64];   // Version of OpenCL supported
                                        // the device's platform
    char opencl_device_version[64];     // OpenCL version supported by device;
                                        // example: "OpenCL 1.1 beta"
    int opencl_device_version_int;      // same, encoded as e.g. 101
    int get_device_version_int();       // call this to encode
    char opencl_driver_version[32];     // For example: "CLH 1.0"
    int device_num;                     // temp used in scan process
    double peak_flops;                  // temp used in scan process
    COPROC_USAGE is_used;               // temp used in scan process
    double opencl_available_ram;        // temp used in scan process
    int opencl_device_index;            // temp used in scan process

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&);
#endif
    int parse(XML_PARSER&);
    void description(char* buf, const char* type);
};


// represents a set of identical coprocessors on a particular computer.
// Abstract class;
// objects will always be a derived class (COPROC_CUDA, COPROC_ATI)
// Used in both client and server.
//
struct COPROC {
    char type[256];     // must be unique
    int count;          // how many are present
    double peak_flops;
    double used;        // how many are in use (used by client)
    bool have_cuda;     // True if this GPU supports CUDA on this computer
    bool have_cal;      // True if this GPU supports CAL on this computer
    bool have_opencl;   // True if this GPU supports openCL on this computer
    double available_ram;
    bool specified_in_config;
        // If true, this coproc was listed in cc_config.xml
        // rather than being detected by the client.
    
    // the following are used in both client and server for work-fetch info
    //
    double req_secs;
        // how many instance-seconds of work requested
    double req_instances;
        // client is requesting enough jobs to use this many instances
    double estimated_delay;
        // resource will be saturated for this long

    // temps used in client (enforce_schedule())
    // to keep track of what fraction of each instance is in use
    // during instance assignment
    //
    double usage[MAX_COPROC_INSTANCES];
    double pending_usage[MAX_COPROC_INSTANCES];

    // the device number of each instance
    // These are not sequential if we omit instances (see above)
    //
    int device_nums[MAX_COPROC_INSTANCES];
    int device_num;     // temp used in scan process
    cl_device_id opencl_device_ids[MAX_COPROC_INSTANCES];
    int opencl_device_count;
    int opencl_device_indexes[MAX_COPROC_INSTANCES];
    PCI_INFO pci_info;
    PCI_INFO pci_infos[MAX_COPROC_INSTANCES];

    bool running_graphics_app[MAX_COPROC_INSTANCES];
        // is this GPU running a graphics app (NVIDIA only)
#if DEFER_ON_GPU_AVAIL_RAM
   double available_ram_temp[MAX_COPROC_INSTANCES];
        // used during job scheduling
#endif

    double last_print_time;
    
    OPENCL_DEVICE_PROP opencl_prop;

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&);
    void write_request(MIOFILE&);
#endif
    int parse(XML_PARSER&);

    inline void clear() {
        // can't just memcpy() - trashes vtable
        type[0] = 0;
        count = 0;
        peak_flops = 0;
        used = 0;
        have_cuda = false;
        have_cal = false;
        have_opencl = false;
        specified_in_config = false;
        available_ram = -1;
        req_secs = 0;
        req_instances = 0;
        opencl_device_count = 0;
        estimated_delay = 0;
        available_ram = 0;
        for (int i=0; i<MAX_COPROC_INSTANCES; i++) {
            device_nums[i] = 0;
            opencl_device_ids[i] = 0;
            running_graphics_app[i] = true;
        }
        memset(&opencl_prop, 0, sizeof(opencl_prop));
        memset(&pci_info, 0, sizeof(pci_info));
    }
    inline void clear_usage() {
        for (int i=0; i<count; i++) {
            usage[i] = 0;
            pending_usage[i] = 0;
        }
    }
    COPROC() {
        clear();
    }
    int device_num_index(int n) {
        for (int i=0; i<count; i++) {
            if (device_nums[i] == n) return i;
        }
        return -1;
    }
    void merge_opencl(
        std::vector<OPENCL_DEVICE_PROP> &opencls, 
        std::vector<int>& ignore_dev
    );
    void find_best_opencls(
        bool use_all,
        std::vector<OPENCL_DEVICE_PROP> &opencls, 
        std::vector<int>& ignore_dev
    );
};

// Based on cudaDeviceProp from /usr/local/cuda/include/driver_types.h
// doesn't have to match exactly since we get the attributes one at a time.
//
// This is used for 2 purposes:
// - it's exported via GUI RPC for GUIs or other tools
// - it's sent from client to scheduler, for use by app plan functions
// Properties not relevant to either of these can be omitted.
//
struct CUDA_DEVICE_PROP {
    char  name[256];
    double totalGlobalMem;
    double   sharedMemPerBlock;
    int   regsPerBlock;
    int   warpSize;
    double   memPitch;
    int   maxThreadsPerBlock;
    int   maxThreadsDim[3];
    int   maxGridSize[3]; 
    int   clockRate;
    double   totalConstMem; 
    int   major;     // compute capability
    int   minor;
    double   textureAlignment;
    int   deviceOverlap;
    int   multiProcessorCount;
};

typedef int CUdevice;

struct COPROC_NVIDIA : public COPROC {
    int cuda_version;  // CUDA runtime version
    int display_driver_version;
    CUDA_DEVICE_PROP prop;
    COPROC_USAGE is_used;               // temp used in scan process

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc);
#endif
    COPROC_NVIDIA(): COPROC() {
        strcpy(type, proc_type_name_xml(PROC_TYPE_NVIDIA_GPU));
    }
    void get(
        bool use_all,
        std::vector<std::string>&,
        std::vector<int>& ignore_devs
    );
    void description(char*);
    void clear();
    int parse(XML_PARSER&);
    void get_available_ram();
    void set_peak_flops();
    bool check_running_graphics_app();
    void fake(int driver_version, double ram, double avail_ram, int count);

};

// encode a 3-part version as // 10000000*major + 10000*minor + release
// Note: ATI release #s can exceed 1000
//
inline int ati_version_int(int major, int minor, int release) {
    return major*10000000 + minor*10000 + release;
}

struct COPROC_ATI : public COPROC {
    char name[256];
    char version[50];
    int version_num;
        // CAL version (not driver version) encoded as an int
    bool atirt_detected;
    bool amdrt_detected;
    CALdeviceattribs attribs; 
    CALdeviceinfo info;
    COPROC_USAGE is_used;               // temp used in scan process

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc);
#endif
    COPROC_ATI(): COPROC() {
        strcpy(type, proc_type_name_xml(PROC_TYPE_AMD_GPU));
    }
    void get(
        bool use_all,
        std::vector<std::string>&,
        std::vector<int>& ignore_devs
    );
    void description(char*);
    void clear();
    int parse(XML_PARSER&);
    void get_available_ram();
    void set_peak_flops();
    void fake(double ram, double avail_ram, int);
};

struct COPROC_INTEL : public COPROC {
    char name[256];
    char version[50];
    double global_mem_size;
    COPROC_USAGE is_used;               // temp used in scan process

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc);
#endif
    COPROC_INTEL(): COPROC() {
        strcpy(type, proc_type_name_xml(PROC_TYPE_INTEL_GPU));
    }
    void get(
        bool use_all,
        std::vector<std::string>&,
        std::vector<int>& ignore_devs
    );
    void description(char*);
    void clear();
    int parse(XML_PARSER&);
    void set_peak_flops();
    void fake(double ram, double avail_ram, int);
};

struct COPROCS {
    int n_rsc;
    COPROC coprocs[MAX_RSC];
    COPROC_NVIDIA nvidia;
    COPROC_ATI ati;
    COPROC_INTEL intel_gpu;

    void write_xml(MIOFILE& out, bool scheduler_rpc);
    void get(
        bool use_all, 
        std::vector<std::string> &descs,
        std::vector<std::string> &warnings,
        std::vector<int>& ignore_nvidia_dev,
        std::vector<int>& ignore_ati_dev,
        std::vector<int>& ignore_intel_gpu_dev
    );
    void get_opencl(
        bool use_all, 
        std::vector<std::string> &warnings,
        std::vector<int>& ignore_nvidia_dev, 
        std::vector<int>& ignore_ati_dev,
        std::vector<int>& ignore_intel_gpu_dev
    );
    cl_int get_opencl_info(
        OPENCL_DEVICE_PROP& prop, 
        cl_uint device_index, 
        std::vector<std::string>& warnings
    );
    int parse(XML_PARSER&);
#ifdef __APPLE__
    void opencl_get_ati_mem_size_from_opengl();
#endif
    void summary_string(char* buf, int len);

    // Copy a coproc set, possibly setting usage to zero.
    // used in round-robin simulator and CPU scheduler,
    // to avoid messing w/ master copy
    //
    void clone(COPROCS& c, bool copy_used) {
        n_rsc = c.n_rsc;
        for (int i=0; i<n_rsc; i++) {
            coprocs[i] = c.coprocs[i];
            if (!copy_used) {
                coprocs[i].used = 0;
            }
        }
    }
    void clear() {
        n_rsc = 0;
        for (int i=0; i<MAX_RSC; i++) {
            coprocs[i].clear();
        }
        nvidia.clear();
        ati.clear();
        intel_gpu.clear();
        COPROC c;
        strcpy(c.type, "CPU");
        add(c);
    }
    inline void clear_usage() {
        for (int i=0; i<n_rsc; i++) {
            coprocs[i].clear_usage();
        }
    }
    inline bool none() {
        return (n_rsc == 1);
    }
    inline int ndevs() {
        int n=0;
        for (int i=1; i<n_rsc; i++) {
            n += coprocs[i].count;
        }
        return n;
    }
    inline bool have_nvidia() {
        return (nvidia.count > 0);
    }
    inline bool have_ati() {
        return (ati.count > 0);
    }
    inline bool have_intel_gpu() {
        return (intel_gpu.count > 0);
    }
    int add(COPROC& c) {
        if (n_rsc >= MAX_RSC) return ERR_BUFFER_OVERFLOW;
        for (int i=1; i<n_rsc; i++) {
            if (!strcmp(c.type, coprocs[i].type)) {
                return ERR_DUP_NAME;
            }
        }
        coprocs[n_rsc++] = c;
        return 0;
    }
    COPROC* type_to_coproc(int t) {
        switch(t) {
        case PROC_TYPE_NVIDIA_GPU: return &nvidia;
        case PROC_TYPE_AMD_GPU: return &ati;
        case PROC_TYPE_INTEL_GPU: return &intel_gpu;
        }
        return NULL;
    }
    COPROCS() {
        n_rsc = 0;
        nvidia.count = 0;
        ati.count = 0;
        intel_gpu.count = 0;
        COPROC c;
        strcpy(c.type, "CPU");
        add(c);
    }
};

#endif
