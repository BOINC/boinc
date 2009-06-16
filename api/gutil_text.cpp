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

// The part of BOINC's graphics utilities that uses GLUT char-drawing

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#include "jpeglib.h"
}
#else
#include "jpeglib.h"
#endif
#include "bmplib.h"
#include "tgalib.h"
#endif

#ifndef _WIN32
#ifdef __APPLE__
#undef HAVE_STDLIB_H /* Avoid compiler warning (redefined in jconfig,h) */
#endif
extern "C"{
#include <jpeglib.h>
}
#endif

#include "boinc_gl.h"
#include "boinc_glut.h"

#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"

#include "gutil.h"

#define STROKE_SCALE    120
    // GLUT stroke characters are about 120 units high

GLfloat text_width(const char* text) {
    GLfloat sum = 0;
    for (const char* p = text; *p; p++) {
        sum += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    }
    return sum/STROKE_SCALE;
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
	GLfloat* pos, GLfloat height, GLfloat width, GLfloat /*spacing*/, const char *text,
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
	GLfloat* _pos, GLfloat /*char_height*/, GLfloat /*line_width*/,
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
    GLfloat* _pos, GLfloat /*char_height*/, GLfloat /*line_width*/,
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


MOVING_TEXT_PANEL::MOVING_TEXT_PANEL() : 
	theta(0), dtheta(0), color(0), char_height(0), line_width(0), line_spacing(0), margin(0) 
{
	int i;
	for (i=0;i<3;i++) {
		base_pos[i]=size[i]=0;
	}
	for (i=0;i<PANEL_MAX_LINES;i++) {
		memset(&(text[i][0]),0,256);
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

void MOVING_TEXT_PANEL::set_text(int lineno, const char* str) {
	char buf[8192];
	strcpy(buf, str);
	char* p = buf;
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
    const MOVING_TEXT_PANEL* tp1=(const MOVING_TEXT_PANEL*)p1, *tp2 = (const MOVING_TEXT_PANEL*)p2;
    if (tp1->pos[2] > tp2->pos[2]) return 1;
    if (tp2->pos[2] > tp1->pos[2]) return -1;
    return 0;
}

void MOVING_TEXT_PANEL::sort(MOVING_TEXT_PANEL* tp, int n) {
    qsort(tp, n, sizeof(MOVING_TEXT_PANEL), compare_tp);
}

