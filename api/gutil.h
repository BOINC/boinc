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

// various utility classes for OpenGL programming,
// used in Astropulse and SETI@home
// See also graphics_data.C,h

#ifndef GUTIL_H
#define GUTIL_H

struct COLOR {
    float r;
    float g;
    float b;
    float a;
};

extern void HLStoRGB( double H, double L, double S, COLOR& c);

extern float frand();
extern void set_viewport_full(int w, int h);
extern void set_viewport_fixed(int w, int h);
extern void scale_screen(int w,int h);
extern void center_screen(int w,int h);
extern void center_screen_ratio(int w,int h);


extern void drawSphere(float* pos, float rad);
extern void drawCylinder(bool vertical, float* pos, float len, float rad);

#define TEXT_LEFT       0
#define TEXT_CENTER     1
#define TEXT_RIGHT      2
extern void draw_text_line(
    float* pos, float height, float width, char *text,
    int justify=TEXT_LEFT
);

void draw_text_simple(char* text,float line_width,float char_height);

extern void draw_text(
    float* pos, float height, float width, float spacing, char *text
);

extern void draw_text_new(
    float* pos, float height, float width, float spacing, char *text
);

extern void draw_text_right(
    float* pos, float height, float width, float spacing, char *text
);

extern void draw_text_new_3d(
    float* pos, float height, float width, float spacing, char *text
);

extern void draw_rotated_text(
	float* pos, float height, float width, float spacing, char *text, float rotation, float* rotation_vector
);

extern float text_width(char* text);
extern float text_width_new(char* text);
extern void draw_text_panel(
    float* _pos, float* size, float margin, COLOR color,
    float char_height, float line_width, float line_spacing,
    char* text
);

extern void mode_texture();
extern void mode_ortho();
extern void mode_ortho_ratio();
extern void mode_shaded(float*);
extern void mode_unshaded();
extern void mode_lines();
extern void ortho_done();
extern bool get_matrix_invert(float[16]);
extern bool get_matrix(double src[16]);
extern bool get_projection(double src[16]);
extern bool get_viewport(int view[4]);
extern void get_2d_positions(float p1,float p2,float p3,
	double model[16], double proj[16], int viewport[4], double proj_pos[3]
);

// a progress bar represented as an opaque cylinder within a translucent cylinder
//
class PROGRESS {
    float color[4], inner_color[4];
    float len, rad, inner_rad;
public:
	float pos[3];
    PROGRESS(float* pos, float len, float diam, float inner, float* c, float* ic);
    void draw(float);
};

//2d progress bar
class PROGRESS_2D {
    float color[4], inner_color[4];
    float len, width, inner_width;
	float pos[3];
public:
    void set_pos(float*);
    PROGRESS_2D(float* pos, float len, float width, float inner_width, float* c, float* ic);
    void draw(float);
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
    RIBBON_GRAPH(float* pos, float* size, float* color, float* tick_color, float tick_yfrac=0.2);
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
	float pos[3];
    void init(float* pos, float* size, COLOR& color, double dtheta, double ch, double lw, double ls, double margin);
    void draw();
    void set_text(int lineno, char* t);
    void get_pos(int lineno, float* pos);
    static void sort(MOVING_TEXT_PANEL* tp, int n);
    void move(double dt);
};


// ----- STUFF RELATED TO STARFIELDS
//
#define COS_30 0.8720254037f
#define XY_HEIGHT 1656.85f

struct STAR {
	float x,y,z,v;
};

class STARFIELD {
	float camera[3];
	float eye[3];
	float up[3];
	float right[3];
    float speed;
	float size;
	void replace_star(int);
	bool is_visible(int,double[16],double[16],int[4]);
    STAR* stars;
public:	
    void build_stars(int, float);
    void update_stars(float);
};



// ----- STUFF RELATED TO TEXTURES AND IMAGES

#if 0
#define IMAGE_TYPE_JPG     0
#define IMAGE_TYPE_PPM      1
#define IMAGE_TYPE_BMP      2
#define IMAGE_TYPE_TGA      3
#endif

#define ALIGN_BOTTOM    0
#define ALIGN_CENTER    1
#define ALIGN_TOP       2

struct TEXTURE_DESC {
	bool present;
    unsigned int id;
    double xsize;          // size of underlying image
    double ysize;
    void draw(float* pos, float* size, int xalign, int yalign);
    int load_image_file(char* filename);
    int CreateTextureJPG(char* strFileName);
    int CreateTextureBMP(char* strFileName);
    int CreateTexturePPM(char* strFileName);
    int CreateTextureTGA(char* strFileName);
};

struct tImageJPG {
	int rowSpan;
	int sizeX;
	int sizeY;
	unsigned char *data;
};

extern tImageJPG *LoadJPG(const char *filename);


// ----- STUFF RELATED TO FONTS
//
extern unsigned int listBase;
extern unsigned int MyCreateFont(char *fontName, int Size,int weight);
extern void print_text(char* string);

#endif


