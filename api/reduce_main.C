// Copyright 2003 Regents of the University of California

// SETI_BOINC is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2, or (at your option) any later
// version.

// SETI_BOINC is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with SETI_BOINC; see the file COPYING.  If not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// main-program part of the implementation of REDUCE

#ifdef _WIN32
#include "boinc_win.h"
#include <GL/gl.h>
#endif

#ifndef _WIN32
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#ifdef __APPLE_CC__
#include <OpenGL/gl.h>
#endif
#endif

//#include "std_fixes.h"
#include "boinc_gl.h"
#include "gutil.h"
#include "reduce.h"

REDUCED_ARRAY::REDUCED_ARRAY() : sdimx(0), sdimy(0), rdimx(0), rdimy(0), rdimx_max(0), rdimy_max(0), scury(0),
	rdata(0), rdata_max(0), rdata_min(0), ftemp(0), itemp(0), last_ry(0), last_ry_count(0), nvalid_rows(0), 
	ndrawn_rows(0), draw_deltax(0), draw_deltaz(0), reduce_method(REDUCE_METHOD_AVG), hue0(0), dhue(0), 
	alpha(0), xlabel(0), ylabel(0), zlabel(0)
{
		memset(draw_pos,0,sizeof(draw_pos));
		memset(draw_size,0,sizeof(draw_size));
}


REDUCED_ARRAY::~REDUCED_ARRAY() {
    if (rdata) free(rdata);
    if (ftemp) free(ftemp);
    if (itemp) free(itemp);
}

// (mx, my) are maximum reduced dimensions
// (typically based on window size, e.g. half of window size in pixels)
//
void REDUCED_ARRAY::set_max_dims(int mx, int my) {
    rdimx_max = mx;
    rdimy_max = my;
    rdimx = rdimx_max;
    rdimy = rdimy_max;
}

// Prepare to receive a source array.
// (sx, sy) are dimensions of source array
//
void REDUCED_ARRAY::init_data(int sx, int sy) {
    sdimx = sx;
    sdimy = sy;
    if (sdimx > rdimx_max) {
        rdimx = rdimx_max;
    } else {
        rdimx = sdimx;
    }
    if (sdimy > rdimy_max) {
        rdimy = rdimy_max;
    } else {
        rdimy = sdimy;
    }
    rdata = (float*)realloc(rdata, rdimx*rdimy*sizeof(float));
    ftemp = (float*)realloc(ftemp, rdimx*sizeof(float));
    itemp = (int*)realloc(itemp, rdimx*sizeof(int));
    nvalid_rows = 0;
    ndrawn_rows = 0;
    scury = 0;
    last_ry = 0;
    last_ry_count = 0;
    rdata_max = 0;
    rdata_min = (float)1e20;
}

bool REDUCED_ARRAY::full() {
    return nvalid_rows==rdimy;
}

void REDUCED_ARRAY::reset() {
    nvalid_rows = 0;
    ndrawn_rows = 0;
    scury = 0;
    last_ry = 0;
    last_ry_count = 0;
}

void REDUCED_ARRAY::init_display(
    GRAPH_STYLE st, float* p, float* s, double h0, double dh, float trans,
    char* xl, char* yl, char* zl
) {
    memcpy(draw_pos, p, sizeof(draw_pos));
    memcpy(draw_size, s, sizeof(draw_size));
    draw_deltax = draw_size[0]/rdimx;
    draw_deltaz = draw_size[2]/rdimy;
    hue0 = h0;
    dhue = dh;
    alpha = trans;
    draw_style = st;
	xlabel=xl;
	ylabel=yl;
	zlabel=zl;
}

// reduce a single row.  This is called only if sdimx > rdimx;
//
void REDUCED_ARRAY::reduce_source_row(float* in, float* out) {
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

void REDUCED_ARRAY::update_max(int row) {
    int i;
    float* p = rrow(row);

    for (i=0; i<rdimx; i++) {
        if (p[i] > rdata_max) rdata_max = p[i];
        if (p[i] < rdata_min) rdata_min = p[i];
    }
}

// Add a row of data from the source array
//
void REDUCED_ARRAY::add_source_row(float* in) {
    float* p;
    int i, ry;

    if (scury >= sdimy) {
        printf("too many calls to add_source_row()!\n");
        *(int*)0 = 0;
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

const char *BOINC_RCSID_70f1fa52c7 = "$Id$";
