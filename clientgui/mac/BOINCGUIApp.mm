// This file is part of BOINC.
// https://boinc.berkeley.edu
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

//  BOINCGUIApp.mm

#include "MacGUI.pch"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#import <Cocoa/Cocoa.h>

#ifndef NSEventTypeApplicationDefined
#define NSEventTypeApplicationDefined NSApplicationDefined
#endif

// Cocoa routines which are part of CBOINCGUIApp
// Override standard wxCocoa wxApp::CallOnInit() to allow Manager
// to run properly when launched hidden on login via Login Item.
bool CBOINCGUIApp::CallOnInit() {
        NSAutoreleasePool *mypool = [[NSAutoreleasePool alloc] init];

        NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0.0, 0.0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0 data1:0 data2:0];
        [NSApp postEvent:event atStart:FALSE];

   bool retVal = wxApp::CallOnInit();

    [mypool release];
    return retVal;
}


// Our application can get into a strange state
// if our login item launched it hidden and the
// first time the user "opens" it he either
// double-clicks on our Finder icon or uses
// command-tab.  It becomes the frontmost
// application but with an incomplete menubar.
//
// We can stop the polling after the windows
// have been shown once, since this state only
// occurs when the windows first appear.
//
void CBOINCGUIApp::CheckPartialActivation() {
    static bool wasHidden = [ NSApp isHidden ];

    if (wasHidden) {
        if (m_bAboutDialogIsOpen) return;

        if (! [ NSApp isHidden ]) {
            wasHidden = false;
            ShowInterface();
            wxGetApp().GetFrame()->CreateMenus();
        }
    }
}


// Returns true if file was modified since system was booted, else false
//
bool CBOINCGUIApp::WasFileModifiedBeforeSystemBoot(char * filePath) {
    NSTimeInterval upTime = [[NSProcessInfo processInfo] systemUptime];
    NSString *path = [NSString stringWithUTF8String:filePath];
    NSError *error = nil;
    NSDictionary * attrs = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:&error];
    if (attrs && !error) { // If file exists, then ...
        NSDate *fileLastModifiedDate = [attrs fileModificationDate];
        NSTimeInterval ageOfFile = -[fileLastModifiedDate timeIntervalSinceNow];
        return (ageOfFile > upTime);
    }

    return false;
}


// HideThisApp() is called from CBOINCGUIApp::ShowApplication(bool)
// and replaces a call of ShowHideProcess() which is deprecated
// under OS 10.9.
void CBOINCGUIApp::HideThisApp() {
    [ NSApp hide:NSApp ];
}


/// Determines if the current process is visible.
///
/// @return
///  true if the current process is visible, otherwise false.
///
bool CBOINCGUIApp::IsApplicationVisible() {
    return (! [ NSApp isHidden ]);
}


///
/// Shows or hides the current process.
///
/// @param bShow
///   true will show the process, false will hide the process.
///
void CBOINCGUIApp::ShowApplication(bool bShow) {
    if (bShow) {
        // unhide restores hidden windows and activates the app but doesn't bring it to front
//        [ NSApp unhide:NSApp ];
        if ([ NSApp respondsToSelector: @selector(activate)]) {
            [ NSApp activate ]; // Unavailable before MacOS 14.0
        } else {
            [ NSApp activateIgnoringOtherApps:YES ];    // Deprecated as of MacOS 14.0
        }
    } else {
        // Hides all the windows and deactivates the app
        [ NSApp hide:NSApp ];
    }
}


///
/// Gets the display name for this app
void CBOINCGUIApp::getDisplayNameForThisApp(char* pathBuf, size_t bufSize) {
    // Get the app's main bundle
    NSBundle *main = [NSBundle mainBundle];
    NSString *thePath = main.localizedInfoDictionary[(NSString *)kCFBundleNameKey];
    if (thePath == nil) {
        thePath = [NSProcessInfo processInfo].processName;
    }
    strlcpy(pathBuf, [thePath UTF8String], bufSize);
}


// NSTitledWindowMask is deprecated in OS 10.12 and is replaced by
// NSWindowStyleMaskTitled, which is not defined before OS 10.12
#ifndef NSWindowStyleMaskTitled
#define NSWindowStyleMaskTitled NSTitledWindowMask
#endif

// Returns true if at least a 5 X 5 pixel area of the
// window's title bar is entirely on the displays
// Note: Arguments are wxWidgets / Quickdraw-style coordinates
// (y=0 at top) but NSScreen coordinates have y=0 at bottom
Boolean IsWindowOnScreen(int iLeft, int iTop, int iWidth, int iHeight) {
    NSRect testFrameRect = NSMakeRect(100, 100, 200, 200);
    NSRect testContentRect = [NSWindow contentRectForFrameRect:testFrameRect
                                styleMask:NSWindowStyleMaskTitled];
    CGFloat titleBarHeight = testFrameRect.size.height - testContentRect.size.height;
    // Make sure at least a 5X5 piece of title bar is visible
    NSRect titleRect = NSMakeRect(iLeft, iTop, iWidth, titleBarHeight);
    titleRect = NSInsetRect(titleRect, 5, 5);

    NSArray *allScreens = [NSScreen screens];
    unsigned int i;
    // The geometries of windows and display arangements are such
    // that even if the title bar spans multiple windows, a 5X5
    // section is on-screen only if at least one 5X5 section is
    // entirely on one or more displays, so this test is sufficient.
    unsigned int numDisplays = [ allScreens count ];
    for (i=0; i<numDisplays; i++) {
        NSScreen *aScreen = (NSScreen *)[ allScreens objectAtIndex:i ];
        NSRect visibleRect = [aScreen visibleFrame];    // Usable area of screen
        // Convert to wxWidgets coordinates (Y=0 at top of screen)
        NSRect fullScreenRect = [aScreen frame];
        visibleRect.origin.y = fullScreenRect.size.height - visibleRect.origin.y - visibleRect.size.height;

        if (NSIntersectsRect(visibleRect, titleRect)) {
            return true;
        }
    }

    return false;
}

// A second instance of BOINC Manager is sometimes launched at login
// when the Manager is launched by the login startup item and also
// by the "restore open windows" or "restore open applications"
// feature of MacOS.
// Each time a second instance of BOINC Manager is launched, it
// normally adds an additional BOINC icon to the "recently run apps"
// section of the Dock, cluttering it up. To avoid this, we call
// this routine on the second instance to tell the system to hide
// the icon in the Dock.
// https://stackoverflow.com/questions/620841/how-to-hide-the-dock-icon
void CBOINCGUIApp::SetActivationPolicyAccessory(bool hideDock) {
    if (hideDock) {
        [NSApp setActivationPolicy: NSApplicationActivationPolicyAccessory];
    } else {
        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
    }
}

extern bool s_bSkipExitConfirmation;

// Set s_bSkipExitConfirmation to true if cancelled because of logging out or shutting down
//
OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon ) {
    DescType            senderType;
    Size                actualSize;
    pid_t               senderPid;
    OSStatus            anErr;

    // Refuse to quit if a modal dialog is open.
    // Unfortunately, I know of no way to disable the Quit item in our Dock menu
    if (wxGetApp().IsModalDialogDisplayed()) {
        return userCanceledErr;
    }

    anErr = AEGetAttributePtr(appleEvt, keySenderPIDAttr, typeSInt32,
                                &senderType, &senderPid, sizeof(senderPid), &actualSize);
    if (anErr == noErr) {
        NSString * bundleID = [[NSRunningApplication runningApplicationWithProcessIdentifier:senderPid] bundleIdentifier];
        // Consider a Quit command from our Dock menu as coming from this application
        if (bundleID) {
            if (([bundleID compare:@"com.apple.dock"] != NSOrderedSame)
                    && ([bundleID compare:@"edu.berkeley.boinc"] != NSOrderedSame)) {
                s_bSkipExitConfirmation = true; // Not from our app, our dock icon or our taskbar icon
                if ([ NSApp isHidden ]) {
                // If Manager was hidden and was shut down by system when user last logged
                // out, MacOS's "Reopen windows when logging in" functionality may relaunch
                // us visible before our LaunchAgent launches us with the "autostart" arg.
                // Set the WasShutDownBySystem in our configuraiton file to tell us to
                // treat this as an autostart and launch hidden.
                    wxGetApp().SetBOINCMGRWasShutDownBySystemWhileHidden(1);
                }
                // The following may no longer be needed under wxCocoa-3.0.0
                wxGetApp().ExitMainLoop();  // Prevents wxMac from issuing events to closed frames
            }
        }
    }

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_EXIT);
    wxGetApp().GetFrame()->GetEventHandler()->AddPendingEvent(evt);
    return noErr;
}

#include "wx/osx/private.h"
#include "MacBitmapComboBox.h"

void CBOINCBitmapChoice::SetItemBitmap(unsigned int index, const wxBitmap& bitmap) {
    int x, y, topx, topy, h, w;
    if (!bitmap.IsOk()) return;
    // Get the center of this control within our window
    GetScreenPosition(&x, &y);
    GetSize(&w, &h);
    wxWindow *top = wxApp::GetMainTopWindow();
    top->GetScreenPosition(&topx, &topy);
    x = x-topx+w/2;
    y = y-topy+h/2;

    // Find the NSPopupButton control used by wxChoice
    // using its position within our window.
    NSPoint pointInWindow;
    NSWindow *window = (NSWindow*)MacGetTopLevelWindowRef();

    // Convert wxWidgets coordinates to MacOS coordinates.
    float wh = [window frame].size.height;
    pointInWindow.x = (float)x;
    pointInWindow.y = wh-(float)y;

    NSView *hitView = [[window contentView] hitTest:pointInWindow];
    if (! [hitView isKindOfClass:[NSPopUpButton class]]) {
        return;
    }
    // 'hitView' is the NSPopupButton control. Set the bitmap
    //  for menu item # index in the NSPopupButton's menu.
    NSPopUpButton *foundButton = (NSPopUpButton *)hitView;
    NSMenu *nsmenu = ((NSMenu*)[foundButton menu]);
    if (index >= [ nsmenu numberOfItems ]) return;
    NSMenuItem * item = [ nsmenu itemAtIndex:index ];
    NSImage *nsImage = bitmap.GetNSImage();
    [item setImage:nsImage];
 }

