// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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

#ifdef _WIN32
#include "boinc_win.h"
//extern "C"{
#include "jpeglib.h"
//}
#include "bmplib.h"
#include "tgalib.h"
#endif

#ifndef _WIN32
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <csetjmp>
extern "C"{
#include <jpeglib.h>
}
#endif

#include "boinc_gl.h"

#include "gutil.h"
#include "filesys.h"
#include "util.h"

GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_shininess[] = {40.0};

// call this to draw 3D stuff with shaded color
//
void mode_shaded(GLfloat* color) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel (GL_SMOOTH);
    glDepthMask(GL_TRUE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

// call this to use textures (turn off lighting)
//
void mode_texture() {

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

// call this to draw unshaded color
//
void mode_unshaded() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glDepthMask(GL_TRUE);
}

// call this to render 2D stuff, with 0..1x0..1 getting mapped
// to the full window.  You must call ortho_done() when done.
//
void mode_ortho() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(
        0.0, 0.0, 1.0,        // eye position
        0, 0, 0,              // where we're looking
        0.0, 1.0, 0.          // up is in positive Y direction
    );
    int viewport[4];
    get_viewport(viewport);
    center_screen(viewport[2], viewport[3]);
    scale_screen(viewport[2], viewport[3]);
}

void ortho_done() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

bool get_matrix(double src[16]) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glGetDoublev(GL_MODELVIEW_MATRIX, src);
    glPopMatrix();

    return true;
}

bool get_projection(double src[16]) {
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glGetDoublev(GL_PROJECTION_MATRIX, src);
	glPopMatrix();
	return true;
}

bool get_viewport(int view[4]) {
	glMatrixMode(GL_MODELVIEW);
	glGetIntegerv(GL_VIEWPORT, (GLint*)view);
	return true;
}

void get_2d_positions(double x, double y, double z,
	double model[16], double proj[16], int viewport[4], double proj_pos[3]
) {
	gluProject(x, y, z,model,proj,viewport,&proj_pos[0],&proj_pos[1],&proj_pos[2]);
}

void mode_lines() {
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

static double HuetoRGB(double m1, double m2, double h ) {
   if( h < 0 ) h += 1.0;
   if( h > 1 ) h -= 1.0;
   if( 6.0*h < 1 ) return (m1+(m2-m1)*h*6.0);
   if( 2.0*h < 1 ) return m2;
   if( 3.0*h < 2.0 ) return (m1+(m2-m1)*((2.0/3.0)-h)*6.0);
   return m1;
}

void HLStoRGB( double H, double L, double S, COLOR& c) {
    double m1, m2;

    if(S==0) {
        c.r=c.g=c.b=L;
    } else {
        if(L <=0.5) {
            m2 = L*(1.0+S);
        } else {
            m2 = L+S-L*S;
        }
        m1 = 2.0*L-m2;
        c.r = HuetoRGB(m1,m2,H+1.0/3.0);
        c.g = HuetoRGB(m1,m2,H);
        c.b = HuetoRGB(m1,m2,H-1.0/3.0);
    }
}

static inline float frand() {
    return rand()/(float)RAND_MAX;
}

void scale_screen(int iw, int ih) {
	double aspect_ratio = 4.0/3.0;
    double w=iw, h=ih;
    double xs, ys;
	if (h*aspect_ratio > w) {
        xs = 1.0;
        ys = (w/aspect_ratio)/h;
    } else {
		xs = (h*aspect_ratio)/w;
        ys = 1.0;
    }
	glScalef(xs, ys*4./3., 1);
}

void center_screen(int iw, int ih) {
	double aspect_ratio = 4.0/3.0;
    double w=iw, h=ih;
	if (h*aspect_ratio > w) {
		glTranslatef(0.0, (h/2.0-(w/aspect_ratio/2.0))/h, 0.0);
    } else {
		glTranslatef((w/2.0-(h*aspect_ratio/2.0))/w, 0.0, 0.0);
    }
}

void drawSphere(GLfloat* pos, GLfloat rad) {
    GLUquadricObj* x = gluNewQuadric();
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
    gluSphere(x, rad, 20, 20);
    gluDeleteQuadric(x);
    glPopMatrix();
}

void drawCylinder(bool vertical, GLfloat* pos, GLfloat len, GLfloat rad) {
    GLUquadricObj* x = gluNewQuadric();
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
    if (vertical) {
        glRotated(-90., 1., 0., 0.);
    } else {
        glRotated(90., 0., 1., 0.);
    }
    gluCylinder(x, rad, rad, len, 20, 1);
    gluDeleteQuadric(x);
    glPopMatrix();
}

#define STROKE_SCALE    120
    // GLUT stroke characters are about 120 units high

GLfloat text_width(const char* text) {
    GLfloat sum = 0;
    for (const char* p = text; *p; p++) {
        sum += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    }
    return sum/STROKE_SCALE;
}

#ifdef _WIN32
extern float get_char_width(unsigned char c);
#endif

float text_width_new(const char* text) {
	float sum=0;
#ifdef _WIN32
	for(const char* p = text; *p; p++) {
//		sum += get_char_width(p[0]);
	}
#endif
	return sum;
}

static void draw_text_line_aux(const char *text) {
    for (const char* p = text; *p; p++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    }
}

static void draw_text_start(GLfloat* pos, GLfloat char_height, GLfloat line_width) {
    glLineWidth(line_width);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
	glRasterPos3d(pos[0],pos[1],pos[2]);
    float w = char_height/STROKE_SCALE;
    glScalef(w, w, w);
}

static void draw_text_end() {
    glPopMatrix();
}

// draw a line of text in the XY plane at the given starting position,
// character height, and line width.
void draw_text_simple(const char* text, float line_width, float char_height)
{
    glLineWidth(line_width);
    float w = char_height/STROKE_SCALE;
    glScalef(w, w, w);
	draw_text_line_aux(text);
}

void draw_text_line(
    GLfloat* _pos, GLfloat char_height, GLfloat line_width, const char *text,
    int justify
) {
    GLfloat pos[3];
    GLfloat w;

    memcpy(pos, _pos, sizeof(pos));
    switch(justify) {
    case TEXT_LEFT:
        break;
    case TEXT_CENTER:
        w = text_width(text);
        pos[0] -= w/2;
        break;
    case TEXT_RIGHT:
        w = text_width(text);
        pos[0] -= w;
        break;
    }
    draw_text_start(pos, char_height, line_width);
    draw_text_line_aux(text);
    draw_text_end();
}

// draw rotated text
void draw_rotated_text(
	GLfloat* pos, GLfloat height, GLfloat width, GLfloat spacing, const char *text,
	GLfloat rotation, GLfloat* rotation_vector)
{
	draw_text_start(pos, height, width);
	glRotatef(rotation,rotation_vector[0],rotation_vector[1],rotation_vector[2]);
    draw_text_line_aux(text);
    draw_text_end();
}

// draw multiple lines of text
//
void draw_text(
    GLfloat* _pos, GLfloat char_height, GLfloat line_width,
    GLfloat line_spacing, const char* text
) {
    char* q, *p;
    char buf[4096];
    GLfloat pos[3];
    memcpy(pos, _pos, sizeof(pos));
    strlcpy(buf, text, 4096);

    p = buf;
    while (*p) {
        q = strchr(p, '\n');
        if (q) *q = 0;
        draw_text_start(pos, char_height, line_width);
        draw_text_line_aux(p);
        draw_text_end();
        pos[1] -= line_spacing;
        if (!q) break;
        p = q+1;
    }
}

void draw_text_new_3d(
	GLfloat* _pos, GLfloat char_height, GLfloat line_width,
    GLfloat line_spacing, const char* text
) {
	char* q, *p;
    char buf[4096];
    GLfloat pos[3];
    memcpy(pos, _pos, sizeof(pos));
    strlcpy(buf, text, 4096);

    p = buf;
	glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
    while (*p) {
        q = strchr(p, '\n');
        if (q) *q = 0;
		glRasterPos3d(pos[0],pos[1],pos[2]);
		print_text(p);
        pos[1] -= line_spacing;
        if (!q) break;
        p = q+1;
    }
	glPopMatrix();
}

void draw_text_new(
    GLfloat* _pos, GLfloat char_height, GLfloat line_width,
    GLfloat line_spacing, const char* text
) {
	char *q, *p;
	char buf[4096];
	GLfloat pos[3];
	memcpy(pos,_pos,sizeof(pos));
    strlcpy(buf, text, 4096);
	p=buf;
	int viewport[4];
	get_viewport(viewport);
	while(*p) {
		q = strchr(p, '\n');
        if (q) *q = 0;
		glRasterPos3d(pos[0],pos[1],pos[2]);
		print_text(p);
        pos[1] -= line_spacing;
        if (!q) break;
        p = q+1;
	}
}

void draw_text_right(
    GLfloat* _pos, GLfloat char_height, GLfloat line_width,
    GLfloat line_spacing, const char* text
) {
	char *q, *p;
	char buf[4096];
	GLfloat pos[3];
	memcpy(pos,_pos,sizeof(pos));
	float orig = pos[0];
    strlcpy(buf, text, 4096);
	p=buf;
	float w;

	while (*p) {
		q = strchr(p, '\n');
        if (q) *q = 0;
        w = text_width(p)/66.5f;
        pos[0] -= w;
	    draw_text_start(pos, char_height, line_width);
	    draw_text_line_aux(p);
	    draw_text_end();
		pos[1] -= line_spacing;
		pos[0]=orig;
        if (!q) break;
        p = q+1;
	}
}

void MOVING_TEXT_PANEL::init(
    float* p, float* s, COLOR& c, double d, double ch, double lw, double ls, double m
) {
    memcpy(pos, p, sizeof(pos));
    memcpy(base_pos, p, sizeof(base_pos));
    memcpy(size, s, sizeof(size));
    color = c;
    theta = 0;
    dtheta = d;
    char_height = ch;
    line_width = lw;
    line_spacing = ls;
    margin = m;
    memset(text, 0, sizeof(text));
}

// draw a rectangle of the given color in the XY plane
// and draw the given test in it
//
void MOVING_TEXT_PANEL::draw() {
    COLOR side_color = color;
    GLfloat pos0[3], pos1[3], pos2[3], pos3[3];
    memcpy(pos0, pos, sizeof(pos0));
    memcpy(pos1, pos, sizeof(pos0));
    pos1[0] += size[0];
    memcpy(pos2, pos1, sizeof(pos0));
    pos2[1] += size[1];
    memcpy(pos3, pos2, sizeof(pos0));
    pos3[0] -= size[0];
    mode_unshaded();
    glColor4fv(&color.r);
    glBegin(GL_QUADS);
    glVertex3fv(pos0);
    glVertex3fv(pos1);
    glVertex3fv(pos2);
    glVertex3fv(pos3);

    // draw flanges
    //
    side_color.r /= 2;
    side_color.g /= 2;
    side_color.b /= 2;
    glColor4fv(&side_color.r);
    GLfloat posa0[3], posa1[3], posa2[3], posa3[3];
    memcpy(posa0, pos0, sizeof(pos0));
    memcpy(posa1, pos1, sizeof(pos0));
    memcpy(posa2, pos2, sizeof(pos0));
    memcpy(posa3, pos3, sizeof(pos0));
    posa0[2] -= .2;
    posa1[2] -= .2;
    posa2[2] -= .2;
    posa3[2] -= .2;
    glVertex3fv(pos0);
    glVertex3fv(pos1);
    glVertex3fv(posa1);
    glVertex3fv(posa0);
    glVertex3fv(pos1);
    glVertex3fv(pos2);
    glVertex3fv(posa2);
    glVertex3fv(posa1);
    glVertex3fv(pos2);
    glVertex3fv(pos3);
    glVertex3fv(posa3);
    glVertex3fv(posa2);
    glVertex3fv(pos3);
    glVertex3fv(pos0);
    glVertex3fv(posa0);
    glVertex3fv(posa3);
    glEnd();

    pos3[0] += margin;
    pos3[1] -= (margin+char_height);
    pos3[2] += 0.01;
    glColor3f(1, 1, 1);
    for (int i=0; i<PANEL_MAX_LINES; i++) {
        if (strlen(text[i])) {
            draw_text(pos3, char_height, line_width, line_spacing, text[i]);
        }
        pos3[1] -= line_spacing;
    }
}

void MOVING_TEXT_PANEL::set_text(int lineno, const char* p) {
    char* q = strchr(p, '\n');
    while (p) {
        if (q) *q = 0;
        strlcpy(text[lineno++], p, 256);
        if (!q) break;
        p = q+1;
        q = strchr(p, '\n');
    }
}

void MOVING_TEXT_PANEL::move(double dt) {
    pos[0] = base_pos[0] + sin(theta);
    pos[1] = base_pos[1];
    pos[2] = base_pos[2] + cos(theta);
    theta += dtheta*dt;
}

void MOVING_TEXT_PANEL::get_pos(int lineno, float* p) {
    memcpy(p, pos, sizeof(pos));
    p[0] += margin;
    p[1] += (size[1] - margin - lineno*line_spacing);
}

static int compare_tp(const void* p1, const void* p2) {
    MOVING_TEXT_PANEL* tp1=(MOVING_TEXT_PANEL*)p1, *tp2 = (MOVING_TEXT_PANEL*)p2;
    if (tp1->pos[2] > tp2->pos[2]) return 1;
    if (tp2->pos[2] > tp1->pos[2]) return -1;
    return 0;
}

void MOVING_TEXT_PANEL::sort(MOVING_TEXT_PANEL* tp, int n) {
    qsort(tp, n, sizeof(MOVING_TEXT_PANEL), compare_tp);
}

void PROGRESS::init(
    GLfloat* p, GLfloat l, GLfloat r, GLfloat in, GLfloat* c, GLfloat* ic
) {
    memcpy(pos, p, sizeof(pos));
    len  = l;
    rad = r;
    inner_rad = in;
    memcpy(color, c, sizeof(color));
    memcpy(inner_color, ic, sizeof(inner_color));
}

void PROGRESS::draw(float x) {
    mode_shaded(inner_color);
    drawCylinder(false, pos, x*len, inner_rad);
    mode_shaded(color);
    drawCylinder(false, pos, len, rad);
}

void PROGRESS_2D::init(
	GLfloat* p, GLfloat l, GLfloat w, GLfloat in, GLfloat* c, GLfloat* ic
) {
    memcpy(pos, p, sizeof(pos));
    len  = l;
    width = w;
    inner_width = in;
    memcpy(color, c, sizeof(color));
    memcpy(inner_color, ic, sizeof(inner_color));
}

void PROGRESS_2D::set_pos(float* p) {
    memcpy(pos, p, sizeof(pos));
}

//pos specifies top left of graph
void PROGRESS_2D::draw(float x) {
	glBegin(GL_QUADS);
	glColor4d(color[0],color[1],color[2],color[3]);
	glVertex3d(pos[0],pos[1],pos[2]);
	glVertex3d(pos[0],pos[1]-width,pos[2]);
	glVertex3d(pos[0]+len,pos[1]-width,pos[2]);
	glVertex3d(pos[0]+len,pos[1],pos[2]);
	glEnd();

	float dif=width-inner_width;
	float zoffset=.01;
    glBegin(GL_QUADS);
	glColor4d(inner_color[0],inner_color[1],inner_color[2],inner_color[3]);
	glVertex3d(pos[0],pos[1]-(dif/2.),pos[2]+zoffset);
	glVertex3d(pos[0],pos[1]-(inner_width+dif/2.),pos[2]+zoffset);
	glVertex3d(pos[0]+x*len,pos[1]-(inner_width+dif/2.),pos[2]+zoffset);
	glVertex3d(pos[0]+x*len,pos[1]-(dif/2.),pos[2]+zoffset);
	glEnd();
#if 0
	glColor4f(1,1,1,1);
	glLineWidth(.8f);
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINE_STRIP);
	glVertex3d(pos[0],pos[1],pos[2]);
	glVertex3d(pos[0],pos[1]-width,pos[2]);
	glVertex3d(pos[0]+len,pos[1]-width,pos[2]);
	glVertex3d(pos[0]+len,pos[1],pos[2]);
	glVertex3d(pos[0],pos[1],pos[2]);
	glEnd();
	glDisable(GL_LINE_SMOOTH);
#endif
}


//----------------- RIBBON_GRAPH ---------------------

void RIBBON_GRAPH::init(float* p, float* s, float* c, float* tc, float ty) {
    memcpy(pos, p, sizeof(pos));
    memcpy(size, s, sizeof(size));
    memcpy(color, c, sizeof(color));
    memcpy(tick_color, tc, sizeof(tick_color));
    tick_yfrac = ty;
}

void RIBBON_GRAPH::set_pos(float* p) {
    memcpy(pos, p, sizeof(pos));
}

static float yvec[] = {0., 1., 0.};
static float xvec[] = {1., 0., 0.};
static float xvecneg[] = {-1., 0., 0.};
static float zvec[] = {0, 0, 1};

// draw XZ rect from i to i+1, with height data[i]
//
void RIBBON_GRAPH::draw_x(int i) {
    GLfloat pt[3];
    double r1 = i/(double)len;
    double r2 = (i+1)/(double)len;

    glNormal3fv(yvec);
    pt[0] = pos[0] + r1*size[0];
    pt[1] = pos[1] + data[i]*size[1]/dmax;
    pt[2] = pos[2];
    glVertex3fv(pt);
    pt[0] = pos[0] + r2*size[0];
    glVertex3fv(pt);
    pt[2] = pos[2] + size[2];
    glVertex3fv(pt);
    pt[0] = pos[0] + r1*size[0];
    glVertex3fv(pt);

    // also draw XY rect
    glNormal3fv(zvec);
    pt[0] = pos[0] + r1*size[0];
    pt[1] = pos[1] + data[i]*size[1]/dmax;
    pt[2] = pos[2]+size[2];
    glVertex3fv(pt);
    pt[1] = pos[1];
    glVertex3fv(pt);
    pt[0] = pos[0] + r2*size[0];
    glVertex3fv(pt);
    pt[1] = pos[1] + data[i]*size[1]/dmax;
    glVertex3fv(pt);
}

// draw YZ rect at position i, with height from data[i-1] to data[i]
//
void RIBBON_GRAPH::draw_y(int i) {
    GLfloat pt[3];
    double r1 = i/(double)len;

    (data[i]>data[i-1])?glNormal3fv(xvecneg):glNormal3fv(xvec);
    pt[0] = pos[0] + r1*size[0];
    pt[1] = pos[1] + data[i-1]*size[1]/dmax;
    pt[2] = pos[2];
    glVertex3fv(pt);
    pt[1] = pos[1] + data[i]*size[1]/dmax;
    glVertex3fv(pt);
    pt[2] = pos[2] + size[2];
    glVertex3fv(pt);
    pt[1] = pos[1] + data[i-1]*size[1]/dmax;
    glVertex3fv(pt);
}

void RIBBON_GRAPH::draw_tick(int i) {
    GLfloat pt[3];
    double r1 = ticks[i]/(double)len;

    pt[0] = pos[0] + r1*size[0];
    pt[1] = pos[1] + (1.-tick_yfrac)*size[1];
    pt[2] = pos[2];
    glVertex3fv(pt);
    pt[1] = pos[1] + size[1]*1.1;
    glVertex3fv(pt);
    pt[2] = pos[2] + size[2];
    glVertex3fv(pt);
    pt[1] = pos[1] + (1.-tick_yfrac)*size[1];
    glVertex3fv(pt);
}

void RIBBON_GRAPH::draw(float* d, int ln, bool with_ticks) {
    int i;

    data = d;
    len = ln;
    dmax = 0;
    for (i=0; i<len; i++) {
        if (data[i] > dmax) dmax = data[i];
    }
    if (dmax ==0) dmax = 1;

    mode_shaded(color);
    glBegin(GL_QUADS);
    draw_x(0);
    for (i=1; i<len-1; i++) {
        draw_y(i);
        draw_x(i);
    }
    draw_x(len-1);
    if (with_ticks) {
        mode_shaded(tick_color);
        for (i=0; i<3; i++) {
            draw_tick(i);
        }
    }
    glEnd();
}

void RIBBON_GRAPH::add_tick(float x, int index) {
    ticks[index] = x;
}


void normalize(float a[3]) {
	float mag = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
	if(mag!=0)  {
		a[0]/=mag;
		a[1]/=mag;
		a[2]/=mag;
    } else {
		a[0]=0;
		a[1]=0;
		a[2]=0;
	}
}

float dotProd(float a, float b, float c, float x, float y, float z) {
	return(a*x+b*y+c*z);
}

void crossProd(float a[3], float b[3], float out[3]) {
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];

	normalize(out);
}

// ------------  STARFIELD STUFF --------------------
//
STARFIELD::STARFIELD() {
    stars = NULL;
    zmax = 8;
    zmaxinv = 1/zmax;
}

//
//
void STARFIELD::build_stars(int sz, float sp) {
	int i;

	speed=sp;
	nstars=sz;

    if (stars) free(stars);
    stars = (STAR*)calloc(sizeof(STAR), (long unsigned int)nstars);

	for (i=0; i<nstars; i++) {
		replace_star(i);
	}
}


// draw the starfield,
// move stars
//
void STARFIELD::update_stars(float dt) {
    int i;

    mode_ortho();
    mode_lines();
	glColor4f(1.0, 1.0, 1.0, 1.0);
	for (i=0; i<nstars; i++) {
		stars[i].z -= speed*dt/500;
        if (stars[i].z < 0) stars[i].z += zmax;
        if (stars[i].z > zmax) stars[i].z -= zmax;

        double x = stars[i].x/stars[i].z;
        double y = stars[i].y/stars[i].z;
        x = (x*zmax+1)/2;
        y = (y*zmax+1)/2;

        if (stars[i].z > zmax/2) glPointSize(1);
        else glPointSize(2);
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();
	}
    ortho_done();
}

void STARFIELD::replace_star(int i) {
    stars[i].x = frand()*2-1;
    stars[i].y = frand()*2-1;
    stars[i].z = frand()*zmax;
}

// ------------  TEXTURE STUFF --------------------
//

struct tImageJPG {
	int rowSpan;
	int sizeX;
	int sizeY;
	unsigned char *data;
};

struct Vertex {
    float tu, tv;
    float x, y, z;
};

Vertex g_quadVertices[] = {
    { 0.0f,0.0f, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f, -1.0f, 1.0f, 0.0f }
};
float white[4] = {1., 1., 1., 1.};

// read a PPM file
// to generate PPM from JPEG:
// mogrify -format ppm foo.jpg
// or xv foo.jpg; right click on image, choose PPM
//
int read_ppm_file(const char* name, int& w, int& h, unsigned char** arrayp) {
    FILE* f;
    char buf[256];
    char img_type;
    unsigned char* array;
    int i;

    f = boinc_fopen(name, "rb");
    if (!f) return -1;
    do {fgets(buf, 256, f);} while (buf[0] == '#');
    if (buf[0] != 'P') {
        return -1;
    }
    img_type = buf[1];
    do {fgets(buf, 256, f);} while (buf[0] == '#');
    sscanf(buf, "%d %d", &w, &h);
    do {fgets(buf, 256, f);} while (buf[0] == '#');
    array = (unsigned char*)malloc(w*h*3);
    switch(img_type) {  // TODO: pad image dimension to power of 2
    case '3':
        for (i=0; i<w*h*3; i++) {
            fscanf(f, "%d", array+i);
        }
    case '6':
        fread(array, 3, w*h, f);
        break;
    }
    *arrayp = array;
    return 0;
}


// draw a texture at a given position and size.
// Change size if needed so aspect ratio of texture isn't changed
//
void TEXTURE_DESC::draw(float* p, float* size, int xalign, int yalign) {
    float pos[3];
    double tratio, sratio, new_size;
    memcpy(pos, p, sizeof(pos));
    glColor4f(1.,1.,1.,1.);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);

    tratio = xsize/ysize;
    sratio = size[0]/size[1];

    if (tratio > sratio) {      // texture is wider than space
        new_size = size[0]/tratio;
        if (yalign == ALIGN_CENTER) pos[1] += (size[1]-new_size)/2;
        if (yalign == ALIGN_TOP) pos[1] += size[1]-new_size;
        size[1] = new_size;
    }
    if (sratio > tratio) {      // space is wider than texture
        new_size = size[1]*tratio;
        if (xalign == ALIGN_CENTER) pos[0] += (size[0]-new_size)/2;
        if (xalign == ALIGN_TOP) pos[0] += size[0]-new_size;
        size[0] = new_size;
    }

#if 1
    glBegin(GL_QUADS);
    glTexCoord2f(0., 1.);
    glVertex3fv(pos);
    pos[0] += size[0];
    glTexCoord2f(1., 1.);
    glVertex3fv(pos);
    pos[1] += size[1];
    glTexCoord2f(1., 0.);
    glVertex3fv(pos);
    pos[0] -= size[0];
    glTexCoord2f(0., 0.);
    glVertex3fv(pos);
    glEnd();
#else
    glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
    glDrawArrays( GL_QUADS, 0, 4 );
#endif

    glDisable(GL_TEXTURE_2D);
}

void DecodeJPG(jpeg_decompress_struct* cinfo, tImageJPG *pImageData) {
	jpeg_read_header(cinfo, TRUE);
	jpeg_start_decompress(cinfo);
	int rem = cinfo->output_width%4;
	pImageData->rowSpan = cinfo->output_width * cinfo->output_components;
	pImageData->sizeX   = cinfo->output_width;
	pImageData->sizeY   = cinfo->output_height;

	pImageData->data = new unsigned char[pImageData->rowSpan * pImageData->sizeY];

	unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];
	for (int i = 0; i < pImageData->sizeY; i++)
		rowPtr[i] = &(pImageData->data[i*pImageData->rowSpan]);

	int rowsRead = 0;

	while (cinfo->output_scanline < cinfo->output_height) {
		rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}
	delete [] rowPtr;
	jpeg_finish_decompress(cinfo);
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

tImageJPG *LoadJPG(const char *filename) {
	struct jpeg_decompress_struct cinfo;
	tImageJPG *pImageData = NULL;
	FILE *pFile;

	if((pFile = boinc_fopen(filename, "rb")) == NULL) {
		fprintf(stderr,"Unable to load JPG File!");
		return NULL;
	}

	struct my_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
      jpeg_destroy_decompress(&cinfo);
      fclose(pFile);
      return NULL;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, pFile);
	pImageData = (tImageJPG*)malloc(sizeof(tImageJPG));
	DecodeJPG(&cinfo, pImageData);
	jpeg_destroy_decompress(&cinfo);
	fclose(pFile);
	return pImageData;
}

void printdata(const char* filename, int x, int y, unsigned char* data) {
	FILE* bmpfile = boinc_fopen(filename,"w");
	fprintf(bmpfile,"%i,%i\n",x,y);
	for(int i=0;i<y;i++) {
        for(int c=0;c<8;c++) {
			fprintf(bmpfile,"%d ",data[x*i+c]);
        }
		fprintf(bmpfile,"\n");
	}
	fclose(bmpfile);
}

int TEXTURE_DESC::CreateTextureJPG(const char* strFileName) {
	if(!strFileName) return -1;
	tImageJPG *pImage = LoadJPG(strFileName);			// Load the image and store the data
	if(pImage == NULL) return -1;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, (GLuint*)&id);
	glBindTexture(GL_TEXTURE_2D, id);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pImage->sizeX, pImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pImage->data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    xsize = pImage->sizeX;
    ysize = pImage->sizeY;

	if (pImage) {
		if (pImage->data) {
			free(pImage->data);
		}
		free(pImage);
	}
	return 0;
}

int TEXTURE_DESC::CreateTextureBMP(const char* strFileName) {
#ifdef _WIN32
	DIB_BITMAP image;
    if(image.loadBMP(strFileName) == false) {
		return -1;
    }
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	gluBuild2DMipmaps(GL_TEXTURE_2D, image.get_channels(), image.get_width(),
        image.get_height(), GL_BGR_EXT, GL_UNSIGNED_BYTE,
        image.getLinePtr(0)
    );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    xsize = image.get_width();
    ysize = image.get_height();
#endif
	return 0;
}

int TEXTURE_DESC::CreateTexturePPM(const char* strFileName) {
#ifdef _WIN32
	unsigned char* pixels;
    int width, height, retval;
    retval = read_ppm_file(strFileName, width, height, &pixels);
    if (retval) return retval;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
	gluBuild2DMipmaps(GL_TEXTURE_2D,3,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    xsize = width;
    ysize = height;
#endif
	return 0;
}

int TEXTURE_DESC::CreateTextureTGA(const char* strFileName) {
#ifdef _WIN32
	if(!strFileName)									// Return from the function if no file name was passed in
		return -1;

	tImageTGA *pImage = LoadTGA(strFileName);			// Load the image and store the data
    if(pImage == NULL) {
		return -1;
    }
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	int textureType = GL_RGB;
	if(pImage->channels == 4) {
		textureType = GL_RGBA;
	}
	gluBuild2DMipmaps(GL_TEXTURE_2D, pImage->channels, pImage->sizeX,
	pImage->sizeY, textureType, GL_UNSIGNED_BYTE, pImage->data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    xsize = pImage->sizeX;
    ysize = pImage->sizeY;

    if (pImage)	{									// If we loaded the image
        if (pImage->data) {							// If there is texture data
			delete[] pImage->data;					// Free the texture data, we don't need it anymore
		}
		free(pImage);								// Free the image structure
	}
#endif
	return 0;
}

int TEXTURE_DESC::load_image_file(const char* filename) {
    int retval;
    FILE* f;
    f = boinc_fopen(filename, "r");
    if (!f) goto done;
    fclose(f);

    // for now, just try all the image types in turn

    present = true;
	retval = CreateTextureJPG(filename);
    if (!retval) return 0;
    retval = CreateTexturePPM(filename);
    if (!retval) return 0;
    retval = CreateTextureBMP(filename);
    if (!retval) return 0;
    retval = CreateTextureTGA(filename);
    if (!retval) return 0;

done:
    present = false;
    return -1;
}


//text
unsigned int listBase;

void print_text(const char* string) {
	if(string==NULL) return;
	glPushAttrib(GL_LIST_BIT);
	glListBase(listBase);
	glCallLists((GLsizei)strlen(string), GL_UNSIGNED_BYTE, string);
	glPopAttrib();
}



const char *BOINC_RCSID_12bffca9ae = "$Id$";
