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

#ifdef _WIN32
#include "boinc_win.h"
#include <GL/gl.h>
#endif

#ifndef _WIN32
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "GL/gl.h"
#define max(a,b)  (a>b?a:b)
#define min(a,b)  (a>b?b:a)

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#ifdef __APPLE_CC__
#include <OpenGL/gl.h>
#endif
#endif

#include "boinc_gl.h"
#include "gutil.h"
#include "reduce.h"

REDUCED_ARRAY::REDUCED_ARRAY() {
    rdata = 0;
    ftemp = 0;
    itemp = 0;
    reduce_method = REDUCE_METHOD_AVG;
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
            out[ri] = min(out[ri],in[i]);
            break;
            case REDUCE_METHOD_MAX:
            out[ri] = max(out[ri],in[i]);
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

void REDUCED_ARRAY::draw_row_quad(int row) {
    float z0 = draw_pos[2] + (draw_size[2]*row)/rdimy;
    float z1 = z0 + draw_deltaz;
    float x0, x1, y00, y01, y10, y11;
    float* row0 = rrow(row);
    float* row1 = rrow(row+1);
    int i;

    glBegin(GL_QUADS);          // TODO: use GL_QUADSTRIP
    for (i=0; i<rdimx-1; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        x1 = x0 + draw_deltax;
        float h00 = (row0[i]-rdata_min)/(rdata_max-rdata_min);
        float h01 = (row0[i+1]-rdata_min)/(rdata_max-rdata_min);
        float h10 = (row1[i]-rdata_min)/(rdata_max-rdata_min);
        float h11 = (row1[i+1]-rdata_min)/(rdata_max-rdata_min);

        y00 = draw_pos[1] + draw_size[1]*h00;
        y01 = draw_pos[1] + draw_size[1]*h01;
        y10 = draw_pos[1] + draw_size[1]*h10;
        y11 = draw_pos[1] + draw_size[1]*h11;

        float h = max(h00, max(h01, max(h10, h11)));
        double hue = hue0 + (dhue*i)/rdimx;
        if (hue > 1) hue -= 1;
        double sat = 1.;
        double lum = .5 + h/2;
        COLOR color;
        HLStoRGB(hue, lum, sat, color);
        glColor4f(color.r, color.g, color.b, alpha);

        glVertex3f(x0, y00, z0);
        glVertex3f(x1, y01, z0);
        glVertex3f(x1, y11, z1);
        glVertex3f(x0, y10, z1);
    }
    glEnd();
#if 0
    // draw a black line on front and right edge of each quad
    //
	glLineWidth(4.0);
	glDisable(GL_LINE_SMOOTH);
    glBegin(GL_LINES);
    glColor4f(0., 0., 0., 1.0);
    for (i=0; i<rdimx-1; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        x1 = x0 + draw_deltax;
        float eps=.03f;
        y00 = draw_pos[1] + draw_size[1]*(row0[i]-rdata_min)/rdata_max+eps;
        y01 = draw_pos[1] + draw_size[1]*(row0[i+1]-rdata_min)/rdata_max+eps;
        y10 = draw_pos[1] + draw_size[1]*(row1[i]-rdata_min)/rdata_max+eps;
        y11 = draw_pos[1] + draw_size[1]*(row1[i+1]-rdata_min)/rdata_max+eps;
        glVertex3f(x0, y00, z0);
        glVertex3f(x1, y01, z0);
        glVertex3f(x1, y01, z0);
        glVertex3f(x1, y11, z1);
    }
    glEnd();
#endif
}

void REDUCED_ARRAY::draw_row_rect_x(int row)  {
	float z0=0,z1=0,x0=0,x1=0,y0=0,y1=0,h=0,xm=0;
	int i=0;
	float* row0=0;
	int trow=row-1;
	float* trow0=0;
	double hue,sat,lum;
	COLOR color;

	switch(draw_style) {
	case GRAPH_STYLE_RECTANGLES:
		z0 = draw_pos[2] + (draw_size[2]*row)/rdimy;
		z1 = z0+.14f;
		row0 = rrow(row);

		glBegin(GL_QUADS);
		for (i=0; i<rdimx; i++) {
			x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
			x1 = x0 + draw_deltax*.95f;
			h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

			y0 = draw_pos[1];
			y1 = draw_pos[1] + draw_size[1]*h;

			double hue = hue0 + (dhue*i)/rdimx;
			if (hue > 1) hue -= 1;
			double sat = 1.;
			double lum = .5 + h/2;
			COLOR color;
			HLStoRGB(hue, lum, sat, color);
			glColor4f(color.r, color.g, color.b, alpha);

			//front

			glVertex3f(x0, y0, z0);
			glVertex3f(x1, y0, z0);
			glVertex3f(x1, y1, z0);
			glVertex3f(x0, y1, z0);
	/*
			//back
			glVertex3f(x0, y0, z1);
			glVertex3f(x1, y0, z1);
			glVertex3f(x1, y1, z1);
			glVertex3f(x0, y1, z1);

			//left
			glVertex3f(x0, y0, z0);
			glVertex3f(x0, y0, z1);
			glVertex3f(x0, y1, z1);
			glVertex3f(x0, y1, z0);

			//right
			glVertex3f(x1, y0, z0);
			glVertex3f(x1, y0, z1);
			glVertex3f(x1, y1, z1);
			glVertex3f(x1, y1, z0);

			//top
			glVertex3f(x0, y1, z0);
			glVertex3f(x0, y1, z1);
			glVertex3f(x1, y1, z1);
			glVertex3f(x1, y1, z0);
	*/
		}
		glEnd();


#if 0
		//draw lines
		mode_unshaded();
		glLineWidth(.5f);
		glBegin(GL_LINES);
		glColor4f(0,0,0,1);
		for (i=0; i<rdimx; i++) {
			x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
			x1 = x0 + draw_deltax*.8f;
			float h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

			y1 = draw_pos[1] + draw_size[1]*h;

	//front

			glVertex3f(x0, y0, z0);
			glVertex3f(x0, y1, z0);

			glVertex3f(x0, y0, z0);
			glVertex3f(x1, y0, z0);

			glVertex3f(x1, y0, z0);
			glVertex3f(x1, y1, z0);

			glVertex3f(x0, y1, z0);
			glVertex3f(x1, y1, z0);
	//back
			glVertex3f(x0, y0, z1);
			glVertex3f(x0, y1, z1);

			glVertex3f(x0, y0, z1);
			glVertex3f(x1, y0, z1);

			glVertex3f(x1, y0, z1);
			glVertex3f(x1, y1, z1);

			glVertex3f(x0, y1, z1);
			glVertex3f(x1, y1, z1);
	//right
			glVertex3f(x0, y0, z0);
			glVertex3f(x0, y1, z0);

			glVertex3f(x0, y1, z0);
			glVertex3f(x0, y1, z1);

			glVertex3f(x0, y1, z1);
			glVertex3f(x0, y0, z1);

			glVertex3f(x0, y0, z1);
			glVertex3f(x0, y0, z0);
	//left
			glVertex3f(x1, y0, z0);
			glVertex3f(x1, y1, z0);

			glVertex3f(x1, y1, z0);
			glVertex3f(x1, y1, z1);

			glVertex3f(x1, y1, z1);
			glVertex3f(x1, y0, z1);

			glVertex3f(x1, y0, z1);
			glVertex3f(x1, y0, z0);
		}
#endif
		glEnd();
	    break;
	case GRAPH_STYLE_SURFACE:
		glBegin(GL_TRIANGLE_STRIP);

		z0 = draw_pos[2] + (draw_size[2]*row)/rdimy;
		z1 = z0+.14f;
		row0 = rrow(row);
		trow=row-1;
		if(row!=0) trow0 = rrow(trow);

		for (i=0; i<rdimx; i++) {
			x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
			x1 = x0 + draw_deltax*.8f;
			xm = x0+((x1-x0)/2.0f);
			h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

			y0 = draw_pos[1];
			y1 = draw_pos[1] + draw_size[1]*h;

			hue = hue0 + (dhue*i)/rdimx;
			if (hue > 1) hue -= 1;
			sat = 1.;
			lum = .5 + h/2;
			HLStoRGB(hue, lum, sat, color);
			glColor4f(color.r, color.g, color.b, alpha);

			if(row!=0) {
				float lh = (trow0[i]-rdata_min)/(rdata_max-rdata_min);
				float ly = draw_pos[1] + draw_size[1]*lh;
				float lz = draw_pos[2] + (draw_size[2]*trow)/rdimy + .14f;
				glVertex3f(xm,ly,lz);
				glVertex3f(xm,y1,z1);
			}
		}
		glEnd();
	    break;
	case GRAPH_STYLE_WIREFRAME:
		glLineWidth(1.0f);
		z0 = draw_pos[2] + (draw_size[2]*row)/rdimy;
		z1 = z0+.14f;
		row0 = rrow(row);
		if(row!=0) trow0 = rrow(trow);

		glEnable(GL_LINE_SMOOTH);
		glBegin(GL_LINES);

		for (i=0; i<rdimx; i++) {
			x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
			x1 = x0 + draw_deltax*.8f;
			h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

			y0 = draw_pos[1];
			y1 = draw_pos[1] + draw_size[1]*h;

			hue = hue0 + (dhue*i)/rdimx;
			if (hue > 1) hue -= 1;
			sat = 1.;
			lum = .5 + h/2;
			HLStoRGB(hue, lum, sat, color);
			glColor4f(color.r, color.g, color.b, alpha);

			if(i!=rdimx-1) glVertex3f(x0+((x1-x0)/2.0f), y1, z0);
			if(row!=0)
			{
				float h2 = (trow0[i]-rdata_min)/(rdata_max-rdata_min);
				float z2 = draw_pos[2] + (draw_size[2]*trow)/rdimy;
				float y2 = draw_pos[1] + draw_size[1]*h2;
				glVertex3f(x0+((x1-x0)/2.0f), y1, z0);
				glVertex3f(x0+((x1-x0)/2.0f), y2, z2);
			}
			if(i!=0) glVertex3f(x0+((x1-x0)/2.0f), y1, z0);
		}
		glEnd();
		glDisable(GL_LINE_SMOOTH);
	    break;
	case GRAPH_STYLE_PLANES:
		z0 = draw_pos[2] + (draw_size[2]*row)/rdimy;
		z1 = z0+.14f;
		row0 = rrow(row);
		i=0;

		x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
		x1 = x0 + draw_deltax*.8f;
		h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

		y0 = draw_pos[1];
		y1 = draw_pos[1] + draw_size[1]*h;

		hue = hue0 + (dhue*i)/rdimx;
		if (hue > 1) hue -= 1;
		sat = 1.;
		lum = .5 + h/2;
		HLStoRGB(hue, lum, sat, color);
		glColor4f(color.r, color.g, color.b, alpha);


		glVertex3f(draw_pos[0],draw_pos[1],z0);
		glVertex3f(draw_pos[0],draw_pos[1],draw_pos[2] + (draw_size[2]*trow)/rdimy);

		glBegin(GL_QUAD_STRIP);
		for (i=0; i<rdimx; i++) {
			x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
			x1 = x0 + draw_deltax*.8f;
			h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

			y0 = draw_pos[1];
			y1 = draw_pos[1] + draw_size[1]*h;

			double hue = hue0 + (dhue*i)/rdimx;
			if (hue > 1) hue -= 1;
			double sat = 1.;
			double lum = .5 + h/2;
			COLOR color;
			HLStoRGB(hue, lum, sat, color);
			glColor4f(color.r, color.g, color.b, alpha);

			glVertex3d(x1,y0,z0);
			glVertex3d(x1,y1,z0);
		}
		glEnd();
		break;
	default:
		break;
	}
}

void REDUCED_ARRAY::draw_row_rect_y(int row) {
    float z0 = draw_pos[2] + (draw_size[2]*row)/rdimy;
    float z1 = z0 + draw_deltaz*.8f;
    float x0, y0, y1;
    float* row0 = rrow(row);
    int i;

    glBegin(GL_QUADS);
    for (i=0; i<rdimx-1; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        float h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

        y0 = draw_pos[1];
        y1 = draw_pos[1] + draw_size[1]*h;

        double hue = hue0 + (dhue*i)/rdimx;
        if (hue > 1) hue -= 1;
        double sat = 1.;
        double lum = .5 + h/2;
        COLOR color;
        HLStoRGB(hue, lum, sat, color);
        glColor4f(color.r, color.g, color.b, alpha);

        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y0, z1);
    }
    glEnd();

    // draw a black line on top of rectangle
    //
#if 0
    glBegin(GL_LINES);
    glColor4f(0., 0., 0., 1.0);
    for (i=0; i<rdimx-1; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        float h = (row0[i]-rdata_min)/(rdata_max-rdata_min);

        y1 = draw_pos[1] + draw_size[1]*h;

        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
    }
    glEnd();
#endif
}



void REDUCED_ARRAY::draw_row_line(int row) {
}

void REDUCED_ARRAY::draw(int r0, int rn) {
    int i;
    mode_unshaded();
    if (rdimx == sdimx) {
        if (rdimy == sdimy) {
            for (i=r0; i<rn; i++) {
                draw_row_rect_x(i);
            }
        } else {
            for (i=r0; i<rn; i++) {
                draw_row_rect_x(i);
            }
        }
    } else {
        if (rdimy == sdimy) {
            for (i=r0; i<rn; i++) {
                draw_row_rect_x(i);
            }
        } else {
            for (i=r0; i<rn; i++) {
                draw_row_rect_x(i);
            }
        }
    }
    ndrawn_rows = rn;
}

void REDUCED_ARRAY::draw_all() {
    draw(0, nvalid_rows);
}

void REDUCED_ARRAY::draw_new() {
    draw(ndrawn_rows, nvalid_rows);
}

void REDUCED_ARRAY::draw_part(double frac) {
    int nr = (int)(nvalid_rows*frac);
    draw(0, nr);
}

void REDUCED_ARRAY::draw_axis_labels() {
    GLfloat char_height = .5f;
    GLfloat line_width = 3.0f;
	GLfloat spacing = 2.0f;
	GLfloat rotation = -90;
	GLfloat rotation_vector[3] = {0,0,0};

	float w;

	char* x_label = "Time";
#if 0
	char* y_label = "Frequency";
	char* y_begin_label = "0";
	char* y_end_label = "9 Khz";
#endif

	float x_text_pos[3] = {0,0,0};
	w = text_width(x_label);
	x_text_pos[0]=draw_pos[0];//+draw_size[0];
	x_text_pos[1]=draw_pos[1];//+draw_size[1];
	x_text_pos[2]=draw_pos[2]+draw_size[2]-(w/2.0f);

	rotation_vector[0]=0;
	rotation_vector[1]=draw_size[1];
	rotation_vector[2]=0;

	draw_rotated_text(x_text_pos,char_height,line_width,spacing,x_label,rotation,rotation_vector);
	//draw_text_line(x_text_pos,char_height,line_width,x_label,0);

}

void REDUCED_ARRAY::draw_axes() {

	// comment??

	float adj2=0.0f;//-(draw_size[2]*1)/rdimy;
	float adj=0.0f;

	float adj3=0;
	// box
	mode_unshaded();

	glLineWidth(.5);
	glBegin(GL_LINES);
	glColor4d(1,1,1,.5);

	//back square
	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+adj);

	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0], draw_pos[1]+draw_size[1], draw_pos[2]+adj);

	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]+draw_size[1], draw_pos[2]+adj);

	glVertex3f(draw_pos[0], draw_pos[1]+draw_size[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]+draw_size[1], draw_pos[2]+adj);

	//front square
	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);

	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);
	glVertex3f(draw_pos[0], draw_pos[1]+draw_size[1], draw_pos[2]+draw_size[2]+adj2);

	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]+draw_size[1], draw_pos[2]+draw_size[2]+adj2);

	glVertex3f(draw_pos[0], draw_pos[1]+draw_size[1], draw_pos[2]+draw_size[2]+adj2);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]+draw_size[1], draw_pos[2]+draw_size[2]+adj2);

	//connecting lines
	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);

	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);

	glVertex3f(draw_pos[0], draw_pos[1]+draw_size[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0], draw_pos[1]+draw_size[1], draw_pos[2]+draw_size[2]+adj2);

	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]+draw_size[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]+draw_size[1], draw_pos[2]+draw_size[2]+adj2);
	glEnd();


	mode_unshaded();

	glLineWidth(1.0);
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINES);
	glColor4d(1,1,1,1);


	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+adj);
    glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+adj);

	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);

	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+adj);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);

    glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);
    glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+draw_size[2]+adj2);

	glEnd();

	glColor4d(1,1,1,.2);
    glBegin(GL_QUADS);
	glVertex3f(draw_pos[0], draw_pos[1]-adj3, draw_pos[2]+draw_size[2]+adj2);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]-adj3, draw_pos[2]+draw_size[2]+adj2);
	glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1]-adj3, draw_pos[2]+adj);
    glVertex3f(draw_pos[0], draw_pos[1]-adj3, draw_pos[2]+adj);
    glEnd();

	glDisable(GL_LINE_SMOOTH);
}


void REDUCED_ARRAY::draw_labels() {
	double model[16];
	double proj[16];
	double z_pos[3];
	double x_pos[3];
	double p_pos[3];

	float arrowh = .35f;
	float arroww = .05f;

	float ch = .015f;
	float lw = .02f;
	float ls = 0;

#if 0
	float wd=.015f;
	float l=1.0f;

	float left_of_z = -0.1f;
	float left_of_z2 = -0.04f;
	float below_x = -.03f;
	float center_x = -.06f;
#endif

	glLineWidth(1.4f);
	glBegin(GL_LINES);
	glColor3f(1,1,1);
	glVertex3f(draw_pos[0]+draw_size[0]+.4f,draw_pos[1],draw_pos[2]+draw_size[2]-.5f);
	glVertex3f(draw_pos[0]+draw_size[0]+.4f,draw_pos[1]+1.2f,draw_pos[2]+draw_size[2]-.5f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(draw_pos[0]+draw_size[0]+.4f,draw_pos[1]+1.2f+arrowh,draw_pos[2]+draw_size[2]-.5f);

	glVertex3f(draw_pos[0]+draw_size[0]+.4f-arroww,draw_pos[1]+1.2f,draw_pos[2]+draw_size[2]-.5f-arroww);
	glVertex3f(draw_pos[0]+draw_size[0]+.4f+arroww,draw_pos[1]+1.2f,draw_pos[2]+draw_size[2]-.5f-arroww);
	glVertex3f(draw_pos[0]+draw_size[0]+.4f+arroww,draw_pos[1]+1.2f,draw_pos[2]+draw_size[2]-.5f+arroww);
	glVertex3f(draw_pos[0]+draw_size[0]+.4f-arroww,draw_pos[1]+1.2f,draw_pos[2]+draw_size[2]-.5f+arroww);
	glVertex3f(draw_pos[0]+draw_size[0]+.4f-arroww,draw_pos[1]+1.2f,draw_pos[2]+draw_size[2]-.5f-arroww);
	glEnd();

	int viewport[4];
	get_matrix(model);
	get_projection(proj);
	get_viewport(viewport);
	int w = viewport[2];
	int h = viewport[3];

	glPushMatrix();
	//unscale modelview matrix

	double aspect_ratio = 4.0/3.0;
	if ((double)h*aspect_ratio > (double)w) {
		model[1]*=1.0f/(((double)w/aspect_ratio)/(double)h);
		model[5]*=1.0f/(((double)w/aspect_ratio)/(double)h);
		model[9]*=1.0f/(((double)w/aspect_ratio)/(double)h);
    } else {
		model[0]*=1.0f/(((double)h*aspect_ratio)/(double)w);
		model[4]*=1.0f/(((double)h*aspect_ratio)/(double)w);
		model[8]*=1.0f/(((double)h*aspect_ratio)/(double)w);
    }

	//project to ortho coordinates
	viewport[0]=0;
	viewport[1]=0;
	viewport[2]=1;
	viewport[3]=1;

	get_2d_positions(draw_pos[0],draw_pos[1],draw_pos[2]+(draw_size[2]/2.0f),
        model, proj, viewport,z_pos
    );

	get_2d_positions(draw_pos[0]+draw_size[0]/2.0f,draw_pos[1],draw_pos[2]+draw_size[2],
        model, proj, viewport,x_pos
    );

	get_2d_positions(draw_pos[0]+draw_size[0]+.2f,draw_pos[1]+.6f,draw_pos[2]+draw_size[2]-.4f,
        model, proj, viewport,p_pos
    );
	glPopMatrix();

	mode_ortho();
	mode_unshaded();
	glColor3d(1,1,1);

	float zpos[3]={(float)z_pos[0],(float)z_pos[1],(float)z_pos[2]};
	float xpos[3]={(float)x_pos[0],(float)x_pos[1],(float)x_pos[2]};
	float ppos[3]={(float)p_pos[0],(float)p_pos[1],(float)p_pos[2]};

	draw_text_right(zpos,ch,lw,ls,zlabel);
	draw_text(xpos,ch,lw,ls,xlabel);
	draw_text(ppos,ch,lw,ls,ylabel);

	ortho_done();
}
