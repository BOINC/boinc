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

// The main-program part of the system used by SETI@home
// to draw 3D graphs of data.
// This writes into a shared-memory structure
// that's read by the graphics app

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif


#ifndef _WIN32
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <assert.h>
#include <algorithm>
#endif

#include "reduce.h"

// Prepare to receive a source array.
// (sx, sy) are dimensions of source array
//
void REDUCED_ARRAY_GEN::init_data(int sx, int sy) {
    sdimx = sx;
    sdimy = sy;
    rdimx = sx;
    rdimy = sy;
    if (rdimx > 256) rdimx = 256;
    if (rdimy > 128) rdimy = 128;
    while (rdimx*rdimy > MAX_DATA) {
        if (rdimx>1) rdimx /= 2;
        if (rdimy>1) rdimy /= 2;
    }
    nvalid_rows = 0;
    scury = 0;
    last_ry = 0;
    last_ry_count = 0;
    rdata_max = 0;
    rdata_min = (float)1e20;
}

bool REDUCED_ARRAY_GEN::full() {
    return nvalid_rows==rdimy;
}

#if 0
void REDUCED_ARRAY_GEN::reset() {
    nvalid_rows = 0;
    ndrawn_rows = 0;
    scury = 0;
    last_ry = 0;
    last_ry_count = 0;
}
#endif


// reduce a single row.  This is called only if sdimx > rdimx;
//
void REDUCED_ARRAY_GEN::reduce_source_row(float* in, float* out) {
    int i, ri;

    memset(out, 0, rdimx*sizeof(float));
    memset(itemp, 0, rdimx*sizeof(int));
    for (i=0; i<sdimx; i++) {
        ri = (i*rdimx)/sdimx;
        switch (reduce_method) {
        case REDUCE_METHOD_AVG:
            out[ri] += in[i];
            itemp[ri]++;
            break;
        case REDUCE_METHOD_SUM:
            out[ri] += in[i];
            break;
        case REDUCE_METHOD_MIN:
            out[ri] = std::min(out[ri], in[i]);
            break;
        case REDUCE_METHOD_MAX:
            out[ri] = std::max(out[ri], in[i]);
            break;
        }
    }
    if (reduce_method==REDUCE_METHOD_AVG) {
        for (i=0; i<rdimx; i++) {
            if (itemp[i] > 1) out[i] /= itemp[i];
        }
    }
}

void REDUCED_ARRAY_GEN::update_max(int row) {
    int i;
    float* p = rrow(row);

    for (i=0; i<rdimx; i++) {
        if (p[i] > rdata_max) rdata_max = p[i];
        if (p[i] < rdata_min) rdata_min = p[i];
    }
}

// Add a row of data from the source array
//
void REDUCED_ARRAY_GEN::add_source_row(float* in) {
    float* p;
    int i, ry;

    if (scury >= sdimy) {
        // printf("too many calls to add_source_row()!\n");
        // Crashing is not an appropriate response in release code.
        assert(scury<sdimy);
        return;
    }
    if (rdimy == sdimy) {
        ry = scury;
        if (rdimx == sdimx) {
            memcpy(rrow(ry), in, rdimx*sizeof(float));
        } else {
            reduce_source_row(in, rrow(ry));
        }
        update_max(ry);
        nvalid_rows++;
    } else {
        ry = (scury*rdimy)/sdimy;
        if (scury == 0) memset(rrow(0), 0, rdimx*sizeof(float));

        // if we've moved into a new row, finish up the previous one
        //
        if (ry > last_ry) {
            p = rrow(last_ry);
            if (last_ry_count > 1) {
                for (i=0; i<rdimx; i++) {
                    p[i] /= last_ry_count;
                }
            }
            update_max(last_ry);
            nvalid_rows++;
            last_ry = ry;
            last_ry_count = 0;
            memset(rrow(ry), 0, rdimx*sizeof(float));
        }

        last_ry_count++;
        p = rrow(ry);
        if (rdimx == sdimx) {
            for (i=0; i<sdimx; i++) {
                p[i] += in[i];
            }
        } else {
            reduce_source_row(in, ftemp);
            for (i=0; i<rdimx; i++) {
                p[i] += ftemp[i];
            }
        }

        // if this is last row, finish up
        //
        if (scury == sdimy-1) {
            p = rrow(last_ry);
            if (last_ry_count > 1) {
                for (i=0; i<rdimx; i++) {
                    p[i] /= last_ry_count;
                }
            }
            update_max(ry);
            nvalid_rows++;
        }
    }
    scury++;
}

