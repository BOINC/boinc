// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#include "jpeglib.h"
#include "bmplib.h"
#include "tgalib.h"
#else
#include <jpeglib.h>
#endif

#ifdef HAVE_GL_H
#include "gl.h"
#elif defined(HAVE_GL_GL_H)
#include <GL/gl.h>
#elif defined(HAVE_OPENGL_GL_H)
#include <OpenGL/gl.h>
#else
#endif

#ifdef HAVE_GLU_H
#include "glu.h"
#elif defined(HAVE_GL_GLU_H)
#include <GL/glu.h>
#elif defined(HAVE_OPENGL_GLU_H)
#include <OpenGL/glu.h>
#endif

#ifdef HAVE_GLUT_H
#include "glut.h"
#elif defined(HAVE_GL_GLUT_H)
#include <GL/glut.h>
#elif defined(HAVE_OPENGL_GLUT_H)
#include <OpenGL/glut.h>
#elif defined(HAVE_GLUT_GLUT_H)
#include <GLUT/glut.h>
#endif

#include "gutil.h"

GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_shininess[] = {40.0};

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

void mode_texture() {
#if 0
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
#endif
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

void mode_unshaded() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel (GL_SMOOTH);
    glDepthMask(GL_TRUE);
}

void mode_ortho() {	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0,1,0,1);
	//glScalef(1, -1, 1);	
	//glTranslatef(0, -1, 0);
	
    glMatrixMode(GL_MODELVIEW);	
	glPushMatrix();
    glLoadIdentity();
    gluLookAt(0.0,0.0,1.0,  // eye position
			  0,0,0,      // where we're looking
			  0.0, 1.0, 0.);      // up is in positive Y direction		
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
	glGetDoublev(GL_MODELVIEW_MATRIX,src);	
	glPopMatrix();

	return true;
}

bool get_projection(double src[16]) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glGetDoublev(GL_PROJECTION_MATRIX,src);
	glPopMatrix();
	return true;
}

bool get_viewport(int view[4]) {
	glMatrixMode(GL_MODELVIEW);
	glGetIntegerv(GL_VIEWPORT,view);
	return true;
}

void get_2d_positions(float p1,float p2,float p3,
	double model[16], double proj[16], int viewport[4], double proj_pos[3]
) {
	gluProject(p1,p2,p3,model,proj,viewport,&proj_pos[0],&proj_pos[1],&proj_pos[2]);	
}

bool get_matrix_invert(float src[16]) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX,src);	
	glPopMatrix();

	float tmp[12]; /* temp array for pairs */	
	float dst[16]; /* array of destination matrix */
	float det; /* determinant */

	int i;

	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];

	/* calculate first 8 elements (cofactors) */
	dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];

	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];
	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	/* calculate second 8 elements (cofactors) */
	dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];

	/* calculate determinant */
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

	/* calculate matrix inverse */
	if(det == 0) return false;
	det = 1.0f/det;
	for (i = 0; i < 16; i++) {
	 dst[i] *= det;
	}

	for (i = 0; i < 4; i++) {
	 src[i*4] = dst[i*4];
	 src[i*4+1] = dst[i*4+1];
	 src[i*4+2] = dst[i*4+2];
	 src[i*4+3] = dst[i*4+3];
	}

	return true;
}

void mode_lines() {
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    //glDepthMask(GL_TRUE);
    //glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH, GL_NICEST);
    //glDisable(GL_DEPTH_TEST);
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
     if(L <=0.5)
        m2 = L*(1.0+S);
     else
        m2 = L+S-L*S;
     m1 = 2.0*L-m2;
     c.r = HuetoRGB(m1,m2,H+1.0/3.0);
     c.g = HuetoRGB(m1,m2,H);
     c.b = HuetoRGB(m1,m2,H-1.0/3.0);
  }
}

float frand() {
    return rand()/(float)RAND_MAX;
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

GLfloat text_width(char* text) {
    GLfloat sum = 0;
    char* p;

    for (p=text; *p; p++) {
        sum += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    }
    return sum/STROKE_SCALE;
}

GLfloat text_width_new(char* text) {
	GLfloat sum=0;
	char* p;
	float w=.01; //FIND OUT WHAT IT IS
	for(p=text;*p;p++) {
		sum += w;
	}
	return sum;
}

static void draw_text_line_aux(char *text) {
    char *p;
    for (p = text; *p; p++) {
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
//

void draw_text_simple(char* text,float line_width,float char_height)
{
    glLineWidth(line_width);    
    float w = char_height/STROKE_SCALE;
    glScalef(w, w, w);	
	draw_text_line_aux(text);
}

void draw_text_line(
    GLfloat* _pos, GLfloat char_height, GLfloat line_width, char *text,
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
	GLfloat* pos, GLfloat height, GLfloat width, GLfloat spacing, char *text, 
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
    GLfloat line_spacing, char* text
) {
    char* q, *p;
    char buf[4096];
    GLfloat pos[3];
    memcpy(pos, _pos, sizeof(pos));
    strcpy(buf, text);

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
    GLfloat line_spacing, char* text)
{
	char* q, *p;
    char buf[4096];
    GLfloat pos[3];
    memcpy(pos, _pos, sizeof(pos));
    strcpy(buf, text);

    p = buf;
	glPushMatrix();	
    glTranslatef(pos[0], pos[1], pos[2]);
    while (*p) {
        q = strchr(p, '\n');
        if (q) *q = 0;
//		glRasterPos3d(pos[0],pos[1],pos[2]);    
		print_text(listBase[0], p);
        pos[1] -= line_spacing;
        if (!q) break;		
        p = q+1;
    }
	glPopMatrix();
}

void draw_text_new(
    GLfloat* _pos, GLfloat char_height, GLfloat line_width,
    GLfloat line_spacing, char* text)
{
	char *q, *p;
	char buf[4096];
	GLfloat pos[3];
	memcpy(pos,_pos,sizeof(pos));
	strcpy(buf,text);
	p=buf;

	while(*p)
	{
		q = strchr(p, '\n');
        if (q) *q = 0;
		glRasterPos3d(pos[0],pos[1],pos[2]);
		print_text(listBase[0], p);
        pos[1] -= line_spacing;
        if (!q) break;
        p = q+1;
	}
}

void draw_text_new_right(
    GLfloat* _pos, GLfloat char_height, GLfloat line_width,
    GLfloat line_spacing, char* text)
{
	char *q, *p;
	char buf[4096];
	GLfloat pos[3];
	memcpy(pos,_pos,sizeof(pos));
	strcpy(buf,text);
	p=buf;
	float l;

	while(*p)
	{
		q = strchr(p, '\n');
        if (q) *q = 0;
		l=text_width_new(p);
		glRasterPos3d(pos[0]-l,pos[1],pos[2]);
		print_text(listBase[0], p);
        pos[1] -= line_spacing;
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
    strcpy(text, "");
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
    draw_text(pos3, char_height, line_width, line_spacing, text);
}

void MOVING_TEXT_PANEL::move(double dt) {
    pos[0] = base_pos[0] + sin(theta);
    pos[1] = base_pos[1];
    pos[2] = base_pos[2] + cos(theta);
    theta += dtheta*dt;
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

PROGRESS::PROGRESS(
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

PROGRESS_2D::PROGRESS_2D(
	GLfloat* p, GLfloat l, GLfloat w, GLfloat in, GLfloat* c, GLfloat* ic
) {
    memcpy(pos, p, sizeof(pos));
    len  = l;
    width = w;
    inner_width = in;
    memcpy(color, c, sizeof(color));
    memcpy(inner_color, ic, sizeof(inner_color));
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
    glBegin(GL_QUADS);
	glColor4d(inner_color[0],inner_color[1],inner_color[2],inner_color[3]);
	glVertex3d(pos[0],pos[1]-(dif/2.),pos[2]);
	glVertex3d(pos[0],pos[1]-(inner_width+dif/2.),pos[2]);
	glVertex3d(pos[0]+x*len,pos[1]-(inner_width+dif/2.),pos[2]);
	glVertex3d(pos[0]+x*len,pos[1]-(dif/2.),pos[2]);
	glEnd();
}

GRAPH_2D::GRAPH_2D(float* p, float* s, float* c, float* tc) {
    memcpy(pos, p, sizeof(pos));
    memcpy(size, s, sizeof(size));
    memcpy(color, c, sizeof(color));
    memcpy(tick_color, tc, sizeof(tick_color));
}

float yvec[] = {0., 1., 0.};
float xvec[] = {1., 0., 0.};
float xvecneg[] = {-1., 0., 0.};
float zvec[] = {0, 0, 1};

// draw horizontal plate from i to i+1, with height data[i]
//
void GRAPH_2D::draw_x(int i) {
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

// draw vertical plate at position i, with height from data[i-1] to data[i]
//
void GRAPH_2D::draw_y(int i) {
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

void GRAPH_2D::draw(float* d, int ln) {
    int i;

    data = d;
    len = ln;
    dmax = 0;
    for (i=0; i<len; i++) {
        if (data[i] > dmax) dmax = data[i];
    }

    mode_shaded(color);
    glBegin(GL_QUADS);
    draw_x(0);
    for (i=1; i<len-1; i++) {
        draw_y(i);
        draw_x(i);
    }
    draw_x(len-1);
    glEnd();
}

void GRAPH_2D::add_tick(float x, float yfrac) {
}

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
    { 0.0f,0.0f, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f, -1.0f, 1.0f, 0.0f }
};
float white[4] = {1., 1., 1., 1.};


//star drawing functions -<oliver wang>-
Star stars[MAX_STARFIELD_SIZE];

//makes a list of stars that lie on cocentric circles (inefficient, most will be out of sight)
void build_stars(int size, float speed)
{
	float modelview[16];
	float eye[3];
	float camera[3];
	
	if(get_matrix_invert(modelview)==false)
		fprintf(stderr,"ERROR: 0 determinant in modelview matrix");			

	eye[0]=modelview[2];
	eye[1]=modelview[6];
	eye[2]=modelview[10];

	camera[0]=modelview[2];
	camera[1]=modelview[6];
	camera[2]=modelview[10];

	int i=0;	
	float fov=45.0f;
	for(i=0;i<size;i++)
	{		
#if 0
		float z = -frand()*2000;		
		float x = 2.0f*-z*TAN22_5*frand();		
		float y = 2.0f*-z*TAN22_5*frand();
		
		x-=-z*TAN22_5;
		y-=-z*TAN22_5;
		
		stars[i].x=x;
		stars[i].y=y;
		stars[i].z=z;		
#endif
		//old version
		float z = -frand()*1000;
		float alpha = 2.0*PI*(float)((rand()%359)/359.0) ;
		float beta = asin(z/1000.0f);
		float x = 1000.0f * cos(beta) * cos(alpha);
		float y = 1000.0f * cos(beta) * sin(alpha);				
		stars[i].x=x;
		stars[i].y=y;
		stars[i].z=z;
	
		float v = frand();
		stars[i].v=v;
	}	
}

//moves stars towards the eye vector, and replaces ones that go behind z=0

float dotProd(float a, float b, float c, float x, float y, float z)
{
	return(a*x+b*y+c*z);
}

void update_stars(int size, float speed, float dt)
{
	float modelview[16];

	float dist;
	float eye[3];
	float camera[3];
	
	if(get_matrix_invert(modelview)==false)
		fprintf(stderr,"ERROR: 0 determinant in modelview matrix");			

	eye[0]=modelview[2];
	eye[1]=modelview[6];
	eye[2]=modelview[10];

	camera[0]=modelview[2];
	camera[1]=modelview[6];
	camera[2]=modelview[10];

	GLfloat mat_emission[] = {1, 1, 1, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission );

	glColor3f(1.0, 1.0, 1.0);

	for(int i=0;i<size;i++)
	{	
		dist=sqrt((camera[0]-stars[i].x)*(camera[0]-stars[i].x) + 
				  (camera[1]-stars[i].y)*(camera[1]-stars[i].y) + 
				  (camera[2]-stars[i].z)*(camera[2]-stars[i].z));

		
		if(dotProd(eye[0],eye[1],eye[2],stars[i].x,stars[i].y,stars[i].z)>0) // behind camera
		{
//			replaceStar(i);
			continue;
		}		
		
		stars[i].x+=(eye[0])*stars[i].v*speed*dt;
		stars[i].y+=(eye[1])*stars[i].v*speed*dt;
		stars[i].z+=(eye[2])*stars[i].v*speed*dt;

		
		//grow objects as the approach you
		if(dist>900) glPointSize(1.0f);
		else if(dist>600) glPointSize(2.0f);
		
		glBegin(GL_POINTS);
		glVertex3f(stars[i].x,stars[i].y,stars[i].z);	
		glEnd();
	}	

	GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };	
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, no_mat );		
}

void replaceStar(int i) {
	float z = -frand()*1000;
	float alpha = 2.0*PI*(float)((rand()%359)/359.0) ;
	float beta = asin(z/1000.0f);
	float x = 1000.0f * cos(beta) * cos(alpha);
	float y = 1000.0f * cos(beta) * sin(alpha);				
	stars[i].x=x;
	stars[i].y=y;
	stars[i].z=z;	
	
	float v = (float)((rand()%1000)/1000.0f);
	stars[i].v=v;		
}

// ------------ OLD TEXTURE STUFF --------------------
// read a PPM file
// to generate PPM from JPEG:
// mogrify -format ppm foo.jpg
// or xv foo.jpg; right click on image, choose PPM
//
int read_ppm_file(char* name, int& w, int& h, unsigned char** arrayp) {
    FILE* f;
    char buf[256];
    char img_type;
    unsigned char* array;
    int i;

    f = fopen(name, "rb");
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

#if 0
unsigned int texture_id;

int init_texture(char* filename) {
    unsigned char* pixels;
    int width, height, retVal;
    int err;
    retVal = read_ppm_file(filename, width, height, &pixels);
    if (retVal) return retVal;
    glGenTextures(1, &texture_id);
    err = glGetError();
    if (err) return err;
    glBindTexture(GL_TEXTURE_2D, texture_id);
    err = glGetError();
    if (err) return err;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    err = glGetError();
    if (err) return err;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    err = glGetError();
    if (err) return err;
	//ASSERT(0);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        3,
        //0,
        //0,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        pixels    // dimension of PPM file MUST be power of 2
    );
    err = glGetError();
    if (err) {
		fprintf(stderr, "glTexImage2D returned error # %d: %s\n", err, gluErrorString(err));
		return err;
	}
    return 0;
}
#endif

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

////  --------------- NEW TEXTURE STUFF ----------------------

void DecodeJPG(jpeg_decompress_struct* cinfo, tImageJPG *pImageData) {	
	jpeg_read_header(cinfo, TRUE);
	jpeg_start_decompress(cinfo);

	pImageData->rowSpan = cinfo->image_width * cinfo->num_components;
	//pImageData->rowSpan = cinfo->output_width * cinfo->output_components;
	pImageData->sizeX   = cinfo->image_width;
	pImageData->sizeY   = cinfo->image_height;
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

tImageJPG *LoadJPG(const char *filename) {
	struct jpeg_decompress_struct cinfo;
	tImageJPG *pImageData = NULL;
	FILE *pFile;
	
	if((pFile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr,"Unable to load JPG File!");
		return NULL;
	}
		
	jpeg_error_mgr jerr;	
	cinfo.err = jpeg_std_error(&jerr);	
	jpeg_create_decompress(&cinfo);	
	jpeg_stdio_src(&cinfo, pFile);	
	pImageData = (tImageJPG*)malloc(sizeof(tImageJPG));	
	DecodeJPG(&cinfo, pImageData);	
	jpeg_destroy_decompress(&cinfo);	
	fclose(pFile);	
	return pImageData;
}

int TEXTURE_DESC::CreateTextureJPG(char* strFileName) {
	if(!strFileName) return -1;	
	tImageJPG *pImage = LoadJPG(strFileName);			// Load the image and store the data
	if(pImage == NULL) return -1;
	glGenTextures(1, &id);
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

int TEXTURE_DESC::CreateTextureBMP(char* strFileName) {
#ifdef _WIN32
	DIB_BITMAP image; 
    if(image.loadBMP(strFileName) == false) {
		return -1;
    }

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

int TEXTURE_DESC::CreateTexturePPM(char* strFileName) {
#ifdef _WIN32
	unsigned char* pixels;
    int width, height, retval;    
    retval = read_ppm_file(strFileName, width, height, &pixels);
    if (retval) return retval;
    
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

int TEXTURE_DESC::CreateTextureTGA(char* strFileName) {
#ifdef _WIN32
	if(!strFileName)									// Return from the function if no file name was passed in
		return -1;

	tImageTGA *pImage = LoadTGA(strFileName);			// Load the image and store the data
    if(pImage == NULL) {
		return -1;
    }
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

#if 0
static int getFileType(char* file) {
    int l = strlen(file);
    char f2[64];
	int i;
        
    for(i=l-4;i<l;i++) {
        f2[i-(l-4)]=file[i];
    }

    //MessageBox(NULL,f2,"D",0);    
    
    for(i=0;i<4;i++) {
        f2[i]=tolower(f2[i]);
    }

    if(strncmp(f2,".jpg",4)==0) return IMAGE_TYPE_JPG;
    else if(strncmp(f2,".ppm",4)==0) return IMAGE_TYPE_PPM;
    else if(strncmp(f2,".bmp",4)==0) return IMAGE_TYPE_BMP;
    else if(strncmp(f2,".tga",4)==0) return IMAGE_TYPE_TGA;

    else return -1;
}
#endif

int TEXTURE_DESC::load_image_file(char* filename) {
    int retval;
    FILE* f;
    f = fopen(filename, "r");
    if (!f) goto done;
    fclose(f);

    // for now, just try all the image types in turn

    present = true;
    retval = CreateTexturePPM(filename);    
    if (!retval) return 0;
    retval = CreateTextureJPG(filename);
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
unsigned int listBase[MAX_FONTS];

void print_text(unsigned int base, char *string)
{   
   if((base == 0 || string == NULL))
      return;

   glPushAttrib(GL_LIST_BIT);
   glListBase(base - 32);
   glCallLists(strlen(string), GL_UNSIGNED_BYTE, string);
   glPopAttrib();
}
