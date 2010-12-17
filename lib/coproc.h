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
#include "cal_boinc.h"

#define MAX_COPROC_INSTANCES 64

// represents a requirement for a coproc.
// This is a parsed version of the <coproc> elements in an <app_version>
// (used in client only)
//
struct COPROC_REQ {
    char type[256];     // must be unique
    double count;
    int parse(MIOFILE&);
};

// represents a coproc on a particular computer.
// Abstract class;
// objects will always be a derived class (COPROC_CUDA, COPROC_ATI)
// Used in both client and server.
//
struct COPROC {
    char type[256];     // must be unique
    int count;          // how many are present
    double used;           // how many are in use (used by client)

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
    bool running_graphics_app[MAX_COPROC_INSTANCES];
        // is this GPU running a graphics app (NVIDIA only)
    double available_ram[MAX_COPROC_INSTANCES];
    bool available_ram_unknown[MAX_COPROC_INSTANCES];
        // couldn't get available RAM; don't start new apps on this instance
    double available_ram_fake[MAX_COPROC_INSTANCES];

    double last_print_time;

#ifndef _USING_FCGI_
    virtual void write_xml(MIOFILE&);
    void write_request(MIOFILE&);
#endif
    inline void clear() {
        // can't just memcpy() - trashes vtable
        type[0] = 0;
        count = 0;
        used = 0;
        req_secs = 0;
        req_instances = 0;
        estimated_delay = 0;
        for (int i=0; i<MAX_COPROC_INSTANCES; i++) {
            device_nums[i] = 0;
            running_graphics_app[i] = true;
            available_ram[i] = 0;
            available_ram_fake[i] = 0;
            available_ram_unknown[i] = true;
        }
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
    virtual ~COPROC(){}
    int parse(MIOFILE&);
    void print_available_ram();
};

// based on cudaDeviceProp from /usr/local/cuda/include/driver_types.h
// doesn't have to match exactly since we get the attributes one at a time.
//
struct CUDA_DEVICE_PROP {
  char   name[256];
  unsigned int totalGlobalMem;
    // not used on the server; dtotalGlobalMem is used instead
    // (since some boards have >= 4GB)
  int sharedMemPerBlock;
  int    regsPerBlock;
  int    warpSize;
  int memPitch;
  int    maxThreadsPerBlock;
  int    maxThreadsDim[3];
  int    maxGridSize[3]; 
  int    clockRate;
  int totalConstMem; 
  int    major;     // compute capability
  int    minor;
  int textureAlignment;
  int    deviceOverlap;
  int    multiProcessorCount;
  double dtotalGlobalMem;   // not defined in client
};

struct COPROC_CUDA : public COPROC {
    int cuda_version;  // CUDA runtime version
    int display_driver_version;
    CUDA_DEVICE_PROP prop;

#ifndef _USING_FCGI_
    virtual void write_xml(MIOFILE&, bool include_request);
#endif
    COPROC_CUDA(): COPROC("CUDA"){}
    virtual ~COPROC_CUDA(){}
    void get(
        bool use_all,
        std::vector<std::string>&, std::vector<std::string>&,
        std::vector<int>& ignore_devs
    );
	void description(char*);
    void clear();
    int parse(MIOFILE&);

    // Estimate of peak FLOPS.
    // FLOPS for a given app may be much less;
    // e.g. for SETI@home it's about 0.18 of the peak
    //
    inline double peak_flops() {
        // clock rate is scaled down by 1000;
        // each processor has 8 or 32 cores;
        // each core can do 2 ops per clock
        //
        int cores_per_proc = (prop.major>=2)?32:8;
        double x = (1000.*prop.clockRate) * prop.multiProcessorCount * cores_per_proc * 2.;
        return x?x:5e10;
    }
    void get_available_ram();

    bool check_running_graphics_app();
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
    virtual void write_xml(MIOFILE&, bool include_request);
#endif
    COPROC_ATI(): COPROC("ATI"){}
    virtual ~COPROC_ATI(){}
    void get(
        bool use_all,
        std::vector<std::string>&, std::vector<std::string>&,
        std::vector<int>& ignore_devs
    );
    void description(char*);
    void clear();
    int parse(MIOFILE&);
    inline double peak_flops() {
		double x = attribs.numberOfSIMD * attribs.wavefrontSize * 2.5 * attribs.engineClock * 1.e6;
        // clock is in MHz
        return x?x:5e10;
    }
    void get_available_ram();
    void fake(double, int);
};

struct COPROCS {
    COPROC_CUDA cuda;
    COPROC_ATI ati;

    COPROCS(){}
    ~COPROCS(){}    // don't delete coprocs; else crash in APP_INIT_DATA logic
    void write_xml(MIOFILE& out, bool include_request);
    void get(
        bool use_all, std::vector<std::string> &descs,
        std::vector<std::string> &warnings,
        std::vector<int>& ignore_cuda_dev,
        std::vector<int>& ignore_ati_dev
    );
    int parse(MIOFILE&);
    void summary_string(char*, int);

    // Copy a coproc set, possibly setting usage to zero.
    // used in round-robin simulator and CPU scheduler,
    // to avoid messing w/ master copy
    //
    void clone(COPROCS& c, bool copy_used) {
        cuda = c.cuda;
        ati = c.ati;
        if (!copy_used) {
            cuda.used = 0;
            ati.used = 0;
        }
    }
    inline void clear() {
        cuda.count = 0;
        ati.count = 0;
    }
    inline void clear_usage() {
        cuda.clear_usage();
        ati.clear_usage();
    }
    inline bool none() {
        return (cuda.count==0) && (ati.count==0);
    }
    inline int ndevs() {
        return cuda.count + ati.count;
    }
};

#endif
