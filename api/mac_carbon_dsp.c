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

	Disclaimer:	You may incorporate this sample code into your applications without
				restriction, though the sample code has been provided "AS IS" and the
				responsibility for its operation is 100% yours.  However, what you are
				not permitted to do is to redistribute the source as "DSC Sample Code"
				after having made changes. If you're going to re-distribute the source,
				we require that you make it clear in the source that the code was
				descended from Apple Sample Code, but that you've made changes.
        
        Adapted to BOINC by Eric Heien
*/

// project includes ---------------------------------------------------------

#include "mac_carbon_dsp.h"

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
            fadeTicks = 1;								// ensure we do not have zero fade time
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
