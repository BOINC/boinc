// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

//
//  macglutfix.m
//

#define CREATE_LOG 0    // Set to 1 for debugging

#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

#include <Cocoa/Cocoa.h>
#include <mach/mach_time.h>
#include <pthread.h>
#import <OpenGL/CGLIOSurface.h>
#import <GLKit/GLKit.h>
#include <servers/bootstrap.h>
#import "MultiGPUMig.h"
#import "MultiGPUMigServer.h"
#include "x_opengl.h"
#include "boinc_gl.h"
#include "boinc_glut.h"

extern bool fullscreen; // set in graphics2_unix.cpp

// For unknown reason, "boinc_api.h" gets a compile
// error here so just declare boinc_is_standalone()
//#include "boinc_api.h"
extern int boinc_is_standalone(void);

// int set_realtime(int period, int computation, int constraint);
void MacGLUTFix(bool isScreenSaver);
void BringAppToFront(void);
int compareOSVersionTo(int toMajor, int toMinor);

// The standard ScreenSaverView class actually sets the window
// level to 2002, not the 1000 defined by NSScreenSaverWindowLevel
// and kCGScreenSaverWindowLevel
#define RealSaverLevel 2002
// Glut sets the window level to 100 when it sets full screen mode
#define GlutFullScreenWindowLevel 100

// Delay when switching to screensaver mode to reduce annoying flashes
#define SAVERDELAY 30

// NSClosableWindowMask is deprecated in OS 10.12 and is replaced by
// NSWindowStyleMaskClosable, which is not defined before OS 10.12
#ifndef NSWindowStyleMaskClosable
#define NSWindowStyleMaskClosable NSClosableWindowMask
#endif

static NSWindow* myWindow = nil;

void MacGLUTFix(bool isScreenSaver) {
    static int count = 0;
    static NSMenu * emptyMenu;
    NSOpenGLContext * myContext = nil;
    NSView *myView = nil;
    static int requestedWidth, requestedHeight;

    if (count == 2) ClearDocumentEditedDot();
    if (count++ > 2) return;   // Do the code below only twice

    if (! boinc_is_standalone()) {
        if (emptyMenu == nil) {
            emptyMenu = [ NSMenu alloc ];
            [ NSApp setMainMenu:emptyMenu ];
        }
    }

    myContext = [ NSOpenGLContext currentContext ];
    if (myContext)
        myView = [ myContext view ];
    if (myView)
        myWindow = [ myView window ];
    if (myWindow == nil)
        return;

    // Retina displays have 2X2 pixels per point. When OpenGL / GLUT apps built
    // using Xcode 11 are run on a Retina display under OS 10.15, they fail to
    // adjust their pixel dimesions to double the window size, and so fill only
    // 1/4 of the window (display at half width and height) until they are
    // resized by calls to glutReshapeWindow(), glutFullScreen(). etc.
    // However, they work correctly when run on earlier  versions of OS X.
    //
    // OpenGL / GLUT apps built using earlier versions of Xcode do not have this
    // problem on OS 10.15, but glutReshapeWindow().
    //
    // We work around this by calling glutReshapeWindow() twice, restoring the
    // window's original size. This transparently fixes the problem when necessary
    // without actually changing the window's size, and does no harm when not
    // necessary.

    if (!isScreenSaver) {
        if (count == 1) {
            requestedWidth = glutGet(GLUT_WINDOW_WIDTH);
            requestedHeight = glutGet(GLUT_WINDOW_HEIGHT);
            glutReshapeWindow(requestedWidth+1, requestedHeight);
        } else {
            glutReshapeWindow(requestedWidth, requestedHeight);
        }

        [myWindow setStyleMask:[myWindow styleMask] | NSWindowStyleMaskClosable];

        return;
    }

    // As of OS 10.13, app windows can no longer appear on top of screensaver
    // window, but we still use this method on older versions of OS X for
    // compatibility with older project graphics apps.
    if (!UseSharedOffscreenBuffer()) {
        // In screensaver mode, set our window's level just above
        // our BOINC screensaver's window level so it can appear
        // over it.  This doesn't interfere with the screensaver
        // password dialog because the dialog appears only after
        // our screensaver is closed.
        if ([ myWindow level ] == GlutFullScreenWindowLevel) {
            [ myWindow setLevel:RealSaverLevel+20 ];
        }
    }
}

void ClearDocumentEditedDot(void) {
    if (myWindow) {
        [myWindow setDocumentEdited: NO];
    }
}

#if 0
// NOT USED: See comments in animateOneFrame in Mac_Saver_ModuleView.m
// <https://developer.apple.com/library/content/technotes/tn2169>
int set_realtime(int period, int computation, int constraint) {
    mach_timebase_info_data_t timebase_info;
    mach_timebase_info(&timebase_info);

    const uint64_t NANOS_PER_MSEC = 1000000ULL;
    double clock2abs = ((double)timebase_info.denom / (double)timebase_info.numer) * NANOS_PER_MSEC;

    thread_time_constraint_policy_data_t policy;
    policy.period      = period;
    policy.computation = (uint32_t)(computation * clock2abs); // computation ms of work
    policy.constraint  = (uint32_t)(constraint * clock2abs);
//    policy.preemptible = FALSE;
    policy.preemptible = TRUE;

    int kr = thread_policy_set(pthread_mach_thread_np(pthread_self()),
                   THREAD_TIME_CONSTRAINT_POLICY,
                   (thread_policy_t)&policy,
                   THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "set_realtime() failed.\n");
        return 0;
    }
    return 1;
}
#endif

void BringAppToFront() {
    [ NSApp activateIgnoringOtherApps:YES ];
}

void HideThisApp() {
    [ NSApp hide:NSApp ];
}

// On OS 10.13 or later, use MachO comunication and IOSurfaceBuffer to
// display the graphics output of our child graphics apps in our window.

// Code adapted from Apple Developer Tech Support Sample Code MutiGPUIOSurface:
// <https://developer.apple.com/library/content/samplecode/MultiGPUIOSurface>

#define NUM_IOSURFACE_BUFFERS 2

@interface ServerController : NSObject <NSMachPortDelegate>
{
    NSMachPort *serverPort;

	NSMachPort *clientPort[16];
	uint32_t clientPortNames[16];
	uint32_t clientPortCount;

    bool mach_bootstrap_unavailable_to_screensavers;
}
- (ServerController *)init;
- (kern_return_t)checkInClient:(mach_port_t)client_port index:(int32_t *)client_index;
- (void)portDied:(NSNotification *)notification;
- (void)sendIOSurfaceMachPortToClients: (uint32_t)index withMachPort:(mach_port_t) iosurface_port;

@end

static ServerController *myserverController;

static uint32_t currentFrameIndex;

static IOSurfaceRef ioSurfaceBuffers[NUM_IOSURFACE_BUFFERS];
static mach_port_t ioSurfaceMachPorts[NUM_IOSURFACE_BUFFERS];
static GLuint textureNames[NUM_IOSURFACE_BUFFERS];
static GLuint fboNames[NUM_IOSURFACE_BUFFERS];
static GLuint depthBufferName;

@implementation ServerController

- (ServerController *)init
{
    mach_port_t servicePortNum = MACH_PORT_NULL;
    kern_return_t machErr;
    char *portNameV1 = "edu.berkeley.boincsaver";
    char *portNameV2 = "edu.berkeley.boincsaver-v2";

    mach_bootstrap_unavailable_to_screensavers = false;

// NSMachBootstrapServer is deprecated in OS 10.13, so use bootstrap_look_up
//	serverPort = [(NSMachPort *)([[NSMachBootstrapServer sharedInstance] portForName:@"edu.berkeley.boincsaver"]) retain];
	machErr = bootstrap_look_up(bootstrap_port, portNameV2, &servicePortNum);
    if (machErr == KERN_SUCCESS) {
        // As of MacOS 14.0, the legacyScreenSave sandbox prevents using
        // bootstrap_look_up. I have filed bug report FB13300491 with
        // Apple and hope they will change this in a future MacOS.
        mach_bootstrap_unavailable_to_screensavers = true;

        int32_t dummy_index;
        [self checkInClient:servicePortNum index:&dummy_index];
    } else {
// NSMachBootstrapServer is deprecated in OS 10.13, so use bootstrap_check_in
//	    serverPort = [(NSMachPort *)([[NSMachBootstrapServer sharedInstance] servicePortWithName:@"edu.berkeley.boincsaver"]) retain];
        machErr = bootstrap_check_in(bootstrap_port, portNameV1, &servicePortNum);
        if (machErr != KERN_SUCCESS) {  // maybe BOOTSTRAP_UNKNOWN_SERVICE
            [NSApp terminate:self];
        }
    }

    serverPort = (NSMachPort*)[NSMachPort portWithMachPort:servicePortNum];
	[[NSNotificationCenter defaultCenter] addObserver:self
	    selector:@selector(portDied:) name:NSPortDidBecomeInvalidNotification object:nil];

    if (!mach_bootstrap_unavailable_to_screensavers) {
        // Register server port with the current runloop.
        [serverPort setDelegate:self];  // CAF STD
        [serverPort scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes]; // CAF STD
    }

    // NOT USED: See comments in animateOneFrame in Mac_Saver_ModuleView.m
#if 0
    // This is an alternate method to get enough CPU cycles when we
    // are running in the background behind ScreensaverEngine.app
    set_realtime(0, 5, 33);
    //set_realtime(0, 5, 10);
    //set_realtime(33, 5, 33);
    //set_realtime(30, 3, 6);
    //set_realtime(30, 10, 20);
#endif

    return self;
}

- (void)portDied:(NSNotification *)notification
{
	NSPort *port = [notification object];
	if(port == serverPort)
	{
		[NSApp terminate:self];
	}
	else
	{
		int i;
		for(i = 0; i < clientPortCount+1; i++)
		{
			if([clientPort[i] isEqual:port])
			{
				[clientPort[i] release];
				clientPort[i] = nil;
				clientPortNames[i] = 0;
			}
		}
	}
}

- (void)handleMachMessage:(void *)msg
{
	union __ReplyUnion___MGCMGSServer_subsystem reply;

	mach_msg_header_t *reply_header = (void *)&reply;
	kern_return_t kr;

	if(MGSServer_server(msg, reply_header) && reply_header->msgh_remote_port != MACH_PORT_NULL)
	{
		kr = mach_msg(reply_header, MACH_SEND_MSG, reply_header->msgh_size, 0, MACH_PORT_NULL,
			     0, MACH_PORT_NULL);
        if(kr != 0)
			[NSApp terminate:nil];
	}
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
	return [myserverController checkInClient:client_port index:client_index];
}

// For the MachO server, this is a no-op
kern_return_t _MGSDisplayFrame(mach_port_t server_port, int32_t frame_index, uint32_t iosurface_port)
{
	return 0;
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


// OpenGL / GLUT apps which call glutFullScreen() and are built using
// Xcode 11 apparently use window dimensions based on the number of
// backing store pixels. That is, they double the window dimensions
// for Retina displays (which have 2X2 pixels per point.) But OpenGL
// apps built under earlier versions of Xcode don't.
//
// OS 10.15 Catalina assumes OpenGL / GLUT apps work as built under
// Xcode 11, so it displays older builds at half width and height,
// unless we compensate in our code. To ensure that BOINC graphics apps
// built on all versions of Xcode work properly on all versions of OS X,
// we set the IOSurface dimensions in this module to the viewportRect
// dimensions.
//
// See also:
// [BOINC_Saver_ModuleView initWithFrame:] in clientscr/Mac_Saver_ModuleView.m
// clientscr/Mac_Saver_ModuleCiew.m and MacGLUTFix(bool isScreenSaver) above.
//
// NOTE: Graphics apps must now be linked with the IOSurface framework.
//
void MacPassOffscreenBufferToScreenSaver() {
    NSOpenGLContext * myContext = [ NSOpenGLContext currentContext ];
    int viewportRect[4];
    GLsizei w, h;
    GLuint name, namef;

    glGetIntegerv(GL_VIEWPORT, (GLint*)viewportRect);
    w = viewportRect[2];
    h = viewportRect[3];

    if (!myserverController) {
        myserverController = [[[ServerController alloc] init] retain];
    }

    if (!ioSurfaceBuffers[0]) {
        // Set up all of our iosurface buffers
        for(int i = 0; i < NUM_IOSURFACE_BUFFERS; i++) {
            ioSurfaceBuffers[i] = IOSurfaceCreate((CFDictionaryRef)@{
                (id)kIOSurfaceWidth: [NSNumber numberWithInt: w],
                (id)kIOSurfaceHeight: [NSNumber numberWithInt: h],
                (id)kIOSurfaceBytesPerElement: @4
                });
            ioSurfaceMachPorts[i] = IOSurfaceCreateMachPort(ioSurfaceBuffers[i]);
        }
    }

    if(!textureNames[currentFrameIndex])
    {
        CGLContextObj cgl_ctx = (CGLContextObj)[myContext CGLContextObj];

        glGenTextures(1, &name);

        glBindTexture(GL_TEXTURE_RECTANGLE, name);
        // At the moment, CGLTexImageIOSurface2D requires the GL_TEXTURE_RECTANGLE target
        CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE, GL_RGBA, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                        ioSurfaceBuffers[currentFrameIndex], 0);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Generate an FBO and bind the texture to it as a render target.
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);

        glGenFramebuffers(1, &namef);
        glBindFramebuffer(GL_FRAMEBUFFER, namef);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, name, 0);

        if(!depthBufferName)
        {
            glGenRenderbuffers(1, &depthBufferName);
            glRenderbufferStorage(GL_TEXTURE_RECTANGLE, GL_DEPTH, w, h);
        }
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, depthBufferName);

        fboNames[currentFrameIndex] = namef;
        textureNames[currentFrameIndex] = name;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // First, draw to default FBO (screen FBO)

    // To see the original rendering in the graphics app's full-screen window
    // for debugging, temporarily enable this "glutSwapBuffers" statement
//    glutSwapBuffers();  // FOR DEBUGGING ONLY

    // Copy the default FBO to the IOSurface texture's FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboNames[currentFrameIndex]);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBlitFramebuffer(0,0,w,h, 0,0,w,h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // To see the contents of the IOSurface in the graphics app's full-screen window
    // for debugging, temporarily change "#if 0" to #if 1" bin the next line:
 #if 0  // FOR DEBUGGING ONLY
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboNames[currentFrameIndex]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0,0,w,h, 0,0,w,h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#endif

   glutSwapBuffers();
    [myserverController sendIOSurfaceMachPortToClients: currentFrameIndex
                        withMachPort:ioSurfaceMachPorts[currentFrameIndex]];
    glFlush();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    currentFrameIndex = (currentFrameIndex + 1) % NUM_IOSURFACE_BUFFERS;
}

// Test OS version number on all versions of OS X without using deprecated Gestalt
// compareOSVersionTo(x, y) returns:
// -1 if the OS version we are running on is less than x.y
//  0 if the OS version we are running on is equal to x.y
// +1 if the OS version we are running on is lgreater than x.y
int compareOSVersionTo(int toMajor, int toMinor) {
    static SInt32 major = -1;
    static SInt32 minor = -1;

    if (major < 0) {
        char vers[100], *p1 = NULL;
        FILE *f;
        vers[0] = '\0';
        f = popen("sw_vers -productVersion", "r");
        if (f) {
            fscanf(f, "%s", vers);
            pclose(f);
        }
        if (vers[0] == '\0') {
            fprintf(stderr, "popen(\"sw_vers -productVersion\" failed\n");
            fflush(stderr);
            return 0;
        }
        // Extract the major system version number
        major = atoi(vers);
        // Extract the minor system version number
        p1 = strchr(vers, '.');
        minor = atoi(p1+1);
    }

    if (major < toMajor) return -1;
    if (major > toMajor) return 1;
    // if (major == toMajor) compare minor version numbers
    if (minor < toMinor) return -1;
    if (minor > toMinor) return 1;
    return 0;
}

// Code for debugging:

#if CREATE_LOG
void strip_cr(char *buf)
{
    char *theCR;

    theCR = strrchr(buf, '\n');
    if (theCR)
        *theCR = '\0';
    theCR = strrchr(buf, '\r');
    if (theCR)
        *theCR = '\0';
}
#endif

void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    va_list args;
    char buf[256];
    time_t t;
    FILE *f;
    if (fullscreen) {
        // We can't write to our home directory if running as user / group boinc_project
        f = fopen("/Users/Shared/test_log.txt", "a");
    } else {
        strlcpy(buf, getenv("HOME"), sizeof(buf));
        strlcat(buf, "/Documents/test_log.txt", sizeof(buf));
        f = fopen(buf, "a");
        // freopen(buf, "a", stdout);
        //freopen(buf, "a", stderr);
    }
    if (!f) return;
    time(&t);
    strcpy(buf, asctime(localtime(&t)));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);

    fputs("\n", f);
    fflush(f);
    fclose(f);
#endif
}

