#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#include "gutil.h"
#include "reduce.h"

REDUCED_ARRAY::REDUCED_ARRAY() {
    rdata = 0;
    ftemp = 0;
    itemp = 0;
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
void REDUCED_ARRAY::init(int sx, int sy) {
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

void REDUCED_ARRAY::init_draw(float* p, float* s, double h0, double dh) {
    memcpy(draw_pos, p, sizeof(draw_pos));
    memcpy(draw_size, s, sizeof(draw_size));
    draw_deltax = draw_size[0]/rdimx;
    draw_deltaz = draw_size[2]/rdimy;
    hue0 = h0;
    dhue = dh;
}

// reduce a single row.  This is called only if sdimx > rdimx;
//
void REDUCED_ARRAY::reduce_source_row(float* in, float* out) {
    int i, ri;

    memset(out, 0, rdimx*sizeof(float));
    memset(itemp, 0, rdimx*sizeof(int));
    for (i=0; i<sdimx; i++) {
        ri = (i*rdimx)/sdimx;
        out[ri] += in[i];
        itemp[ri]++;
    }
    for (i=0; i<rdimx; i++) {
        if (itemp[i] > 1) out[i] /= itemp[i];
    }
}

void REDUCED_ARRAY::update_max(int row) {
    int i;
    float* p = rrow(row);

    for (i=0; i<rdimx; i++) {
        if (p[i] > rdata_max) rdata_max = p[i];
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
        float h00 = row0[i]/rdata_max;
        float h01 = row0[i+1]/rdata_max;
        float h10 = row1[i]/rdata_max;
        float h11 = row1[i+1]/rdata_max;

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
        glColor3f(color.r, color.g, color.b);

        glVertex3f(x0, y00, z0);
        glVertex3f(x1, y01, z0);
        glVertex3f(x1, y11, z1);
        glVertex3f(x0, y10, z1);
    }
    glEnd();
#if 0
    // draw a black line on front and right edge of each quad
    //
    glBegin(GL_LINES);
    glColor3f(0., 0., 0.);
    for (i=0; i<rdimx-1; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        x1 = x0 + draw_deltax;
        float eps=.03f;
        y00 = draw_pos[1] + draw_size[1]*row0[i]/rdata_max+eps;
        y01 = draw_pos[1] + draw_size[1]*row0[i+1]/rdata_max+eps;
        y10 = draw_pos[1] + draw_size[1]*row1[i]/rdata_max+eps;
        y11 = draw_pos[1] + draw_size[1]*row1[i+1]/rdata_max+eps;
        glVertex3f(x0, y00, z0);
        glVertex3f(x1, y01, z0);
        glVertex3f(x1, y01, z0);
        glVertex3f(x1, y11, z1);
    }
    glEnd();
#endif
}

void REDUCED_ARRAY::draw_row_rect_x(int row) {
    float z0 = draw_pos[2] + (draw_size[2]*row)/rdimy;
    float x0, x1, y0, y1;
    float* row0 = rrow(row);
    int i;

    glBegin(GL_QUADS);
    for (i=0; i<rdimx; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        x1 = x0 + draw_deltax*.8f;
        float h = row0[i]/rdata_max;

        y0 = draw_pos[1];
        y1 = draw_pos[1] + draw_size[1]*h;

        double hue = hue0 + (dhue*i)/rdimx;
        if (hue > 1) hue -= 1;
        double sat = 1.;
        double lum = .5 + h/2;
        COLOR color;
        HLStoRGB(hue, lum, sat, color);
        glColor3f(color.r, color.g, color.b);

        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x0, y1, z0);
    }
    glEnd();

    // draw a black line on top of rectangle
    //
    glBegin(GL_LINES);
    glColor3f(0., 0., 0.);
    for (i=0; i<rdimx; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        x1 = x0 + draw_deltax*.8f;
        float h = row0[i]/rdata_max;

        y1 = draw_pos[1] + draw_size[1]*h;

        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
    }
    glEnd();
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
        float h = row0[i]/rdata_max;

        y0 = draw_pos[1];
        y1 = draw_pos[1] + draw_size[1]*h;

        double hue = hue0 + (dhue*i)/rdimx;
        if (hue > 1) hue -= 1;
        double sat = 1.;
        double lum = .5 + h/2;
        COLOR color;
        HLStoRGB(hue, lum, sat, color);
        glColor3f(color.r, color.g, color.b);

        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y0, z1);
    }
    glEnd();

    // draw a black line on top of rectangle
    //
    glBegin(GL_LINES);
    glColor3f(0., 0., 0.);
    for (i=0; i<rdimx-1; i++) {
        x0 = draw_pos[0] + (draw_size[0]*i)/rdimx;
        float h = row0[i]/rdata_max;

        y1 = draw_pos[1] + draw_size[1]*h;

        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
    }
    glEnd();
}

void REDUCED_ARRAY::draw_row_line(int row) {
}

void REDUCED_ARRAY::draw(int r0, int rn) {
    int i;
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
    glFlush();
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

void REDUCED_ARRAY::draw_axes() {
    glBegin(GL_QUADS);
    glColor3d(.2, .2, .2);

    glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]);
    glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]);
    glVertex3f(draw_pos[0]+draw_size[0], draw_pos[1], draw_pos[2]+draw_size[2]);
    glVertex3f(draw_pos[0], draw_pos[1], draw_pos[2]+draw_size[2]);
    glEnd();
}
