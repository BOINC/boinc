// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// Interface functions for tex_info stuff.
// Contributed by Tolu Aina

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "filesys.h"

#include "texfont.h"
#include "txf_util.h"

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif 

static TexFont* txf[TXF_NUM_FONT];

// load fonts. call once.
//
void txf_load_fonts(char* dir) {
    char filename[_MAX_PATH];
    for (int i=0 ; i<TXF_NUM_FONT; i++){
		sprintf(filename, "%s/%s", dir, font_names[i]);
		if (is_file(filename)) {
			txf[i] = txfLoadFont(filename);
			if(txf[i]) CreateTexFont(txf[i], 0, GL_TRUE);
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
	char * s				  	  // string ptr
){
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	if((i < TXF_NUM_FONT) && txf[i]) {
	    glBindTexture(GL_TEXTURE_2D, txf[i]->texobj);
	    glTranslated(x, y, z);
	    glScalef(1/fscale, 1/fscale, 1/fscale);
	    glEnable(GL_ALPHA_TEST);
	    // use .1 and .5 for a dark and bright background respectively
	    glAlphaFunc(GL_GEQUAL, alpha_value);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glColor4fv(col);
	    txfRenderString(txf[i], s, (int)strlen(s));
	}
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

