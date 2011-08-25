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
#include <cstring>

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "miofile.h"
#include "parse.h"
#include "cal_boinc.h"
#include "cl_boinc.h"

#define MAX_COPROC_INSTANCES 64
#define MAX_RSC 8
    // max # of processing resources types

#define MAX_OPENCL_PLATFORMS 16

// represents a requirement for a coproc.
// This is a parsed version of the <coproc> elements in an <app_version>
// (used in client only)
//
struct COPROC_REQ {
    char type[256];     // must be unique
    double count;
    int parse(XML_PARSER&);
};

// For now, there will be some duplication between the values present in 
// the OPENCL_DEVICE_PROP struct and the NVIDA and / or ATI structs
struct OPENCL_DEVICE_PROP {
    cl_device_id device_id;
    char name[256];                     // Device name
    char vendor[256];                   // Device vendor (NVIDIA, ATI, AMD, etc.)
    cl_uint vendor_id;                  // OpenCL ID of device vendor
    cl_bool available;                  // Is this device available?
    cl_device_fp_config hp_fp_config;   // Half precision floating point capabilities
    cl_device_fp_config sp_fp_config;   // Single precision floating point capabilities
    cl_device_fp_config dp_fp_config;   // Double precision floating point capabilities
    cl_bool little_endian;              // TRUE if little-endian
    cl_device_exec_capabilities exec_capab; // Execution capabilities
    char extensions[1024];              // List of device extensions
    cl_ulong global_RAM;                // Size of global memory
    cl_ulong local_RAM;                 // Size of local memory
    cl_uint max_clock_freq;             // Max configured clock frequencin in MHz
    cl_uint max_cores;                  // Max number of parallel computer cores
    char openCL_platform_version[64];   // Version of OpenCL platform for this device
    char openCL_device_version[64];     // OpenCL version supported by device; example: "OpenCL 1.1 beta"
    char openCL_driver_version[32];     // For example: "CLH 1.0"
    int device_num;                     // temp used in scan process
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
    bool running_graphics_app[MAX_COPROC_INSTANCES];
        // is this GPU running a graphics app (NVIDIA only)
    double available_ram[MAX_COPROC_INSTANCES];
    bool available_ram_unknown[MAX_COPROC_INSTANCES];
        // couldn't get available RAM; don't start new apps on this instance
    double available_ram_fake[MAX_COPROC_INSTANCES];

    double last_print_time;
    
    OPENCL_DEVICE_PROP opencl_prop;

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&);
    void write_request(MIOFILE&);
    int parse(XML_PARSER&);
    void opencl_write_xml(MIOFILE&);
#endif
    int parse_opencl(XML_PARSER&);

    inline void clear() {
        // can't just memcpy() - trashes vtable
        type[0] = 0;
        count = 0;
        peak_flops = 0;
        used = 0;
        have_cuda = false;
        have_cal = false;
        have_opencl = false;
        req_secs = 0;
        req_instances = 0;
        opencl_device_count = 0;
        estimated_delay = 0;
        for (int i=0; i<MAX_COPROC_INSTANCES; i++) {
            device_nums[i] = 0;
            opencl_device_ids[i] = 0;
            running_graphics_app[i] = true;
            available_ram[i] = 0;
            available_ram_fake[i] = 0;
            available_ram_unknown[i] = true;
        }
        memset(&opencl_prop, 0, sizeof(opencl_prop));
    }
    inline void clear_usage() {
        for (int i=0; i<count; i++) {
            usage[i] = 0;
            pending_usage[i] = 0;
        }
    }
    COPROC(const char* t){
        clear();
        strcpy(type, t);
    }
    COPROC() {
        clear();
    }
    void print_available_ram();
};

// based on cudaDeviceProp from /usr/local/cuda/include/driver_types.h
// doesn't have to match exactly since we get the attributes one at a time.
//
struct CUDA_DEVICE_PROP {
  char  name[256];
  int   deviceHandle;
  unsigned int totalGlobalMem;
    // not used on the server; dtotalGlobalMem is used instead
    // (since some boards have >= 4GB)
  int   sharedMemPerBlock;
  int   regsPerBlock;
  int   warpSize;
  int   memPitch;
  int   maxThreadsPerBlock;
  int   maxThreadsDim[3];
  int   maxGridSize[3]; 
  int   clockRate;
  int   totalConstMem; 
  int   major;     // compute capability
  int   minor;
  int   textureAlignment;
  int   deviceOverlap;
  int   multiProcessorCount;
  double dtotalGlobalMem;   // not defined in client
};

struct COPROC_NVIDIA : public COPROC {
    int cuda_version;  // CUDA runtime version
    int display_driver_version;
    CUDA_DEVICE_PROP prop;

#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool include_request);
#endif
    COPROC_NVIDIA(): COPROC("NVIDIA"){}
    void get(
        bool use_all,
        std::vector<std::string>&, std::vector<std::string>&,
        std::vector<int>& ignore_devs
    );
	void description(char*);
    void clear();
    int parse(XML_PARSER&);
    void get_available_ram();
	void set_peak_flops() {
        int flops_per_clock=0, cores_per_proc=0;
        switch (prop.major) {
        case 1:
            flops_per_clock = 3;
            cores_per_proc = 8;
            break;
        case 2:
            flops_per_clock = 2;
            switch (prop.minor) {
            case 0:
                cores_per_proc = 32;
                break;
            default:
                cores_per_proc = 48;
                break;
            }
        }
        // clock rate is scaled down by 1000
        //
        double x = (1000.*prop.clockRate) * prop.multiProcessorCount * cores_per_proc * flops_per_clock;
        peak_flops =  (x>0)?x:5e10;
	}

    bool check_running_graphics_app();
    bool matches(OPENCL_DEVICE_PROP& OpenCLprop);
    void fake(int driver_version, double ram, int count);

};

struct COPROC_ATI : public COPROC {
    char name[256];
    char version[50];
    int version_num;
        // based on CAL version (not driver version)
        // encoded as 1000000*major + 1000*minor + release
    bool atirt_detected;
    bool amdrt_detected;
    CALdeviceattribs attribs; 
    CALdeviceinfo info;
#ifndef _USING_FCGI_
    void write_xml(MIOFILE&, bool include_request);
#endif
    COPROC_ATI(): COPROC("ATI"){}
    void get(
        bool use_all,
        std::vector<std::string>&, std::vector<std::string>&,
        std::vector<int>& ignore_devs
    );
    void description(char*);
    void clear();
    int parse(XML_PARSER&);
    void get_available_ram();
    bool matches(OPENCL_DEVICE_PROP& OpenCLprop);
	void set_peak_flops() {
        double x = attribs.numberOfSIMD * attribs.wavefrontSize * 2.5 * attribs.engineClock * 1.e6;
        // clock is in MHz
        peak_flops = (x>0)?x:5e10;
	}
    void fake(double, int);
};

struct COPROCS {
    int n_rsc;
    COPROC coprocs[MAX_RSC];
    COPROC_NVIDIA nvidia;
    COPROC_ATI ati;

    void write_xml(MIOFILE& out, bool include_request);
    void get(
        bool use_all, std::vector<std::string> &descs,
        std::vector<std::string> &warnings,
        std::vector<int>& ignore_nvidia_dev,
        std::vector<int>& ignore_ati_dev
    );
    void get_opencl(bool use_all, std::vector<std::string> &warnings,
        std::vector<int>& ignore_nvidia_dev, 
        std::vector<int>& ignore_ati_dev
    );
    cl_int get_opencl_info(
        OPENCL_DEVICE_PROP& prop, 
        cl_uint device_index, 
        std::vector<std::string> &warnings
    );
    int parse(XML_PARSER&);
    void summary_string(char*, int);

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
    int add(COPROC& c) {
        coprocs[n_rsc++] = c;
        return 0;
    }
    COPROCS() {
        n_rsc = 0;
        nvidia.count = 0;
        ati.count = 0;
        COPROC c;
        strcpy(c.type, "CPU");
        add(c);
    }
};

#endif
