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
//  sharedGraphicsController.m

// On OS 10.13 or later, use MachO communication and IOSurfaceBuffer to
// display the graphics output of our child graphics apps in our window.
// All code past this point is for that implementation.

// Adapted from Apple Developer Tech Support Sample Code MultiGPUIOSurface:
// <https://developer.apple.com/library/content/samplecode/MultiGPUIOSurface>

#import "sharedGraphicsController.h"
#include <servers/bootstrap.h>
#include <notify.h>

#define CREATE_LOG 0

#if CREATE_LOG
#ifdef __cplusplus
extern "C" {
#endif
static void print_to_log_file(const char *format, ...);
static void strip_cr(char *buf);
#ifdef __cplusplus
}
#endif
#else
#define print_to_log_file(...)
#endif

SharedGraphicsController *GFXIn_SharedGraphicsController = NULL;
SharedGraphicsController *GFXOut_SharedGraphicsController = NULL;
int notification_token = 0;
mach_port_t notification_port;


@implementation SharedGraphicsController

- (void)init:(const char*)nameToLookUp direction:(bool)fromGFXApp{
    isFromGFXApp = fromGFXApp;
    strlcpy(portNameToLookUp, nameToLookUp, sizeof(portNameToLookUp));
    clientPortCount = 0;
    for(int i = 0; i < NUM_IOSURFACE_BUFFERS; i++) {
        _ioSurfaceMachPorts[i] = MACH_PORT_NULL;
    }

    if (fromGFXApp) {
        [[NSNotificationCenter defaultCenter] removeObserver:self
            name:NSPortDidBecomeInvalidNotification object:nil];

        if (myTimer == NULL) {
            myTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval) 1./60. target:self selector:@selector(testConnection)
                                        userInfo:nil repeats:YES];

        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(portDied:) name:NSPortDidBecomeInvalidNotification object:nil];
        }

        [self testConnection];
    } else {
        mach_port_t servicePortNum = MACH_PORT_NULL;
        kern_return_t machErr;

        // NSMachBootstrapServer is deprecated in OS 10.13, so use bootstrap_look_up
        //	serverPort = [(NSMachPort *)([[NSMachBootstrapServer sharedInstance] portForName:@"edu.berkeley.boincsaver"]) retain];
        machErr = bootstrap_look_up(bootstrap_port, portNameToLookUp, &servicePortNum);
        if (machErr != KERN_SUCCESS) {
            fprintf(stderr, "Failed to connect to screensaver\n");
            fflush(stderr);
            print_to_log_file("terminated in testConnection");
            exit(0);
        }

        int32_t dummy_index;
        [self checkInClient:servicePortNum index:&dummy_index];

        serverPort = (NSMachPort*)[NSMachPort portWithMachPort:servicePortNum];

        // Register server port with the current runloop.
        [serverPort setDelegate:self];
        notification_port = servicePortNum;
        //int retval =
            notify_register_mach_port([NSPortDidBecomeInvalidNotification UTF8String], &notification_port, NOTIFY_REUSE, & notification_token);
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(portDied:) name:NSPortDidBecomeInvalidNotification object:nil];
    }

    if (fromGFXApp) {
        [[NSRunLoop currentRunLoop] run];
    }
}


- (void) testConnection
{
    mach_port_t servicePortNum = MACH_PORT_NULL;
    kern_return_t machErr;

    // Try to check in with master.
    // NSMachBootstrapServer is deprecated in OS 10.13, so use bootstrap_look_up
    //	serverPort = [(NSMachPort *)([[NSMachBootstrapServer sharedInstance] portForName:@"edu.berkeley.boincsaver"]) retain];
    machErr = bootstrap_look_up(bootstrap_port, portNameToLookUp, &servicePortNum);

    if (machErr == KERN_SUCCESS) {
        serverPort = (NSMachPort*)[NSMachPort portWithMachPort:servicePortNum];
    } else {
        serverPort = MACH_PORT_NULL;
    }

    if(serverPort != MACH_PORT_NULL) {
        // Create our own local port.
        localPort = [[NSMachPort alloc] init];

        // Retrieve raw mach port names.
        serverPortName = [serverPort machPort];
        localPortName  = [localPort machPort];

        // Register our local port with the current runloop.
        [localPort setDelegate:self];
        [localPort scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

        // Check in with server.
        int kr;
        kr = _MGCCheckinClient(serverPortName, localPortName, &clientIndex);
        if(kr != 0) {
            print_to_log_file("terminated in testConnection");
            exit(0);
        }

        if (myTimer) {
            [ myTimer invalidate ];
            myTimer = NULL;
        }
    }
}


- (void)portDied:(NSNotification *)notification
{
	NSPort *port = [notification object];
    if(port == serverPort) {
        if (isFromGFXApp) {
            [ self closeServerPort ];
            if (serverPort != MACH_PORT_NULL) {
               [serverPort removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            }

            if ((serverPort == MACH_PORT_NULL) && (localPort == MACH_PORT_NULL)) {
                int i;
                for(i = 0; i < NUM_IOSURFACE_BUFFERS; i++) {
                    if (_ioSurfaceMachPorts[i] != MACH_PORT_NULL) {
                        mach_port_deallocate(mach_task_self(), _ioSurfaceMachPorts[i]);
                        _ioSurfaceMachPorts[i] = MACH_PORT_NULL;
                    }
                }

                if (myTimer == NULL) {
                    myTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval) 1./60. target:self selector:@selector(testConnection)
                                                userInfo:nil repeats:YES];
                }
            }
        } else {
            print_to_log_file("terminated in portDied");
            exit(0);
        }
    } else if (!isFromGFXApp) {
        int i;
		for(i = 0; i < clientPortCount+1; i++)
		{
			if([clientPort[i] isEqual:port])
			{
//				[clientPort[i] release];
				clientPort[i] = nil;
				clientPortNames[i] = 0;
			}
		}
    }
}

- (void)closeServerPort
{
    if ([serverPort isValid]) {
        [serverPort invalidate];
//            [serverPort release];
    }
    serverPort = MACH_PORT_NULL;

    if (isFromGFXApp) {
        if (localPort != MACH_PORT_NULL) {
            [localPort removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

            if ([localPort isValid]) {
                [localPort invalidate];
            }
    //          [localPort release];
            localPort = MACH_PORT_NULL;

            int i;
            for(i = 0; i < NUM_IOSURFACE_BUFFERS; i++) {
                if (_ioSurfaceMachPorts[i] != MACH_PORT_NULL) {
                    mach_port_deallocate(mach_task_self(), _ioSurfaceMachPorts[i]);
                    _ioSurfaceMachPorts[i] = MACH_PORT_NULL;
                }
            }
        }
    }
}

- (void)handleMachMessage:(void *)msg
{
	union __ReplyUnion___MGCMGSServer_subsystem reply;

	mach_msg_header_t *reply_header = (mach_msg_header_t *)&reply;
	kern_return_t kr;

	if(MGSServer_server((mach_msg_header_t *)msg, reply_header) && reply_header->msgh_remote_port != MACH_PORT_NULL)
	{
		kr = mach_msg(reply_header, MACH_SEND_MSG, reply_header->msgh_size, 0, MACH_PORT_NULL,
			     0, MACH_PORT_NULL);
        if(kr != 0) {
            print_to_log_file("terminated in MGSServer_server");
			exit(0);
        }
	}
}

- (kern_return_t)displayFrame:(int32_t)frameIndex surfacemachport:(mach_port_t)iosurface_port
{
	nextFrameIndex = frameIndex;

	if(!_ioSurfaceMachPorts[frameIndex])
	{
        _ioSurfaceMachPorts[frameIndex] = iosurface_port;
	}

    if (isFromGFXApp) {
        // TODO: Should we keep frame index of GFXIn_SharedGraphicsController separate from GFXOut_SharedGraphicsController?
        [ GFXOut_SharedGraphicsController sendIOSurfaceMachPortToClients:frameIndex withMachPort:iosurface_port];
    }
	return 0;
}

- (kern_return_t)checkInClient:(mach_port_t)client_port index:(int32_t *)client_index
{
	clientPortCount++;			// clients always start at index 1
	clientPortNames[clientPortCount] = client_port;
	clientPort[clientPortCount] = [[NSMachPort alloc] initWithMachPort:client_port];

	*client_index = clientPortCount;
	return 0;
}

kern_return_t _MGSCheckinClient(mach_port_t server_port, mach_port_t client_port,
			       int32_t *client_index)
{
//    if (isFromGFXApp) {
        // For the MachO client, this is a no-op.
//        return 0;
//    }

    return [GFXOut_SharedGraphicsController checkInClient:client_port index:client_index];
}

kern_return_t _MGSDisplayFrame(mach_port_t server_port, int32_t frame_index, mach_port_t iosurface_port)
{
//    if (!isFromGFXApp) {
    // For the MachO server, this is a no-op
//    return 0;
//    }

    return [GFXIn_SharedGraphicsController displayFrame:frame_index surfacemachport:iosurface_port];
}

- (void)sendIOSurfaceMachPortToClients:(uint32_t)index withMachPort:(mach_port_t)iosurface_port
{
	int i;
	for(i = 0; i < clientPortCount+1; i++)
	{
		if(clientPortNames[i])
		{
			_MGCDisplayFrame(clientPortNames[i], index, iosurface_port);
		}
	}
}

@end

#if CREATE_LOG

#include <sys/stat.h>

static void print_to_log_file(const char *format, ...) {
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;

    f = fopen("/Users/Shared/test_log_gfx_ss_bridge.txt", "a");
    if (!f) return;

//  freopen(buf, "a", stdout);
//  freopen(buf, "a", stderr);

    time(&t);
    strlcpy(buf, asctime(localtime(&t)), sizeof(buf));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);

    fputs("\n", f);
    fflush(f);
    fclose(f);
    chmod("/Users/Shared/test_log_gfx_ss_bridge.txt", 0666);
}

static void strip_cr(char *buf)
{
    char *theCR;

    theCR = strrchr(buf, '\n');
    if (theCR)
        *theCR = '\0';
    theCR = strrchr(buf, '\r');
    if (theCR)
        *theCR = '\0';
}
#endif	// CREATE_LOG
