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


#import <Cocoa/Cocoa.h>
#import "sharedGraphicsController.h"

extern SharedGraphicsController *GFXIn_SharedGraphicsController;
extern SharedGraphicsController *GFXOut_SharedGraphicsController;
extern int IOSurfaceWidth;
extern int IOSurfaceHeight;


int main(int argc, const char * argv[]) {
    if (!GFXOut_SharedGraphicsController) {
        GFXOut_SharedGraphicsController = [SharedGraphicsController alloc];
        [GFXOut_SharedGraphicsController init:"edu.berkeley.boincsaver-v3" direction:false];
    }

    if (!GFXIn_SharedGraphicsController) {
        GFXIn_SharedGraphicsController = [SharedGraphicsController alloc];
        [GFXIn_SharedGraphicsController init:"edu.berkeley.boincsaver" direction:true];
    }

    return 0;
}
