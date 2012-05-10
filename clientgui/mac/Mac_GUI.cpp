// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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


// Determine if the currently logged-in user is auhorized to 
// perform operations which have potential security risks.  
// An example is "Attach to Project", where a dishonest user might
// attach to a rogue project which could then read private files 
// belonging to the user who owns the BOINC application.  This 
// would be possible because the BOINC Manager runs with the 
// effectve user ID of its owner on the Mac.

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


// Localize the items in the Mac's BOINC menu
void MacLocalizeBOINCMenu() {
    MenuRef BOINCMenu;
    MenuItemIndex itemIndex;
    wxString originalText = wxEmptyString;
    char originalCharStr[1024];
    CFStringRef localizedText;
    CFStringRef menuItemString;
    OSStatus err;
    UInt16 count;
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);
    
    const wxChar *shortName = pSkinAdvanced->GetApplicationShortName().c_str();
    if (!shortName) return;     // Should never happen
    
    err = GetIndMenuItemWithCommandID(NULL, kHICommandAbout, 1, &BOINCMenu, &itemIndex);
    if (!err) {
        originalText.Printf(_("About %s"), shortName);
        strlcpy(originalCharStr, originalText.utf8_str(), sizeof(originalCharStr));
        if (originalCharStr[0]) {
            localizedText = CFStringCreateWithCString(NULL, originalCharStr, kCFStringEncodingUTF8);
            if (localizedText) {
                SetMenuItemTextWithCFString(BOINCMenu, itemIndex, localizedText);
                CFRelease( localizedText );
            }
        }
    }
    
    originalText.Clear();
    err = GetIndMenuItemWithCommandID(NULL, kHICommandPreferences, 1, &BOINCMenu, &itemIndex);
    if (!err) {
        originalText = _("Preferences…");
        strlcpy(originalCharStr, originalText.utf8_str(), sizeof(originalCharStr));
        if (originalCharStr[0]) {
            localizedText = CFStringCreateWithCString(NULL, originalCharStr, kCFStringEncodingUTF8);
            if (localizedText) {
                SetMenuItemTextWithCFString(BOINCMenu, itemIndex, localizedText);
                CFRelease( localizedText );
            }
        }
    }
    
    originalText.Clear();
    originalText = _("Services");
        strlcpy(originalCharStr, originalText.utf8_str(), sizeof(originalCharStr));
        if (originalCharStr[0]) {
        localizedText = CFStringCreateWithCString(NULL, originalCharStr, kCFStringEncodingUTF8);
        count = CountMenuItems(BOINCMenu);
        if (localizedText) {
            for (itemIndex=1; itemIndex<=count; ++itemIndex) {
                err = CopyMenuItemTextAsCFString(BOINCMenu, itemIndex, &menuItemString);
                if (err == noErr) {
                    if (CFStringCompare(menuItemString, CFSTR("Services"), 0) == kCFCompareEqualTo) {
                        SetMenuItemTextWithCFString(BOINCMenu, itemIndex, localizedText);
                        break;
                    }
                }
            }
            CFRelease( localizedText );
        }
    }
    
    originalText.Clear();
    err = GetIndMenuItemWithCommandID(NULL, kHICommandHide, 1, &BOINCMenu, &itemIndex);
    if (!err) {
        originalText.Printf(_("Hide %s"), shortName);
        strlcpy(originalCharStr, originalText.utf8_str(), sizeof(originalCharStr));
        if (originalCharStr[0]) {
            localizedText = CFStringCreateWithCString(NULL, originalCharStr, kCFStringEncodingUTF8);
            if (localizedText) {
                SetMenuItemTextWithCFString(BOINCMenu, itemIndex, localizedText);
                CFRelease( localizedText );
            }
        }
    }

    originalText.Clear();
    err = GetIndMenuItemWithCommandID(NULL, kHICommandHideOthers, 1, &BOINCMenu, &itemIndex);
    if (!err) {
        originalText = _("Hide Others");
        strlcpy(originalCharStr, originalText.utf8_str(), sizeof(originalCharStr));
        if (originalCharStr[0]) {
            localizedText = CFStringCreateWithCString(NULL, originalCharStr, kCFStringEncodingUTF8);
            if (localizedText) {
                SetMenuItemTextWithCFString(BOINCMenu, itemIndex, localizedText);
                CFRelease( localizedText );
            }
        }
    }
    
    originalText.Clear();
    err = GetIndMenuItemWithCommandID(NULL, kHICommandShowAll, 1, &BOINCMenu, &itemIndex);
    if (!err) {
        originalText = _("Show All");
        strlcpy(originalCharStr, originalText.utf8_str(), sizeof(originalCharStr));
        if (originalCharStr[0]) {
            localizedText = CFStringCreateWithCString(NULL, originalCharStr, kCFStringEncodingUTF8);
            if (localizedText) {
                SetMenuItemTextWithCFString(BOINCMenu, itemIndex, localizedText);
                CFRelease( localizedText );
            }
        }
    }
    
    originalText.Clear();
    err = GetIndMenuItemWithCommandID(NULL, kHICommandQuit, 1, &BOINCMenu, &itemIndex);
    if (!err) {
        originalText.Printf(_("Quit %s"), shortName);
        strlcpy(originalCharStr, originalText.utf8_str(), sizeof(originalCharStr));
        if (originalCharStr[0]) {
            localizedText = CFStringCreateWithCString(NULL, originalCharStr, kCFStringEncodingUTF8);
            if (localizedText) {
                SetMenuItemTextWithCFString(BOINCMenu, itemIndex, localizedText);
                CFRelease( localizedText );
            }
        }
    }
}


#define MAX_DISPLAYS 32

// Returns true if at least a 5 X 5 pixel area of the 
// window's title bar is entirely on the displays
// Note: Arguments are Quickdraw-style coordinates, 
// but CGDisplayBounds() sets top left corner as (0, 0)
Boolean IsWindowOnScreen(int iLeft, int iTop, int iWidth, int iHeight) {
    CGDirectDisplayID displays[MAX_DISPLAYS];
    CGDisplayCount numDisplays;
    CGDisplayCount i;
    CGRect displayRect, intersectedRect;
    CGFloat mBarHeight = GetMBarHeight();

    CGRect titleRect = CGRectMake(iLeft, iTop, iWidth, 22);
    // Make sure at least a 5X5 piece of title bar is visible
    titleRect = CGRectInset(titleRect, 5, 5);   

    CGGetActiveDisplayList (MAX_DISPLAYS, displays, &numDisplays);
 
    // The geometries of windows and display arangements are such
    // that even if the title bar spans multiple windows, a 5X5
    // section is on-screen only if at least one 5X5 section is
    // entirely on one or more displays, so this test is sufficient.
    for (i = 0; i < numDisplays; i++)
    {
        displayRect = CGDisplayBounds(displays[i]);
        if (i == 0) {   // CGDisplayBounds returns main display first
            displayRect.origin.y += mBarHeight;
            displayRect.size.height -= mBarHeight;
        }
    
        intersectedRect = CGRectIntersection(displayRect, titleRect);
        if (! CGRectIsNull(intersectedRect)) {
            return true;
        }
    }

    return false;
}
