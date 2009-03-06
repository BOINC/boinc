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

#ifndef _USING_FCGI_
    virtual void write_xml(MIOFILE&);
#endif
    COPROC(const char* t){
        memset(this, 0, sizeof(COPROC));
        strcpy(type, t);
    }
    COPROC() {
        memset(this, 0, sizeof(COPROC));
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

    // Copy a coproc set, possibly setting usage to zero.
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
  double dtotalGlobalMem;
};

struct COPROC_CUDA : public COPROC {
    int drvVersion;  // display driver version, obtained from NVAPI
    cudaDeviceProp prop;

#ifndef _USING_FCGI_
    virtual void write_xml(MIOFILE&);
#endif
    COPROC_CUDA(): COPROC("CUDA"){}
    virtual ~COPROC_CUDA(){}
    static std::string get(COPROCS&);
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
    static std::string get(COPROCS&);
    COPROC_CELL_SPE() : COPROC("Cell SPE"){}
    virtual ~COPROC_CELL_SPE(){}
};

void fake_cuda(COPROCS&, int);

#endif
