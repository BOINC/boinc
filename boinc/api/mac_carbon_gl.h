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

/*
    Contains:    Functions to enable build and destory a GL fullscreen context

    Written by:    Geoff Stahl (ggs)

    Copyright:    Copyright © 1999 Apple Computer, Inc., All Rights Reserved

    Disclaimer:    You may incorporate this sample code into your applications without
                restriction, though the sample code has been provided "AS IS" and the
                responsibility for its operation is 100% yours.  However, what you are
                not permitted to do is to redistribute the source as "DSC Sample Code"
                after having made changes. If you're going to re-distribute the source,
                we require that you make it clear in the source that the code was
                descended from Apple Sample Code, but that you've made changes.

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

// structures (public) -----------------------------------------------

// structure for creating a fullscreen context
struct structGLInfo // storage for setup info
{
    SInt16 width;        // input: width of drawable (screen width in full screen mode), return: actual width allocated
    SInt16 height;        // input: height of drawable (screen height in full screen mode), return: actual height allocated
    UInt32 pixelDepth;        // input: requested pixel depth
    GLint aglAttributes[64];     // input: pixel format attributes always required (reset to what was actually allocated)
    AGLPixelFormat fmt;        // input: none; output pixel format...
};
typedef struct structGLInfo structGLInfo;
typedef struct structGLInfo * pstructGLInfo;

// structure for creating a context from a window
struct structGLWindowInfo // storage for setup info
{
    GLint aglAttributes[64];     // input: pixel format attributes always required (reset to what was actually allocated)
    AGLPixelFormat fmt;        // input: none; output pixel format...
                                //        if so renderer check (accel) will look at all renderers vice just the current one
                                //        if window is not dragable renderer check will either check the single device or short 
                                //            circuit to software if window spans multiple devices 
};
typedef struct structGLWindowInfo structGLWindowInfo;
typedef struct structGLWindowInfo * pstructGLWindowInfo;


// public function declarations -------------------------------------
// Destroys drawable and context
// Ouputs: *paglDraw, *paglContext and *pdspContext should be 0 on exit

OSStatus BuildGLonWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo);

// same as above but destorys a context that was associated with an existing window, window is left intacted
OSStatus DestroyGLFromWindow (AGLContext* paglContext, pstructGLWindowInfo pcontextInfo);

// Special suspend function to ensure the the GL window is hidden
OSStatus SuspendFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext);

// Special resume function to ensure the the GL window is shown
OSStatus ResumeFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext);

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

extern WindowRef appGLWindow;


// structures (public) -----------------------------------------------

enum { fadeTicks = 10 };

// public function declarations -------------------------------------

OSStatus StartDSp (void);
void ShutdownDSp (void);

CGrafPtr GetDSpDrawable (DSpContextReference dspContext);
void DestroyDSpContext (DSpContextReference* pdspContext);

OSStatus DSpContext_CustomFadeGammaOut (DSpContextReference inContext, long fadeTicks); 
OSStatus DSpContext_CustomFadeGammaIn (DSpContextReference inContext, long fadeTicks);

extern Boolean gDSpStarted;

#endif // SetupGL_h
