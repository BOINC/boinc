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

// Stores a "reduced" (i.e. lower-resolution) version of a 2-D array.
// The source data is passed one row at a time.
//
// This is used to draw graphs of 2-D data that may have much higher
// resolution in either or both dimensions than is useful for screen display.
//
// To use this:
//
// call set_max_dims() to set maximum dimensions of reduced array.
//     (change this when window size changes)
// call init() to prepare to reduce a source array.
// call add_source_row() for each row of the source array.
// read nvalid_rows to see how many rows are in reduced array so far
// call rrow() to get a pointer to a particular row
// or call draw() to generate a picture.
//     This is currently implemented as follows:
//     If both dimensions are reduced (source dimension greater than reduced dim)
//     then draw the graph as vertical lines.
//     If reduced in one dimension, draw vertical rectangles.
//     Otherwise draw quadrilaterals.

#ifndef BOINC_REDUCE_H
#define BOINC_REDUCE_H

#define MAX_DATA    65536
#define MAX_DIMX    1024

struct REDUCED_ARRAY_DATA {
    float rdata[MAX_DATA];
    int rdimx, rdimy;           // size of the reduced array
    float rdata_max;            // max value in array
    float rdata_min;            // min value in array
    inline float* rrow(int j) {
        return rdata + j*rdimx;
    }
};


// THE PART RELEVANT TO GENERATION FOLLOWS

#define REDUCE_METHOD_AVG	0		// Take the average of reduced elements
#define REDUCE_METHOD_SUM	1		// Take the sum of reduced elements
#define REDUCE_METHOD_MAX	2		// Take the maximum of reduced elements
#define REDUCE_METHOD_MIN	3		// Take the minimum of reduced elements

struct REDUCED_ARRAY_GEN: REDUCED_ARRAY_DATA {
    float ftemp[MAX_DIMX];
    int itemp[MAX_DIMX];
    int sdimx, sdimy;           // size of the source array
    int scury;                  // next row of source array
        // the following 2 are relevant only if sdimy > rdimy
    int last_ry;                // last row index accumulated into
    int last_ry_count;          // number of source rows accumulated so far
    int nvalid_rows;            // number of valid rows in reduced array
	int reduce_method;			// Which method to use for data row reduction

    void init_data(int, int);
    void reduce_source_row(float*, float*);
    void add_source_row(float*);
    bool full();
    void reset();
    void update_max(int row);
};

// THE PART RELEVANT TO RENDERING FOLLOWS

enum GRAPH_STYLE {
    GRAPH_STYLE_RECTANGLES,
    GRAPH_STYLE_PLANES,
    GRAPH_STYLE_WIREFRAME,
    GRAPH_STYLE_SURFACE
};

struct REDUCED_ARRAY_RENDER: REDUCED_ARRAY_DATA {
    int ndrawn_rows;            // number of rows drawn so far
    float draw_pos[3];
    float draw_size[3];
    float draw_deltax;
    float draw_deltaz;
    double hue0;
    double dhue;
	float alpha;
	char* xlabel,*ylabel,*zlabel;
    GRAPH_STYLE draw_style;
    double start_time;

    void init(double grow, double hold);
    void init_display(
        GRAPH_STYLE, float*, float*, double, double, float, char*, char*, char*
    );
    void new_array();
    void draw_row_quad(int);
	void draw_row_rect_x(int);
    void draw_row_rect_y(int);
    void draw_row_line(int);
	void draw(int, int);
    void draw_new();
    void draw_all();
	void draw_part(double frac);
    void draw_axes();
	void draw_axis_labels();
	void draw_labels();
};

#endif
