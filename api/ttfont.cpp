// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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


// utility functions for TrueType font OpenGL graphics

// FTGL OpenGL TrueType/FreeType font rendering
// you will need to build & link against freetype2 and ftgl
// tested using ftgl version 2.1.3 rc5 and freetype version 2.4.10

// ftgl library:  http://sourceforge.net/projects/ftgl/files/FTGL%20Source/
// freetype2 library:  http://www.freetype.org/

// this should basically be a drop-in for the old boinc txf_* functions i.e.
// txf_load_fonts and txf_render_string,
// with extra options on the latter for rotating etc

// originally adapted by Carl Christensen

//#define TTFONT_DEBUG

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "boinc_gl.h"
#include <FTGL/ftgl.h>

#include "ttfont.h"
#include "filesys.h"  // from boinc for file_exists

// I put in it's own namespace so call TTFont::ttf_load_fonts() etc
//
namespace TTFont {

// The Liberation version 2.00.0 fonts referenced below are free
// fonts under the SIL Open Font License version 1.1.  You can
// download the license and fonts from
// https://fedorahosted.org/liberation-fonts
//
// Another source of free fonts is the GNU FreeFont project
// at http://www.gnu.org/software/freefont

// you'll want to define a 2-d array of char as appropriate for your
// truetype font filenames (path is set in the call to ttf_load_fonts)
//
static const char *g_cstrFont[] = {
    "LiberationSans-Regular.ttf",       // 0, this is the default
    "LiberationSans-Bold.ttf",          // 1
    "LiberationSans-Italic.ttf",        // 2
    "LiberationSans-BoldItalic.ttf",    // 3
    "LiberationSerif-Regular.ttf",      // 4
    "LiberationSerif-Bold.ttf",         // 5
    "LiberationSerif-Italic.ttf",       // 6
    "LiberationSerif-BoldItalic.ttf",   // 7
    "LiberationMono-Regular.ttf",       // 8
    "LiberationMono-Bold.ttf",          // 9
    "LiberationMono-Italic.ttf",        // 10
    "LiberationMono-BoldItalic.ttf"     // 11
};

// define the number of fonts supported
#define NUM_FONT (sizeof(g_cstrFont) / sizeof(char*))

FTFont* g_font[NUM_FONT];
int g_iFont = -1;

// Load fonts. call once.
// Pass in the font directory (normally the project directory:
// APP_INIT_DATA::project_dir for BOINC apps),
// and any special-scaling font name & size
// (e.g. if I want a huge typeface for Sans-Serif Regular
// then I pass in "LiberationSans-Regular.ttf", 3000)
//
void ttf_load_fonts(
    const char* dir, const char* strScaleFont, const int& iScaleFont
) {
    static bool bInit = false;
        // flag so we don't call again, otherwise we'll have memory leaks
        // each subsequent call to new FTTextureFont etc
    if (bInit) return; // we've already been here
    bInit = true; // we're in now!
    ttf_cleanup();
    memset(g_font, 0x00, sizeof(FTFont*) * NUM_FONT);
        // initialize to null's for error checking later
    char vpath[MAXPATHLEN];
    g_iFont = -1;
    for (unsigned int i=0 ; i < NUM_FONT; i++){
        snprintf(vpath, sizeof(vpath), "%s/%s", dir, g_cstrFont[i]);
        if (boinc_file_exists(vpath)) {
            //g_font[i] = new FTBitmapFont(vpath);
            //g_font[i] = new FTPixmapFont(vpath);
            //g_font[i] = new FTPolygonFont(vpath);
            g_font[i] = new FTTextureFont(vpath);
            if(!g_font[i]->Error()) {
#ifdef TTFONT_DEBUG
                fprintf(stderr, "Successfully loaded '%s'...\n", vpath);
#endif
                int iScale = 30;
                if (strScaleFont && !strcmp(strScaleFont, g_cstrFont[i])) iScale = iScaleFont;
                if(!g_font[i]->FaceSize(iScale)) {
                    fprintf(stderr, "Failed to set size");
                }

                g_font[i]->Depth(3.);
                g_font[i]->Outset(-.5f, 1.5f);

                g_font[i]->CharMap(ft_encoding_unicode);
                g_iFont = i;

            }
#ifdef TTFONT_DEBUG
            else {
                fprintf(stderr, "Failed to load '%s'...\n", vpath);
            }
#endif
        }
    }
}

// remove our objects
//
void ttf_cleanup() {
    for (unsigned int i = 0; i < NUM_FONT; i++) {
        if (g_font[i]) {
            delete g_font[i];
            g_font[i] = NULL;
        }
    }
}


void ttf_render_string(
    const double& x,
    const double& y,
    const double& z,                // text position
    const float& fscale,            // scale factor
    const GLfloat* col,             // colour 4vf
    const char* s,                  // string ptr
    const int& iFont,               // font index
    const float& fRotAngle,         // optional rotation angle
    const float& fRotX,             // optional rotation vector for X
    const float& fRotY,             // optional rotation vector for Y
    const float& fRotZ,             // optional rotation vector for Z
    const float& fRadius            // circular radius to draw along
) {
        // http://ftgl.sourceforge.net/docs/html/

    // if requested font isn't available, find first one that is
    //
    unsigned int theFont = iFont;
    while((theFont < NUM_FONT) && !g_font[theFont]) theFont++;
	if((theFont >= NUM_FONT) || !g_font[theFont]) {
        // bad font index
        return;
    }

    int renderMode = FTGL::RENDER_FRONT; //ALL; //FRONT | FTGL::FTGL_RENDER_BACK;

    glColor4fv(col);
    glPushMatrix();

    glTranslated(x, y, z);
    glScaled(1.0f / fscale, 1.0f / fscale, 1.0f / fscale);
    glEnable(GL_TEXTURE_2D);

    if (fRotAngle != 0.0f) {
        glRotatef(fRotAngle, fRotX, fRotY, fRotZ);
    }

    if (fRadius == 0.0f) {
        g_font[theFont]->Render(s, -1, FTPoint(), FTPoint(), renderMode);
    }
    else {
        int i = 0;
        float fAdvance = 1.0f;
        while ( *(s+i) ) {
            fAdvance = g_font[theFont]->Advance((s+i), 1, FTPoint());
            g_font[theFont]->Render((s+i), 1, FTPoint(), FTPoint(), renderMode);
            glTranslated(fAdvance, 0.0f, 0.0f);
            glRotatef(fRadius * fAdvance / (float) 20.0f, 0.0f, 0.0f, 1.0f);
            i++;
        }
    }

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

}   // namespace


