// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

/*
	Contains:	Functions to enable build and destory a GL fullscreen context

	Written by:	Geoff Stahl (ggs)

	Copyright:	Copyright © 1999 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):

         <2>     3/26/01    ggs     Add DSp version check and other items for full screen on X
         <1>     1/19/01    ggs     Initial re-add
         <7>     3/21/00    ggs     Added windowed mode and clean up various implementation details
         <6>     1/26/00    ggs     Add fade code back in, ensure NULL pointer/context/drawable
                                    checks are in, add Preflight
         <5>     1/24/00    ggs     Added get device num and get gdhandle from point routines, add
                                    support for compiling from C++
         <4>    12/18/99    ggs     Fix headers
         <3>    11/28/99    ggs     Split out DSp and error handling.  Added texture memory
                                    considerations, assume VRAM is required if other than zero
         <3>    11/12/99    ggs     add pixel format and freq return
         <2>    11/12/99    ggs     1.0 Interface complete
         <1>    11/11/99    ggs     Initial Add

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        Adapted to BOINC by Eric Heien
*/

// include control --------------------------------------------------

#ifndef SetupGL_h
#define SetupGL_h

// includes ---------------------------------------------------------

#ifdef __APPLE_CC__
    #include <DrawSprocket/DrawSprocket.h>
    #include <AGL/agl.h>
#else
    #include <DrawSprocket.h>
    #include <agl.h>
    #include <gl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// structures (public) -----------------------------------------------

// structure for creating a fullscreen context
struct structGLInfo // storage for setup info
{
    SInt16 width;		// input: width of drawable (screen width in full screen mode), return: actual width allocated
    SInt16 height;		// input: height of drawable (screen height in full screen mode), return: actual height allocated
    Boolean fSizeMust;		// input: dspContext must be requested display size (ignored in window mode)
                                // if fSizeMust display size will not be stepped down to try to find a match, 
                                // if display is stepped down aspect ratio will be maintained for returned size
    UInt32 pixelDepth;		// input: requested pixel depth
    Boolean fDepthMust;		// input: pixel depth must be set (if false then current depth will be used if able)
    Boolean fFullscreen;	// input: use DSp to get fullscreen? (or find full screen renderer)
                                // if fFullscreen, will search for full screen renderers first then use DSp for others
                                //  unless a device is specified, in which case we will try there first
    Boolean fAcceleratedMust; 	// input: must renderer be accelerated?
    GLint aglAttributes[64]; 	// input: pixel format attributes always required (reset to what was actually allocated)
    SInt32 VRAM;		// input: minimum VRAM; output: actual (if successful otherwise input)
    SInt32 textureRAM;		// input: amount of texture RAM required on card; output: same (used in allcoation to ensure enough texture
    AGLPixelFormat fmt;		// input: none; output pixel format...
    SInt32 freq;		// input: frequency request for display; output: actual
};
typedef struct structGLInfo structGLInfo;
typedef struct structGLInfo * pstructGLInfo;

// structure for creating a context from a window
struct structGLWindowInfo // storage for setup info
{
    Boolean fAcceleratedMust; 	// input: must renderer be accelerated?
    GLint aglAttributes[64]; 	// input: pixel format attributes always required (reset to what was actually allocated)
    SInt32 VRAM;		// input: minimum VRAM; output: actual (if successful otherwise input)
    SInt32 textureRAM;		// input: amount of texture RAM required on card; output: same (used in allcoation to ensure enough texture
    AGLPixelFormat fmt;		// input: none; output pixel format...
    Boolean fDraggable;		// input: is window going to be dragable, 
                                //        if so renderer check (accel, VRAM, textureRAM) will look at all renderers vice just the current one
                                //        if window is not dragable renderer check will either check the single device or short 
                                //            circuit to software if window spans multiple devices 
                                //        software renderer is consider to have unlimited VRAM, unlimited textureRAM and to not be accelerated
};
typedef struct structGLWindowInfo structGLWindowInfo;
typedef struct structGLWindowInfo * pstructGLWindowInfo;


// public function declarations -------------------------------------

// Runtime check to see if we are running on Mac OS X
// Inputs:  None
// Returns: 0 if < Mac OS X or version number of Mac OS X (10.0 for GM)
UInt32 CheckMacOSX (void);

// Checks for presense of OpenGL and DSp (if required)
// Inputs: checkFullscreen: true if one wants to run fullscreen (which requires DrwSprocket currently)
// Ouputs: true if OpenGL is installed (and DrawSprocket if checkFullscreen is true
Boolean PreflightGL (Boolean checkFullscreen);

// Takes device # and geometry request and tries to build best context and drawable
// 	If requested device does not work, will start at first device and walk down devices 
//	 looking for first one that satisfies requirments
//  Devices are numbered in order that DMGetFirstScreenDevice/DMGetNextScreenDevice returns, 
//	 fullscreen devices are numbered after this, but they will be searched first if fFullscreen == true,
//	 they will not be searched in the non-fullscreen case

// Inputs: 	*pnumDevice: -1: main device, 0: any device, other #: attempt that device first, then any device
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglDraw, *paglContext and *pdspContext as allocated
//			*pnumDevice to device number in list that was used 
//			*pcontextInfo:  allocated parameters

// If fail to build context: paglDraw, paglContext and pdspContext will be NULL
// If fatal error: will return error and paglDraw, paglContext and pdspContext will be NULL
// Note: Errors can be generated internally when a specific device fails, this is normal and these
//		  will not be returned is a subsequent device succeeds

OSStatus BuildGL (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, 
				  short* pnumDevice, pstructGLInfo pcontextInfo, AGLContext aglShareContext);
				  
// Destroys drawable and context
// Ouputs: *paglDraw, *paglContext and *pdspContext should be 0 on exit

OSStatus DestroyGL (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, pstructGLInfo pcontextInfo);

// same as above except that it takes a window as input and attempts to build requested conext on that
OSStatus BuildGLFromWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo, AGLContext aglShareContext);

// same as above but destorys a context that was associated with an existing window, window is left intacted
OSStatus DestroyGLFromWindow (AGLContext* paglContext, pstructGLWindowInfo pcontextInfo);

// Special suspend function to ensure the the GL window is hidden
OSStatus SuspendFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext);

// Special resume function to ensure the the GL window is shown
OSStatus ResumeFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext);

// Pauses gl to allow toolbox drawing
OSStatus PauseGL (AGLContext aglContext);

// resumes gl to allow gl drawing
OSStatus ResumeGL (AGLContext aglContext);

short FindGDHandleFromWindow (WindowPtr pWindow, GDHandle * phgdOnThisDevice);

GLuint BuildFontGL (AGLContext ctx, GLint fontID, Style face, GLint size);

void DeleteFontGL (GLuint fontList);

// Error reporter, can be set to report however the application desires
void ReportError (char * strError);

// Error with numeric code reporter, can be set to report however the application desires
void ReportErrorNum (char * strError, long numError);

// Handle reporting of DSp errors, error code is passed through
OSStatus DSpReportError (OSStatus error);

// Handle reporting of agl errors, error code is passed through
OSStatus aglReportError (void);

// Handle reporting of OpenGL errors, error code is passed through
OSStatus glReportError (void);

void DoUpdate (AGLContext aglContext);

GLvoid glPrint(const char *fmt, ...);

int DrawGLScene(GLvoid);      // Here's Where We Do All The Drawing

#ifdef __cplusplus
}
#endif

#endif // SetupGL_h
