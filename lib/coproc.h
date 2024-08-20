// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
//  for example, a scheduler request contains a request
// (#instances, instance-seconds) for CUDA jobs.
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
//  with its own backoffs, etc.
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
//
//  Modified (as of 23 July 14) to allow coprocessors (OpenCL GPUs and OpenCL
//  accelerators) from vendors other than original 3: NVIDIA, AMD and Intel.
//  For these original 3 GPU vendors, we still use the above approach, and the
//  COPROC::type field contains a standardized vendor name "NVIDIA", "ATI" or
//  "intel_gpu".  But for other, "new" vendors, we treat each device as a
//  separate resource, creating an entry for each instance in the
//  COPROCS::coprocs[] array and copying the device name COPROC::opencl_prop.name
//  into the COPROC::type field (instead of the vendor name.)

#ifndef BOINC_COPROC_H
#define BOINC_COPROC_H

#include <vector>
#include <string>

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "boinc_stdio.h"
#include "miofile.h"
#include "error_numbers.h"
#include "parse.h"
#include "cal_boinc.h"
#include "cl_boinc.h"
#include "opencl_boinc.h"
#include "common_defs.h"

#define MAX_COPROC_INSTANCES 64
#define MAX_RSC 8
    // max # of processing resources types
#define GPU_MAX_PEAK_FLOPS  1.e15
    // sanity-check bound for peak FLOPS
    // for now (Feb 2019) 1000 TeraFLOPS.
    // As of now, the fastest GPU is 20 TeraFLOPS (NVIDIA).
    // May need to increase this at some point
#define GPU_DEFAULT_PEAK_FLOPS  100.e9
    // value to use if sanity check fails
    // as of now (Feb 2019) 100 GigaFLOPS is a typical low-end GPU

// arguments to proc_type_name() and proc_type_name_xml().
//
#define PROC_TYPE_CPU        0
#define PROC_TYPE_NVIDIA_GPU 1
#define PROC_TYPE_AMD_GPU    2
#define PROC_TYPE_INTEL_GPU  3
#define PROC_TYPE_APPLE_GPU  4
#define NPROC_TYPES          5

extern const char* proc_type_name(int);
    // user-readable name
extern const char* proc_type_name_xml(int);
    // name used in XML and COPROC::type
extern int coproc_type_name_to_num(const char* name);

// deprecated, but keep for simplicity
#define GPU_TYPE_NVIDIA proc_type_name_xml(PROC_TYPE_NVIDIA_GPU)
#define GPU_TYPE_ATI proc_type_name_xml(PROC_TYPE_AMD_GPU)
#define GPU_TYPE_INTEL proc_type_name_xml(PROC_TYPE_INTEL_GPU)
#define GPU_TYPE_APPLE proc_type_name_xml(PROC_TYPE_APPLE_GPU)

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

    void clear() {
        present = false;
        bus_id = 0;
        device_id = 0;
        domain_id = 0;
    }
    PCI_INFO() {
        clear();
    }
    void write(MIOFILE&);
    int parse(XML_PARSER&);
};


// represents a set of identical coprocessors on a particular computer.
// Abstract class;
// objects will always be a derived class (COPROC_CUDA, COPROC_ATI)
// Used in both client and server.
//
struct COPROC {
    char type[256];     // must be unique
    int count;          // how many are present
    bool non_gpu;       // coproc is not a GPU
    double peak_flops;
    double used;        // how many are in use (used by client)
    bool have_cuda;     // this GPU supports CUDA
    bool have_cal;      // this GPU supports CAL
    bool have_opencl;   // this GPU supports openCL
    bool have_metal;
    double available_ram;
    bool specified_in_config;
        // If true, this coproc was listed in cc_config.xml
        // rather than being detected by the client.
    COPROC_USAGE is_used;               // temp used in scan process

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

    // The vendor-specific APIs (CUDA etc.) return instances in some order.
    // We call an instance's order its  'device_num'.
    // These are not sequential if we omit instances (see above)
    //
    int device_nums[MAX_COPROC_INSTANCES];
    int device_num;     // temp used in scan process
    bool instance_has_opencl[MAX_COPROC_INSTANCES];
    cl_device_id opencl_device_ids[MAX_COPROC_INSTANCES];
    int opencl_device_count;
    int opencl_device_indexes[MAX_COPROC_INSTANCES];
    PCI_INFO pci_info;
    PCI_INFO pci_infos[MAX_COPROC_INSTANCES];

    bool running_graphics_app[MAX_COPROC_INSTANCES];
        // is this GPU running a graphics app (NVIDIA only)

    double last_print_time;

    OPENCL_DEVICE_PROP opencl_prop;

    COPROC(DUMMY_TYPE){}
    inline void clear() {
        static const COPROC x(DUMMY);
        *this = x;
    }
    COPROC(){
        clear();
    }

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc=false);
    void write_request(MIOFILE&);
#endif
    int parse(XML_PARSER&);

    inline void clear_usage() {
        for (int i=0; i<count; i++) {
            usage[i] = 0;
            pending_usage[i] = 0;
        }
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

    // sanity check GPU peak FLOPS
    //
    inline bool bad_gpu_peak_flops(const char* source, std::string& msg) {
        if (peak_flops <= 0 || peak_flops > GPU_MAX_PEAK_FLOPS) {
            char buf[256];
            snprintf(buf, sizeof(buf), "%s reported bad GPU peak FLOPS %f; using %f",
                source, peak_flops, GPU_DEFAULT_PEAK_FLOPS
            );
            msg = buf;
            peak_flops = GPU_DEFAULT_PEAK_FLOPS;
            return true;
        }
        return false;
    }
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

    CUDA_DEVICE_PROP(DUMMY_TYPE){}
    void clear() {
        static const CUDA_DEVICE_PROP x(DUMMY);
        *this = x;
    }
    CUDA_DEVICE_PROP() {
        clear();
    }
};

typedef int CUdevice;

struct COPROC_NVIDIA : public COPROC {
    int cuda_version;  // CUDA runtime version
    int display_driver_version;
    CUDA_DEVICE_PROP prop;

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc);
#endif
    COPROC_NVIDIA(): COPROC() {clear();}
    COPROC_NVIDIA(int): COPROC() {}
    void get(std::vector<std::string>& warnings);
    void correlate(
        bool use_all,
        std::vector<int>& ignore_devs
    );
    void description(char* buf, int buflen);
    void clear();
    int parse(XML_PARSER&);
    void set_peak_flops();
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

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc);
#endif
    COPROC_ATI(int): COPROC() {}
    COPROC_ATI(): COPROC() {clear();}
    void get(std::vector<std::string>& warnings);
    void correlate(
        bool use_all,
        std::vector<int>& ignore_devs
    );
    void description(char* buf, int buflen);
    void clear();
    int parse(XML_PARSER&);
    void set_peak_flops();
    void fake(double ram, double avail_ram, int);
};

struct COPROC_INTEL : public COPROC {
    char name[256];
    char version[50];

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc);
#endif
    COPROC_INTEL(int): COPROC() {}
    COPROC_INTEL(): COPROC() {clear();}
    void get(std::vector<std::string>& ) {};
    void correlate(bool , std::vector<int>& ) {};
    void clear();
    int parse(XML_PARSER&);
    void set_peak_flops();
    void fake(double ram, double avail_ram, int);
};

struct COPROC_APPLE : public COPROC {
    char model[256];    // from metal, else OpenCL

    // Metal info:
    int ncores;
    int metal_support;

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool scheduler_rpc);
#endif
    COPROC_APPLE(int): COPROC() {}
    COPROC_APPLE(): COPROC() {clear();}
    void get(std::vector<std::string>&);
    void correlate(bool , std::vector<int>&);
    void clear();
    int parse(XML_PARSER&);
    void set_peak_flops();
    void fake(double ram, double avail_ram, int);
};

typedef std::vector<int> IGNORE_GPU_INSTANCE[NPROC_TYPES];

struct COPROCS {
    int n_rsc;
    COPROC coprocs[MAX_RSC];
        // array of processor types on this host.
        // element 0 always represents the CPU.
        // The remaining elements, if any, are GPUs or other coprocessors

    // The following contain vendor-specific info about GPUs.
    // (These GPUs are also represented by elements in the coprocs array)
    //
    COPROC_NVIDIA nvidia;
    COPROC_ATI ati;
    COPROC_INTEL intel_gpu;
    COPROC_APPLE apple_gpu;

    void write_xml(MIOFILE& out, bool scheduler_rpc);
    void get(
        bool use_all,
        std::vector<std::string> &descs,
        std::vector<std::string> &warnings,
        IGNORE_GPU_INSTANCE &ignore_gpu_instance
    );
    void detect_gpus(std::vector<std::string> &warnings);
    int launch_child_process_to_detect_gpus();
    void correlate_gpus(
        bool use_all,
        std::vector<std::string> &descs,
        IGNORE_GPU_INSTANCE &ignore_gpu_instance
    );
    void get_opencl(
        std::vector<std::string> &warnings
    );
    void correlate_opencl(
        bool use_all,
        IGNORE_GPU_INSTANCE& ignore_gpu_instance
    );
    cl_int get_opencl_info(
        OPENCL_DEVICE_PROP& prop,
        cl_uint device_index,
        std::vector<std::string>& warnings
    );
    int parse(XML_PARSER&);
    void set_path_to_client(char *path);
    int write_coproc_info_file(std::vector<std::string> &warnings);
    int read_coproc_info_file(std::vector<std::string> &warnings);
    int add_other_coproc_types();

#ifdef __APPLE__
    void opencl_get_ati_mem_size_from_opengl(std::vector<std::string> &warnings);
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
        c.clear_usage();
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
    inline bool have_apple_gpu() {
        return (apple_gpu.count > 0);
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
    void bound_counts();
        // make sure instance counts are within legal range

    COPROC* lookup_type(const char* t) {
        for (int i=1; i<n_rsc; i++) {
            if (!strcmp(t, coprocs[i].type)) {
                return &coprocs[i];
            }
        }
        return NULL;
    }
    COPROC* proc_type_to_coproc(int t) {
        switch(t) {
        case PROC_TYPE_NVIDIA_GPU: return &nvidia;
        case PROC_TYPE_AMD_GPU: return &ati;
        case PROC_TYPE_INTEL_GPU: return &intel_gpu;
        case PROC_TYPE_APPLE_GPU: return &apple_gpu;
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
        c.clear_usage();
        add(c);
    }
};

extern void fake_opencl_gpu(char*);

#endif
