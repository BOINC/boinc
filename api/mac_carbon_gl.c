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
	Contains:	Functions to enable building and destorying a GL full screen or windowed context

	Written by:	Geoff Stahl (ggs)

	Copyright:	Copyright © 1999 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):

         <4>     8/23/01    ggs     Fixed texture sharing and added number of bug fixes
         <3>     4/20/01    ggs     Added support for texture sharing by sharing all contexts by default
         <2>     3/26/01    ggs     Add DSp version check and other items for full screen on X
         <1>     1/19/01    ggs     Initial re-add
         <7>     3/22/00    ggs     remove extranious prototype
         <6>     3/21/00    ggs     Added windowed mode and clean up various implementation details
         <5>     1/26/00    ggs     Add fade code back in, ensure NULL pointer/context/drawable
                                    checks are in, add Preflight
         <4>     1/24/00    ggs     Added glFinish to shutdown code
         <3>     1/24/00    ggs     update to latest, better rendrere info handling for 3dfx, better
                                    checks on pause and resume, added frin devce numer and gdhandle
                                    from point
   	     <2.7>   11/28/99    ggs     Split out DSp and error handling.  Added texture memory
                                    considerations, assume VRAM is required if other than zero
         <2.6>   11/14/99    ggs     Fix source server copy
         <2.5>   11/13/99    ggs     fixed default pixel depth (0) condition that was causing failures
         <2.4>   11/13/99    ggs     added custom fade code
         <2.3>   11/13/99    ggs     Reset for Quake 3 use
         <2.2>   11/12/99    ggs     re-add
         <2.1>   11/12/99    ggs     added support for frequency retrieval, fixed display number
                                    output to be correct if display number input was -1
         <2>    11/12/99    ggs     1.0 functionality
         <1>    11/11/99    ggs     Initial Add

	Disclaimer:	You may incorporate this sample code into your applications without
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
    #include <Carbon/Carbon.h>
    #include <AGL/agl.h>
#else
    #include <Gestalt.h>
    #include <sound.h>
    #include <MacTypes.h>
    #include <DriverServices.h>
    #include <Timer.h>
    #include <Math64.h>
    #include <agl.h>
    #include <gl.h>
#endif

#include <stdio.h>

// project includes ---------------------------------------------------------

#include "mac_carbon_dsp.h"
#include "mac_carbon_gl.h"
#include "graphics_api.h"
#include "mac_app_opengl.h"

extern WindowRef appGLWindow;

// globals (internal/private) -----------------------------------------------

const RGBColor	rgbBlack	= { 0x0000, 0x0000, 0x0000 };

const short kWindowType = kWindowDocumentProc;

// prototypes (internal/private) --------------------------------------------

static Boolean CheckRenderer (GDHandle hGD, long *VRAM, long *textureRAM, GLint*  , Boolean fAccelMust);
static Boolean CheckAllDeviceRenderers (long* pVRAM, long* pTextureRAM, GLint* pDepthSizeSupport, Boolean fAccelMust);
static Boolean CheckWindowExtents (GDHandle hGD, short width, short height);
static void DumpCurrent (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, pstructGLInfo pcontextInfo);

static OSStatus BuildGLContext (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, GDHandle hGD, 
								pstructGLInfo pcontextInfo, AGLContext aglShareContext);
static OSStatus BuildDrawable (AGLDrawable* paglDraw, GDHandle hGD, pstructGLInfo pcontextInfo);
static OSStatus BuildGLonDevice (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, 
				  		  GDHandle hGD, pstructGLInfo pcontextInfo, AGLContext aglShareContext);
static OSStatus BuildGLonWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo, AGLContext aglShareContext);

extern GLuint			monacoFontList;


// functions (internal/private) ---------------------------------------------

// CheckRenderer

// looks at renderer attributes it has at least the VRAM is accelerated

// Inputs: 	hGD: GDHandle to device to look at
//			pVRAM: pointer to VRAM in bytes required; out is actual VRAM if a renderer was found, otherwise it is the input parameter
//			pTextureRAM:  pointer to texture RAM in bytes required; out is same (implementation assume VRAM returned by card is total so we add texture and VRAM)
//			fAccelMust: do we check for acceleration

// Returns: true if renderer for the requested device complies, false otherwise

static Boolean CheckRenderer (GDHandle hGD, long* pVRAM, long* pTextureRAM, GLint* pDepthSizeSupport, Boolean fAccelMust)
{
    AGLRendererInfo info, head_info;
    GLint inum;
    GLint dAccel = 0;
    GLint dVRAM = 0, dMaxVRAM = 0;
    Boolean canAccel = false, found = false;
    head_info = aglQueryRendererInfo(&hGD, 1);
    aglReportError ();
    if(!head_info)
    {
        ReportError ("aglQueryRendererInfo error");
        return false;
    }
    else
    {
        info = head_info;
        inum = 0;
        // see if we have an accelerated renderer, if so ignore non-accelerated ones
        // this prevents returning info on software renderer when actually we'll get the hardware one
        while (info)
        {	
            aglDescribeRenderer(info, AGL_ACCELERATED, &dAccel);
            aglReportError ();
            if (dAccel)
                canAccel = true;
            info = aglNextRendererInfo(info);
            aglReportError ();
            inum++;
        }
                
        info = head_info;
        inum = 0;
        while (info)
        {
            aglDescribeRenderer (info, AGL_ACCELERATED, &dAccel);
            aglReportError ();
            // if we can accel then we will choose the accelerated renderer 
            // how about compliant renderers???
            if ((canAccel && dAccel) || (!canAccel && (!fAccelMust || dAccel)))
            {
                aglDescribeRenderer (info, AGL_VIDEO_MEMORY, &dVRAM);	// we assume that VRAM returned is total thus add texture and VRAM required
                aglReportError ();
                if (dVRAM >= (*pVRAM + *pTextureRAM))
                {
                    if (dVRAM >= dMaxVRAM) // find card with max VRAM
                    {
                        aglDescribeRenderer (info, AGL_DEPTH_MODES, pDepthSizeSupport);	// which depth buffer modes are supported
                        aglReportError ();
                        dMaxVRAM = dVRAM; // store max
                        found = true;
                    }
                }
            }
            info = aglNextRendererInfo(info);
            aglReportError ();
            inum++;
        }
    }
    aglDestroyRendererInfo(head_info);
    if (found) // if we found a card that has enough VRAM and meets the accel criteria
    {
        *pVRAM = dMaxVRAM; // return VRAM
        return true;
    }
    // VRAM will remain to same as it did when sent in
    return false;
}

//-----------------------------------------------------------------------------------------------------------------------

// CheckAllDeviceRenderers 

// looks at renderer attributes and each device must have at least one renderer that fits the profile

// Inputs: 	pVRAM: pointer to VRAM in bytes required; out is actual min VRAM of all renderers found, otherwise it is the input parameter
//		pTextureRAM:  pointer to texture RAM in bytes required; out is same (implementation assume VRAM returned by card is total so we add texture and VRAM)
//		fAccelMust: do we check fro acceleration

// Returns: true if any renderer for on each device complies (not necessarily the same renderer), false otherwise

static Boolean CheckAllDeviceRenderers (long* pVRAM, long* pTextureRAM, GLint* pDepthSizeSupport, Boolean fAccelMust)
{
    AGLRendererInfo info, head_info;
    GLint inum;
    GLint dAccel = 0;
    GLint dVRAM = 0, dMaxVRAM = 0;
    Boolean canAccel = false, found = false, goodCheck = true; // can the renderer accelerate, did we find a valid renderer for the device, are we still successfully on all the devices looked at
    long MinVRAM = 0x8FFFFFFF; // max long
    GDHandle hGD = GetDeviceList (); // get the first screen
    while (hGD && goodCheck)
    {
        head_info = aglQueryRendererInfo(&hGD, 1);
        aglReportError ();
        if(!head_info)
        {
            ReportError ("aglQueryRendererInfo error");
            return false;
        }
        else
        {
            info = head_info;
            inum = 0;
            // see if we have an accelerated renderer, if so ignore non-accelerated ones
            // this prevents returning info on software renderer when actually we'll get the hardware one
            while (info)
            {
                aglDescribeRenderer(info, AGL_ACCELERATED, &dAccel);
                aglReportError ();
                if (dAccel)
                    canAccel = true;
                info = aglNextRendererInfo(info);
                aglReportError ();
                inum++;
            }
                    
            info = head_info;
            inum = 0;
            while (info)
            {	
                aglDescribeRenderer(info, AGL_ACCELERATED, &dAccel);
                aglReportError ();
                // if we can accel then we will choose the accelerated renderer 
                // how about compliant renderers???
                if ((canAccel && dAccel) || (!canAccel && (!fAccelMust || dAccel)))
                {
                    aglDescribeRenderer(info, AGL_VIDEO_MEMORY, &dVRAM);	// we assume that VRAM returned is total thus add texture and VRAM required
                    aglReportError ();
                    if (dVRAM >= (*pVRAM + *pTextureRAM))
                    {
                        if (dVRAM >= dMaxVRAM) // find card with max VRAM
                        {
                            aglDescribeRenderer(info, AGL_DEPTH_MODES, pDepthSizeSupport);	// which depth buffer modes are supported
                            aglReportError ();
                            dMaxVRAM = dVRAM; // store max
                            found = true;
                        }
                    }
                }
                info = aglNextRendererInfo(info);
                aglReportError ();
                inum++;
            }
        }
        aglDestroyRendererInfo(head_info);
        if (found) // if we found a card that has enough VRAM and meets the accel criteria
        {
            if (MinVRAM > dMaxVRAM)
                MinVRAM = dMaxVRAM; // return VRAM
        }
        else
            goodCheck = false; // one device failed thus entire requirement fails
        hGD = GetNextDevice (hGD); // get next device
    } // while
    if (goodCheck) // we check all devices and each was good
    {
        *pVRAM = MinVRAM; // return VRAM
        return true;
    }
    return false; //at least one device failed to have mins
}

//-----------------------------------------------------------------------------------------------------------------------

// CheckWindowExtents

// checks to see window fits on screen completely

// Inputs: 	hGD: GDHandle to device to look at
//			width/height: requested width and height of window

// Returns: true if window and borders fit, false otherwise

static Boolean CheckWindowExtents (GDHandle hGD, short width, short height)
{
    Rect strucRect, rectWin = {0, 0, 1, 1};
    short deviceHeight = (short) ((**hGD).gdRect.bottom - (**hGD).gdRect.top - GetMBarHeight ());	
    short deviceWidth = (short) ((**hGD).gdRect.right - (**hGD).gdRect.left);
    short windowWidthExtra, windowHeightExtra;
    // build window (not visible)
    WindowPtr pWindow = NewCWindow (NULL, &rectWin, "\p", true, kWindowType, (WindowPtr)-1, 0, 0);
    
    GetWindowBounds (pWindow, kWindowStructureRgn, &strucRect);
    windowWidthExtra = (short) ((strucRect.right - strucRect.left) - 1);
    windowHeightExtra = (short) ((strucRect.bottom - strucRect.top) - 1);
    DisposeWindow (pWindow);
    if ((width + windowWidthExtra <= deviceWidth) &&
        (height + windowHeightExtra <= deviceHeight))
        return true;
    return false;
}

// --------------------------------------------------------------------------

// DumpCurrent

// Kills currently allocated context
// does not care about being pretty (assumes display is likely faded)

// Inputs: 	paglDraw, paglContext, pdspContext: things to be destroyed

void DumpCurrent (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, pstructGLInfo pcontextInfo)
{
	if (*pdspContext)
		DSpReportError (DSpContext_CustomFadeGammaOut (NULL, NULL, fadeTicks));

	if (*paglContext)
	{
		aglSetCurrentContext (NULL);
		aglReportError ();
		aglSetDrawable (*paglContext, NULL);
		aglReportError ();
		aglDestroyContext (*paglContext);
		aglReportError ();
		*paglContext = NULL;
	}
	
	if (pcontextInfo->fmt)
	{
		aglDestroyPixelFormat (pcontextInfo->fmt); // pixel format is no longer needed
		aglReportError ();
	}
	pcontextInfo->fmt = 0;

	if (*paglDraw && !(pcontextInfo->fFullscreen && CheckMacOSX ())) // do not destory a window on DSp if in Mac OS X
																	 // since there is no window built in X
        DisposeWindow (GetWindowFromPort (*paglDraw));
	*paglDraw = NULL;
	DestroyDSpContext (pdspContext); // fades in, safe to call at all times
}

#pragma mark -
// --------------------------------------------------------------------------

// BuildGLContext

// Builds OpenGL context

// Inputs: 	hGD: GDHandle to device to look at
//			pcontextInfo: request and requirements for cotext and drawable

// Outputs: paglContext as allocated
//			pcontextInfo:  allocated parameters

// if fail to allocate: paglContext will be NULL
// if error: will return error paglContext will be NULL

static OSStatus BuildGLContext (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext,
							  GDHandle hGD, pstructGLInfo pcontextInfo, AGLContext aglShareContext)
{
    OSStatus err = noErr;
    NumVersion versionDSp = GetDSpVersion ();

    if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) // check for existance of OpenGL
    {
        ReportError ("OpenGL not installed");
        return noErr;
    }	
    
    // DSp has problems on Mac OS X with DSp version less than 1.99 so use agl full screen
    if ((pcontextInfo->fFullscreen) && (CheckMacOSX ()) && ((versionDSp.majorRev == 0x01) && (versionDSp.minorAndBugRev < 0x99))) // need to set pixel format for full screen
    {
        short i = 0;
        while (pcontextInfo->aglAttributes[i++] != AGL_NONE) {}
        i--; // point to AGL_NONE
        pcontextInfo->aglAttributes [i++] = AGL_FULLSCREEN;
        pcontextInfo->aglAttributes [i++] = AGL_PIXEL_SIZE;
        pcontextInfo->aglAttributes [i++] = (SInt32) pcontextInfo->pixelDepth;
        pcontextInfo->aglAttributes [i++] = AGL_NONE;
    }

    pcontextInfo->fmt = aglChoosePixelFormat (&hGD, 1, pcontextInfo->aglAttributes); // get an appropriate pixel format
    aglReportError ();
    if (NULL == pcontextInfo->fmt) 
    {
        ReportError("Could not find valid pixel format");
        return noErr;
    }

    // using a default method of sharing all the contexts enables texture sharing across these contexts by default
    *paglContext = aglCreateContext (pcontextInfo->fmt, aglShareContext);				// Create an AGL context
    if (AGL_BAD_MATCH == aglGetError())
        *paglContext = aglCreateContext (pcontextInfo->fmt, 0); // unable to sahre context, create without sharing
    aglReportError ();
    if (NULL == *paglContext) 
    {
            ReportError ("Could not create context");
            return paramErr;
    }
    if (aglShareContext == NULL)
            aglShareContext = *paglContext;
    
    // set our drawable
    
    // DSp has problems on Mac OS X use DSp only when version is not less than 1.99
    if ((pcontextInfo->fFullscreen) && (CheckMacOSX ()) && !((versionDSp.majorRev == 0x01) && (versionDSp.minorAndBugRev < 0x99))) // fullscreen X late DSp
    {
            // use DSp's front buffer on Mac OS X
            *paglDraw = GetDSpDrawable (*pdspContext);
            // there is a problem in Mac OS X GM CoreGraphics that may not size the port pixmap correctly
            // this will check the vertical sizes and offset if required to fix the problem
            // this will not center ports that are smaller then a particular resolution
            {
                    short deltaV, deltaH;
                    Rect portBounds;
                    PixMapHandle hPix = GetPortPixMap (*paglDraw);
                    Rect pixBounds = (**hPix).bounds;
                    GetPortBounds (*paglDraw, &portBounds);
                    deltaV = (short) ((portBounds.bottom - portBounds.top) - (pixBounds.bottom - pixBounds.top) +
                                (portBounds.bottom - portBounds.top - pcontextInfo->height) / 2);
                    deltaH = (short) (-(portBounds.right - portBounds.left - pcontextInfo->width) / 2);
                    if (deltaV || deltaH)
                    {
                            GrafPtr pPortSave;
                            GetPort (&pPortSave);
                            SetPort ((GrafPtr)*paglDraw);
                            // set origin to account for CG offset and if requested drawable smaller than screen rez
                            SetOrigin (deltaH, deltaV);
                            SetPort (pPortSave);
                    }
            }
            if (!aglSetDrawable (*paglContext, *paglDraw))			// attach the CGrafPtr to the context
                    return aglReportError ();
    }
    // DSp has problems on Mac OS X with DSp version less than 1.99 so use agl full screen
    else if ((pcontextInfo->fFullscreen) && (CheckMacOSX ()) && ((versionDSp.majorRev == 0x01) && (versionDSp.minorAndBugRev < 0x99))) // fulscreen X early DSp
    {
            // use aglFullScreen
            short display = 0;
            if (!aglSetFullScreen (*paglContext, pcontextInfo->width, pcontextInfo->height, 60, display)) // attach fulls screen device to the context
            {
                    ReportError ("SetFullScreen failed");
                    aglReportError ();
                    return paramErr;
            }
    }
    else // not Mac OS X fullscreen:  this is for three cases 1) Mac OS 9 windowed 2) Mac OS X windowed 3) Mac OS 9 fullscreen (as you need to build a window on top of DSp for GL to work correctly
    {
        // build window as late as possible
        err = BuildDrawable (paglDraw, hGD, pcontextInfo);
        if (err != noErr)
        {
            ReportError ("Could not build drawable");
            return err;
        }
        if (!aglSetDrawable (*paglContext, *paglDraw))			// attach the CGrafPtr to the context
            return aglReportError ();
    }
    if(!aglSetCurrentContext (*paglContext))					// make the context the current context
        return aglReportError ();
    
    return err;
}

// --------------------------------------------------------------------------

// BuildDrawable

// Builds window to be used as drawable

// Inputs: 	hGD: GDHandle to device to look at
//			pcontextInfo: request and requirements for cotext and drawable

// Outputs: paglDraw as allocated
//			pcontextInfo:  allocated parameters

// if fail to allocate: paglDraw will be NULL
// if error: will return error paglDraw will be NULL

static OSStatus BuildDrawable (AGLDrawable* paglDraw, GDHandle hGD, pstructGLInfo pcontextInfo)
{
    Rect rectWin;
    RGBColor rgbSave;
    GrafPtr pGrafSave;
    OSStatus err = noErr;
    
    // center window in our context's gdevice
    rectWin.top  = (short) ((**hGD).gdRect.top + ((**hGD).gdRect.bottom - (**hGD).gdRect.top) / 2); // v center
    rectWin.top  -= pcontextInfo->height / 2;
    rectWin.left  = (short) ((**hGD).gdRect.left + ((**hGD).gdRect.right - (**hGD).gdRect.left) / 2);	// h center
    rectWin.left  -= pcontextInfo->width / 2;
    rectWin.right = (short) (rectWin.left + pcontextInfo->width);
    rectWin.bottom = (short) (rectWin.top + pcontextInfo->height);
    

    if (pcontextInfo->fFullscreen)
        *paglDraw = GetWindowPort (NewCWindow (NULL, &rectWin, "\p", 0, plainDBox, (WindowPtr)-1, 0, 0));
    else
        *paglDraw = GetWindowPort (NewCWindow (NULL, &rectWin, "\p", 0, kWindowType, (WindowPtr)-1, 0, 0));
    ShowWindow (GetWindowFromPort (*paglDraw));
    GetPort (&pGrafSave);
    SetPort ((GrafPtr)*paglDraw);
    GetForeColor (&rgbSave);
    RGBForeColor (&rgbBlack);
    GetWindowBounds (GetWindowFromPort (*paglDraw), kWindowContentRgn, &rectWin);
    PaintRect (&rectWin);
    RGBForeColor (&rgbSave); // ensure color is reset for proper blitting
    SetPort (pGrafSave);
    return err;
}

// --------------------------------------------------------------------------

// BuildGLonDevice

// Takes device single device and tries to build on it

// Inputs: 	hGD: GDHandle to device to look at
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglDraw, *paglContext and *pdspContext as allocated
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglDraw, paglContext and pdspContext will be NULL
// if error: will return error and paglDraw, paglContext and pdspContext will be NULL
// Note: *paglDraw and *pdspContext can be null is aglFullScreen is used

static OSStatus BuildGLonDevice (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, 
				  				 GDHandle hGD, pstructGLInfo pcontextInfo, AGLContext aglShareContext)
{
    GLint depthSizeSupport;
    OSStatus err = noErr;
    Boolean fCheckRenderer = false;
    NumVersion versionDSp = GetDSpVersion ();

    if (pcontextInfo->fFullscreen)
    {
        // if we are in 16 or 32 bit mode already, we can check the renderer now (we will double check later)
        if (16 <= (**(**hGD).gdPMap).pixelSize)
        {
            // check for VRAM and accelerated
            if (!CheckRenderer (hGD, &(pcontextInfo->VRAM), &(pcontextInfo->textureRAM), &depthSizeSupport, pcontextInfo->fAcceleratedMust))
            {
                ReportError ("Renderer check failed");
                return err;
            }
            else
                fCheckRenderer = true;
        }

        // only for  Mac OS 9 or less and greater than Mac OS X 10.0.2
        // DSp has problems on Mac OS X with DSp version less than 1.99 (10.0.2 or less)
        if ((!CheckMacOSX ()) || ((versionDSp.majorRev > 0x01) || ((versionDSp.majorRev == 0x01) && (versionDSp.minorAndBugRev >= 0x99))))  // DSp should be supported in version after 1.98
        {
            err = BuildDSpContext (pdspContext, hGD, depthSizeSupport, pcontextInfo);
            // we are now faded
            if ((err != noErr) || (*pdspContext == NULL))
            {
                if (err != noErr)
                    ReportErrorNum ("BuildDSpContext failed with error:", err);
                else
                    ReportError ("Could not build DrawSprocket context");
                if (*pdspContext)
                    DSpReportError (DSpContext_CustomFadeGammaIn (NULL, NULL, fadeTicks));
                return err;
            }
        }
        // else we are using aglFullScreen and no DSp work is required
    }
    else
    {
        if (pcontextInfo->pixelDepth == 0)	// default
        {
            pcontextInfo->pixelDepth = (**(**hGD).gdPMap).pixelSize;
            if (16 > pcontextInfo->pixelDepth)
                pcontextInfo->pixelDepth = 16;
        }
        if (pcontextInfo->fDepthMust && (pcontextInfo->pixelDepth != (**(**hGD).gdPMap).pixelSize))	// device depth must match and does not
        {
            ReportError ("Pixel Depth does not match device in windowed mode.");
            if (*pdspContext)
                DSpReportError (DSpContext_CustomFadeGammaIn (NULL, NULL, fadeTicks));
            return err;
        }
        // copy back the curretn depth
        pcontextInfo->pixelDepth = (**(**hGD).gdPMap).pixelSize;
        if (!CheckWindowExtents (hGD, pcontextInfo->width, pcontextInfo->height))
        {
            ReportError ("Window will not fit on device in windowed mode.");
            if (*pdspContext)
                DSpReportError (DSpContext_CustomFadeGammaIn (NULL, NULL, fadeTicks));
            return err;
        }
    }
    
    // if we have not already checked the renderer, check for VRAM and accelerated
    if (!fCheckRenderer)
    {
        if (!CheckRenderer (hGD, &(pcontextInfo->VRAM), &(pcontextInfo->textureRAM), &depthSizeSupport, pcontextInfo->fAcceleratedMust))
        {
            ReportError ("Renderer check failed");
            if (*pdspContext)
                DSpReportError (DSpContext_CustomFadeGammaIn (NULL, NULL, fadeTicks));
            return err;
        }
    }
    
    // do agl
    // need to send device #'s through this
    err = BuildGLContext (paglDraw, paglContext, pdspContext, hGD, pcontextInfo, aglShareContext);
            
    // DSp has problems on Mac OS X with DSp version less than 1.99
    if ((!CheckMacOSX ()) || ((versionDSp.majorRev > 0x01) || ((versionDSp.majorRev == 0x01) && (versionDSp.minorAndBugRev >= 0x99))))// DSp should be supported in version after 1.98
    {
        if (*pdspContext)
            DSpReportError (DSpContext_CustomFadeGammaIn (NULL, NULL, fadeTicks));
    }
    return err;
}


// --------------------------------------------------------------------------

// BuildGLonDrawable

// Takes a drawable and tries to build on it

// Inputs: 	aglDraw: a valid AGLDrawable
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglContext as allocated
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglContext will be NULL
// if error: will return error and paglContext will be NULL

static OSStatus BuildGLonWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo, AGLContext aglShareContext)
{
    GDHandle hGD = NULL;
    GrafPtr cgrafSave = NULL;
    short numDevices;
    GLint depthSizeSupport;
    OSStatus err = noErr;
    
    if (!pWindow || !pcontextInfo)
    {
        ReportError ("NULL parameter passed to BuildGLonDrawable.");
        return paramErr;
    }
    
    GetPort (&cgrafSave);
    SetPortWindowPort(pWindow);

    // check renderer VRAM and acceleration
    numDevices = FindGDHandleFromWindow (pWindow, &hGD);
    if (!pcontextInfo->fDraggable) 	// if numDevices > 1 then we will only be using the software renderer otherwise check only window device
    {
        if ((numDevices > 1) || (numDevices == 0)) // this window spans mulitple devices thus will be software only
        {
            // software renderer
            // infinite VRAM, infinite textureRAM, not accelerated
            if (pcontextInfo->fAcceleratedMust)
            {
                ReportError ("Unable to accelerate window that spans multiple devices");
                return err;
            }
        }
        else // not draggable on single device
        {
            if (!CheckRenderer (hGD, &(pcontextInfo->VRAM), &(pcontextInfo->textureRAM), &depthSizeSupport, pcontextInfo->fAcceleratedMust))
            {
                ReportError ("Renderer check failed");
                return err;
            }
        }
    }
    // else draggable so must check all for support (each device should have at least one renderer that meets the requirements)
    else if (!CheckAllDeviceRenderers (&(pcontextInfo->VRAM), &(pcontextInfo->textureRAM), &depthSizeSupport, pcontextInfo->fAcceleratedMust))
    {
        ReportError ("Renderer check failed");
        return err;
    }
    
    // do agl
    if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) // check for existance of OpenGL
    {
        ReportError ("OpenGL not installed");
        return NULL;
    }	
    // we successfully passed the renderer check

    if ((!pcontextInfo->fDraggable && (numDevices == 1)))  // not draggable on a single device
        pcontextInfo->fmt = aglChoosePixelFormat (&hGD, 1, pcontextInfo->aglAttributes); // get an appropriate pixel format
    else
        pcontextInfo->fmt = aglChoosePixelFormat (NULL, 0, pcontextInfo->aglAttributes); // get an appropriate pixel format
    aglReportError ();
    if (NULL == pcontextInfo->fmt) 
    {
        ReportError("Could not find valid pixel format");
        return NULL;
    }

    *paglContext = aglCreateContext (pcontextInfo->fmt, aglShareContext); // Create an AGL context
    if (AGL_BAD_MATCH == aglGetError())
        *paglContext = aglCreateContext (pcontextInfo->fmt, 0); // unable to sahre context, create without sharing
    aglReportError ();
    if (NULL == *paglContext) 
    {
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

// CheckMacOSX

// Runtime check to see if we are running on Mac OS X

// Inputs:  None

// Returns: 0 if < Mac OS X or version number of Mac OS X (10.0 for GM)

UInt32 CheckMacOSX (void)
{
    UInt32 response;

    if ((Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x01000))
        return response;
    else
        return 0;
}

// --------------------------------------------------------------------------

// PreflightGL

// Checks for presense of OpenGL and DSp (if required)
// Inputs: checkFullscreen: true if one wants to run fullscreen (which requires DrwSprocket currently)
// Ouputs: true if OpenGL is installed (and DrawSprocket if checkFullscreen is true

Boolean PreflightGL (Boolean checkFullscreen)
{
    if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) // check for existance of OpenGL
        return false;
    if (checkFullscreen && ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpStartup)) // check for existance of DSp
        return false;
    return true;
}

// --------------------------------------------------------------------------

// BuildGL

// Takes device and geometry request and tries to build best context and drawable
// if device does not work will walk down devices looking for first one that satisfies requirments

// Inputs: 	*pnumDevice: 0 any device, # attempt that device first, then any device
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglDraw, *paglContext and *pdspContext as allocated
//			*pnumDevice to device number in list that was used 
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglDraw, paglContext and pdspContext will be NULL
// if error: will return error and paglDraw, paglContext and pdspContext will be NULL

OSStatus BuildGL (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, 
				  short* pnumDevice, pstructGLInfo pcontextInfo, AGLContext aglShareContext)
{
    OSStatus err = noErr;
    GDHandle hGD = NULL;
    structGLInfo contextInfoSave;
    
    // clear
    *paglDraw = NULL;
    *paglContext = 0;
    *pdspContext = 0;
    contextInfoSave = *pcontextInfo; // save info to reset on failures
    
    // if we are full screen and not on Mac OS X (which will use aglFullScreen)
    if (pcontextInfo->fFullscreen)
    {
        NumVersion versionDSp = GetDSpVersion ();
        // DSp has problems on Mac OS X with DSp version less than 1.99
        if ((!CheckMacOSX ()) || ((versionDSp.majorRev > 0x01) || ((versionDSp.majorRev == 0x01) && (versionDSp.minorAndBugRev >= 0x99))))// DSp should be supported in version after 1.98
        {
            err = StartDSp ();
            if (gDSpStarted)
                gNeedFade = true;
            else
                return err;
        }
    }
    
    
    //find main device
    if (*pnumDevice == -1)
    {
        GDHandle hDevice; // check number of screens
        hGD = GetMainDevice ();
        if (NULL != hGD)
        {
            err = BuildGLonDevice (paglDraw, paglContext, pdspContext, hGD, pcontextInfo, aglShareContext);
            // find device number
            *pnumDevice = 0;
            hDevice = DMGetFirstScreenDevice (true);
            do
            {
                if (hDevice == hGD)
                    break;
                hDevice = DMGetNextScreenDevice (hDevice, true);
                (*pnumDevice)++;
            }
            while (hDevice);
            if (!hDevice)
                ReportError ("main device match not found");
        }
        else
            ReportError ("Cannot get main device");
    }

    if ((err != noErr) || (*paglContext == 0))
    {
        err = noErr;
        DumpCurrent (paglDraw, paglContext, pdspContext, pcontextInfo); // dump what ever partial solution we might have
        *pcontextInfo = contextInfoSave; // restore info
        //find target device and check this first is one exists
        if (*pnumDevice)
        {
            short i;
            hGD = DMGetFirstScreenDevice (true);
            for (i = 0; i < *pnumDevice; i++)
            {
                GDHandle hGDNext = DMGetNextScreenDevice (hGD, true);
                if (NULL == hGDNext) // ensure we did not run out of devices
                    break; // if no more devices drop out
                else
                    hGD = hGDNext; // otherwise continue
            }
            *pnumDevice = i; // record device we actually got
            err = BuildGLonDevice (paglDraw, paglContext, pdspContext, hGD, pcontextInfo, aglShareContext);
        }
    }
    
    // while we have not allocated a context or there were errors
    if ((err != noErr) || (*paglContext == 0))
    {
        err = noErr;
        DumpCurrent (paglDraw, paglContext, pdspContext, pcontextInfo); // dump what ever partial solution we might have
        *pcontextInfo = contextInfoSave; // restore info
        // now look through the devices in order
        hGD = DMGetFirstScreenDevice (true);	
        *pnumDevice = -1;
        do 
        {
            (*pnumDevice)++;
            err = BuildGLonDevice (paglDraw, paglContext, pdspContext, hGD, pcontextInfo, aglShareContext);
            if ((err != noErr) || (*paglDraw == NULL) || (*paglContext == 0))	// reset hGD only if we are not done
            {
                hGD = DMGetNextScreenDevice (hGD, true);
                DumpCurrent (paglDraw, paglContext, pdspContext, pcontextInfo); // dump what ever partial solution we might have
                *pcontextInfo = contextInfoSave; // restore info
            }
        }
        while (((err != noErr) || (*paglContext == 0)) && hGD);
    }
    return err;
}

// --------------------------------------------------------------------------

// DestroyGL

// Destroys drawable and context
// Ouputs: *paglDraw, *paglContext and *pdspContext should be 0 on exit

OSStatus DestroyGL (AGLDrawable* paglDraw, AGLContext* paglContext, DSpContextReference* pdspContext, pstructGLInfo pcontextInfo)
{
    if ((!paglContext) || (!*paglContext))
        return paramErr; // not a valid context
    glFinish ();
    DumpCurrent (paglDraw, paglContext, pdspContext, pcontextInfo);
    ShutdownDSp (); // safe to call anytime
    return noErr;
}

//-----------------------------------------------------------------------------------------------------------------------

// BuildGLFromWindow

// Takes window in the form of an AGLDrawable and geometry request and tries to build best context

// Inputs: 	aglDraw: a valid AGLDrawable (i.e., a WindowPtr)
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglContext as allocated
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglContext will be NULL
// if error: will return error and paglContext will be NULL

OSStatus BuildGLFromWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo, AGLContext aglShareContext)
{
    if (!pWindow)
        return paramErr;
    return BuildGLonWindow (pWindow, paglContext, pcontextInfo, aglShareContext);
}

// --------------------------------------------------------------------------

// DestroyGLFromWindow

// Destroys context that waas allocated with BuildGLFromWindow
// Ouputs: *paglContext should be NULL on exit

OSStatus DestroyGLFromWindow (AGLContext* paglContext, pstructGLWindowInfo pcontextInfo)
{
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

    if (pcontextInfo->fmt)
    {
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

OSStatus SuspendFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext)
{
    if (aglDraw && aglContext) // will only have a drawable
    {
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

OSStatus ResumeFullScreenGL (AGLDrawable aglDraw, AGLContext aglContext)
{
    if (aglDraw && aglContext)
    {
        ShowWindow (GetWindowFromPort (aglDraw));
        aglSetCurrentContext (aglContext);
        aglUpdateContext (aglContext);
        return aglReportError ();
    }
    return paramErr;
}

//-----------------------------------------------------------------------------------------------------------------------

// PauseGL

// Pauses gl to allow toolbox drawing

OSStatus PauseGL (AGLContext aglContext)
{
    if (aglContext)
    {
        glFinish (); // must do this to ensure the queue is complete
        aglSetCurrentContext (NULL);
        return aglReportError ();
    }
    return paramErr;
}

//-----------------------------------------------------------------------------------------------------------------------

// ResumeGL
// resumes gl to allow gl drawing

OSStatus ResumeGL (AGLContext aglContext)
{
    if (aglContext)
    {
        aglSetCurrentContext (aglContext);
        aglUpdateContext (aglContext);
        return aglReportError ();
    }
    return paramErr;
}

// --------------------------------------------------------------------------

// GetWindowDevice
// Inputs:	a valid WindowPtr
// Outputs:	the GDHandle that that window is mostly on
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
    LocalToGlobal ((Point*)& rectWind.top);	// convert to global coordinates
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

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
    char		text[256];								// Holds Our String
    va_list		ap;										// Pointer To List Of Arguments

    if (fmt == NULL)									// If There's No Text
            return;											// Do Nothing

    va_start(ap, fmt);									// Parses The String For Variables
        vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
    va_end(ap);											// Results Are Stored In Text

    glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
    glListBase(monacoFontList);								// Sets The Base Character to 32
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
    glPopAttrib();										// Pops The Display List Bits
}

//-----------------------------------------------------------------------------------------------------------------------

GLuint BuildFontGL (AGLContext ctx, GLint fontID, Style face, GLint size)
{
    GLuint listBase = glGenLists (256);
    if (aglUseFont (ctx, fontID , face, size, 0, 256, (long) listBase))
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        return listBase;
    }
    else
    {
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
//		ReportError ("DSp Warning: Must confirm switch"); // removed since it is just a warning, add back for debugging
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
    if (aglContext && ok_to_draw)
    {
        aglSetCurrentContext (aglContext);
        aglUpdateContext (aglContext);
        
        DrawGLScene();      // Here's Where We Do All The Drawing

        aglSwapBuffers(aglContext);		// send swap command
        
        ok_to_draw = 0;
        MPNotifyQueue( drawQueue, NULL, NULL, NULL );
    }
}
