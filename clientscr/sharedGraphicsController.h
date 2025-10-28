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
//  sharedGraphicsController.h

#ifndef BOINC_GFX_SS_bridge_SHARED_GFX_CONTROLLER_H
#define BOINC_GFX_SS_bridge_SHARED_GFX_CONTROLLER_H

#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

#import <Foundation/Foundation.h>
#import <OpenGL/gl.h>
#import <GLKit/GLKit.h>
#include <servers/bootstrap.h>
#import "MultiGPUMig.h"
#import "MultiGPUMigServer.h"

static NSTimer * myTimer = NULL;

#define NUM_IOSURFACE_BUFFERS 2



@interface saverOpenGLView : NSOpenGLView

- (GLuint)setupIOSurfaceTexture:(IOSurfaceRef)ioSurfaceBuffer;

@end


@interface SharedGraphicsController : NSObject <NSMachPortDelegate>
{
    bool isFromGFXApp;
    char portNameToLookUp[128];

	NSMachPort *serverPort;
	NSMachPort *localPort;

	uint32_t serverPortName;
	uint32_t localPortName;

	int32_t clientIndex;
	uint32_t nextFrameIndex;

	NSMachPort *clientPort[16];
	uint32_t clientPortNames[16];
	uint32_t clientPortCount;

    mach_port_t _ioSurfaceMachPorts[NUM_IOSURFACE_BUFFERS];
}

@property (NS_NONATOMIC_IOSONLY, readonly) GLuint currentTextureName;
- (void)init:(const char*)nameToLookUp direction:(bool)fromGFXApp;
- (void)testConnection;
- (kern_return_t)checkInClient:(mach_port_t)client_port index:(int32_t *)client_index;
- (void)portDied:(NSNotification *)notification;
- (void)sendIOSurfaceMachPortToClients: (uint32_t)index withMachPort:(mach_port_t) iosurface_port;
- (void)closeServerPort;

@end

#endif // BOINC_GFX_SS_bridge_SHARED_GFX_CONTROLLER_H
