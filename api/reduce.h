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

#ifndef REDUCE_H
#define REDUCE_H

#define REDUCE_METHOD_AVG	0		// Take the average of reduced elements
#define REDUCE_METHOD_SUM	1		// Take the sum of reduced elements
#define REDUCE_METHOD_MAX	2		// Take the maximum of reduced elements
#define REDUCE_METHOD_MIN	3		// Take the minimum of reduced elements

enum GRAPH_STYLE {
    GRAPH_STYLE_RECTANGLES,
    GRAPH_STYLE_PLANES,
    GRAPH_STYLE_WIREFRAME,
    GRAPH_STYLE_SURFACE
};

class REDUCED_ARRAY {
public:
    int sdimx, sdimy;           // size of the source array
    int rdimx, rdimy;           // size of the reduced array
    int rdimx_max, rdimy_max;   // maximum sizes of reduced array
                                // (determined by window size)
    int scury;                  // next row of source array
    float* rdata;
    float rdata_max;            // estimated or actual maximum
    float rdata_min;            // estimated or actual minimum
    float* ftemp;
    int* itemp;
        // the following 2 are relevant only if sdimy > rdimy
    int last_ry;                // last row index accumulated into
    int last_ry_count;          // number of source rows accumulated so far
    int nvalid_rows;            // number of valid rows in reduced array
    int ndrawn_rows;            // number of rows drawn so far
    float draw_pos[3];
    float draw_size[3];
    float draw_deltax;
    float draw_deltaz;
	int reduce_method;			// Which method to use for data row reduction
    double hue0;
    double dhue;
	float alpha;
    GRAPH_STYLE draw_style;

    REDUCED_ARRAY();
    ~REDUCED_ARRAY();
    void init(int, int);
    void init_draw(GRAPH_STYLE, float*, float*, double, double, float);
    void set_max_dims(int, int);
    void reduce_source_row(float*, float*);
    void add_source_row(float*);
    bool full();
    void reset();
    void update_max(int row);
    float* rrow(int j) {
        return rdata + j*rdimx;
    }
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
	void draw_3dlabels();	
};

#endif