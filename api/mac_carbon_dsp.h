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
	Contains:	Functions to enable building and destorying a DSp fullscreen context

	Written by:	Geoff Stahl (ggs)

	Copyright:	Copyright © 1999 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):

         <2>     3/26/01    ggs     Add DSp version check and other items for full screen on X
         <1>     1/19/01    ggs     Initial re-add
         <4>     1/26/00    ggs     Add fade code back in, ensure NULL pointer/context checks are in
         <3>     1/24/00    ggs     Add C++ support
         <2>    12/18/99    ggs     Fix headers
         <1>    11/28/99    ggs     Initial add.  Split of just DSp handling functions.  Added total
                                    device RAM checks, better step downs using actual supported
                                    resolutions. Need to add user verify for contexts that require
                                    it, integration of this in context step down, and a freq bit
                                    field.
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

#ifndef SetupDSp_h
#define SetupDSp_h

// includes ---------------------------------------------------------

#ifdef __APPLE_CC__
    #include <DrawSprocket/DrawSprocket.h>
#else
    #include <DrawSprocket.h>
#endif

#include "mac_carbon_gl.h"

#ifdef __cplusplus
extern "C" {
#endif

// structures (public) -----------------------------------------------

enum { fadeTicks = 10 };

// public function declarations -------------------------------------

NumVersion GetDSpVersion (void);
OSStatus StartDSp (void);
void ShutdownDSp (void);

CGrafPtr GetDSpDrawable (DSpContextReference dspContext);
OSStatus BuildDSpContext (DSpContextReference* pdspContext, GDHandle hGD, GLint depthSizeSupport, pstructGLInfo pcontextInfo);
void DestroyDSpContext (DSpContextReference* pdspContext);

OSStatus DSpContext_CustomFadeGammaOut (DSpContextReference inContext, const RGBColor *fadeColor, long fadeTicks); 
OSStatus DSpContext_CustomFadeGammaIn (DSpContextReference inContext, const RGBColor *fadeColor,  long fadeTicks);

extern Boolean gDSpStarted;
extern Boolean gNeedFade;

#ifdef __cplusplus
}
#endif

#endif // SetupDSp_h
