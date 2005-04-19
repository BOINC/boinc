// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//
//  macglutfix.m
//

#include <Cocoa/Cocoa.h>

#include "boinc_api.h"

void MacGLUTFix(bool isScreenSaver);
void BringAppToFront(void);

void MacGLUTFix(bool isScreenSaver) {
    static NSMenu * emptyMenu;
    NSWindow* myWindow = nil;
    int newWindowLevel = isScreenSaver ? 
                NSScreenSaverWindowLevel : NSNormalWindowLevel;

    if (! boinc_is_standalone()) {
        if (emptyMenu == nil) {
            emptyMenu = [ NSMenu alloc ];
            [ NSApp setMainMenu:emptyMenu ];
        }
    }
    
    myWindow = [ NSApp mainWindow ];
    if (myWindow == nil)
        return;
    if ([ myWindow level ] == newWindowLevel)
        return;
    [ myWindow setLevel:newWindowLevel ];
    //fprintf(stdout, "myWindow = %ld  \n", (long)myWindow);
}

void BringAppToFront() {
    [ NSApp activateIgnoringOtherApps:YES ];
}
