#include <math.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "gutil.h"

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