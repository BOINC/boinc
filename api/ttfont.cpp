// utility functions for TrueType font OpenGL graphics

// FTGL OpenGL TrueType/FreeType font rendering
// you will need to build & link against freetype2 and ftgl
// tested using ftgl version 2.1.3 rc5 and freetype version 2.3.9

// ftgl library:  http://sourceforge.net/projects/ftgl/files/FTGL%20Source/
// freetype2 library:  http://www.freetype.org/

// this should basically be a drop-in for the old boinc txf_* functions i.e.
//  txf_load_fonts and txf_render_string, with extra options on the latter for rotating etc

// adapted by Carl Christensen, freely released under LGPL for BOINC

#include "ttfont.h"
#include "filesys.h"  // from boinc for file_exists

// I put in it's own namespace so call TTFont::ttf_load_fonts() etc
namespace TTFont {  
 
// define the number of fonts required
#define NUM_FONT 2

FTFont* g_font[NUM_FONT] = {NULL, NULL};
int g_iFont = -1;

// you'll want to define a 2-d array of char as appropriate for your 
// truetype font filenames (path is set in the call to ttf_load_fonts)
static const char g_cstrFont[NUM_FONT][4] = {"hvt", "cbt"};
	
static GLfloat g_transColor[3] = { 255, 0, 255 };  // this should be magenta - red = 255, green = 0, blue = 255

// load fonts. call once.
// pass in the font directory, and any special-scaling font name & size (i.e. I want a huge typeface for hvtb so pass in "hvtb", 3000
void ttf_load_fonts(const char* dir, const char* strScaleFont, const int& iScaleFont) {
	static bool bInit = false; // flag so we don't call again, otherwise we'll have memory leaks each subsequent call to new FTTextureFont etc
	if (bInit) return; // we've already been here
	bInit = true; // we're in now!
	ttf_cleanup();
        memset(g_font, 0x00, sizeof(FTFont*) * NUM_FONT); // initialize to null's for error checking later]
        char vpath[_MAX_PATH];
        g_iFont = -1;
        for (int i=0 ; i < NUM_FONT; i++){
           sprintf(vpath, "%s/%s", dir, g_cstrFont[i]);
           if (boinc_file_exists(vpath)) {
                   //g_font[i] = new FTBitmapFont(vpath);
                   //g_font[i] = new FTPixmapFont(vpath);
                   //g_font[i] = new FTPolygonFont(vpath);
                   g_font[i] = new FTTextureFont(vpath);
                  if(!g_font[i]->Error()) {
#ifdef _DEBUG
               fprintf(stderr, "Successfully loaded '%s'...\n", vpath);
#endif
					  int iScale = 30;
					  if (strScaleFont && !strcmp(strScaleFont, g_cstrFont[i])) iScale = iScaleFont;
                         if(!g_font[i]->FaceSize(iScale))
                         {
                                 fprintf(stderr, "Failed to set size");
                         }

                         g_font[i]->Depth(3.);
                         g_font[i]->Outset(-.5f, 1.5f);

                         g_font[i]->CharMap(ft_encoding_unicode);
                         g_iFont = i;

                 }
#ifdef _DEBUG
                 else {
                         fprintf(stderr, "Failed to load '%s'...\n", vpath);
                 }
#endif
           }
    }  
}

// remove our objects?
void ttf_cleanup()
{
	for (int i = 0; i < NUM_FONT; i++) {
		if (g_font[i]) {
			delete g_font[i];
			g_font[i] = NULL;
		}
	}
}


void ttf_render_string(
	const double& alpha_value,
	// reference value to which incoming alpha values are compared.
	// 0 through to 1
	const double& x,
	const double& y,
	const double& z, // text position
	const float& fscale,                 // scale factor
	const GLfloat* col,                // colour 4vf
	const int& iFont,                        // font index
	const char* s,                                          // string ptr
	const float& fRotAngle,        // optional rotation angle
	const float& fRotX,            // optional rotation vector for X
	const float& fRotY,            // optional rotation vector for Y
	const float& fRotZ,            // optional rotation vector for Z
	const float& fRadius           // circular radius to draw along
)
{
        // http://ftgl.sourceforge.net/docs/html/

                if(iFont < 0 || iFont > NUM_FONT || !g_font[iFont]) return;  //invalid font

				int renderMode = FTGL::FTGL_RENDER_FRONT; //ALL; //FRONT | FTGL::FTGL_RENDER_BACK;

                GLfloat color[4];
                memcpy(color, col, sizeof(GLfloat) * 4);
                color[3] = (GLfloat) alpha_value;  // force the alpha value passed in
				glColor4fv(color);

				glPushMatrix();

                    glTranslated(x, y, z);
                    glScaled(1.0f / fscale, 1.0f / fscale, 1.0f / fscale);
                        glEnable(GL_TEXTURE_2D);

                        if (fRotAngle != 0.0f) {
                glRotatef(fRotAngle, fRotX, fRotY, fRotZ);
                        }

                        if (fRadius == 0.0f) {
                                g_font[iFont]->Render(s, -1, FTPoint(), FTPoint(), renderMode);
                    }
                        else {
                                int i = 0;
                                float fAdvance = 1.0f;
                            while ( *(s+i) ) {
                                  fAdvance = g_font[iFont]->Advance((s+i), 1, FTPoint());
                  g_font[iFont]->Render((s+i), 1, FTPoint(), FTPoint(), renderMode);
                                  glTranslated(fAdvance, 0.0f, 0.0f);
                  glRotatef(fRadius * fAdvance / (float) 20.0f, 0.0f, 0.0f, 1.0f);
                                i++;
                            }
                        }

                glDisable(GL_TEXTURE_2D);

                glPopMatrix();
        }


// based on boinc/api/gutil.cpp -- but makes an alpha map out of an RGB file for transparency
// basic pic editors such as Gimp are pretty easy to set a weird color not in the main image of course)
// or even easier -- use Gimp to set "Color to Alpha" and just embed it in the .rgb file as an RGBA (A = "alpha channel")
GLuint CreateRGBTransparentTexture(const char* strFileName, float* transColor)   // default in prototype to transColor = NULL i.e. no "filter color" required
{   
        // transcolor is an optional (but necessary for Z=3 i.e. RGB) which will flip the alpha of an RGB color (i.e. Magenta would be 255/0/255 passed in transColor

        GLuint uiTexture = 0;
        int sizeX, sizeY, sizeZ;
        // Load the image and store the data - this is in rgba format but the a is 255 (max)
        unsigned int *pImage = read_rgb_texture(strFileName,&sizeX,&sizeY,&sizeZ);  // read_rgb_texgture makes a 4channel (1 byte per R/G/B/A) anyway, so may as well use the "A"!
        if(pImage == NULL) return 0;
        if (sizeZ != 3 && sizeZ != 4) { // needs to be RGB i.e. z=3 or RGBA z=4
                  free(pImage);
                  return 0;
            }

        // need to set transparency bytes/alpha value every place in pImage  using magenta 255/0/255!
        // also note image needs to be flipped vertically when you save it! (at least in gimp)
        if (sizeZ == 3 && transColor) { //rgb -> rgba via the RGB values in transColor[3] array, if transColor not set not much use to this so just default to the RGBA created above (A=255 for all pixels)
                for(int i = 0; i < (sizeX * sizeY); i++)
                {
                        unsigned char* bb = (unsigned char*) (pImage+i);    // easy pointer to our image pixels
                        // just take the avg of the RGB -- as it approaches 0, the alpha goes to 0 (so basically the blacker, the more transperent)
                        if(*bb == g_transColor[0]
                          && *(bb+1) == g_transColor[1]
                          && *(bb+2) == g_transColor[2] )
                        {
                                *(bb+3) = 0x00;   // If so, set alpha to fully transparent.   (note this sets all 4 bytes to 0)
                        } // alpha already set to 255 if not transparent

                        //iTest = (*bb + *(bb+1) + *(bb+2)) / 3;
                        //*(bb+3) = iTest > 255 ? 255 : iTest;
                }
        }
        // sizeZ == 4 is just "straight" RGBA where alpha is embedded in the file; which is already taken care of by the above load_file

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glGenTextures( 1, &uiTexture );
        glBindTexture( GL_TEXTURE_2D, uiTexture );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // this isn't as fast supposedly, but looks pretty good
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);                // also not as fast as GL_NEAREST but looks better!
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA,
                sizeX, sizeY, GL_RGBA, GL_UNSIGNED_BYTE, pImage);

        free(pImage);  // free the mem allocated by the rgb_texture function
        return uiTexture;
}


// based on boinc/api/gutil.cpp -- but makes a GL_ALPHA out of an RGB file (sums values each point to make the alpha)
// the basic idea is you can make a simple image that can translate to a complex map -- white values (1) "pass" and black values (0) "block"
GLuint CreateRGBAlpha(const char* strFileName)
{
        //if(!strFileName && !boinc_file_exists(strFileName)) return 0;
        GLuint uiTexture = 0;
        int sizeX;
        int sizeY;
        int sizeZ;
        // Load the image and store the data
        unsigned int *pImage = read_rgb_texture(strFileName,&sizeX,&sizeY,&sizeZ);
        if(pImage == NULL) return 0;
        if (sizeZ > 1) { // error - just want a 1-D for alpha levels
            fprintf(stderr, "Improper RGB Image for Alpha: %s needs just 1 level, this file has %d\n", strFileName, sizeZ);
            free(pImage);
            return 0;
        }
        unsigned char* pByte = new unsigned char[sizeX*sizeY*sizeZ];
        memset(pByte, 0x00, sizeX*sizeY*sizeZ);
        for (int i = 0 ; i < (sizeX*sizeY*sizeZ) ; i++)  {
            pByte[i] = (unsigned char) *(pImage+i) & ~0xffffff00;   // get the final char from masking the higher bits
        }
        free(pImage); // don't need the image data, may as well free it

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glGenTextures(1, &uiTexture);
        glBindTexture(GL_TEXTURE_2D, uiTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, sizeX, sizeY, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pByte);
                //glBindTexture(GL_TEXTURE_2D, 0);
        delete [] pByte;  // free the temporary byte array
        return uiTexture;
}

}   // namespace


