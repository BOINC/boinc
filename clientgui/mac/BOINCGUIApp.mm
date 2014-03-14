// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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
#import <Cocoa/Cocoa.h>


// Cocoa routines which are part of CBOINCGUIApp

// Weak linking of objective-C classes is not supported before 
// OS 10.6.8 so to be compatible with OS 10.5 we must use the
// objective-C equivalent of dlopen() and dlsym().
static Class NSRunningApplicationClass = nil;   // Requires OS 10.6
static bool hasOwnsMenuBar = false;             // Requires OS 10.7


// HideThisApp() is called from CBOINCGUIApp::ShowApplication(bool)
// and replaces a call of ShowHideProcess() which is deprecated
// under OS 10.9.
void CBOINCGUIApp::HideThisApp() {
    [ NSApp hide:NSApp ];
}

// Override standard wxCocoa wxApp::CallOnInit() to allow Manager
// to run properly when launched hidden on login via Login Item. 
bool CBOINCGUIApp::CallOnInit() {
        NSAutoreleasePool *mypool = [[NSAutoreleasePool alloc] init];

        NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined 
                                        location:NSMakePoint(0.0, 0.0) 
                                   modifierFlags:0 
                                       timestamp:0 
                                    windowNumber:0 
                                         context:nil
                                         subtype:0 data1:0 data2:0]; 
        [NSApp postEvent:event atStart:FALSE];

    bool retVal = wxApp::CallOnInit();

    // Determine whether [[NSRunningApplication currentApplication] ownsMenuBar] is available
    NSBundle *bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/AppKit.framework"];
    NSError *err = nil;
    bool loaded = [bundle loadAndReturnError:&err];
    if (loaded) {
        NSRunningApplicationClass = NSClassFromString(@"NSRunningApplication");
        if (NSRunningApplicationClass != nil) {
            if ([[NSRunningApplicationClass currentApplication] respondsToSelector:@selector(ownsMenuBar)]) {
                hasOwnsMenuBar = true;
            }
        }
    }

    [mypool release];
    return retVal;
}


// Our application can get into a strange state 
// if our login item launched it hidden and the
// first time the user "opens" it he either
// double-clicks on our Finder icon or uses
// command-tab.  It becomes the frontmost
// application (with its menu in the menubar)
// but the windows remain hidden, and it does
// not receive an activate event, so we must 
// handle this case by polling.
//
// We can stop the polling after the windows
// have been shown once, since this state only
// occurs if the windows have never appeared.
//
// TODO: Can we avoid polling by using notification
// TODO: [NSApplicationDelegate applicationDidUnhide: ] ?
//
void CBOINCGUIApp::CheckPartialActivation() {
    // This code is not needed and has bad effects on OS 10.5.
    // Initializing wasVisible this way avoids the problem 
    // because we briefly own the menu bar at login on OS 10.5.
    static bool wasVisible = ![ NSApp isHidden ];
    
    ProcessSerialNumber frontPSN;
    Boolean isSame = false;
        
    if (m_bAboutDialogIsOpen) return;
    
    if (!wasVisible) {
        
        if (hasOwnsMenuBar) {   // Requires OS 10.7
            isSame = (bool)[[NSRunningApplicationClass
                                performSelector:NSSelectorFromString(@"currentApplication")]
                                performSelector:NSSelectorFromString(@"ownsMenuBar")];
        } else {
            GetFrontProcess(&frontPSN); // Deprecated in OS 10.9
            SameProcess(&m_psnCurrentProcess, &frontPSN, &isSame);
        }
        
        if (isSame) {
            ShowInterface();
        }
        wasVisible = ![ NSApp isHidden ];
    }
}
