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

#include <stdio.h>

struct COLOR {
    float r;
    float g;
    float b;
    float a;
};

extern void HLStoRGB( double H, double L, double S, COLOR& c);

extern float frand();

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

extern void draw_rotated_text(
	float* pos, float height, float width, float spacing, char *text, float rotation, float* rotation_vector
);

extern float text_width(char* text);
extern void draw_text_panel(
    float* _pos, float* size, float margin, COLOR color,
    float char_height, float line_width, float line_spacing,
    char* text
);

extern void mode_texture();
extern void mode_ortho();
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

// draw a progress bar as an opaque cylinder within a translucent cylinder
//
class PROGRESS {
    float pos[3];
    float color[4], inner_color[4];
    float len, rad, inner_rad;
public:
    PROGRESS(float* pos, float len, float diam, float inner, float* c, float* ic);
    void draw(float);
};

// draw a graph as a ribbon
//
class GRAPH_2D {
    float pos[3], size[3];
    float color[4], tick_color[4];
    float *data, dmax;
    int len;
    void draw_x(int);
    void draw_y(int);
public:
    GRAPH_2D(float* pos, float* size, float* color, float* tick_color);
    void draw(float* data, int len);
    void add_tick(float x, float yfrac);
};

// ----- STUFF RELATED TO STARFIELDS
//
struct Star {	
	float x,y,z,v;	
	Star* next;	
};

extern void build_stars(int, float);
extern void update_stars(int, float);
extern void replaceStar(Star* tmpStar);


// ----- STUFF RELATED TO TEXTURES AND IMAGES

#define IMAGE_TYPE_JPG     0
#define IMAGE_TYPE_PPM      1
#define IMAGE_TYPE_BMP      2
#define IMAGE_TYPE_TGA      3

// read a portable pixmap file
//
extern int read_ppm(char* name, int& w, int& h, unsigned char** arrayp);
extern int init_texture(char* filename);
extern void draw_texture(float* pos, float* size);

struct tImageJPG {
	int rowSpan;
	int sizeX;
	int sizeY;
	unsigned char *data;
};

typedef unsigned int UINT;

#define MAX_TEXTURES 16
extern UINT g_Texture[MAX_TEXTURES];
extern bool CreateTextureJPG(UINT textureArray[], char* strFileName, int textureID);
extern bool CreateTextureBMP(UINT textureArray[], char* strFileName, int textureID);
extern bool CreateTexturePPM(UINT textureArray[], char* strFileName, int textureID);
extern bool CreateTextureTGA(UINT textureArray[], char* strFileName, int textureID);
extern void create_texture(char* filename, int id);
extern tImageJPG *LoadJPG(const char *filename);


// ----- STUFF RELATED TO FONTS
//
#define MAX_FONTS 16
extern UINT listBase[MAX_FONTS];
extern void MyCreateFont(unsigned int &base, char *fontName, int Size,int weight);
extern void print_text(unsigned int base, char *string);

#endif