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

// various utility classes for OpenGL programming,
// used in Astropulse and SETI@home
// See also graphics_data.C,h

#ifndef GUTIL_H
#define GUTIL_H

#include <cstdio>
#include <cstdlib>
#include <cstring>

struct COLOR {
    float r;
    float g;
    float b;
    float a;
	COLOR(float rr=0,float gg=0, float bb=0, float aa=0) : r(rr),g(gg),b(bb),a(aa) {};
};

extern void HLStoRGB( double H, double L, double S, COLOR& c);

extern void scale_screen(int w,int h);
extern void center_screen(int w,int h);

extern void drawSphere(float* pos, float rad);
extern void drawCylinder(bool vertical, float* pos, float len, float rad);

#define TEXT_LEFT       0
#define TEXT_CENTER     1
#define TEXT_RIGHT      2
extern void draw_text_line(
    float* pos, float height, float width, const char *text,
    int justify=TEXT_LEFT
);

void draw_text_simple(const char* text,float line_width,float char_height);

extern void draw_text(
    float* pos, float height, float width, float spacing, const char *text
);

extern void draw_text_new(
    float* pos, float height, float width, float spacing, const char *text
);

extern void draw_text_right(
    float* pos, float height, float width, float spacing, const char *text
);

extern void draw_text_new_3d(
    float* pos, float height, float width, float spacing, const char *text
);

extern void draw_rotated_text(
    float* pos, float height, float width, float spacing, const char *text, float rotation, float* rotation_vector
);

extern float text_width(const char* text);
extern float text_width_new(const char* text);
extern void draw_text_panel(
    float* _pos, float* size, float margin, COLOR color,
    float char_height, float line_width, float line_spacing,
    const char* text
);

extern void mode_texture();
extern void mode_ortho();
extern void mode_shaded(float*);
extern void mode_unshaded();
extern void mode_lines();
extern void ortho_done();
extern bool get_matrix(double src[16]);
extern bool get_projection(double src[16]);
extern bool get_viewport(int view[4]);
extern void get_2d_positions(double p1, double p2, double p3,
    double model[16], double proj[16], int viewport[4], double proj_pos[3]
);

// a progress bar represented as an opaque cylinder within a translucent cylinder
//
class PROGRESS {
    float color[4], inner_color[4];
    float len, rad, inner_rad;
public:
    float pos[3];
    void init(float* pos, float len, float diam, float inner, float* c, float* ic);
    void draw(float);
	PROGRESS() : len(0), rad(0), inner_rad(0) {
		memset(color,0,sizeof(color));
		memset(inner_color,0,sizeof(inner_color));
		memset(pos,0,sizeof(pos));
	};
};

//2d progress bar
class PROGRESS_2D {
    float color[4], inner_color[4];
    float len, width, inner_width;
    float pos[3];
public:
    void set_pos(float*);
    void init(float* pos, float len, float width, float inner_width, float* c, float* ic);
    void draw(float);
	PROGRESS_2D() : len(0), width(0), inner_width(0) {
		memset(color,0,sizeof(color));
		memset(inner_color,0,sizeof(inner_color));
		memset(pos,0,sizeof(pos));
	};
};

// a graph of a function of 1 variable drawn as a ribbon in 3D
//
class RIBBON_GRAPH {
    float size[3];
    float color[4], tick_color[4];
    float *data, dmax;
    float tick_yfrac;
    float ticks[3];
    int len;
    void draw_x(int);
    void draw_y(int);
    void draw_tick(int i);
    float pos[3];
public:
    void set_pos(float*);
    void init(float* pos, float* size, float* color, float* tick_color, float tick_yfrac=0.2);
    void draw(float* data, int len, bool with_ticks=false);
    void add_tick(float x, int index);
};

#define PANEL_MAX_LINES  10
// a colored panel with some text, that can move cyclically
//
class MOVING_TEXT_PANEL {
    float base_pos[3];
    float theta;
    float dtheta;
    COLOR color;
    float char_height;
    float line_width;
    float line_spacing;
    float size[3];
    double margin;
    char text[PANEL_MAX_LINES][256];
public:
	MOVING_TEXT_PANEL();
    float pos[3];
    void init(float* pos, float* size, COLOR& color, double dtheta, double ch, double lw, double ls, double margin);
    void draw();
    void set_text(int lineno, const char* t);
    void get_pos(int lineno, float* pos);
    static void sort(MOVING_TEXT_PANEL* tp, int n);
    void move(double dt);
};


// ----- STARFIELDS
//
// stars have their own coord system
// x = -1..1
// y = -1..1
// z = 0..zmax
// the projection plane is z=1,
// and the square +- 1/zmax is mapped to the window
//

struct STAR {
    double x, y, z;
};

class STARFIELD {
    double zmax, zmaxinv;
    double speed;
    int nstars;
    void replace_star(int);
    STAR* stars;
public:
    STARFIELD();
    void build_stars(int, float);
    void update_stars(float);
};



// ----- TEXTURES AND IMAGES

#define ALIGN_BOTTOM    0
#define ALIGN_CENTER    1
#define ALIGN_TOP       2

unsigned int* read_rgb_texture (const char *, int*, int* ,int *);

struct TEXTURE_DESC {
    bool present;
    unsigned int id;
    double xsize;          // size of underlying image
    double ysize;
	TEXTURE_DESC() : present(false),id(0),xsize(0),ysize(0) {};
    void draw(float* pos, float* size, int xalign, int yalign, float alpha=1.);
    int load_image_file(const char* filename);
    int CreateTextureJPG(const char* strFileName);
    int CreateTextureBMP(const char* strFileName);
    int CreateTexturePPM(const char* strFileName);
    int CreateTextureTGA(const char* strFileName);
	int CreateTextureRGB(const char* strFileName);
};


// ----- FONTS
//
extern unsigned int listBase;
extern unsigned int MyCreateFont(const char *fontName, int Size,int weight);
extern void print_text(const char* string);

#endif


