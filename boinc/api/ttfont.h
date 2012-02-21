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
// tested using ftgl version 2.1.3 rc5 and freetype version 2.3.9

// ftgl library:  http://sourceforge.net/projects/ftgl/files/FTGL%20Source/
// freetype2 library:  http://www.freetype.org/

// this should basically be a drop-in for the old boinc txf_* functions i.e.
//  txf_load_font and txf_render_string, with extra options on the latter for rotating etc

// adapted by Carl Christensen


#ifndef _TTFONT_H_
#define _TTFONT_H_

#include "qcn_graphics.h"
#include <FTGL/ftgl.h>

// I put in it's own namespace so call TTFont::ttf_load_fonts() etc
namespace TTFont {
        extern FTFont* g_font[NUM_FONT];
        extern int g_iFont;

        extern void ttf_load_fonts(const char* dir = NULL, const char* strScaleFont = NULL, const int& iScaleFont =30);

        extern void ttf_render_string(
           const double& alpha_value,
           // reference value to which incoming alpha values are compared.
           // 0 through to 1
           const double& x,
           const double& y,
           const double& z, // text position
           const float& fscale,                 // scale factor
           const GLfloat * col,                // colour
           const int& iFont,                        // font index
           const char* s,                                         // string ptr
           const float& fRotAngle = 0.0f,        // optional rotation angle
           const float& fRotX = 0.0f,            // optional rotation vector for X
           const float& fRotY = 0.0f,            // optional rotation vector for Y
           const float& fRotZ = 1.0f,            // optional rotation vector for Z
           const float& fRadius = 0.0f           // circular radius to draw along
         );

         void ttf_cleanup();

       GLuint CreateRGBAlpha(const char* strFileName);
       GLuint CreateRGBTransparentTexture(const char* strFileName, float* transColor = NULL);   // default in prototype to transColor = NULL i.e. no "filter color" required

}  // namespace

#endif // inclusion


