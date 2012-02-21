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

// Interface functions for tex_info stuff.
// Contributed by Tolu Aina

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include <cstring>
#include "filesys.h"
#include "boinc_api.h"

#include "texfont.h"
#include "txf_util.h"

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif 

static TexFont* txf[TXF_NUM_FONT];

// load fonts. call once.
//
void txf_load_fonts(char* dir) {
    char vpath[_MAX_PATH], phys_path[_MAX_PATH];
    for (int i=0 ; i<TXF_NUM_FONT; i++){
		sprintf(vpath, "%s/%s", dir, font_names[i]);
        boinc_resolve_filename(vpath, phys_path, sizeof(phys_path));
		if (is_file(phys_path)) {
			txf[i] = txfLoadFont(phys_path);
            if(txf[i]) {
                fprintf(stderr, "Successfully loaded '%s'...\n", phys_path);
                CreateTexFont(txf[i], 0, GL_TRUE);
            } else {
                fprintf(stderr, "Failed to load '%s' error message: '%s'...\n", phys_path, txfErrorString());
            }
		}
    }	
}


void txf_render_string(
	float alpha_value,
        // reference value to which incoming alpha values are compared.
        // 0 through to 1
	double x, double y, double z, // text position
	float fscale,                 // scale factor
	GLfloat * col,                // colour 
	int i,                        // font index see texfont.h 
	char * s,				  	  // string ptr
	float fRotAngle,        // optional rotation angle
	float fRotX,            // optional rotation vector for X
	float fRotY,            // optional rotation vector for Y
	float fRotZ            // optional rotation vector for Z
){
    // if requested font isn't available, find first one that is
    //
    while((i < TXF_NUM_FONT) && !txf[i]) i++;
	if((i >= TXF_NUM_FONT) || !txf[i]) {
        // bad font index
        return;
    }
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, txf[i]->texobj);
    glTranslated(x, y, z);
    glScalef(1/fscale, 1/fscale, 1/fscale);
    if (fRotAngle != 0.0f) {
        // we want to rotate around Z y default,
        // which keeps our text in 2D plane
       glRotatef(fRotAngle, fRotX, fRotY, fRotZ);
    }
    glEnable(GL_ALPHA_TEST);
    // use .1 and .5 for a dark and bright background respectively
    glAlphaFunc(GL_GEQUAL, alpha_value);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor4fv(col);
    txfRenderString(txf[i], s, (int)strlen(s));
	glDisable(GL_TEXTURE_2D);	
	glPopMatrix();
}


#if 0
int main (){
    // usage:
    // first parameter is alpha value :  use .1 and .5 for dark and bright background seems to work
    // 7th can probably be list of #defines of font names.
    render_string(.1f, -1, -1, 0, 200.0f, white, 0, "hello world.");
}
#endif

