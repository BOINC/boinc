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

#define MAX_COPROC_INSTANCES 64

// represents a set of equivalent coprocessors
//
struct COPROC {
    char type[256];     // must be unique
    int count;          // how many are present
    int used;           // how many are in use (used by client)

    // the following are used in both client and server for work-fetch info
    //
    double req_secs;    // how many instance-seconds of work requested
    int req_instances;  // requesting enough jobs to use this many instances
    double estimated_delay; // resource will be saturated for this long

    // Used in client to keep track of which tasks are using which instances
    // The pointers point to ACTIVE_TASK
    //
    void* owner[MAX_COPROC_INSTANCES];

    // the device number of each instance
    // These are not sequential if we omit instances (see above)
    //
    int device_nums[MAX_COPROC_INSTANCES];
    int device_num;     // temp used in scan process

#ifndef _USING_FCGI_
    virtual void write_xml(MIOFILE&);
#endif
    inline void clear() {
        // can't just memcpy() - trashes vtable
        type[0] = 0;
        count = 0;
        used = 0;
        req_secs = 0;
        req_instances = 0;
        estimated_delay = 0;
        memset(owner, 0, sizeof(owner));
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
};

struct COPROCS {
    std::vector<COPROC*> coprocs;   // not deleted in destructor
        // so any structure that includes this needs to do it manually

    COPROCS(){}
    ~COPROCS(){}
    void delete_coprocs(){
        for (unsigned int i=0; i<coprocs.size(); i++) {
            delete coprocs[i];
        }
    }
#ifndef _USING_FCGI_
    void write_xml(MIOFILE& out) {
        for (unsigned int i=0; i<coprocs.size(); i++) {
            coprocs[i]->write_xml(out);
        }
    }
#endif
    std::vector<std::string> get();
    int parse(FILE*);
    void summary_string(char*, int);
    COPROC* lookup(const char*);
    bool sufficient_coprocs(COPROCS&, bool log_flag, const char* prefix);
    void reserve_coprocs(COPROCS&, bool log_flag, const char* prefix);
    void free_coprocs(COPROCS&, bool log_flag, const char* prefix);
    bool fully_used() {
        for (unsigned int i=0; i<coprocs.size(); i++) {
            COPROC* cp = coprocs[i];
            if (cp->used < cp->count) return false;
        }
        return true;
    }

    // Copy a coproc set, setting usage to zero.
    // used in round-robin simulator and CPU scheduler,
    // to avoid messing w/ master copy
    //
    void clone(COPROCS& c, bool copy_used) {
        for (unsigned int i=0; i<c.coprocs.size(); i++) {
            COPROC* cp = c.coprocs[i];
            COPROC* cp2 = new COPROC(cp->type);
            cp2->count = cp->count;
			if (copy_used) cp2->used = cp->used;
            coprocs.push_back(cp2);
        }
    }
};

// the following copied from /usr/local/cuda/include/driver_types.h
//
struct cudaDeviceProp {
  char   name[256];
  size_t totalGlobalMem;
    // not used on the server; dtotalGlobalMem is used instead
    // (since some boards have >= 4GB)
  size_t sharedMemPerBlock;
  int    regsPerBlock;
  int    warpSize;
  size_t memPitch;
  int    maxThreadsPerBlock;
  int    maxThreadsDim[3];
  int    maxGridSize[3]; 
  int    clockRate;
  size_t totalConstMem; 
  int    major;
  int    minor;
  size_t textureAlignment;
  int    deviceOverlap;
  int    multiProcessorCount;
  int    __cudaReserved[40];
  double dtotalGlobalMem;   // not defined in client
};

struct COPROC_CUDA : public COPROC {
    int drvVersion;  // display driver version, obtained from NVAPI
    cudaDeviceProp prop;

#ifndef _USING_FCGI_
    virtual void write_xml(MIOFILE&);
#endif
    COPROC_CUDA(): COPROC("CUDA"){}
    virtual ~COPROC_CUDA(){}
    static void get(COPROCS&, std::vector<std::string>&);
	void description(char*);
    void clear();
    int parse(FILE*);

    // rough estimate of FLOPS
    // The following is based on SETI@home CUDA,
    // which gets 50 GFLOPS on a Quadro FX 3700,
    // which has 14 MPs and a clock rate of 1.25 MHz
    //
    inline double flops_estimate() {
        double x = (prop.clockRate * prop.multiProcessorCount)*5e10/(14*1.25e6);
        return x?x:5e10;
    }
};


struct COPROC_CELL_SPE : public COPROC {
    static void get(COPROCS&, std::vector<std::string>&);
    COPROC_CELL_SPE() : COPROC("Cell SPE"){}
    virtual ~COPROC_CELL_SPE(){}
};

void fake_cuda(COPROCS&, int);

#endif
