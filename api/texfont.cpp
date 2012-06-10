// Copyright (c) Mark J. Kilgard, 1997.
// This program is freely distributable without licensing fees and is
// provided without guarantee or warrantee expressed or implied.
// This program is -not- in the public domain.

#if defined(_WIN32)
#pragma warning (disable:4244)          // disable bogus conversion warnings
#endif

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
#include <GL/gl.h>
#endif

#ifdef __APPLE_CC__
#include <OpenGL/gl.h>
#endif

using std::tolower;
using std::toupper;
using std::isupper;
using std::islower;
using std::isprint;
using std::strcmp;
using std::strncmp;
using std::memcpy;

using std::FILE;
using std::fprintf;
using std::fopen;
using std::fclose;
using std::fread;

using std::abort;

using std::malloc;
using std::calloc;
using std::free;

#include "texfont.h"

const char *font_names [] =
{
    "Helvetica.txf",                   // 0. This is the default
    "Helvetica-Bold.txf",               // 1
    "Helvetica-Oblique.txf",           // 2
    "Helvetica-BoldOblique.txf",       // 3
    "AvantGarde-Book.txf",             // 4
    "AvantGarde-BookOblique.txf",      // 5
    "AvantGarde-Demi.txf",               // 6
    "AvantGarde-DemiOblique.txf",      // 7
    "Bookman-Light.txf",               // 8
    "Bookman-LightItalic.txf",           // 9
    "Bookman-Demi.txf",                   // 10
    "Bookman-DemiItalic.txf",           // 11
    "Courier.txf",                       // 12
    "Courier-Bold.txf",                   // 13
    "Courier-Oblique.txf",               // 14
    "Courier-BoldOblique.txf",           // 15
    "NewCenturySchlbk-Roman.txf",      // 16
    "NewCenturySchlbk-Italic.txf",       // 17
    "NewCenturySchlbk-Bold.txf",       // 18
    "NewCenturySchlbk-BoldItalic.txf", // 19
    "Palatino-Roman.txf",               // 20
    "Palatino-Italic.txf",               // 21
    "Palatino-Bold.txf",               // 22
    "Palatino-BoldItalic.txf",           // 23
    "Times-Roman.txf",                   // 24
    "Times-Italic.txf",                   // 25
    "Times-Bold.txf",                   // 26
    "Times-BoldItalic.txf",               // 27
    "ZapfChancery-MediumItalic.txf",   // 28
     NULL
} ;


#if 1
// Uncomment to debug various scenarios.
#undef GL_VERSION_1_1
#undef GL_EXT_texture_object
#undef GL_EXT_texture
#endif

#ifndef GL_VERSION_1_1

#  if defined(GL_EXT_texture_object)
#    define glGenTextures glGenTexturesEXT
#    define glBindTexture glBindTextureEXT
#  else
     // Without OpenGL 1.1 or the texture object extension, use display lists.
#    define USE_DISPLAY_LISTS
#  endif

#  if defined(GL_EXT_texture)
#    define GL_INTENSITY4 GL_INTENSITY4_EXT
     int useLuminanceAlpha = 0;
#  else
     // Intensity texture format not in OpenGL 1.0; added by the EXT_texture
     // extension and now part of OpenGL 1.1.
     int useLuminanceAlpha = 1;
#  endif

#else
   // OpenGL 1.1 case.
   int useLuminanceAlpha = 0;
#endif

// byte swap a 32-bit value
#define SWAPL(x, n) { \
                 n = ((char *) (x))[0];\
                 ((char *) (x))[0] = ((char *) (x))[3];\
                 ((char *) (x))[3] = n;\
                 n = ((char *) (x))[1];\
                 ((char *) (x))[1] = ((char *) (x))[2];\
                 ((char *) (x))[2] = n; }

// byte swap a short
#define SWAPS(x, n) { \
                 n = ((char *) (x))[0];\
                 ((char *) (x))[0] = ((char *) (x))[1];\
                 ((char *) (x))[1] = n; }


static TexGlyphVertexInfo * getTCVI(TexFont * txf, int c){
    TexGlyphVertexInfo *tgvi;
    // Automatically substitute uppercase letters with lowercase if not
    // uppercase available (and vice versa).
    if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
        tgvi = txf->lut[c - txf->min_glyph];
        if (tgvi) {
            return tgvi;
        }
        if (islower(c)) {
            c = toupper(c);
            if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
                return txf->lut[c - txf->min_glyph];
            }
        }
        if (isupper(c)) {
            c = tolower(c);
            if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
                return txf->lut[c - txf->min_glyph];
            }
        }
    }
    fprintf(stderr, "texfont: tried to access unavailable font character \"%c\" (%d)\n",isprint(c) ? c : ' ', c);
    abort();
    // NOT REACHED
    return NULL;
}


static const char *lastError;

const char * txfErrorString(void) {
    return lastError;
}


TexFont * txfLoadFont(const char *filename) {
    TexFont *txf;
    FILE *file;
    GLfloat w, h, xstep, ystep;
    char fileid[4], tmp;
    unsigned char *texbitmap;
    int min_glyph, max_glyph;
    int endianness, swap, format, stride, width, height;
    int i, j;
    unsigned long got;
    txf = NULL;
    file = fopen(filename, "rb");
    if (file == NULL) {
        lastError = "file open failed.";
        goto error;
    }
    txf = (TexFont *) malloc(sizeof(TexFont));
    if (txf == NULL) {
        lastError = "out of memory.";
        goto error;
    }
    // For easy cleanup in error case.
    txf->tgi = NULL;
    txf->tgvi = NULL;
    txf->lut = NULL;
    txf->teximage = NULL;
    got = (unsigned long)fread(fileid, 1, 4, file);
    if (got != 4 || strncmp(fileid, "\377txf", 4)) {
        lastError = "not a texture font file.";
        goto error;
    }
    // CONSTANT CONDITION
    assert(sizeof(int) == 4);  // Ensure external file format size.
    got = (unsigned long)fread(&endianness, sizeof(int), 1, file);
    if (got == 1 && endianness == 0x12345678) {
        swap = 0;
    } else if (got == 1 && endianness == 0x78563412) {
        swap = 1;
    } else {
        lastError = "not a texture font file.";
        goto error;
    }
#define EXPECT(n) if (got != (unsigned long) n) { lastError = "premature end of file."; goto error; }
    got = (unsigned long)fread(&format, sizeof(int), 1, file);
    EXPECT(1);
    got = (unsigned long)fread(&txf->tex_width, sizeof(int),  1, file);
    EXPECT(1);
    got = (unsigned long)fread(&txf->tex_height, sizeof(int), 1, file);
    EXPECT(1);
    got = (unsigned long)fread(&txf->max_ascent, sizeof(int), 1, file);
    EXPECT(1);
    got = (unsigned long)fread(&txf->max_descent, sizeof(int),1, file);
    EXPECT(1);
    got = (unsigned long)fread(&txf->num_glyphs, sizeof(int), 1, file);
    EXPECT(1);

    if (swap) {
        SWAPL(&format, tmp);
        SWAPL(&txf->tex_width, tmp);
        SWAPL(&txf->tex_height, tmp);
        SWAPL(&txf->max_ascent, tmp);
        SWAPL(&txf->max_descent,tmp);
        SWAPL(&txf->num_glyphs, tmp);
    }
    txf->tgi = (TexGlyphInfo *) malloc(txf->num_glyphs * sizeof(TexGlyphInfo));
    if (txf->tgi == NULL) {
        lastError = "out of memory.";
        goto error;
    }
    // CONSTANT CONDITION
    assert(sizeof(TexGlyphInfo) == 12);  // Ensure external file format size.
    got = (unsigned long)fread(txf->tgi, sizeof(TexGlyphInfo), txf->num_glyphs, file);
    EXPECT(txf->num_glyphs);

    if (swap) {
        for (i = 0; i < txf->num_glyphs; i++) {
            SWAPS(&txf->tgi[i].c, tmp);
            SWAPS(&txf->tgi[i].x, tmp);
            SWAPS(&txf->tgi[i].y, tmp);
        }
    }
    txf->tgvi = (TexGlyphVertexInfo *)
    malloc(txf->num_glyphs * sizeof(TexGlyphVertexInfo));
    if (txf->tgvi == NULL) {
        lastError = "out of memory.";
        goto error;
    }
    w = txf->tex_width;
    h = txf->tex_height;
    xstep = 0.5 / w;
    ystep = 0.5 / h;
    for (i = 0; i < txf->num_glyphs; i++) {
        TexGlyphInfo *tgi;
        tgi = &txf->tgi[i];
        txf->tgvi[i].t0[0] = tgi->x / w + xstep;
        txf->tgvi[i].t0[1] = tgi->y / h + ystep;
        txf->tgvi[i].v0[0] = tgi->xoffset;
        txf->tgvi[i].v0[1] = tgi->yoffset;
        txf->tgvi[i].t1[0] = (tgi->x + tgi->width) / w + xstep;
        txf->tgvi[i].t1[1] = tgi->y / h + ystep;
        txf->tgvi[i].v1[0] = tgi->xoffset + tgi->width;
        txf->tgvi[i].v1[1] = tgi->yoffset;
        txf->tgvi[i].t2[0] = (tgi->x + tgi->width) / w + xstep;
        txf->tgvi[i].t2[1] = (tgi->y + tgi->height) / h + ystep;
        txf->tgvi[i].v2[0] = tgi->xoffset + tgi->width;
        txf->tgvi[i].v2[1] = tgi->yoffset + tgi->height;
        txf->tgvi[i].t3[0] = tgi->x / w + xstep;
        txf->tgvi[i].t3[1] = (tgi->y + tgi->height) / h + ystep;
        txf->tgvi[i].v3[0] = tgi->xoffset;
        txf->tgvi[i].v3[1] = tgi->yoffset + tgi->height;
        txf->tgvi[i].advance = tgi->advance;
    }

    min_glyph = txf->tgi[0].c;
    max_glyph = txf->tgi[0].c;
    for (i = 1; i < txf->num_glyphs; i++) {
        if (txf->tgi[i].c < min_glyph) {
            min_glyph = txf->tgi[i].c;
        }
        if (txf->tgi[i].c > max_glyph) {
            max_glyph = txf->tgi[i].c;
        }
    }
    txf->min_glyph = min_glyph;
    txf->range = max_glyph - min_glyph + 1;

    txf->lut = (TexGlyphVertexInfo **)
    calloc(txf->range, sizeof(TexGlyphVertexInfo *));
    if (txf->lut == NULL) {
        lastError = "out of memory.";
        goto error;
    }
    for (i = 0; i < txf->num_glyphs; i++) {
        txf->lut[txf->tgi[i].c - txf->min_glyph] = &txf->tgvi[i];
    }

    switch (format) {
    case TXF_FORMAT_BYTE:
        if (useLuminanceAlpha) {
            unsigned char *orig;
            orig = (unsigned char *) malloc(txf->tex_width * txf->tex_height);
            if (orig == NULL) {
                lastError = "out of memory.";
                goto error;
            }
            got = (unsigned long)fread(orig, 1, txf->tex_width * txf->tex_height, file);
            EXPECT(txf->tex_width * txf->tex_height);
            txf->teximage = (unsigned char *) malloc(
                2 * txf->tex_width * txf->tex_height
            );
            if (txf->teximage == NULL) {
                lastError = "out of memory.";
                goto error;
            }
            for (i = 0; i < txf->tex_width * txf->tex_height; i++) {
                txf->teximage[i * 2] = orig[i];
                txf->teximage[i * 2 + 1] = orig[i];
            }
            free(orig);
        } else {
            txf->teximage = (unsigned char *) malloc(
                txf->tex_width * txf->tex_height
            );
            if (txf->teximage == NULL) {
                lastError = "out of memory.";
                goto error;
            }
            got = (unsigned long)fread(txf->teximage, 1, txf->tex_width * txf->tex_height, file);
            EXPECT(txf->tex_width * txf->tex_height);
        }
        break;
    case TXF_FORMAT_BITMAP:
        width = txf->tex_width;
        height = txf->tex_height;
        stride = (width + 7) >> 3;
        texbitmap = (unsigned char *) malloc(stride * height);
        if (texbitmap == NULL) {
            lastError = "out of memory.";
            goto error;
        }
        got = (unsigned long)fread(texbitmap, 1, stride * height, file);
        EXPECT(stride * height);
        if (useLuminanceAlpha) {
            txf->teximage = (unsigned char *) calloc(width * height * 2, 1);
            if (txf->teximage == NULL) {
                lastError = "out of memory.";
                goto error;
            }
            for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                    if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) {
                        txf->teximage[(i * width + j) * 2] = 255;
                        txf->teximage[(i * width + j) * 2 + 1] = 255;
                    }
                }
            }
        } else {
            txf->teximage = (unsigned char *) calloc(width * height, 1);
            if (txf->teximage == NULL) {
                lastError = "out of memory.";
                goto error;
            }
            for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                    if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) {
                        txf->teximage[i * width + j] = 255;
                    }
                }
            }
        }
        free(texbitmap);
        break;
    }
    fclose(file);
    return txf;
error:
    if (txf) {
        if (txf->tgi) free(txf->tgi);
        if (txf->tgvi)free(txf->tgvi);
        if (txf->lut) free(txf->lut);
        if (txf->teximage) free(txf->teximage);
        free(txf);
    }
    if (file) fclose(file);
    return NULL;
}


GLuint CreateTexFont(TexFont * txf,GLuint texobj, GLboolean setupMipmaps){
    glGenTextures(1, &txf->texobj);
    glBindTexture(GL_TEXTURE_2D, txf->texobj);
#if 1
    // XXX Indigo2 IMPACT in IRIX 5.3 and 6.2 does not support the GL_INTENSITY
    // internal texture format. Sigh. Win32 non-GLX users should disable this code.
    if (useLuminanceAlpha == 0) {
        char *vendor, *renderer, *version;
        renderer = (char *) glGetString(GL_RENDERER);
        vendor = (char *) glGetString(GL_VENDOR);
        if (!strcmp(vendor, "SGI") && !strncmp(renderer, "IMPACT", 6)) {
            version = (char *) glGetString(GL_VERSION);
            if (!strcmp(version, "1.0 Irix 6.2") || !strcmp(version, "1.0 Irix 5.3")) {
                unsigned char *latex;
                int width = txf->tex_width;
                int height = txf->tex_height;
                int i;
                useLuminanceAlpha = 1;
                latex = (unsigned char *) calloc(width * height * 2, 1);
                // XXX unprotected alloc.
                for (i = 0; i < height * width; i++) {
                    latex[i * 2] = txf->teximage[i];
                    latex[i * 2 + 1] = txf->teximage[i];
                }
                free(txf->teximage);
                txf->teximage = latex;
            }
        }
    }
#endif
    if (useLuminanceAlpha) {
        if (setupMipmaps) {
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA,
            txf->tex_width, txf->tex_height,
            GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
            txf->tex_width, txf->tex_height, 0,
            GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage);
        }
    }
    else {
#if defined(GL_VERSION_1_1) || defined(GL_EXT_texture)
        // Use GL_INTENSITY4 as internal texture format since we want to use as
        // little texture memory as possible.
        if (setupMipmaps) {
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_INTENSITY4,
            txf->tex_width, txf->tex_height,
            GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY4,
            txf->tex_width, txf->tex_height, 0,
            GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage);
        }
#endif
    }
    return txf->texobj;
}


void txfUnloadFont(TexFont * txf){
    if (txf->teximage) {
        free(txf->teximage);
    }
    free(txf->tgi);
    free(txf->tgvi);
    free(txf->lut);
    free(txf);
}


void txfGetStringMetrics(
    TexFont * txf,
    char *string,
    int len,
    int *width,
    int *max_ascent,
    int *max_descent){
    TexGlyphVertexInfo *tgvi;
    int w, i;
    w = 0;
    for (i = 0; i < len; i++) {
        if (string[i] == 27) {
            switch (string[i + 1]) {
            case 'M': i += 4; break;
            case 'T': i += 7; break;
            case 'L': i += 7; break;
            case 'F': i += 13;break;
            }
        } else {
            tgvi = getTCVI(txf, string[i]);
            w += (int)tgvi->advance;
        }
    }
    *width = w;
    *max_ascent  = txf->max_ascent;
    *max_descent = txf->max_descent;
}


void txfRenderGlyph(TexFont * txf, int c){
    TexGlyphVertexInfo *tgvi;
    tgvi = getTCVI(txf, c);
    glBegin(GL_QUADS);
    glTexCoord2fv(tgvi->t0);
    glVertex2sv(tgvi->v0);
    glTexCoord2fv(tgvi->t1);
    glVertex2sv(tgvi->v1);
    glTexCoord2fv(tgvi->t2);
    glVertex2sv(tgvi->v2);
    glTexCoord2fv(tgvi->t3);
    glVertex2sv(tgvi->v3);
    glEnd();
    glTranslatef(tgvi->advance, 0.0, 0.0);
}


void txfRenderString(TexFont * txf, char *string, int len){
    int i;
    for (i = 0; i < len; i++) {
        if (!isascii(string[i])) {
            string[i] = '?';
        }
        txfRenderGlyph(txf, string[i]);
    }
}


enum {
    MONO, TOP_BOTTOM, LEFT_RIGHT, FOUR
};


void txfRenderFancyString( TexFont * txf, char *string, int len){
    TexGlyphVertexInfo *tgvi;
    GLubyte c[4][3];
    int mode = MONO;
    int i;
    for (i = 0; i < len; i++) {
        if (string[i] == 27) {
            switch (string[i + 1]) {
            case 'M':
                mode = MONO;
                glColor3ubv((GLubyte *) & string[i + 2]);
                i += 4;
                break;
            case 'T':
                mode = TOP_BOTTOM;
                memcpy(c, &string[i + 2], 6);
                i += 7;
                break;
            case 'L':
                mode = LEFT_RIGHT;
                memcpy(c, &string[i + 2], 6);
                i += 7;
                break;
            case 'F':
                mode = FOUR;
                memcpy(c, &string[i + 2], 12);
                i += 13;
                break;
            }
        } else {
            switch (mode) {
            case MONO:
                txfRenderGlyph(txf, string[i]);
                break;
            case TOP_BOTTOM:
                tgvi = getTCVI(txf, string[i]);
                glBegin(GL_QUADS);
                glColor3ubv(c[0]);
                glTexCoord2fv(tgvi->t0);
                glVertex2sv(tgvi->v0);
                glTexCoord2fv(tgvi->t1);
                glVertex2sv(tgvi->v1);
                glColor3ubv(c[1]);
                glTexCoord2fv(tgvi->t2);
                glVertex2sv(tgvi->v2);
                glTexCoord2fv(tgvi->t3);
                glVertex2sv(tgvi->v3);
                glEnd();
                glTranslatef(tgvi->advance, 0.0, 0.0);
                break;
            case LEFT_RIGHT:
                tgvi = getTCVI(txf, string[i]);
                glBegin(GL_QUADS);
                glColor3ubv(c[0]);
                glTexCoord2fv(tgvi->t0);
                glVertex2sv(tgvi->v0);
                glColor3ubv(c[1]);
                glTexCoord2fv(tgvi->t1);
                glVertex2sv(tgvi->v1);
                glColor3ubv(c[1]);
                glTexCoord2fv(tgvi->t2);
                glVertex2sv(tgvi->v2);
                glColor3ubv(c[0]);
                glTexCoord2fv(tgvi->t3);
                glVertex2sv(tgvi->v3);
                glEnd();
                glTranslatef(tgvi->advance, 0.0, 0.0);
                break;
            case FOUR:
                tgvi = getTCVI(txf, string[i]);
                glBegin(GL_QUADS);
                glColor3ubv(c[0]);
                glTexCoord2fv(tgvi->t0);
                glVertex2sv(tgvi->v0);
                glColor3ubv(c[1]);
                glTexCoord2fv(tgvi->t1);
                glVertex2sv(tgvi->v1);
                glColor3ubv(c[2]);
                glTexCoord2fv(tgvi->t2);
                glVertex2sv(tgvi->v2);
                glColor3ubv(c[3]);
                glTexCoord2fv(tgvi->t3);
                glVertex2sv(tgvi->v3);
                glEnd();
                glTranslatef(tgvi->advance, 0.0, 0.0);
                break;
            }
        }
    }
}


int txfInFont(TexFont * txf, int c) {
    // NOTE: No uppercase/lowercase substituion.
    if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
        if (txf->lut[c - txf->min_glyph]) {
            return 1;
        }
    }
    return 0;
}
