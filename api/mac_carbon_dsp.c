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
	Contains:	Functions to enable building and destroying a DSp fullscreen context

	Written by:	Geoff Stahl (ggs)

	Copyright:	Copyright © 1999 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):

         <3>     3/26/01    ggs     Add DSp version check and other items for full screen on X
         <2>     3/26/01    ggs     Add new DSp functinality for Mac OS X
         <1>     1/19/01    ggs     Initial re-add
         <7>     3/21/00    ggs     Added windowed mode and clean up various implementation details
         <6>     2/22/00    ggs     fix fades
         <5>     1/26/00    ggs     Add fade code back in, ensure NULL pointer/context checks are in
         <4>     1/24/00    ggs     add new disclaimer, protection from NULL dispose, better
                                    software renderer handling
         <3>    12/18/99    ggs     Fixed err use before init
         <2>    12/18/99    ggs     Fix headers
         <1>    11/28/99    ggs     Initial add.  Split of just DSp handling functions.  Added total
                                    device RAM checks, better step downs using actual supported
                                    resolutions. Need to add user verify for contexts that require
                                    it, integration of this in context step down, and a freq bit
                                    field.
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

// Usage notes: 
//kUseRAMCheck enables estimated video card RAM checks
#define kUseRAMCheck

// system includes ----------------------------------------------------------

#ifndef __APPLE_CC__
    #include <events.h>
    #include <sound.h>

    #include <fp.h>
#endif

// project includes ---------------------------------------------------------

#include "mac_carbon_dsp.h"

// globals (internal/private) -----------------------------------------------

enum 
{ 
	kMaxNumRes = 64, // max number of resolution slots
	kMaxRefreshFreq = 75 
}; 

Boolean gDSpStarted = false; // will never be true unless DSp is installed and start succeeds
Boolean gNeedFade = false;

// prototypes (internal/private) --------------------------------------------

DSpContextReference * ReserveUnusedDevices (GDHandle hGD);
OSStatus FreeUnusedDevices (GDHandle hGD, DSpContextReference ** ppContextRefUnused);
void BuildResolutionList (GDHandle hGD, Point * pResList, SInt32 * pFreqList);
OSStatus DoDeviceRAMCheck (pstructGLInfo pcontextInfo, Point * pResList, SInt32 * pFreqList, GLint depthSizeSupport);
Boolean DoContextStepDown (pstructGLInfo pcontextInfo, DSpContextAttributes * pContextAttributes, Point * pResList, SInt32 * pFreqList);

// functions (internal/private) ---------------------------------------------

// ReserveUnusedDevices
// reserves contexts on unused devices to vprevent their selection by DSp, returns list of these devices

DSpContextReference * ReserveUnusedDevices (GDHandle hGD)
{
    DSpContextAttributes theContextAttributes;
    DSpContextReference * pContextRefUnused = NULL;
    GDHandle hDevice = DMGetFirstScreenDevice (true); // check number of screens
    DisplayIDType displayID = 0;
    short numDevices = 0, indexDevice = 0;
    
    do {
        numDevices++;
        hDevice = DMGetNextScreenDevice (hDevice, true);
    } while (hDevice);
    numDevices--; // only count unused screens
    if (numDevices) {
        pContextRefUnused = (DSpContextReference *) NewPtr ((long) sizeof (DSpContextReference) * numDevices);
        hDevice = DMGetFirstScreenDevice (true); // check number of screens
        do {
            if (hDevice != hGD) {	// if this device is not the one the user chose
                if (noErr == DSpReportError (DMGetDisplayIDByGDevice (hDevice, &displayID, false)))
                    if (noErr == DSpReportError (DSpGetFirstContext (displayID, &pContextRefUnused [indexDevice]))) // get a context and
                        if (noErr == DSpReportError (DSpContext_GetAttributes (pContextRefUnused [indexDevice], &theContextAttributes))) // find attributes
                            DSpReportError (DSpContext_Reserve (pContextRefUnused [indexDevice], &theContextAttributes)); // reserve it
                indexDevice++;
            }
            hDevice = DMGetNextScreenDevice (hDevice, true);
        }
        while (hDevice);
    }
    return pContextRefUnused;
}

// --------------------------------------------------------------------------
// FreeUnusedDevices
// frees screen that were previously reserved to prevent selection

OSStatus FreeUnusedDevices (GDHandle hGD, DSpContextReference ** ppContextRefUnused) {
    OSStatus err = noErr;
    GDHandle hDevice = DMGetFirstScreenDevice (true); // check number of screens
    short indexDevice = 0;

    do {
        if (hDevice != hGD) {	// if this device is not the one the user chose
            err = DSpContext_Release (*ppContextRefUnused [indexDevice]); // release it
            DSpReportError (err);
            indexDevice++;
        }
        hDevice = DMGetNextScreenDevice (hDevice, true);
    }
    while (hDevice);
    
    if (*ppContextRefUnused)
        DisposePtr ((Ptr) *ppContextRefUnused);
    *ppContextRefUnused = NULL;
    return err;
}

// --------------------------------------------------------------------------
// BuildResolutionList
// builds a list of supported resolutions and frequencies for GDevice

void BuildResolutionList (GDHandle hGD, Point * pResList, SInt32 * pFreqList)
{
    DSpContextAttributes theContextAttributes;
    DSpContextReference currContext;
    OSStatus err;
    DisplayIDType displayID = 0;
    short i;
    
    for (i = 0; i < kMaxNumRes; i++)	// clear resolution list
    {
        pResList [i].h = 0x7FFF;
        pResList [i].v = 0x7FFF;
        pFreqList [i] = 0;			// some context require certain frequencies find highest for each (not higher than 85
    }

    err = DMGetDisplayIDByGDevice (hGD, &displayID, true);
    if (noErr != err)
        ReportErrorNum ("DMGetDisplayIDByGDevice error", err);
    else
    {
        if (noErr == DSpReportError (DSpGetFirstContext (displayID, &currContext)))
            do
            {	
                // insertion sort into resolution list
                if (noErr == DSpReportError (DSpContext_GetAttributes (currContext, &theContextAttributes)))
                {
                    Point pntTemp;
                    Boolean fDone = false;
                    short i = 0;
                    while ((i < kMaxNumRes) && (!fDone))
                    {
                        if ((theContextAttributes.displayWidth == pResList [i].h) && (theContextAttributes.displayHeight == pResList [i].v)) //skip
                        {
                            if ((pFreqList [i] == 0) || ((theContextAttributes.frequency <= (kMaxRefreshFreq << 16)) && (theContextAttributes.frequency > pFreqList [i])))
                                    pFreqList [i] = theContextAttributes.frequency;
                            break;
                        }
                        if (theContextAttributes.displayWidth * theContextAttributes.displayHeight < pResList [i].h * pResList [i].v) //insert
                        {
                            pntTemp = pResList [i];
                            pResList [i].h = (short) theContextAttributes.displayWidth;
                            pResList [i].v = (short) theContextAttributes.displayHeight;
                            pFreqList [i] = theContextAttributes.frequency;
                            fDone = true;
                        }
                        i++;
                    }
                    // i points to next element to switch; finish array swaps (if 
                    while ((i < kMaxNumRes) && (fDone))
                    {
                        Point pntSwitch = pResList [i];
                        pResList [i++] = pntTemp;
                        pntTemp = pntSwitch;
                    }
                }
                err = DSpGetNextContext (currContext, &currContext);
                if (noErr != err)
                {
                    if (kDSpContextNotFoundErr != err) 
                        DSpReportError (err);
                    currContext = 0;	// ensure we drop out
                }
            } 
            while (currContext);
        else
            ReportErrorNum ("DSpGetFirstContext error", err);
    }	
    // zeroize unused elements
    for (i = 0; i < kMaxNumRes; i++)
    {
        if ((pResList [i].h == 0x7FFF) || (pResList [i].v == 0x7FFF))
        {
            pResList [i].h = 0;
            pResList [i].v = 0;
        }
    }
}

// --------------------------------------------------------------------------
// DoDeviceRAMCheck
// checks requested allocation against device RAM
// Note: may modify pcontextInfo
// this should be equal or less strigent than OpenGL actual allocation to avoid failing on valid drawables

OSStatus DoDeviceRAMCheck (pstructGLInfo pcontextInfo, Point * pResList, SInt32 * pFreqList, GLint depthSizeSupport)
{
    float frontBufferFactor = 1.0f, backBufferFactor = 0.0f; // amount of screen(front) or request(back) sized buffers required, in bytes
    Point pntFrontBuffer; // size of front buffer that wil be allocated
    short i, indexFrontBuffer; 
    OSStatus err = noErr;

    // must take into account the entire front buffer, so figure out what screen resolution we are really going to use
    // find front buffer for request
    i = 0;
    while (((pResList [i].h < pcontextInfo->width) || (pResList [i].v < pcontextInfo->height)) &&
            ((pResList [i].h != 0) || (pResList [i].v != 0)) &&
            (i < kMaxNumRes))
        i++;
    // save front buffer sizes
    pntFrontBuffer.h = pResList [i].h;
    pntFrontBuffer.v = pResList [i].v;
    // if we have a valid frequnecy for the context set it (to ensure a good selection
    pcontextInfo->freq = pFreqList [i] >> 16;
    indexFrontBuffer = i;
    
    // front buffers required
    if (16 == pcontextInfo->pixelDepth)
        frontBufferFactor *= 2.0;
    else if (32 == pcontextInfo->pixelDepth)
        frontBufferFactor *= 4.0;
            
    // back buffers required	
    backBufferFactor = 0.0f;
    i = 0;
    while (64 > i)
        if (AGL_DOUBLEBUFFER == pcontextInfo->aglAttributes[i++])
        {
            if (16 == pcontextInfo->pixelDepth)
                backBufferFactor = 2.0f;
            else if (32 == pcontextInfo->pixelDepth)
                backBufferFactor = 4.0f;
            break;
        }
    i = 0;
    while (64 > i)
            if (AGL_DEPTH_SIZE == pcontextInfo->aglAttributes[i++])
            {
                    long requestDepth = pcontextInfo->aglAttributes[i];
                    GLint bit = 0x00000001;
                    short currDepth = 0, prevDepth = 0;
//			if (depthSizeSupport)
//			{
                    do
                    {
                            if (bit & depthSizeSupport)	// if the card supports the depth
                            {
                                    prevDepth = currDepth;
                                    switch (bit)
                                    {
                                            case AGL_1_BIT:
                                                    currDepth = 1;
                                                    break;
                                            case AGL_2_BIT:
                                                    currDepth = 2;
                                                    break;
                                            case AGL_3_BIT:
                                                    currDepth = 3;
                                                    break;
                                            case AGL_4_BIT:
                                                    currDepth = 4;
                                                    break;
                                            case AGL_5_BIT:
                                                    currDepth = 5;
                                                    break;
                                            case AGL_6_BIT:
                                                    currDepth = 6;
                                                    break;
                                            case AGL_8_BIT:
                                                    currDepth = 8;
                                                    break;
                                            case AGL_10_BIT:
                                                    currDepth = 10;
                                                    break;
                                            case AGL_12_BIT:
                                                    currDepth = 12;
                                                    break;
                                            case AGL_16_BIT:
                                                    currDepth = 16;
                                                    break;
                                            case AGL_24_BIT:
                                                    currDepth = 24;
                                                    break;
                                            case AGL_32_BIT:
                                                    currDepth = 32;
                                                    break;
                                            case AGL_48_BIT:
                                                    currDepth = 48;
                                                    break;
                                            case AGL_64_BIT:
                                                    currDepth = 64;
                                                    break;
                                            case AGL_96_BIT:
                                                    currDepth = 96;
                                                    break;
                                            case AGL_128_BIT:
                                                    currDepth = 128;
                                                    break;
                                    }
                            }
                            bit *= 2;
                    } while (!((requestDepth > prevDepth) && (requestDepth <= currDepth)) && (bit < AGL_128_BIT + 1));
//			}
//			else // no card depth support info
//				currDepth = requestDepth;  // we don't have card info thus assume we can support exact depth requested (may fail later but will always be equal or less stringent)
                    if ((AGL_128_BIT >= bit) && (0 != currDepth))
                            backBufferFactor += (float) currDepth / 8.0;
                    break;
            }
            
    // What we now have:
    //  pcontextInfo->width, height: request width and height
    //  pResList: sorted list of resolutions supported on this display
    //  pntFrontBuffer : size of front buffer that will currently be allocated
    //  indexFrontBuffer: position in array of current front buffer request
    //  frontBufferFactor: number of screen resolution size buffers that will be needed
    //  backBufferFactor: number of request size buffers that will be needed
    
    // if we see zero VRAM here we must be looking at the software renderer thus this check is moot.
    if (pcontextInfo->VRAM == 0)
    {
        // no changes required
        return noErr;
    }
    
    
    // find a context size that can support our texture requirements in the current total VRAM
    if ((pcontextInfo->VRAM - pcontextInfo->textureRAM) < (pntFrontBuffer.h * pntFrontBuffer.v * frontBufferFactor + 
                                                                                                                pcontextInfo->width * pcontextInfo->height * backBufferFactor))
    {
        if (pcontextInfo->fDepthMust && pcontextInfo->fSizeMust)
        {
            // cannot accomdate request
            ReportError ("Not enough total VRAM for drawable and textures (depth buffer and pixel size must be as requested)");
            return err;
        }
        else if (pcontextInfo->fSizeMust) // if we can adjust the size, try adjusting the 
        {
            // try 16 bit if must size is true
            if ((pcontextInfo->pixelDepth > 16) &&
                    (pcontextInfo->VRAM - pcontextInfo->textureRAM) > (pntFrontBuffer.h * pntFrontBuffer.v * frontBufferFactor / 2.0 + 
                                                                                                                        pcontextInfo->width * pcontextInfo->height * (backBufferFactor - 2.0)))
                    pcontextInfo->pixelDepth = 16;
            else
            {
                // cannot accomdate request
                ReportError ("Not enough total VRAM for drawable and textures");
                return err;
            }
        }
        else // can adjust size and might be able to adjust depth
        {	// make drawable fit
            Boolean fFound = false;
            // see if we can just adjust the pixel depth
            if ((pcontextInfo->pixelDepth > 16) &&	// if we are requesting 32 bit
                    (!pcontextInfo->fDepthMust) && 		// if we can adjust the pixel depth
                    (pcontextInfo->VRAM - pcontextInfo->textureRAM) > (pntFrontBuffer.h * pntFrontBuffer.v * frontBufferFactor / 2.0 + 
                                                                                                                        pcontextInfo->width * pcontextInfo->height * (backBufferFactor - 2.0)))
            {
                    fFound = true;
                    pcontextInfo->pixelDepth = 16;
            }
            else // pixel depth alone wont do it
            {
                i = (short) (indexFrontBuffer - 1);
                while (i >= 0)
                {
                    //
                    if ((pcontextInfo->VRAM - pcontextInfo->textureRAM) > (pResList [i].h * pResList [i].v * frontBufferFactor + 
                                                                                                                                pResList [i].h * pResList [i].v * backBufferFactor))
                    {
                        fFound = true;
                        pcontextInfo->width = pResList [i].h;
                        pcontextInfo->height = pResList [i].v;
                        pcontextInfo->freq = pFreqList [i] >> 16;
                        break;
                    }
                    else if ((pcontextInfo->pixelDepth > 16) &&	// if we are requesting 32 bit
                                        (!pcontextInfo->fDepthMust) && 	// if we can adjust the pixel depth
                                        (pcontextInfo->VRAM - pcontextInfo->textureRAM) > (pResList [i].h * pResList [i].v * frontBufferFactor / 2.0 + 
                                                                                                                                            pResList [i].h * pResList [i].v * (backBufferFactor - 2.0)))
                    {
                        fFound = true;
                        pcontextInfo->width = pResList [i].h;
                        pcontextInfo->height = pResList [i].v;
                        pcontextInfo->freq = pFreqList [i] >> 16;
                        pcontextInfo->pixelDepth = 16;
                        break;
                    }
                    i--;
                }
                // we tried the smallest screen size and still need to use less VRAM, adjust backbuffer to what is available
                if ((!fFound) && (((pcontextInfo->VRAM - pcontextInfo->textureRAM) - pResList [0].h * pResList [0].v * frontBufferFactor) > 0))
                {
                    float factor;
                    fFound = true;
                    factor = (float) sqrt((float) (pcontextInfo->width * pcontextInfo->height * backBufferFactor) / 
                                                (float) ((pcontextInfo->VRAM - pcontextInfo->textureRAM) - pResList [0].h * pResList [0].v * frontBufferFactor));
                    pcontextInfo->width /= factor;
                    pcontextInfo->height /= factor;
                    pcontextInfo->freq = pFreqList [0] >> 16;
                }
            }
            if (!fFound)
            {
                // cannot accomdate request
                ReportError ("Not enough total VRAM for drawable and textures");
                return err;
            }

        }
    }
    return noErr;
}

// --------------------------------------------------------------------------

// DoContextStepDown
// steps down through frequencies, depths and sizes to try to find a valid context
// bounded by flags for SizeMust and DepthMust
// Note: may modify pcontextInfo

Boolean DoContextStepDown (pstructGLInfo pcontextInfo, DSpContextAttributes * pContextAttributes, Point * pResList, SInt32 * pFreqList)
{
	// find current resolution
	short i = 0; 
	while (((pResList [i].h <= pContextAttributes->displayWidth) || (pResList [i].v <= pContextAttributes->displayHeight)) && 
			((pResList [i].h != 0) || (pResList [i].v != 0)) &&
			(i < kMaxNumRes))
		i++;
	i--; // i points to index of current resolution
	
	if (pcontextInfo->fSizeMust) // adjust depth only
	{
		if (pcontextInfo->pixelDepth > 16)	// also try pixel depth step down
		{
			pContextAttributes->displayBestDepth = 16;
			pContextAttributes->backBufferBestDepth = 16;
		}
		else
			return false; // no more options to try
	}
	else if (pcontextInfo->fDepthMust) // adjust size only
	{
		if (i > 0)
		{
			i--; // i was pointing at current resolution, now it is pointing at new resolution to try
			// set new resolution
			pContextAttributes->displayWidth = pResList [i].h;
			pContextAttributes->displayHeight = pResList [i].v;
			pcontextInfo->freq = pFreqList [i] >> 16;
		}
		else
			return false;
	}
	else // adjust size and depth
	{
		if (pContextAttributes->displayBestDepth > 16)
		{
			pContextAttributes->displayBestDepth = 16;
			pContextAttributes->backBufferBestDepth = 16;
		}
		else if (i > 0)
		{
			i--; // i was pointing at current resolution, now it is pointing at new resolution to try
			// reset pixel depth
			pContextAttributes->displayBestDepth = pcontextInfo->pixelDepth;
			pContextAttributes->backBufferBestDepth = pcontextInfo->pixelDepth;
			// set new resolution
			pContextAttributes->displayWidth = pResList [i].h;
			pContextAttributes->displayHeight = pResList [i].v;
			pcontextInfo->freq = pFreqList [i] >> 16;
		}
		else
			return false;
			
	}
	return true;
}

#pragma mark -
// functions (public) -------------------------------------------------------
// GetDSpVersion
// Gets the current version of DSp

NumVersion GetDSpVersion (void)
{
	NumVersion versionDSp = { 0, 0, 0, 0 };
	OSStatus err = noErr;
	if (!gDSpStarted)
		err = StartDSp ();
	if (noErr == err)
		versionDSp = DSpGetVersion ();
	return versionDSp;		
}

// --------------------------------------------------------------------------
// StartDSp
// handles starting up DrawSprocket

OSStatus StartDSp (void)
{
	OSStatus err = noErr;
	if (!gDSpStarted)
	{
		// check for DSp
		if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpStartup) 
		{
			ReportError ("DSp not installed");
			return kDSpNotInitializedErr;
		}	
		else
		{
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
    if (gDSpStarted)
    {
        DSpShutdown ();
        gDSpStarted = false;
    }
}

#pragma mark -
// --------------------------------------------------------------------------

// GetDSpDrawable
// Just returns the front buffer
// Inputs: 	*pdspContext
//			pcontextInfo: request and requirements for cotext and drawable
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

// --------------------------------------------------------------------------

// BuildDSpContext
// contextInfo and tries to allocate the corresponding DSp context
// Inputs: 	hGD: GDHandle to device to look at
//			pcontextInfo: request and requirements for cotext and drawable
// Outputs: *pdspContext as allocated
//			pcontextInfo:  allocated parameters
// if fail to allocate: pdspContext will be NULL
// if error: will return error pdspContext will be NULL

OSStatus BuildDSpContext (DSpContextReference* pdspContext, GDHandle hGD, GLint depthSizeSupport, pstructGLInfo pcontextInfo)
{
    DSpContextAttributes theContextAttributes, foundAttributes;
    DSpContextReference * pContextRefUnused;
    SInt32 aFreqList [kMaxNumRes];
    Point aResList [kMaxNumRes]; // list for resolution information
    OSStatus err = noErr;
    
    *pdspContext = 0;

    // check for DSp
    if (noErr != StartDSp ()) 
    {
        ReportError ("DSp startup failed");
        return noErr; // already reported
    }
    
    // reserve contexts on other screens to prevent their selection
    pContextRefUnused = ReserveUnusedDevices (hGD);
    
    // build resolution list
    BuildResolutionList (hGD, aResList, aFreqList);
    
    // handle default pixel depths
    if (pcontextInfo->pixelDepth == 0)	// default
    {
        pcontextInfo->pixelDepth = (**(**hGD).gdPMap).pixelSize;
        if (pcontextInfo->pixelDepth < 16)
            pcontextInfo->pixelDepth = 16;
    }

#ifdef kUseRAMCheck
    if (noErr != DoDeviceRAMCheck (pcontextInfo, aResList, aFreqList, depthSizeSupport))
        return err;
#endif // kUseRAMCheck

    // Note: DSp < 1.7.3 REQUIRES the back buffer attributes even if only one buffer is required
    BlockZero (&theContextAttributes, sizeof (DSpContextAttributes));
//	memset(&theContextAttributes, 0, sizeof (DSpContextAttributes));
    theContextAttributes.displayWidth				= pcontextInfo->width;
    theContextAttributes.displayHeight				= pcontextInfo->height;
    theContextAttributes.displayBestDepth			= pcontextInfo->pixelDepth;
    theContextAttributes.backBufferBestDepth			= pcontextInfo->pixelDepth;
    do
    {	
        theContextAttributes.frequency				= pcontextInfo->freq * 0x10000;
        theContextAttributes.colorNeeds				= kDSpColorNeeds_Require;
        theContextAttributes.displayDepthMask			= kDSpDepthMask_All;
        theContextAttributes.backBufferDepthMask		= kDSpDepthMask_All;
        theContextAttributes.pageCount				= 1; // only the front buffer is needed
        err = DSpFindBestContext(&theContextAttributes, pdspContext);
        if (noErr != err)	// if we had any errors, reset for next try
            if (!DoContextStepDown (pcontextInfo, &theContextAttributes, aResList, aFreqList))
                break; // have run out of options
    } while (err == kDSpContextNotFoundErr);

    // check find best context errors
    if (kDSpContextNotFoundErr == err)
    {	
        *pdspContext = 0;
        return noErr;
    }
    else if (noErr != err)
    {
        DSpReportError (err);
        *pdspContext = 0;
        return err;
    }
    
    err = DSpReportError (DSpContext_GetAttributes (*pdspContext, &foundAttributes));
    if (noErr != err)
    {
        *pdspContext = 0;
        return err;
    }
    // reset width and height to full screen and handle our own centering
    // HWA will not correctly center less than full screen size contexts
    theContextAttributes.displayWidth 		= foundAttributes.displayWidth;
    theContextAttributes.displayHeight 		= foundAttributes.displayHeight;
    theContextAttributes.pageCount		= 1; // only the front buffer is needed
    theContextAttributes.contextOptions		= 0 | kDSpContextOption_DontSyncVBL; // no page flipping and no VBL sync needed

    err = DSpReportError (DSpContext_Reserve(*pdspContext, &theContextAttributes )); // reserve our context
    if (noErr != err)
    {
        *pdspContext = 0;
        return err;
    }
    if (gNeedFade == true)
    {
        DSpReportError (DSpContext_CustomFadeGammaOut (NULL, NULL, fadeTicks));
        gNeedFade = false;
    }
    err = DSpReportError (DSpContext_SetState (*pdspContext, kDSpContextState_Active)); // activate our context
    if (noErr != err)
    {
        DSpContext_Release (*pdspContext);
        DSpReportError (DSpContext_CustomFadeGammaIn (NULL, NULL, fadeTicks));
        *pdspContext = 0;
        return err;
    }

    FreeUnusedDevices (hGD, &pContextRefUnused);
    
    if (!pcontextInfo->fSizeMust)	// if we got whatever was available
    {
        // reset inputs to what was allocated (constrain aspect ratio)
        // unless we ask for smaller, then leave the same
        if ((pcontextInfo->width > foundAttributes.displayWidth) || (pcontextInfo->height > foundAttributes.displayHeight))
        {
            float hFactor = (float) pcontextInfo->width / (float) foundAttributes.displayWidth;
            float vFactor = (float) pcontextInfo->height / (float) foundAttributes.displayHeight;
            if (hFactor > vFactor)
            {
                pcontextInfo->width = (short) foundAttributes.displayWidth;
                pcontextInfo->height /= hFactor;	
            }
            else
            {
                pcontextInfo->height = (short) foundAttributes.displayHeight;
                pcontextInfo->width /= vFactor;	
            }
        }
    }
    // else still use inputs to allocate drawable
    
    pcontextInfo->freq = foundAttributes.frequency / 0x10000;
    pcontextInfo->pixelDepth = foundAttributes.displayBestDepth;

    return noErr;
}

//-----------------------------------------------------------------------------------------------------------------------

// Deactivates and dumps context

void DestroyDSpContext (DSpContextReference* pdspContext)
{
    if (gDSpStarted)
    {
        if (*pdspContext)
        {
            DSpReportError (DSpContext_SetState(*pdspContext, kDSpContextState_Inactive));
            DSpReportError (DSpContext_CustomFadeGammaIn (NULL, NULL, fadeTicks));
            DSpReportError (DSpContext_Release (*pdspContext));
            *pdspContext = NULL;
        }
    }
}


#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

OSStatus DSpContext_CustomFadeGammaIn (DSpContextReference inContext, const RGBColor *fadeColor,  long fadeTicks) 
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
		if (fadeColor == NULL) 
		{
			inZeroIntensityColor.red = 0x0000;
			inZeroIntensityColor.green = 0x0000;
			inZeroIntensityColor.blue = 0x0000;
		}
		else 
			inZeroIntensityColor = *fadeColor;
		currTick = TickCount ();
		for (x = 1; x <= fadeTicks; x++) 
		{
			percent = step * x / 8;
			err = DSpContext_FadeGamma(inContext, percent, &inZeroIntensityColor);
			if (err != noErr) 
				break;
			while (currTick >= TickCount ()) {}
//				SystemTask ();
			currTick = TickCount ();
		}
		if (err == noErr)
			err = DSpContext_FadeGamma(inContext, 100, &inZeroIntensityColor);
	}
	return err;
}

//-----------------------------------------------------------------------------------------------------------------------

OSStatus DSpContext_CustomFadeGammaOut (DSpContextReference inContext, const RGBColor *fadeColor, long fadeTicks ) 
{
    
    OSStatus err = noErr;
    RGBColor inZeroIntensityColor;
    UInt32 currTick;
    UInt16 step = (UInt16) (800 / fadeTicks);
    long x, percent = 0;

    if (gDSpStarted)
    {
        if (fadeTicks == 0)
            fadeTicks = 1;								// ensure we do not have zero fade time
        if (fadeColor == NULL) 
        {
            inZeroIntensityColor.red = 0x0000;
            inZeroIntensityColor.green = 0x0000;
            inZeroIntensityColor.blue = 0x0000;
        }
        else 
            inZeroIntensityColor = *fadeColor;
        currTick = TickCount ();
        for (x = fadeTicks - 1; x >= 0; x--) 
        {
            percent = step * x / 8;
            err = DSpContext_FadeGamma(inContext, percent, &inZeroIntensityColor);
            if (err != noErr) 
                break;
            while (currTick >= TickCount ()) {}
//		SystemTask ();
            currTick = TickCount ();
        }
    }
    return err;
}
