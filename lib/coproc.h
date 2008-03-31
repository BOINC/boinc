// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _COPROC_
#define _COPROC_

#include <vector>

struct COPROC {
    char name[256];     // must be unique
    int count;          // how many are present
    int used;           // how many are in use (used by client)

    virtual void write_xml(FILE*){}
    virtual ~COPROC(){}
};

struct COPROCS {
    std::vector<COPROC*> coprocs;

    COPROCS(){}
    ~COPROCS() {
        for (unsigned int i=0; i<coprocs.size(); i++) {
            delete coprocs[i];
        }
    }
    void get();
    int parse(FILE*);
};

// the following copied from /usr/local/cuda/include/driver_types.h
//
struct cudaDeviceProp {
  char   name[256];
  size_t totalGlobalMem;
  size_t sharedMemPerBlock;
  int    regsPerBlock;
  int    warpSize;
  size_t memPitch;
  int    maxThreadsPerBlock;
  int    maxThreadsDim[3];
  int    maxGridSize[3];
  size_t totalConstMem;
  int    major;
  int    minor;
  int    clockRate;
  size_t textureAlignment;
};

struct COPROC_CUDA : public COPROC {
    cudaDeviceProp prop;

    virtual void write_xml(FILE*);
    virtual ~COPROC_CUDA(){}
    static void get(COPROCS&);
    void clear();
    int parse(FILE*);
};


struct COPROC_CELL_SPE : public COPROC {
    static void get(COPROCS&);
    virtual ~COPROC_CELL_SPE(){}
};

void fake_cuda(COPROCS&);

#endif
