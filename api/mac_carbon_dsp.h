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

	Disclaimer:	You may incorporate this sample code into your applications without
				restriction, though the sample code has been provided "AS IS" and the
				responsibility for its operation is 100% yours.  However, what you are
				not permitted to do is to redistribute the source as "DSC Sample Code"
				after having made changes. If you're going to re-distribute the source,
				we require that you make it clear in the source that the code was
				descended from Apple Sample Code, but that you've made changes.

        Adapted to BOINC by Eric Heien
*/

// include control --------------------------------------------------

#ifndef SetupDSp_h
#define SetupDSp_h

// includes ---------------------------------------------------------

#include "mac_carbon_gl.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif // SetupDSp_h
