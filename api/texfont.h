// Copyright (c) Mark J. Kilgard, 1997. 
// This program is freely distributable without licensing fees  and is
// provided without guarantee or warrantee expressed or  implied. 
// This program is -not- in the public domain. 

#ifndef __TEXFONT_H__
#define __TEXFONT_H__

#include <GL/gl.h>

#define TXF_FORMAT_BYTE		0
#define TXF_FORMAT_BITMAP	1

#define TXF_NUM_FONT	   28

static const char *font_names [] = 
{
	"Helvetica.txf",                   // 0. This is the default 
	"Helvetica-Bold.txf",		       // 1
	"Helvetica-Oblique.txf",	       // 2
	"Helvetica-BoldOblique.txf",       // 3
	"AvantGarde-Book.txf",             // 4
	"AvantGarde-BookOblique.txf",      // 5
	"AvantGarde-Demi.txf",		       // 6
	"AvantGarde-DemiOblique.txf",      // 7
	"Bookman-Light.txf",		       // 8
	"Bookman-LightItalic.txf",	       // 9
	"Bookman-Demi.txf",			       // 10
	"Bookman-DemiItalic.txf",	       // 11
	"Courier.txf",				       // 12
	"Courier-Bold.txf",			       // 13
	"Courier-Oblique.txf",		       // 14
	"Courier-BoldOblique.txf",	       // 15
	"NewCenturySchlbk-Roman.txf",      // 16
	"NewCenturySchlbk-Italic.txf",	   // 17
	"NewCenturySchlbk-Bold.txf",       // 18
	"NewCenturySchlbk-BoldItalic.txf", // 19
	"Palatino-Roman.txf",		       // 20
	"Palatino-Italic.txf",		       // 21
	"Palatino-Bold.txf",		       // 22
	"Palatino-BoldItalic.txf",	       // 23
	"Times-Roman.txf",			       // 24
	"Times-Italic.txf",			       // 25
	"Times-Bold.txf",			       // 26
	"Times-BoldItalic.txf",		       // 27
	"ZapfChancery-MediumItalic.txf",   // 28
	 NULL
} ;


typedef struct {
	unsigned short c;       // Potentially support 16-bit glyphs. 
	unsigned char width;
	unsigned char height;
	signed char xoffset;
	signed char yoffset;
	signed char advance;
	char dummy;             // Space holder for alignment reasons.
	short x;
	short y;
} TexGlyphInfo;


typedef struct {
	GLfloat t0[2];
	GLshort v0[2];
	GLfloat t1[2];
	GLshort v1[2];
	GLfloat t2[2];
	GLshort v2[2];
	GLfloat t3[2];
	GLshort v3[2];
	GLfloat advance;
} TexGlyphVertexInfo;


typedef struct {
	GLuint texobj;
	int tex_width;
	int tex_height;
	int max_ascent;
	int max_descent;
	int num_glyphs;
	int min_glyph;
	int range;
	unsigned char *teximage;
	TexGlyphInfo *tgi;
	TexGlyphVertexInfo *tgvi;
	TexGlyphVertexInfo **lut;
} TexFont;

extern char *txfErrorString(void);

extern TexFont *txfLoadFont(const char *filename);

extern void txfUnloadFont(TexFont * txf);

extern GLuint CreateTexFont(
	TexFont * txf,
	GLuint texobj,
	GLboolean setupMipmaps);

extern void txfBindFontTexture(TexFont * txf);

extern void txfGetStringMetrics(
	TexFont * txf,
	char *string,
	int len,
	int *width,
	int *max_ascent,
	int *max_descent);

extern void txfRenderGlyph(
	TexFont * txf,
	int c);

extern void txfRenderString(
	TexFont * txf,
	char *string,
	int len);

extern void txfRenderFancyString(
	TexFont * txf,
	char *string,
	int len);

#endif // __TEXFONT_H__ 
