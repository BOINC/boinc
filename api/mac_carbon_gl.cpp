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
    Contains:    Functions to enable building and destorying a GL full screen or windowed context

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

// system includes ----------------------------------------------------------

#ifdef __APPLE_CC__
    #include <AGL/agl.h>
#else
    #include <Gestalt.h>
    #include <MacTypes.h>
    #include <DriverServices.h>
    #include <Timer.h>
    #include <Math64.h>
    #include <agl.h>
    #include <gl.h>
#endif

#include <stdio.h>

// project includes ---------------------------------------------------------

#include "mac_carbon_gl.h"
#include "graphics_api.h"
#include "mac_app_opengl.h"
#include "util.h"

extern WindowRef               appGLWindow;

// --------------------------------------------------------------------------
// BuildGLonWindow
// Takes a window and tries to build on it
// Inputs:     aglDraw: a valid AGLDrawable
//            *pcontextInfo: request and requirements for cotext and drawable
// Outputs: *paglContext as allocated
//            *pcontextInfo:  allocated parameters
// if fail to allocate: paglContext will be NULL
// if error: will return error and paglContext will be NULL

OSStatus BuildGLonWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo)
{
    GDHandle hGD = NULL;
    GrafPtr cgrafSave = NULL;
    short numDevices;
    OSStatus err = noErr;
    
    GetPort (&cgrafSave);
    SetPortWindowPort(pWindow);

    // check renderer VRAM and acceleration
    numDevices = FindGDHandleFromWindow (pWindow, &hGD);
    
    // do agl
    if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) {   // check for existance of OpenGL
        ReportError ("OpenGL not installed");
        return NULL;
    }    
    // we successfully passed the renderer check

    pcontextInfo->fmt = aglChoosePixelFormat (NULL, 0, pcontextInfo->aglAttributes); // get an appropriate pixel format
    aglReportError ();
    if (NULL == pcontextInfo->fmt)  {
        ReportError("Could not find valid pixel format");
        return NULL;
    }

    *paglContext = aglCreateContext (pcontextInfo->fmt, NULL); // Create an AGL context
    aglReportError ();
    if (NULL == *paglContext) {
        ReportError ("Could not create context");
        return NULL;
    }
    
    if (!aglSetDrawable (*paglContext, GetWindowPort (pWindow))) // attach the CGrafPtr to the context
        return aglReportError ();
    
    if(!aglSetCurrentContext (*paglContext)) // make the context the current context
        return aglReportError ();

    SetPort (cgrafSave);

    return err;
}

#pragma mark -

// functions (public) -------------------------------------------------------
// DestroyGLFromWindow

// Destroys context that waas allocated with BuildGLFromWindow
// Ouputs: *paglContext should be NULL on exit

OSStatus DestroyGLFromWindow (AGLContext* paglContext, pstructGLWindowInfo pcontextInfo) {
    OSStatus err;
    
    if ((!paglContext) || (!*paglContext))
        return paramErr; // not a valid context
    glFinish ();
    aglSetCurrentContext (NULL);
    err = aglReportError ();
    aglSetDrawable (*paglContext, NULL);
    err = aglReportError ();
    aglDestroyContext (*paglContext);
    err = aglReportError ();
    *paglContext = NULL;

    if (pcontextInfo->fmt) {
        aglDestroyPixelFormat (pcontextInfo->fmt); // pixel format is no longer valid
        err = aglReportError ();
    }
    pcontextInfo->fmt = 0;
    
    return err;
}

//-----------------------------------------------------------------------------------------------------------------------
// SuspendFullScreenGL
// Special suspend function to ensure the the GL window is hidden
// needs to be reviewed

OSStatus SuspendFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext) {
    if (aglDraw && aglContext) {   // will only have a drawable
        glFinish (); // must do this to ensure the queue is complete
        aglSetCurrentContext (NULL);
        HideWindow (GetWindowFromPort (aglDraw));
        return aglReportError ();
    }
    return noErr;
}

//-----------------------------------------------------------------------------------------------------------------------
// ResumeFullScreenGL
// Needs a special resume function to ensure the the GL window is shown
// needs to be reviewed

OSStatus ResumeFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext) {
    if (aglDraw && aglContext) {
        ShowWindow (GetWindowFromPort (aglDraw));
        aglSetCurrentContext (aglContext);
        aglUpdateContext (aglContext);
        return aglReportError ();
    }
    return paramErr;
}

// --------------------------------------------------------------------------
// GetWindowDevice
// Inputs:    a valid WindowPtr
// Outputs:    the GDHandle that that window is mostly on
// returns the number of devices that the windows content touches

short FindGDHandleFromWindow (WindowPtr pWindow, GDHandle * phgdOnThisDevice)
{
    GrafPtr pgpSave;
    Rect rectWind, rectSect;
    long greatestArea, sectArea;
    short numDevices = 0;
    GDHandle hgdNthDevice;
    
    if (!pWindow || !phgdOnThisDevice)
            return NULL;
            
    *phgdOnThisDevice = NULL;
    
    GetPort (&pgpSave);
    SetPortWindowPort (pWindow);
    
    GetWindowPortBounds (pWindow, &rectWind);
    LocalToGlobal ((Point*)& rectWind.top);    // convert to global coordinates
    LocalToGlobal ((Point*)& rectWind.bottom);
    hgdNthDevice = GetDeviceList ();
    greatestArea = 0;
    // check window against all gdRects in gDevice list and remember 
    //  which gdRect contains largest area of window}
    while (hgdNthDevice)
    {
        if (TestDeviceAttribute (hgdNthDevice, screenDevice)) {
            if (TestDeviceAttribute (hgdNthDevice, screenActive))
            {
                // The SectRect routine calculates the intersection 
                //  of the window rectangle and this gDevice 
                //  rectangle and returns TRUE if the rectangles intersect, 
                //  FALSE if they don't.
                SectRect (&rectWind, &(**hgdNthDevice).gdRect, &rectSect);
                // determine which screen holds greatest window area
                //  first, calculate area of rectangle on current device
                sectArea = (long) (rectSect.right - rectSect.left) * (rectSect.bottom - rectSect.top);
                if (sectArea > 0)
                    numDevices++;
                if (sectArea > greatestArea)
                {
                    greatestArea = sectArea; // set greatest area so far
                    *phgdOnThisDevice = hgdNthDevice; // set zoom device
                }
                hgdNthDevice = GetNextDevice(hgdNthDevice);
            }
        }
    }
    
    SetPort (pgpSave);
    return numDevices;
}

#pragma mark -

//-----------------------------------------------------------------------------------------------------------------------

GLuint BuildFontGL (AGLContext ctx, GLint fontID, Style face, GLint size)
{
    GLuint listBase = glGenLists (256);
    if (aglUseFont (ctx, fontID , face, size, 0, 256, (long) listBase)) {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        return listBase;
    } else {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glDeleteLists (listBase, 256);
        return 0;
    }
}

//-----------------------------------------------------------------------------------------------------------------------

void DeleteFontGL (GLuint fontList)
{
    if (fontList)
        glDeleteLists (fontList, 256);
}

#pragma mark -
// --------------------------------------------------------------------------

// central error reporting

void ReportErrorNum (char * strError, long numError)
{
    fprintf (stderr, "%s %ld (0x%lx)\n", strError, numError, numError); 
}

// --------------------------------------------------------------------------

void ReportError (char * strError)
{
    fprintf (stderr, "%s\n", strError); 
}

//-----------------------------------------------------------------------------------------------------------------------

OSStatus DSpReportError (OSStatus error)
{
    switch (error)
    {
        case noErr:
            break;
        case kDSpNotInitializedErr:
            ReportError ("DSp Error: Not initialized");
            break;
        case kDSpSystemSWTooOldErr:
            ReportError ("DSp Error: system Software too old");
            break;
        case kDSpInvalidContextErr:
            ReportError ("DSp Error: Invalid context");
            break;
        case kDSpInvalidAttributesErr:
            ReportError ("DSp Error: Invalid attributes");
            break;
        case kDSpContextAlreadyReservedErr:
            ReportError ("DSp Error: Context already reserved");
            break;
        case kDSpContextNotReservedErr:
            ReportError ("DSp Error: Context not reserved");
            break;
        case kDSpContextNotFoundErr:
            ReportError ("DSp Error: Context not found");
            break;
        case kDSpFrameRateNotReadyErr:
            ReportError ("DSp Error: Frame rate not ready");
            break;
        case kDSpConfirmSwitchWarning:
//        ReportError ("DSp Warning: Must confirm switch"); // removed since it is just a warning, add back for debugging
            return 0; // don't want to fail on this warning
            break;
        case kDSpInternalErr:
            ReportError ("DSp Error: Internal error");
            break;
        case kDSpStereoContextErr:
            ReportError ("DSp Error: Stereo context");
            break;
    }
    return error;
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump agl errors to debugger string, return error

OSStatus aglReportError (void)
{
    GLenum err = aglGetError();
    if (AGL_NO_ERROR != err)
        ReportError ((char *)aglErrorString(err));
    // ensure we are returning an OSStatus noErr if no error condition
    if (err == AGL_NO_ERROR)
        return noErr;
    else
        return (OSStatus) err;
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump gl errors to debugger string, return error

OSStatus glReportError (void)
{
    GLenum err = glGetError();
    switch (err)
    {
        case GL_NO_ERROR:
            break;
        case GL_INVALID_ENUM:
            ReportError ("GL Error: Invalid enumeration");
            break;
        case GL_INVALID_VALUE:
            ReportError ("GL Error: Invalid value");
            break;
        case GL_INVALID_OPERATION:
            ReportError ("GL Error: Invalid operation");
            break;
        case GL_STACK_OVERFLOW:
            ReportError ("GL Error: Stack overflow");
            break;
        case GL_STACK_UNDERFLOW:
            ReportError ("GL Error: Stack underflow");
            break;
        case GL_OUT_OF_MEMORY:
            ReportError ("GL Error: Out of memory");
            break;
    }
    // ensure we are returning an OSStatus noErr if no error condition
    if (err == GL_NO_ERROR)
        return noErr;
    else
        return (OSStatus) err;
}

// --------------------------------------------------------------------------

void DoUpdate (AGLContext aglContext)
{
    Rect    portRect;
    int        width, height;
    
    if (aglContext)
    {
        aglSetCurrentContext (aglContext);
        aglUpdateContext (aglContext);
        
        GetWindowBounds( appGLWindow, kWindowContentRgn, &portRect );
        width = portRect.right - portRect.left;
        height = portRect.bottom - portRect.top;
        ReSizeGLScene(width,height);
        app_render(width, height, time(0));      // Here's Where We Do All The Drawing

        aglSwapBuffers(aglContext);        // send swap command
    }
}

/*
    Contains:    Functions to enable building and destroying a DSp fullscreen context

*/

// globals (internal/private) -----------------------------------------------

Boolean gDSpStarted = false; // will never be true unless DSp is installed and start succeeds

// functions (public) -------------------------------------------------------
// StartDSp
// handles starting up DrawSprocket

OSStatus StartDSp (void)
{
    OSStatus err = noErr;
    if (!gDSpStarted) {
        // check for DSp
        if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpStartup) {
            ReportError ("DSp not installed");
            return kDSpNotInitializedErr;
        } else {
            err = DSpReportError (DSpStartup()); // start DSp
            if (noErr != err)
                return err;
            else
                gDSpStarted = true;
        }
    }
    return err;        
}

// --------------------------------------------------------------------------
// ShutdownDSpContext
// shuts down DrawSprocket

void ShutdownDSp (void)
{
    if (gDSpStarted) {
        DSpShutdown ();
        gDSpStarted = false;
    }
}

#pragma mark -
// --------------------------------------------------------------------------
// GetDSpDrawable
// Just returns the front buffer
// Inputs:     *pdspContext
//            pcontextInfo: request and requirements for cotext and drawable
// Outputs: returns CGrafPtr thaat is front buffer of context
// if error: will return NULL

CGrafPtr GetDSpDrawable (DSpContextReference dspContext)
{
    CGrafPtr pCGraf = NULL;
    if (noErr == DSpReportError (DSpContext_GetFrontBuffer (dspContext, &pCGraf)))
        return pCGraf;
    else
        return NULL;
}

//-----------------------------------------------------------------------------------------------------------------------

// Deactivates and dumps context

void DestroyDSpContext (DSpContextReference* pdspContext)
{
    if (gDSpStarted) {
        if (*pdspContext) {
            DSpReportError (DSpContext_SetState(*pdspContext, kDSpContextState_Inactive));
            DSpReportError (DSpContext_CustomFadeGammaIn (NULL, fadeTicks));
            DSpReportError (DSpContext_Release (*pdspContext));
            *pdspContext = NULL;
        }
    }
}

#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

OSStatus DSpContext_CustomFadeGammaIn (DSpContextReference inContext, long fadeTicks) 
{
    OSStatus err = noErr;
    RGBColor inZeroIntensityColor;
    UInt32 currTick;
    UInt16 step = (UInt16) (800 / fadeTicks);
    long x, percent = 0;

    if (gDSpStarted)
    {
        if (fadeTicks == 0)
            fadeTicks = 1;
                inZeroIntensityColor.red = 0x0000;
                inZeroIntensityColor.green = 0x0000;
                inZeroIntensityColor.blue = 0x0000;
        currTick = TickCount ();
        for (x = 1; x <= fadeTicks; x++) 
        {
            percent = step * x / 8;
            err = DSpContext_FadeGamma(inContext, percent, &inZeroIntensityColor);
            if (err != noErr) break;
            while (currTick >= TickCount ()) {}
            currTick = TickCount ();
        }
        if (err == noErr)
            err = DSpContext_FadeGamma(inContext, 100, &inZeroIntensityColor);
    }
    return err;
}

//-----------------------------------------------------------------------------------------------------------------------

OSStatus DSpContext_CustomFadeGammaOut (DSpContextReference inContext, long fadeTicks ) 
{
    OSStatus err = noErr;
    RGBColor inZeroIntensityColor;
    UInt32 currTick;
    UInt16 step = (UInt16) (800 / fadeTicks);
    long x, percent = 0;

    if (gDSpStarted)
    {
        if (fadeTicks == 0)
            fadeTicks = 1;                                // ensure we do not have zero fade time
        inZeroIntensityColor.red = 0x0000;
        inZeroIntensityColor.green = 0x0000;
        inZeroIntensityColor.blue = 0x0000;
        currTick = TickCount ();
        for (x = fadeTicks - 1; x >= 0; x--) 
        {
            percent = step * x / 8;
            err = DSpContext_FadeGamma(inContext, percent, &inZeroIntensityColor);
            if (err != noErr) break;
            while (currTick >= TickCount ()) {}
            currTick = TickCount ();
        }
    }
    return err;
}
