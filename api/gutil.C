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
/* the old way... Use the defines below which should be set in 
 * config.h 
 * #include "gl/gl.h"
 * #include "gl/glu.h"
 * #include "gl/glut.h"
*/
#endif

#ifdef HAVE_GL_H
#include "gl.h"
#elif defined(HAVE_GL_GL_H)
#include <GL/gl.h>
#elif defined(HAVE_OPENGL_GL_H)
#include <OpenGL/gl.h>
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

// the old way
// #ifdef __APPLE_CC__
// #include <OpenGL/gl.h>
// #include <GLUT/glut.h>
// #endif
// #ifdef unix
// #include <GL/gl.h>
// #include <GL/glu.h>
// #include <GL/glut.h>
// #endif

#include "gutil.h"
GLfloat mat_diffuse[] = {0.7, 0.5, 1.0, 0.4};
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
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
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

void mode_ortho()
{	
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

void ortho_done()
{	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

bool get_matrix(float src[16])
{
	glPushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX,src);	
	glPopMatrix();

	return true;
}

bool get_matrix_invert(float src[16])
{
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

/*
	char buf[512];
	sprintf(buf,"%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n",
		dst[0],dst[1],dst[2],dst[3],
		dst[4],dst[5],dst[6],dst[7],
		dst[8],dst[9],dst[10],dst[11],
		dst[12],dst[13],dst[14],dst[15]);
	fprintf(stderr, "%s: modelview\n", buf);
	MessageBox(NULL,buf,"F",0);
*/
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

// draw a rectangle of the given color in the XY plane
// and draw the given test in it
//
void draw_text_panel(
    GLfloat* _pos, GLfloat* size, GLfloat margin, COLOR color,
    GLfloat char_height, GLfloat line_width, GLfloat line_spacing,
    char* text
) {
    GLfloat pos0[3], pos1[3], pos2[3], pos3[3];
    memcpy(pos0, _pos, sizeof(pos0));
    memcpy(pos1, _pos, sizeof(pos0));
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
    color.r /= 2;
    color.g /= 2;
    color.b /= 2;
    glColor4fv(&color.r);
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

void draw_texture(float* p, float* size) {
    float pos[3];
    memcpy(pos, p, sizeof(pos));
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    mode_shaded(white);
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


//star drawing functions -<oliver wang>-
#define STARFIELD_SIZE 1000
#define STAR_SPEED 40.0f
#define PI 3.14159265358979323846264

//pointer to the begining of the list
Star* stars = new Star;

void build_stars()
{
	int i=0;	
	Star* tmpStar = stars;		
	float fov=45.0f;
	while(i<STARFIELD_SIZE)
	{
		float z = (float)(rand()%2000-1000);
		float alpha = 2.0*PI*(float)((rand()%359)/359.0) ;
		float beta = asin(z/1000.0f);
		float x = 1000.0f * cos(beta) * cos(alpha);
		float y = 1000.0f * cos(beta) * sin(alpha);				

		tmpStar->x=x;
		tmpStar->y=y;
		tmpStar->z=z;		
		
		float v = (float)((rand()%1000)/1000.0f);
		tmpStar->v=v;

		tmpStar->next = new Star;		
		tmpStar=tmpStar->next;
		i++;
	}	
	tmpStar->next=NULL;
	tmpStar=NULL;
}


void update_stars()
{
	float modelview[16];	
	float dist;
	float eye[3];
	float camera[3];
	Star* tmpStar = stars;	

	if(get_matrix_invert(modelview)==false)
		fprintf(stderr,"ERROR: 0 determinant in modelview matrix");		
		
	eye[0]=modelview[2];
	eye[1]=modelview[6];
	eye[2]=modelview[10];

	camera[0]=modelview[2];
	camera[1]=modelview[6];
	camera[2]=modelview[10];

	while(tmpStar!=NULL)		
	{	
		dist=sqrt((camera[0]-tmpStar->x)*(camera[0]-tmpStar->x) + 
				  (camera[1]-tmpStar->y)*(camera[1]-tmpStar->y) + 
				  (camera[2]-tmpStar->z)*(camera[2]-tmpStar->z));

		if(tmpStar->z>0)  //replace it if its behind the camera
		{
			replaceStar(tmpStar);
			continue;
		}		

		tmpStar->x+=(eye[0])*tmpStar->v*STAR_SPEED;
		tmpStar->y+=(eye[1])*tmpStar->v*STAR_SPEED;
		tmpStar->z+=(eye[2])*tmpStar->v*STAR_SPEED;
		
		
		//grow objects as the approach you
		if(dist>900) glPointSize(1.0f);
		else if(dist>700) glPointSize(2.0f);
		else if(dist>10) glPointSize(3.0f);				

		GLfloat mat_emission[] = {1, 1, 1, 1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission );

		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_POINTS);
		glVertex3f(tmpStar->x,tmpStar->y,tmpStar->z);
		glEnd();

		GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };	
		glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, no_mat );
		tmpStar=tmpStar->next;		
	}
}

void replaceStar(Star* star)
{
	float z = (float)(rand()%2000-1000);
	float alpha = 2.0*PI*(float)((rand()%359)/359.0) ;
	float beta = asin(z/1000.0f);
	float x = 1000.0f * cos(beta) * cos(alpha);
	float y = 1000.0f * cos(beta) * sin(alpha);				
	star->x=x;
	star->y=y;
	star->z=z;
	star->x=x;
	
	float v = (float)((rand()%1000)/1000.0f);
	star->v=v;		
}


