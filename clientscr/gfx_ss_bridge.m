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

//
//  gfx_ss_bridge
//
//  gfx_ss_bridge.m
//
// As of MacOS 14.0, the legacyScreenSaver sandbox prevents using
// bootstrap_look_up, so we use this bridging utility to relay Mach
// communications between the graphics apps and the legacyScreenSaver.


#define ShowGFXWindow false

#import <Cocoa/Cocoa.h>
#import "sharedGraphicsController.h"

extern bool simulateSS;
extern SharedGraphicsController *GFXIn_SharedGraphicsController;
extern SharedGraphicsController *GFXOut_SharedGraphicsController;
extern int IOSurfaceWidth;
extern int IOSurfaceHeight;


int main(int argc, const char * argv[]) {
    simulateSS = false;

#if ShowGFXWindow
    NSRect contentRect = NSMakeRect(300, 300, 500, 500);
    NSWindowStyleMask myWinStyle = NSWindowStyleMaskTitled
 | NSWindowStyleMaskResizable | NSWindowStyleMaskFullSizeContentView;
    NSWindow *win = [ [ NSWindow alloc ] initWithContentRect:contentRect styleMask:myWinStyle backing:NSBackingStoreBuffered defer:false ];
#endif

    if (!GFXOut_SharedGraphicsController) {
        GFXOut_SharedGraphicsController = [SharedGraphicsController alloc];
        [GFXOut_SharedGraphicsController init:NULL thePortName: "edu.berkeley.boincsaver-v3" direction:false];
    }

    if (!GFXIn_SharedGraphicsController) {
        GFXIn_SharedGraphicsController = [SharedGraphicsController alloc];
#if ShowGFXWindow
        [GFXIn_SharedGraphicsController init:win thePortName: "edu.berkeley.boincsaver" direction:true];
#else
        [GFXIn_SharedGraphicsController init:NULL thePortName: "edu.berkeley.boincsaver" direction:true];
#endif
    }

    return 0;
}
