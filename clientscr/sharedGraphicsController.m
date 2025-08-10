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
//  aharedGraphicsController.m

// On OS 10.13 or later, use MachO comunication and IOSurfaceBuffer to
// display the graphics output of our child graphics apps in our window.
// All code past this point is for that implementation.

// Adapted from Apple Developer Tech Support Sample Code MutiGPUIOSurface:
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

bool simulateSS;
static bool okToDraw;
SharedGraphicsController *GFXIn_SharedGraphicsController = NULL;
SharedGraphicsController *GFXOut_SharedGraphicsController = NULL;
int IOSurfaceWidth;
int IOSurfaceHeight;
int notification_token = 0;
mach_port_t notification_port;


@implementation SharedGraphicsController

- (void)init:win thePortName:(const char*)nameToLookUp direction:(bool)fromGFXApp{
    isFromGFXApp = fromGFXApp;
    strlcpy(portNameToLookUp, nameToLookUp, sizeof(portNameToLookUp));
    clientPortCount = 0;
    if (fromGFXApp) {
        runningSharedGraphics = false;

        if (win) {
            screenSaverView = [win contentView];
            [win makeKeyAndOrderFront:self];
        } else {
            screenSaverView = NULL;
        }
        [[NSNotificationCenter defaultCenter] removeObserver:self
            name:NSPortDidBecomeInvalidNotification object:nil];

        openGLView = nil;

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

#if 1
        // Register server port with the current runloop.
        [serverPort setDelegate:self];  // CAF STD
        notification_port = servicePortNum;
        //int retval =
            notify_register_mach_port([NSPortDidBecomeInvalidNotification UTF8String], &notification_port, NOTIFY_REUSE, & notification_token);
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(portDied:) name:NSPortDidBecomeInvalidNotification object:nil];
#else
            localPort = [[NSMachPort alloc] init];

            // Retrieve raw mach port names.
            serverPortName = [serverPort machPort];
            localPortName  = [localPort machPort];

            // Register our local port with the current runloop.
            [localPort setDelegate:self];
            [localPort scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
#endif
    }

    if (fromGFXApp || simulateSS) {
        [[NSRunLoop currentRunLoop] run];
    }
}


- (void) testConnection
{
    mach_port_t servicePortNum = MACH_PORT_NULL;
    kern_return_t machErr;

    if (runningSharedGraphics) {
        return;
    }

    if (simulateSS) {
        machErr = bootstrap_check_in(bootstrap_port, portNameToLookUp, &servicePortNum);
    } else {
        // Try to check in with master.
        // NSMachBootstrapServer is deprecated in OS 10.13, so use bootstrap_look_up
        //	serverPort = [(NSMachPort *)([[NSMachBootstrapServer sharedInstance] portForName:@"edu.berkeley.boincsaver"]) retain];
        machErr = bootstrap_look_up(bootstrap_port, portNameToLookUp, &servicePortNum);
    }

    if (machErr == KERN_SUCCESS) {
        serverPort = (NSMachPort*)[NSMachPort portWithMachPort:servicePortNum];
    } else {
        serverPort = MACH_PORT_NULL;
    }

    if (simulateSS) {
        if ((serverPort != MACH_PORT_NULL) && (localPort == MACH_PORT_NULL)) {
            // Retrieve raw mach port names.
            serverPortName = [serverPort machPort];
            // Register server port with the current runloop.
            [serverPort setDelegate:self];
            [serverPort scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        }
    } else {
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

            runningSharedGraphics = true;
        }
    }
}

- (void)cleanUpOpenGL
{
    if (serverPort != MACH_PORT_NULL) {
       [serverPort removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    }

    if ((serverPort == MACH_PORT_NULL) && (localPort == MACH_PORT_NULL)) {
        if (openGLView) {
            if (NSThread.isMainThread) {
               [openGLView removeFromSuperview];   // Releases openGLView
            } else {
                    static saverOpenGLView *temp = 0;   // Static to allow asynchronous use
                    temp = openGLView;
                    // Both dispatch_sync and dispatch_async cause problems when
                    //  called from CScreensaver::DataManagementProc thread
                    dispatch_sync(dispatch_get_main_queue(), ^{
                        [temp removeFromSuperview];   // Releases openGLView
                });
            }
            openGLView = nil;
        }

        int i;
        for(i = 0; i < NUM_IOSURFACE_BUFFERS; i++) {
            if (_ioSurfaceBuffers[i]) {
               CFRelease(_ioSurfaceBuffers[i]);
                _ioSurfaceBuffers[i] = nil;
            }

            // if (glIsTexture(_textureNames[i])) {
                // glDeleteTextures(1, _textureNames[i]);
            // }
            _textureNames[i] = 0;

            if (_ioSurfaceMachPorts[i] != MACH_PORT_NULL) {
                mach_port_deallocate(mach_task_self(), _ioSurfaceMachPorts[i]);
                _ioSurfaceMachPorts[i] = MACH_PORT_NULL;
            }
        }

        runningSharedGraphics = false;  // Do this last!!
    }
}


- (void)portDied:(NSNotification *)notification
{
	NSPort *port = [notification object];
    if(port == serverPort) {
        if (isFromGFXApp) {
            [ self closeServerPort ];
            [ self cleanUpOpenGL ];
        } else {
            print_to_log_file("terminated in portDied");
            exit(0);
        }
    } else if ((!isFromGFXApp) && (!simulateSS)) {
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

    if ((isFromGFXApp) && (!simulateSS)) {
        if (localPort != MACH_PORT_NULL) {
            [localPort removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

            if ([localPort isValid]) {
                [localPort invalidate];
            }
    //          [localPort release];
            localPort = MACH_PORT_NULL;

#if 1
            int i;
            for(i = 0; i < NUM_IOSURFACE_BUFFERS; i++) {
                if (_ioSurfaceBuffers[i]) {
                    CFRelease(_ioSurfaceBuffers[i]);
                    _ioSurfaceBuffers[i] = nil;
                }

                // if (glIsTexture(_textureNames[i])) {
                    // glDeleteTextures(1, _textureNames[i]);
                // }
                _textureNames[i] = 0;

                if (_ioSurfaceMachPorts[i] != MACH_PORT_NULL) {
                    mach_port_deallocate(mach_task_self(), _ioSurfaceMachPorts[i]);
                    _ioSurfaceMachPorts[i] = MACH_PORT_NULL;
                }
            }
#endif
        }

        if ((serverPort == nil) && (localPort == nil)) {
            if (openGLView) {
                [openGLView removeFromSuperview];   // Releases openGLView
                openGLView = nil;
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

	if(!_ioSurfaceBuffers[frameIndex])
	{
		_ioSurfaceBuffers[frameIndex] = IOSurfaceLookupFromMachPort(iosurface_port);
        _ioSurfaceMachPorts[frameIndex] = iosurface_port;
	}

    IOSurfaceWidth = (int)IOSurfaceGetWidth((IOSurfaceRef)_ioSurfaceBuffers[frameIndex]);
    IOSurfaceHeight = (int)IOSurfaceGetHeight((IOSurfaceRef)_ioSurfaceBuffers[frameIndex]);

    if ( screenSaverView &&(openGLView == nil)) {
        NSRect theframe;
        theframe.origin.x = theframe.origin.y = 0.0;
        theframe.size.width = IOSurfaceWidth;
        theframe.size.height = IOSurfaceHeight;
        openGLView = [[saverOpenGLView alloc] initWithFrame:theframe];
        [screenSaverView addSubview:openGLView];
    }

    if (openGLView) {
        if(!_textureNames[frameIndex])
        {
            _textureNames[frameIndex] = [openGLView setupIOSurfaceTexture:_ioSurfaceBuffers[frameIndex]];
        }

        okToDraw = true;    // Tell drawRect that we have real data to display

        [openGLView setNeedsDisplay:YES];
        [openGLView display];
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

- (GLuint)currentTextureName
{
	return _textureNames[nextFrameIndex];
}

@end

@implementation saverOpenGLView

- (instancetype)initWithFrame:(NSRect)frame {
    NSOpenGLPixelFormatAttribute	attribs []	=
    {
//		NSOpenGLPFAWindow,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAColorSize,		(NSOpenGLPixelFormatAttribute)32,
		NSOpenGLPFAAlphaSize,		(NSOpenGLPixelFormatAttribute)8,
		NSOpenGLPFADepthSize,		(NSOpenGLPixelFormatAttribute)24,
		(NSOpenGLPixelFormatAttribute) 0
	};

    NSOpenGLPixelFormat *pix_fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];

    if(!pix_fmt) {
        print_to_log_file("terminated in saverOpenGLView initWithFrame");
       exit(0);
    }

	self = [super initWithFrame:frame pixelFormat:pix_fmt];


	[[self openGLContext] makeCurrentContext];

    // drawRect is apparently called due to the above code, causing the
    // screen to flash unless we prevent any actual drawing, so tell
    // drawRect that we do not yet have real data to display
    okToDraw = false;

	return self;
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
}

- (void)update
{
    [super update];
	// Override to do nothing.
}

// Create an IOSurface backed texture
- (GLuint)setupIOSurfaceTexture:(IOSurfaceRef)ioSurfaceBuffer
{
	GLuint name;
	CGLContextObj cgl_ctx = (CGLContextObj)[[self openGLContext] CGLContextObj];

	glGenTextures(1, &name);

	glBindTexture(GL_TEXTURE_RECTANGLE, name);
    // At the moment, CGLTexImageIOSurface2D requires the GL_TEXTURE_RECTANGLE target
	CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE, GL_RGBA, (GLsizei)self.bounds.size.width, (GLsizei)self.bounds.size.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
					ioSurfaceBuffer, 0);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return name;
}

- (BOOL)isOpaque
{
	return YES;
}

// Render a quad with the the IOSurface backed texture
- (void)renderTextureFromIOSurfaceWithWidth:(GLsizei)logoWidth height:(GLsizei)logoHeight
{
    GLfloat quad[] = {
        //x, y            s, t
        (GLfloat)logoWidth, 0.0f,    0.0f, 0.0f,
        0.0f, (GLfloat)logoHeight,   0.0f, 0.0f,
        0.0f,  0.0f,     1.0f, 0.0f,
        0.0f,  0.0f,     0.0f, 1.0f
    };

    GLint		saveMatrixMode;

    glGetIntegerv(GL_MATRIX_MODE, &saveMatrixMode);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadMatrixf(quad);
    glMatrixMode(saveMatrixMode);

    glBindTexture(GL_TEXTURE_RECTANGLE, [GFXIn_SharedGraphicsController currentTextureName]);
    glEnable(GL_TEXTURE_RECTANGLE);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	//Draw textured quad
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, -1.0, 0.0);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(1.0, -1.0, 0.0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 1.0, 0.0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, 1.0, 0.0);
	glEnd();

		glDisable(GL_TEXTURE_RECTANGLE);

		glGetIntegerv(GL_MATRIX_MODE, &saveMatrixMode);
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(saveMatrixMode);

}

// OpenGL / GLUT apps which call glutFullScreen() and are built using
// Xcode 11 apparently use window dimensions based on the number of
// backing store pixels. That is, they double the window dimensions
// for Retina displays (which have 2X2 pixels per point.) But OpenGL
// apps built under earlier versions of Xcode don't.
//
// OS 10.15 Catalina assumes OpenGL / GLUT apps work as built under
// Xcode 11, so it displays older builds at half width and height,
// unless we compensate in our code.
//
// To ensure that BOINC graphics apps built on all versions of Xcode work
// properly on different versions of OS X, we set the IOSurface dimensions
// in this module to double the screen dimensions when running under
// OS 10.15 or later.
//
// See also MacGLUTFix(bool isScreenSaver) in api/macglutfix.m for more info.
//
// NOTE: Graphics apps must now be linked with the IOSurface framework.
//
- (void)drawRect:(NSRect)theRect
{
    glViewport(0, 0, (GLint)[[self window]frame].size.width*DPI_multiplier, (GLint)[[self window] frame].size.height*DPI_multiplier);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // drawRect is apparently called before we have real data to display,
    // causing the screen to flash unless we prevent any actual drawing.
    if (!okToDraw) {
        [[self openGLContext] flushBuffer];
        return;
    }

    // MachO client draws with current IO surface contents as texture
    [self renderTextureFromIOSurfaceWithWidth:(GLsizei)self.bounds.size.width height:(GLsizei)self.bounds.size.height];

    [[self openGLContext] flushBuffer];
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
