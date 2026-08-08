// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

//  Mac_GUI.cpp

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

#include <unistd.h>
#include "sandbox.h"
#include "miofile.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"

using std::min;
using std::max;


/* Begin items to include "BOINC Manager" Mac menu items in localization templates */
void ThisDummyRoutineIsNeverCalled() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
    wxString (_("Services"));
    wxString (_("Hide %s"));
    wxString (_("Hide Others"));
    wxString (_("Show All"));
    wxString (_("Quit %s"));
#pragma clang diagnostic pop
}
/* End items to include "BOINC Manager" Mac menu items in localization templates */


// Determine if the currently logged-in user is authorized to
// perform operations which have potential security risks.
// An example is "Attach to Project", where a dishonest user might
// attach to a rogue project which could then read private files
// belonging to the user who owns the BOINC application.  This
// would be possible because the BOINC Manager runs with the
// effective user ID of its owner on the Mac.

Boolean Mac_Authorize()
{
    static Boolean      sIsAuthorized = false;
    AuthorizationRef	ourAuthRef = NULL;
    AuthorizationRights	ourAuthRights;
    AuthorizationFlags	ourAuthFlags;
    AuthorizationItem	ourAuthItem[1];
    OSStatus		err = noErr;

    if (sIsAuthorized)
        return true;

    // User is not the owner, so require admin authentication
    ourAuthItem[0].name = kAuthorizationRightExecute;
    ourAuthItem[0].value = NULL;
    ourAuthItem[0].valueLength = 0;
    ourAuthItem[0].flags = 0;

    ourAuthRights.count = 1;
    ourAuthRights.items = ourAuthItem;

    ourAuthFlags = kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;

    err = AuthorizationCreate (&ourAuthRights, kAuthorizationEmptyEnvironment, ourAuthFlags, &ourAuthRef);

    if (err == noErr) {
        sIsAuthorized = true;
        // We have authenticated user's credentials; we won't actually use the
        // privileges / rights so destroy / discard them.
        err = AuthorizationFree(ourAuthRef, kAuthorizationFlagDestroyRights);
    }

    return sIsAuthorized;
}
