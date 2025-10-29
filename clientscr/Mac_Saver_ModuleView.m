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
//  Mac_Saver_ModuleView.m
//  BOINC_Saver_Module
//

// To debug BOINCSaver.saver under Xcode:
//
// [1] Copy ScreenSaverEngine.app to a location outside the /System directory
//     to allow bypassing limitations caused by System Integrity Protection
//
// [2] Prior to High Sierra ScreenSaverEngine.app is at:
//     /System/Library/Frameworks/ScreenSaver.framework/Resources/ScreenSaverEngine.app/Contents/MacOS/ScreenSaverEngine
//     As of High Sierra ScreenSaverEngine.app is at:
//     /System/Library/CoreServices/ScreenSaverEngine.app
//
// [3] In Xcode, select the ScreenSaver target as the active scheme.
//     Click on the scheme popup and select "Edit scheme ..."
//
// [4] In the Edit Scheme dialog, select "Run"
//
// [5] In the Edit Scheme dialog Info tab, in the "Build Configuration" popup select "Development"
//
// [6] In the Edit Scheme dialog Info tab, in the "Executable" popup select "Other..."
//   then browse to and select the copy of ScreenSaverEngine.app you made in step [1]
//
// [7] In the Edit Scheme dialog Arguments tab, add "-debug" and "-window" to
//     "Arguments passed on launch"
//
// [8] In the Finder, open the directory "/Library/Screen Savers" and remove "BOINCSaver.saver"
//
// [9] In Xcode's Project navigator, under "Products", control-click on "BOINCSaver.saver" and
//     select " Show in Finder"; make sure your are looking at the Development subdirectory.
//
// [10] In the Terminal application, enter "sudo ln -s " then drag the BOINCSaver.saver file
//      from the Development subdirectory onto the Terminal Window, then type
//      "/Library/Screen\ Savers/BOINCSaver.saver" (without the quotes) and press the return key.
//      Enter your password when requested.
//
// [12] In Mac_Saver_ModuleView.m, set the "#define DEBUG_UNDER_XCODE" to 1. (Be sure to set it
//      back to 0 to build the non-debugging version.)
//
// [13] In some cases, it may be useful to set the permissions and owner of gfx_switcher, which is
//      embedded in the development BOINCSaver.saver bundle, by running the Mac_SA_Secure.sh script.
//
// [14] The screensaver display will appear in a window, with graphics apps appearing full screen
//      behind it. Under High Sierra, new-style graphics apps (those using Mach-O communication
//      and IOSurfaceBuffer) will appear all white, but the bottom left portion of their animation
//      will appear in the screensaver window.
//
// It is best if the Xcode window is on a second display; otherwise the graphics apps will cover it.
// If you have only one display, you can dismiss the graphics app by clicking on it, but BOINCSaver
// will soon relaunch it.
//

#define DEBUG_UNDER_XCODE 0 // See instructions above

#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED 1

#import "Mac_Saver_ModuleView.h"
#include <Carbon/Carbon.h>
#include <AppKit/AppKit.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/hidsystem/event_status_driver.h>
#import <OpenGL/gl.h>
#import <GLKit/GLKit.h>
#include <servers/bootstrap.h>
#include <pthread.h>

#include "mac_util.h"
#import "MultiGPUMig.h"
#import "MultiGPUMigServer.h"

#ifndef NSInteger
#if __LP64__ || NS_BUILD_32_LIKE_64
typedef long NSInteger;
#else
typedef int NSInteger;
#endif
#endif

#ifndef CGFLOAT_DEFINED
typedef float CGFloat;
#endif

// NSCompositeSourceOver is deprecated in OS 10.12 and is replaced by
// NSCompositingOperationSourceOver, which is not defined before OS 10.12
#ifndef NSCompositingOperationSourceOver
#define NSCompositingOperationSourceOver NSCompositeSourceOver
#endif

// NSCompositeCopy is deprecated in OS 10.12 and is replaced by
// NSCompositingOperationCopy, which is not defined before OS 10.12
#ifndef NSCompositingOperationCopy
#define NSCompositingOperationCopy NSCompositeCopy
#endif

// NSCriticalAlertStyle is deprecated in OS 10.12 and is replaced by
// NSAlertStyleCritical, which is not defined before OS 10.12
#ifndef NSAlertStyleCritical
#define NSAlertStyleCritical NSCriticalAlertStyle
#endif

static double gSS_StartTime = 0.0;
mach_port_t gEventHandle = 0;
extern bool IsDualGPUMacbook;
extern bool ScreenIsBlanked;
extern pthread_mutex_t saver_mutex;

int gGoToBlank;      // True if we are to blank the screen
int gBlankingTime;   // Delay in minutes before blanking the screen
NSString *gPathToBundleResources = NULL;
NSString *mBundleID = NULL; // our bundle ID
NSImage *gBOINC_Logo = NULL;
NSImage *gPreview_Image = NULL;

int gTopWindowListIndex = -1;
NSInteger myWindowNumber;

NSRect gMovingRect;
float gImageXIndent;
float gTextBoxHeight;

CGContextRef myContext;
bool isErased;
char gLastMsg[2048];
HIThemeTextInfo gTextInfo;
NSPoint gCurrentDelta;
NSPoint gCurrentPosition;
NSPoint imagePosition;
bool gIsBigSur = false;
NSRect myViewBounds;

static SharedGraphicsController *mySharedGraphicsController;
static bool runningSharedGraphics;
static bool useCGWindowList;
static pid_t childPid;  // pid of the currently running graphics app, not actually our chkld
static int gfxAppWindowNum;
static NSView *imageView;
static char gfxAppPath[MAXPATHLEN];
static char gfxWuName[MAXPATHLEN];
static int taskSlot;
static NSRunningApplication *childApp;  // currently running graphics app, not actually our child
static double gfxAppStartTime;
static bool UseSharedOffscreenBuffer(void);
static double lastGetSSMsgTime;
static int CGWindowListTries;
static int DPI_multiplier = 1;
static bool myIsPreview;
static bool screensaverIsVisible = false;
static bool logoIsCovered = false;

//static long counter = 0;    // CAF
static NSTimer * myTimer = NULL;

//static CFMachPortRef eventTap = NULL;

#define TEXTBOXMINWIDTH 400.0
#define MINTEXTBOXHEIGHT 40.0
#define MAXTEXTBOXHEIGHT 300.0
#define TEXTBOXTOPBORDER 15
#define SAFETYBORDER 20.0
#define MINDELTA 8
#define MAXDELTA 16

// On OS 10.13+, assume graphics app is not compatible if no MachO
// connection after MAXWAITFORCONNECTION seconds
#define MAXWAITFORCONNECTION 8.0
#define MAXWAITFORCONNECTIONCATALINA 12.0
#define MAX_CGWINDOWLIST_TRIES 3

#define MAJORBOINCGFXLIBNEEDEDFORSONOMA 7
#define MINORBOINCGFXLIBNEEDEDFORSONOMA 24
#define RELEASEBOINCGFXLIBNEEDEDFORSONOMA 4

int signof(float x) {
    return (x > 0.0 ? 1 : -1);
}

void launchedGfxApp(char * appPath, char * wuName, pid_t thePID, int slot) {
    strlcpy(gfxAppPath, appPath, sizeof(gfxAppPath));
    strlcpy(gfxWuName, wuName, sizeof(gfxWuName));
    childPid = thePID;
    taskSlot = slot;
    gfxAppStartTime = getDTime();
    CGWindowListTries = 0;
    if (thePID == 0) {
        useCGWindowList = false;
        gfxAppStartTime = 0.0;
        gfxWuName[0] = '\0';
        if (screensaverIsVisible) {    // Must be called from main thread
            if (mySharedGraphicsController) {
                [ mySharedGraphicsController cleanUpOpenGL ];
            }
        }
        if (imageView) {
           // removeFromSuperview must be called from main thread
            if (NSThread.isMainThread) {
                [imageView removeFromSuperview];   // Releases imageView
            } else {
                dispatch_sync(dispatch_get_main_queue(), ^{
                    [imageView removeFromSuperview];   // Releases imageView
                });
            }
            imageView = nil;
        }
    } else {    // thePID != 0
        if (childPid) {
            childApp = [NSRunningApplication runningApplicationWithProcessIdentifier:childPid];
        }
    }
}


@implementation BOINC_Saver_ModuleView

// If there are multiple displays, this may get called
// multiple times (once for each display), so we need to guard
// against any problems that may cause.
- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview {
    NSBundle * myBundle;
    int period;

    self = [ super initWithFrame:frame isPreview:isPreview ];

    gIsHighSierra = (compareOSVersionTo(10, 13) >= 0);
    gIsMojave = (compareOSVersionTo(10, 14) >= 0);
    gIsCatalina = (compareOSVersionTo(10, 15) >= 0);
    gIsBigSur = (compareOSVersionTo(11, 0) >= 0);
    gIsSonoma = (compareOSVersionTo(14, 0) >= 0);

    if (gIsCatalina) {
        // Under OS 10.15, isPreview is often true even when it shouldn't be
        // so we use this hack instead
        myIsPreview = (frame.size.width < 500.) || (frame.size.height < 500.);
    } else {
        myIsPreview = isPreview;
    }

    // Apparently, setAnimationTimeInterval now works only when called
    // from initWithFrame and cannot be modified dynamically. I don't
    // know if that was always true or if a more recent change.
    [ self setAnimationTimeInterval:1.0/60.0 ];

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
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101500 // If built on Xcode 10.x or earlier
    if (!gIsBigSur)  // If built with Xcode 10.x or earlier don't do this on Big Sur
#endif
    {
        if (gIsCatalina) {
            NSArray *allScreens = [ NSScreen screens ];
            DPI_multiplier = [((NSScreen*)allScreens[0]) backingScaleFactor];
        }
    }

    screensaverIsVisible = true;
    gLastMsg[0] = '\0';
    initBOINCSaver();

    if (self) {
        if (mBundleID == NULL) {
            myBundle = [ NSBundle bundleForClass:[self class]];
            // grab the screensaver defaults
            mBundleID = [ myBundle bundleIdentifier ];

            // Path to our copy of switcher utility application in this screensaver bundle
            if (gPathToBundleResources == NULL) {
                gPathToBundleResources = [ myBundle resourcePath ];
            }

            [ self setAutoresizesSubviews:YES ];	// make sure the subview resizes.
        }

            ScreenSaverDefaults *defaults = [ ScreenSaverDefaults defaultsForModuleWithName:mBundleID ];

            // try to load the version key, used to see if we have any saved settings
            mVersion = [defaults floatForKey:@"version"];
            if (!mVersion) {
                // no previous settings so define our defaults
                gGoToBlank = NO;
                gBlankingTime = 1;

                // write out the defaults
                [ defaults setInteger:gGoToBlank forKey:@"GoToBlank" ];
                [ defaults setInteger:gBlankingTime forKey:@"BlankingTime" ];
            }

            if (mVersion < 2) {
                mVersion = 2;

                [ defaults setInteger:mVersion forKey:@"version" ];
                period = getGFXDefaultPeriod() / 60;
                [ defaults setInteger:period forKey:@"DefaultPeriod" ];
                period = getGFXSciencePeriod() / 60;
                [ defaults setInteger:period forKey:@"SciencePeriod" ];
                period = getGGFXChangePeriod() / 60;
                [ defaults setInteger:period forKey:@"ChangePeriod" ];

                // synchronize
            }

            [defaults synchronize];

            // get defaults...
            gGoToBlank = [ defaults integerForKey:@"GoToBlank" ];
            gBlankingTime = [ defaults integerForKey:@"BlankingTime" ];
            period = [ defaults integerForKey:@"DefaultPeriod" ];
            setGFXDefaultPeriod((double)(period * 60));
            period = [ defaults integerForKey:@"SciencePeriod" ];
            setGFXSciencePeriod((double)(period * 60));
            period = [ defaults integerForKey:@"ChangePeriod" ];
            setGGFXChangePeriod((double)(period * 60));
    }

    return self;
}


// If there are multiple displays, this may get called
// multiple times (once for each display), so we need to guard
// against any problems that may cause.
- (void)startAnimation {

    gEventHandle = NXOpenEventStatus();

    // Under OS 10.14 Mojave, [super drawRect:] is slow but not needed if we do this:
    [[self window] setBackgroundColor:[NSColor blackColor]];

    if (gBOINC_Logo == NULL) {
        NSString *fileName = [[ NSBundle bundleForClass:[ self class ]] pathForImageResource:@"boinc_ss_logo" ];
        if (! fileName) {
            // What should we do in this case?
            return;
        }

        gBOINC_Logo = [[ NSImage alloc ] initWithContentsOfFile:fileName ];
        gMovingRect.origin.x = 0.0;
        gMovingRect.origin.y = 0.0;
        gMovingRect.size = [gBOINC_Logo size];

        if (gMovingRect.size.width < TEXTBOXMINWIDTH) {
            gImageXIndent = (TEXTBOXMINWIDTH - gMovingRect.size.width) / 2;
            gMovingRect.size.width = TEXTBOXMINWIDTH;
        } else {
            gImageXIndent = 0.0;
        }
        gTextBoxHeight = MINTEXTBOXHEIGHT;
        gMovingRect.size.height += gTextBoxHeight;
        gCurrentPosition.x = SAFETYBORDER + 1;
        gCurrentPosition.y = SAFETYBORDER + 1 + gTextBoxHeight;
        gCurrentDelta.x = 1.0;
        gCurrentDelta.y = 1.0;

//      [ self setAnimationTimeInterval:1/8.0 ];
    }

    [ super startAnimation ];

    NSWindow *myWindow = [ self window ];
#if DEBUG_UNDER_XCODE
    [ myWindow setLevel:2030 ];
#else   // NOT DEBUG_UNDER_XCODE
    NSRect windowFrame = [ myWindow frame ];
    if ( (windowFrame.origin.x == 0) && (windowFrame.origin.y == 0) )   // Main screen
#endif  // NOT DEBUG_UNDER_XCODE
    {
        // If a dual-GPU MacBook Pro was using integrated GPU, switching to discrete GPU will
        // cause ScreenSaverEngine to call stopAnimation, initWithFrame and startAnimation.
        // This will destroy the old ScreenSaverView and create a new one, so we need to
        // pass our new ScreenSaverView to our SharedGraphicsController.
        if (mySharedGraphicsController) {
            [mySharedGraphicsController init:self];
        }

        startBOINCSaver();
        if (myTimer == NULL) {
            myViewBounds = [self bounds];
            // See comments at advancePosition on why this is needed.
            myTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval) 1./60. target:self selector:@selector(advancePosition:)
                                        userInfo:nil repeats:YES];
        }
    }
    gSS_StartTime = getDTime();
}

// If there are multiple displays, this may get called
// multiple times (once for each display), so we need to guard
// against any problems that may cause.
//
// In recent versions of MacOS stopAnimation seems to be called only when
// MacOS puts the displays to sleep, but not when user activity dismisses
// the screensaver. (legacyScreenSaver coneinues running under MacOS 14.0
//  and later.) So we must use the tests in doPeriodicTasks.
- (void)stopAnimation {
    [ super stopAnimation ];
   if (myIsPreview) return;
#if ! DEBUG_UNDER_XCODE
    NSRect windowFrame = [ [ self window ] frame ];
    if ( (windowFrame.origin.x != 0) || (windowFrame.origin.y != 0) ) {
        return;         // We draw only to main screen
    }
#endif
    if (imageView) {
        useCGWindowList = false;
        // removeFromSuperview must be called from main thread
        if (NSThread.isMainThread) {
            [imageView removeFromSuperview];   // Releases imageView
        } else {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [imageView removeFromSuperview];   // Releases imageView
            });
        }
        imageView = nil;
    }

    if (!myIsPreview) {
        [self cleanupSaver:YES];
    }

    gTopWindowListIndex = -1;
}

// If there are multiple displays, this may get called
// multiple times (once for each display), so we need to guard
// against any problems that may cause.
- (void)drawRect:(NSRect)rect {
//  optionally draw here
    if (gIsMojave) {
        [self doPeriodicTasks];
    } else {
        [ super drawRect:rect ];
    }
}

- (void) cleanupSaver:(bool)mayExit {
    closeBOINCSaver();

    if (mayExit) {
        screensaverIsVisible = false;
        if (myTimer) {
            [ myTimer invalidate ];
            myTimer = NULL;
        }
    }
    if (mySharedGraphicsController) {
        [ mySharedGraphicsController cleanUpOpenGL ];   // Must be called from main thread
        [ mySharedGraphicsController closeServerPort ]; // Must be called after cleanUpOpenGL
        if (mayExit) {
            [ NSApp terminate:nil ];    // Comment this out to let legacyScreensaver continue in background
        }
    }
}

// If there are multiple displays, this may get called
// multiple times (once for each display), so we need to guard
// against any problems that may cause.
- (void)doPeriodicTasks {
    int coveredFreq = 0;
    NSRect theFrame = [ self frame ];
    double maxWaitTime;
    NSRect currentDrawingRect, eraseRect;
    CGFloat actualTextBoxHeight = MINTEXTBOXHEIGHT;
    char *msg;
    CFStringRef cf_msg;

    if (myTimer == NULL) {
        return;
    }

    NSWindow *myWindow = [ self window ];
    NSRect windowFrame = [ myWindow frame ];

    if (gIsSonoma) {
        // Under MacOS 14 Sonoma, screensavers continue to run and "draw" invisibly
        // after they are dismissed by user activity, to allow them to be used as
        // wallpaper. Since we don't want the BOINC screensaver to be used as wallpaper,
        // this would waste system resources, so we probably want to terminate the
        // legacyScreenSaver when that happens. Also, legacyScreenSaver under MacOS 26
        // creates other problems; see comments at advancePosition: for details.
        //
        // The only way I've found to determine that the legacyScreenSaver has been
        // dismissed by the user in all 3 OS versions (MacOS 14, MacOS 15 and MacOS 26)
        // is testing the frontmost application: it is LoginWindow if and only if the
        // screensaver is visible.
        if ((windowFrame.size.width > 500.) && (windowFrame.size.height > 500.)) {
            if (screensaverIsVisible) {
                bool isLoginWindow = false;
                NSRunningApplication *frontapp = [[ NSWorkspace sharedWorkspace] frontmostApplication];
                if (frontapp) {
                    NSString *bundleId = [frontapp bundleIdentifier];
                    if (bundleId) {
                        if ([bundleId UTF8String]) {
                            isLoginWindow = (strcmp([bundleId UTF8String], "com.apple.loginwindow") == 0);
                        }
                    }
                }
                if (!isLoginWindow) {
                    [self cleanupSaver:YES];
                    return;
                }
            }
        }
    }

    if (ScreenIsBlanked) {
        if (screensaverIsVisible) {
            [self cleanupSaver:NO];
            return;
        }
    }

    if (myIsPreview) {
#if 1   // Currently drawRect just draws our logo in the preview window
        if (gPreview_Image == NULL) {
            NSString *fileName = [[ NSBundle bundleForClass:[ self class ]] pathForImageResource:@"boinc" ];
            if (fileName) {
                gPreview_Image = [[ NSImage alloc ] initWithContentsOfFile:fileName ];
            }
        }
        if (gPreview_Image) {
            [ gPreview_Image setSize:theFrame.size ];
            [ gPreview_Image drawAtPoint:NSZeroPoint fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 ];
        }
#else   // Code for possible future use if we want to draw more in preview
        myContext = [[NSGraphicsContext currentContext] graphicsPort];
        drawPreview(myContext);
#endif
        return;
    }

#if ! DEBUG_UNDER_XCODE
    // For unkown reasons, OS 10.7 Lion screensaver and later delay several seconds
    // after user activity before calling stopAnimation, so we check user activity here
    if ((compareOSVersionTo(10, 7) >= 0) && ((getDTime() - gSS_StartTime) > 2.0)) {
        if (! gIsMojave) {
            double idleTime =  CGEventSourceSecondsSinceLastEventType
                        (kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);
            if (idleTime < 1.5) {
                [ NSApp terminate:nil ];
            }
        }
    }

    if ( (windowFrame.origin.x != 0) || (windowFrame.origin.y != 0) ) {
        // Hide window on second display to aid in debugging
#ifdef _DEBUG
        // This technique no longer works on newer versions of OS X
        [ myWindow setLevel:kCGMinimumWindowLevel ];
        NSInteger alpha = 0;
        [ myWindow setAlphaValue:alpha ];   // For OS 10.6
        [ myWindow orderOut:self];
#endif
        return;         // We draw only to main screen
    }
#endif  // NOT DEBUG_UNDER_XCODE

   myContext = [[NSGraphicsContext currentContext] graphicsPort];

    // On OS 10.13 or later, use MachO comunication and IOSurfaceBuffer to
    // display the graphics output of our child graphics apps in our window.
    // Graphics apps linked with our current libraries have support for
    // MachO comunication and IOSurfaceBuffer.
    //
    // For graphics apps linked with older libraries, use the API
    // CGWindowListCreateImage to copy the graphic app window's image,
    // but this is far slower because it does not take advantage of GPU
    // acceleration, so it uses more CPU and animation may not appear smooth.
    //
    if (UseSharedOffscreenBuffer() && !mySharedGraphicsController) {
        mySharedGraphicsController = [SharedGraphicsController alloc];
        [mySharedGraphicsController init:self];
    }

    if (runningSharedGraphics || useCGWindowList ) {
        // Since ScreensaverEngine.app is running in the foreground, our child
        // graphics app may not get enough CPU cycles for good animation.
        // Calling [ NSApp activateIgnoringOtherApps:YES ] frequently from the
        // child doesn't help. But activating our child frequently from the
        // front process (this screensaver plugin) does appear to guarantee
        // good animation.
        //
        // An alternate approach that also works is to have the child process
        // tell the kernel it has real time constraints by calling
        // thread_policy_set() with thread_policy_flavor_t set to
        // THREAD_TIME_CONSTRAINT_POLICY as described in
        // <https://developer.apple.com/library/content/technotes/tn2169>.
        //
        // But different graphics apps may have different time requirements,
        // so it is difficult to know the best values to set in the
        // thread_time_constraint_policy_data_t struct. If the graphics app asks
        // for too much time, the worker apps will get less time, and if it asks
        // for too little time the animation won't be smooth.
        //
        // So frequently activating the child app here seems to be best.
        //
        if (childApp) {
             if (![ childApp activateWithOptions:NSApplicationActivateIgnoringOtherApps ]) {
                launchedGfxApp("", "", 0, -1);  // Graphics app is no longer running
             } else if (useCGWindowList) {
                // As a safety precaution, prevent terminating gfx app while copying its window
                pthread_mutex_lock(&saver_mutex);

                // terminate_v6_screensaver may have removed imageView via launchedGfxApp("", 0, -1)
                //
                // CGWindowListCopyWindowInfo and CGWindowListCreateImage can copy
                // windows between user boinc_project and the user running the
                // screensaver only if OS < 10.15 (before Catalina)
                //
                if (imageView) {
                    CGImageRef windowImage = CGWindowListCreateImage(CGRectNull,
                                                kCGWindowListOptionIncludingWindow,
                                                gfxAppWindowNum,
                                                kCGWindowImageBoundsIgnoreFraming);
                    if (windowImage) {
                        // Create a bitmap rep from the image...
                        NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc] initWithCGImage:windowImage];
                        // Create an NSImage and add the bitmap rep to it...
                        NSImage *image = [[NSImage alloc] init];
                        [image addRepresentation:bitmapRep];
                        [image drawInRect:[self frame]];
                        CGImageRelease(windowImage);
                    }
                }

                pthread_mutex_unlock(&saver_mutex);
            }
        }

        isErased = false;
        if (IsDualGPUMacbook) {
            // Check once per second for change in status of running on battery
            double timeNow = getDTime();
            if ((timeNow - lastGetSSMsgTime) >= 1.0) {
                getSSMessage(&msg, &coveredFreq);
                lastGetSSMsgTime = timeNow;
            }
            windowIsCovered();
////            [ self setAnimationTimeInterval:1.0 ];
        }

        return;
    }

    if (imageView && !useCGWindowList) {
        // removeFromSuperview must be called from main thread
        if (NSThread.isMainThread) {
            [imageView removeFromSuperview];   // Releases imageView
        } else {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [imageView removeFromSuperview];   // Releases imageView
            });
        }
        imageView = nil;
    }

    NSRect viewBounds = [self bounds];

    getSSMessage(&msg, &coveredFreq);
    if (UseSharedOffscreenBuffer()) {
        // If runningSharedGraphics is still false after MAXWAITFORCONNECTION,
        // assume the graphics app has not been built with MachO comunication
        // and IOSurfaceBuffer support, so try to use CGWindowListCreateImage
        // method. If that fails MAX_CGWINDOWLIST_TRIES times then assume
        // the graphics app is not compatible with OS 10.13+ and kill it.
        //
        // taskSlot<0 if no worker app is running, so launching default graphics
        if (gfxAppStartTime && (taskSlot >= 0)) {
            maxWaitTime = gIsCatalina ? MAXWAITFORCONNECTIONCATALINA : MAXWAITFORCONNECTION;
            if ((getDTime() - gfxAppStartTime) > maxWaitTime) {
                if (gIsCatalina) {
                    if (gMach_bootstrap_unavailable_to_screensavers) {
                        // Is the gfx app built with a new enough BOINC graphics library version?
                        if (compareBOINCLibVersionTo(MAJORBOINCGFXLIBNEEDEDFORSONOMA, MINORBOINCGFXLIBNEEDEDFORSONOMA, RELEASEBOINCGFXLIBNEEDEDFORSONOMA) >= 0) {
                            runningSharedGraphics = true;
                            if (childPid) {
                                gfxAppStartTime = 0.0;
                                childApp = [NSRunningApplication runningApplicationWithProcessIdentifier:childPid];
                            }
                        }
                    }
                    // CGWindowListCopyWindowInfo and CGWindowListCreateImage can copy
                    // windows between user boinc_project and the user running the
                    // screensaver only if OS < 10.15 (before Catalina)
                    incompatibleGfxApp(gfxAppPath, gfxWuName, childPid, taskSlot);
                } else {
                    if (++CGWindowListTries > MAX_CGWINDOWLIST_TRIES) {
                        // After displaying message for 5 seconds, incompatibleGfxApp
                        // will call launchedGfxApp("", 0, -1) which will clear
                        // gfxAppStartTime and CGWindowListTries
                        incompatibleGfxApp(gfxAppPath, gfxWuName, childPid, taskSlot);
                    } else {
                        if ([self setUpToUseCGWindowList]) {
                            CGWindowListTries = 0;
                            gfxAppStartTime = 0.0;
                        }
                    }
                }
            }
        }
    // As of OS 10.13, app windows can no longer appear on top of screensaver
    // window, but we still use this method on older versions of OS X for
    // compatibility with older project graphics apps (those which have not
    // yet been relinked with the updated libboinc_graphics2.a.)
    } else {
        // NOTE: My tests seem to confirm that the top window is always the first
        // window returned by [NSWindow windowNumbersWithOptions:] However, Apple's
        // documentation is unclear whether we can depend on this.  So I have
        // added some safety by doing two things:
        // [1] Only use the windowNumbersWithOptions test when we have started
        //     project graphics.
        // [2] Assume that our window is covered 45 seconds after starting project
        //     graphics even if the windowNumbersWithOptions test did not indicate
        //     that is so.
        //
        // getSSMessage() returns a non-zero value for coveredFreq only if we have started
        // project graphics.
        //
        // If we should use a different frequency when our window is covered by another
        // window, then check whether there is a window at a higher z-level than ours.

        // Assuming our window(s) are initially the top window(s), determine our position
        // in the window list when no graphics applications have covered us.
        if (gTopWindowListIndex < 0) {
            NSArray *theWindowList = [NSWindow windowNumbersWithOptions:NSWindowNumberListAllApplications];
            myWindowNumber = [ myWindow windowNumber ];
            gTopWindowListIndex = [theWindowList indexOfObjectIdenticalTo:[NSNumber numberWithInt:myWindowNumber]];
        }
    }

    // Draw our moving BOINC logo and screensaver status text

    // Clear the previous drawing area
    currentDrawingRect = gMovingRect;
    currentDrawingRect.origin.x = (float) ((int)gCurrentPosition.x);
    currentDrawingRect.origin.y += (float) ((int)gCurrentPosition.y - gTextBoxHeight);

    if (! logoIsCovered) {
        if ( (msg != NULL) && (msg[0] != '\0') ) {
            cf_msg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingMacRoman);
            if (strncmp(msg, gLastMsg, sizeof(gLastMsg))) {
                strlcpy(gLastMsg, msg, sizeof(gLastMsg));

                CTFontRef myFont = CTFontCreateWithName(CFSTR("Helvetica"), 20, NULL);
                HIThemeTextInfo theTextInfo = {kHIThemeTextInfoVersionOne, kThemeStateActive, kThemeSpecifiedFont,
                            kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop,
                            kHIThemeTextBoxOptionNone, kHIThemeTextTruncationNone, 0, false,
                            0, myFont
                            };
                gTextInfo = theTextInfo;

                HIThemeGetTextDimensions(cf_msg, (float)gMovingRect.size.width, &gTextInfo, NULL, &actualTextBoxHeight, NULL);
                gTextBoxHeight = actualTextBoxHeight + TEXTBOXTOPBORDER;
            }

            if (myTimer == NULL) {
                return;
            }

            // Under MacOS 14 and later, if we allow legacyScreenSaver to continue running in the
            // background when user activity dismisses the screensaver, it appears to create multiple
            // instances of startAnimation, animateOneFrame and (perhaps) drawRect upon each subsequnt
            // activation of the screensaver. As a result it tries to draw the same image in the same
            // location multiple times. Apparently only one of those instances does actually draw to
            // the screen, but we have no way of knowing which instance, so we need to allow them all
            // to go through this code to ensure the drawing actually happens.
            if (!isErased) {
               [[NSColor blackColor] set];

                 // Erasing only 2 small rectangles reduces screensaver's CPU usage by about 25%
                imagePosition.x = (float) ((int)gCurrentPosition.x + gImageXIndent);
                imagePosition.y = (float) (int)gCurrentPosition.y;
                eraseRect.origin.y = imagePosition.y;
                eraseRect.size.height = currentDrawingRect.size.height - gTextBoxHeight;

                if (gCurrentDelta.x > 0) {
                    eraseRect.origin.x = imagePosition.x - 1;
                    eraseRect.size.width = gCurrentDelta.x + 1;
                } else {
                    eraseRect.origin.x = currentDrawingRect.origin.x + currentDrawingRect.size.width - gImageXIndent + gCurrentDelta.x - 1;
                    eraseRect.size.width = -gCurrentDelta.x + 1;
                }

                eraseRect = NSInsetRect(eraseRect, -1, -1);
                NSRectFill(eraseRect);

                eraseRect.origin.x = imagePosition.x;
                eraseRect.size.width = currentDrawingRect.size.width - gImageXIndent - gImageXIndent;

                if (gCurrentDelta.y > 0) {
                    eraseRect.origin.y = imagePosition.y;
                    eraseRect.size.height = gCurrentDelta.y + 1;
                } else {
                    eraseRect.origin.y = imagePosition.y + currentDrawingRect.size.height - gTextBoxHeight - 1;
                    eraseRect.size.height = -gCurrentDelta.y + 1;
                }
                eraseRect = NSInsetRect(eraseRect, -1, -1);
                NSRectFill(eraseRect);

                eraseRect = currentDrawingRect;
                eraseRect.size.height = gTextBoxHeight;
                eraseRect = NSInsetRect(eraseRect, -1, -1);
                NSRectFill(eraseRect);

               isErased = true;
            }

            [ gBOINC_Logo drawAtPoint:imagePosition fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 ];
    #if 0
        // For testing
        gCurrentDelta.x = 0;
        gCurrentDelta.y = 0;
    #endif
            CGRect bounds = CGRectMake((float) ((int)gCurrentPosition.x),
                                 viewBounds.size.height - imagePosition.y + TEXTBOXTOPBORDER,
                                 gMovingRect.size.width,
                                 MAXTEXTBOXHEIGHT
                            );

            CGContextSaveGState (myContext);
            CGContextTranslateCTM (myContext, 0, viewBounds.origin.y + viewBounds.size.height);
            CGContextScaleCTM (myContext, 1.0f, -1.0f);

            CGFloat myWhiteComponents[] = {1.0, 1.0, 1.0, 1.0};
            CGColorSpaceRef myColorSpace = CGColorSpaceCreateDeviceRGB ();
            CGColorRef myTextColor = CGColorCreate(myColorSpace, myWhiteComponents);

            CGContextSetFillColorWithColor(myContext, myTextColor);

            HIThemeDrawTextBox(cf_msg, &bounds, &gTextInfo, myContext, kHIThemeOrientationNormal);

            CGColorRelease(myTextColor);
            CGColorSpaceRelease(myColorSpace);
            CGContextRestoreGState (myContext);
            CFRelease(cf_msg);

            isErased  = false;

        } else {        // Empty or NULL message
            if (!isErased) {
                eraseRect = NSInsetRect(currentDrawingRect, -1, -1);
                [[NSColor blackColor] set];
                isErased  = true;
                NSRectFill(eraseRect);
                gMovingRect.size.height = [gBOINC_Logo size].height + gTextBoxHeight;
            }
        }
    }
    // Check for a new graphics app sending us data
    if (UseSharedOffscreenBuffer() && gfxAppStartTime) {
        if (mySharedGraphicsController) {
            if (!runningSharedGraphics) {
                // wait for a connection from a gfx app
                [mySharedGraphicsController testConnection];
            }
        }
    }
}


- (void)animateOneFrame {
#if ! DEBUG_UNDER_XCODE
    if (!myIsPreview) {
        NSRect windowFrame = [ [ self window ] frame ];
        if ( (windowFrame.origin.x != 0) || (windowFrame.origin.y != 0) ) {
            return;         // We draw only to main screen
        }
    }
#endif
    // Drawing in animateOneFrame doesn't seem to work under OS 10.14 Mojave
    // but drawing in drawRect: seems slow under earlier versions of OS X
    if (gIsMojave) {
        [self display];
    } else {
        [self doPeriodicTasks];
    }
}


// This code was previously part of doPeriodicTasks, which is called from AnimateOneFrame
// in MacOS before MacOS 10.14 Mojave and from drawRect (which is triggered by
// AnimateOneFrame) since then.
// But under MacOS 14 and later, If we allow legacyScreenSaver to continue running in the
// background when user activity dismisses the screensaver, it appears to create multiple
// instances of startAnimation, animateOneFrame and (perhaps) drawRect upon each subsequnt
// activation of the screensaver. As a result, doPeriodicTasks is called several times
// more often, causing the moving logo to move at several times the intended speed.
// We solve this by using our own NSTimer to adjust the position at which to draw the
// logo, while letting doPeriodicTasks prform the actual drawing.
- (void) advancePosition:(NSTimer*)timer {
{
    if (logoIsCovered) {
        return;
    }
    // If text has changed and it now goes below bottom of screen, jump up to show it all.
    if ((gCurrentPosition.y - gTextBoxHeight) <= SAFETYBORDER) {
        gCurrentPosition.y = SAFETYBORDER + 1 + gTextBoxHeight;
        if (gCurrentDelta.y < 0) {
            gCurrentDelta.y = (float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
        }
    }

   // Set direction of motion to "bounce" off edges of screen
   if ( (gCurrentDelta.x < 0) && (gCurrentPosition.x <= SAFETYBORDER) ) {
        gCurrentDelta.x = (float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
        gCurrentDelta.y = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.y)) / 16.;
    }
    if ( (gCurrentDelta.x > 0) && ( (gCurrentPosition.x + gMovingRect.size.width) >=
                (myViewBounds.origin.x + myViewBounds.size.width - SAFETYBORDER) ) ){
        gCurrentDelta.x = -(float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
        gCurrentDelta.y = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.y)) / 16.;
    }
    if ( (gCurrentDelta.y < 0) && (gCurrentPosition.y - gTextBoxHeight <= SAFETYBORDER) ) {
        gCurrentDelta.y = (float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
        gCurrentDelta.x = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.x)) / 16.;
    }
    if ( (gCurrentDelta.y > 0) && ( (gCurrentPosition.y + gMovingRect.size.height) >=
               (myViewBounds.origin.y + myViewBounds.size.height - SAFETYBORDER) ) ) {
        gCurrentDelta.y = -(float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
        gCurrentDelta.x = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.x)) / 16.;
    }
}

    // Get the new drawing area
    gCurrentPosition.x += gCurrentDelta.x;
    gCurrentPosition.y += gCurrentDelta.y;
    imagePosition.x = (float) ((int)gCurrentPosition.x + gImageXIndent);
    imagePosition.y = (float) (int)gCurrentPosition.y;
}


- (BOOL)hasConfigureSheet {
    return YES;
}


// Display the configuration sheet for the user to choose their settings
- (NSWindow*)configureSheet
{
    int period;

	// if we haven't loaded our configure sheet, load the nib named MyScreenSaver.nib
	if (!mConfigureSheet) {
        if ([[ NSBundle bundleForClass:[ self class ]] respondsToSelector: @selector(loadNibNamed: owner: topLevelObjects:)]) {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"
            // [NSBundle loadNibNamed: owner: topLevelObjects:] is not available before OS 10.8
            [ [ NSBundle bundleForClass:[ self class ]] loadNibNamed:@"BOINCSaver" owner:self topLevelObjects:NULL ];
#pragma clang diagnostic pop
        }
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 1080
         else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            // [NSBundle loadNibNamed: owner:] is deprecated in OS 10.8
            [ NSBundle loadNibNamed:@"BOINCSaver" owner:self ];
#pragma clang diagnostic pop
        }
#endif
    }
	// set the UI state
	[ mGoToBlankCheckbox setState:gGoToBlank ];

    mBlankingTimeString = [[ NSString alloc ] initWithFormat:@"%d", gBlankingTime ];
	[ mBlankingTimeTextField setStringValue:mBlankingTimeString ];

    period = getGFXDefaultPeriod() / 60;
    mDefaultPeriodString = [[ NSString alloc ] initWithFormat:@"%d", period ];
	[ mDefaultPeriodTextField setStringValue:mDefaultPeriodString ];

    period = getGFXSciencePeriod() / 60;
    mSciencePeriodString = [[ NSString alloc ] initWithFormat:@"%d", period ];
	[ mSciencePeriodTextField setStringValue:mSciencePeriodString ];

    period = getGGFXChangePeriod() / 60;
    mChangePeriodString = [[ NSString alloc ] initWithFormat:@"%d", period ];
	[ mChangePeriodTextField setStringValue:mChangePeriodString ];

	return mConfigureSheet;
}

// Called when the user clicked the SAVE button
- (IBAction) closeSheetSave:(id) sender
{
    int period = 0;

    NSScanner *scanner, *scanner2;

    // get the defaults
	ScreenSaverDefaults *defaults = [ ScreenSaverDefaults defaultsForModuleWithName:mBundleID ];

	// save the UI state
	gGoToBlank = [ mGoToBlankCheckbox state ];
	mBlankingTimeString = [ mBlankingTimeTextField stringValue ];
    gBlankingTime = [ mBlankingTimeString intValue ];
    scanner = [ NSScanner scannerWithString:mBlankingTimeString];
    if (![ scanner scanInt:&period ]) goto Bad;
    if (![ scanner isAtEnd ]) goto Bad;
    if ((period < 0) || (period > 999)) goto Bad;
    gBlankingTime = period;

	mDefaultPeriodString = [ mDefaultPeriodTextField stringValue ];
    scanner2 = [ scanner initWithString:mDefaultPeriodString];
    if (![ scanner2 scanInt:&period ]) goto Bad;
    if (![ scanner2 isAtEnd ]) goto Bad;
    if ((period < 0) || (period > 999)) goto Bad;
    setGFXDefaultPeriod((double)(period * 60));

	mSciencePeriodString = [ mSciencePeriodTextField stringValue ];
    scanner2 = [ scanner initWithString:mSciencePeriodString];
    if (![ scanner2 scanInt:&period ]) goto Bad;
    if (![ scanner2 isAtEnd ]) goto Bad;
    if ((period < 0) || (period > 999)) goto Bad;
    setGFXSciencePeriod((double)(period * 60));

	mChangePeriodString = [ mChangePeriodTextField stringValue ];
    scanner2 = [ scanner initWithString:mChangePeriodString];
    if (![ scanner2 scanInt:&period ]) goto Bad;
    if (![ scanner2 isAtEnd ]) goto Bad;
    if ((period < 0) || (period > 999)) goto Bad;
    setGGFXChangePeriod((double)(period * 60));

	// write the defaults
	[ defaults setInteger:gGoToBlank forKey:@"GoToBlank" ];
	[ defaults setInteger:gBlankingTime forKey:@"BlankingTime" ];
    period = getGFXDefaultPeriod() / 60;
    [ defaults setInteger:period forKey:@"DefaultPeriod" ];
    period = getGFXSciencePeriod() / 60;
    [ defaults setInteger:period forKey:@"SciencePeriod" ];
    period = getGGFXChangePeriod() / 60;
    [ defaults setInteger:period forKey:@"ChangePeriod" ];

	// synchronize
    [ defaults synchronize ];

	// end the sheet
    [ NSApp endSheet:mConfigureSheet ];
    return;
Bad:
;   // Empty statement is needed to prevent compiler error
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please enter a number between 0 and 999."];
    [alert setAlertStyle:NSCriticalAlertStyle];

    if ([alert respondsToSelector: @selector(beginSheetModalForWindow: completionHandler:)]){
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"
        // [NSAlert beginSheetModalForWindow: completionHandler:] is not available before OS 10.9
        [alert beginSheetModalForWindow:mConfigureSheet completionHandler:^(NSModalResponse returnCode){}];
#pragma clang diagnostic pop
    }
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 1090
        else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            // [NSAlert beginSheetModalForWindow: modalDelegate: didEndSelector: contextInfo:] is deprecated in OS 10.9
            [alert beginSheetModalForWindow:mConfigureSheet modalDelegate:self didEndSelector:nil contextInfo:nil];
#pragma clang diagnostic pop
        }
#endif
}

// Called when the user clicked the CANCEL button
- (IBAction) closeSheetCancel:(id) sender
{
	// nothing to configure
    [ NSApp endSheet:mConfigureSheet ];
}

// Find the gtaphics app's window number (window ID)
//
// CGWindowListCopyWindowInfo and CGWindowListCreateImage can copy
// windows between user boinc_project and the user running the
// screensaver only if OS < 10.15 (before Catalina)
//
- (bool) setUpToUseCGWindowList
{
    NSArray *windowList = (__bridge NSArray*)CGWindowListCopyWindowInfo(
                            kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
                            kCGNullWindowID);
    for (int i=[windowList count]-1; i>=0; i--) {
        NSDictionary *dict = (NSDictionary*)(windowList[i]);
        NSString * pidString = dict[(id)kCGWindowOwnerPID];
        if ((pid_t)[pidString intValue] == childPid) {
            NSString * windowNumString = dict[(id)kCGWindowNumber];
            gfxAppWindowNum = (int)[windowNumString intValue];
            useCGWindowList = true;
            childApp = [NSRunningApplication runningApplicationWithProcessIdentifier:childPid];
            if (imageView == nil) {
                imageView = [[NSView alloc] initWithFrame:[self frame]];
                [self addSubview:imageView];
            }
            return true;    // Success
        }
    }
    return false;   // Not found
}

@end

int IOSurfaceWidth;
int IOSurfaceHeight;

// On OS 10.13 or later, use MachO comunication and IOSurfaceBuffer to
// display the graphics output of our child graphics apps in our window.
// All code past this point is for that implementation.

// Adapted from Apple Developer Tech Support Sample Code MutiGPUIOSurface:
// <https://developer.apple.com/library/content/samplecode/MultiGPUIOSurface>

#define NUM_IOSURFACE_BUFFERS 2

@interface SharedGraphicsController()
{
	NSMachPort *serverPort;
	NSMachPort *localPort;

	uint32_t serverPortName;
	uint32_t localPortName;

	int32_t clientIndex;
	uint32_t nextFrameIndex;

    NSView *screenSaverView;
    saverOpenGLView *openGLView;

	IOSurfaceRef _ioSurfaceBuffers[NUM_IOSURFACE_BUFFERS];
    mach_port_t _ioSurfaceMachPorts[NUM_IOSURFACE_BUFFERS];
	GLuint _textureNames[NUM_IOSURFACE_BUFFERS];
}
@end

static bool okToDraw;

@implementation SharedGraphicsController

- (void)init:(NSView*)saverView {
    screenSaverView = saverView;

    [[NSNotificationCenter defaultCenter] removeObserver:self
        name:NSPortDidBecomeInvalidNotification object:nil];

    openGLView = nil;

    [self testConnection];

    if (! gMach_bootstrap_unavailable_to_screensavers)  {
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(portDied:) name:NSPortDidBecomeInvalidNotification object:nil];
    }
}


- (void) testConnection
{
    mach_port_t servicePortNum = MACH_PORT_NULL;
    kern_return_t machErr;
    char *portNameV1 = "edu.berkeley.boincsaver";
    char *portNameV3 = "edu.berkeley.boincsaver-v3";

    if ((!gMach_bootstrap_unavailable_to_screensavers) || (serverPort == MACH_PORT_NULL)) {
        // Try to check in with master.
        // NSMachBootstrapServer is deprecated in OS 10.13, so use bootstrap_look_up
        //	serverPort = [(NSMachPort *)([[NSMachBootstrapServer sharedInstance] portForName:@"edu.berkeley.boincsaver"]) retain];
        machErr = bootstrap_look_up(bootstrap_port, portNameV1, &servicePortNum);
        if (machErr != KERN_SUCCESS) {
            if (machErr != BOOTSTRAP_NOT_PRIVILEGED) {
            //  bootstrap_look_up() returned error other than BOOTSTRAP_NOT_PRIVILEGED
            // machErr is probably BOOTSTRAP_UNKNOWN_SERVICE because no gfx app is running
            // Keep trying bootstrap_look_up() until we get a connection from gfx app
            serverPort = MACH_PORT_NULL;
            } else {    //  bootstrap_look_up() returned error BOOTSTRAP_NOT_PRIVILEGED
                // As of MacOS 14.0, the legacyScreenSaver sandbox prevents using
                // bootstrap_look_up. I have filed bug report FB13300491 with
                // Apple and hope they will change this in a future MacOS.
                machErr = bootstrap_check_in(bootstrap_port, portNameV3, &servicePortNum);
                if (machErr != KERN_SUCCESS) {
                   [NSApp terminate:nil];
                }
                gMach_bootstrap_unavailable_to_screensavers = true;
            }   //  bootstrap_look_up() returned error BOOTSTRAP_NOT_PRIVILEGED
        }   // bootstrap_look_up() did not return KERN_SUCCESS

         if (machErr == KERN_SUCCESS) {
           serverPort = (NSMachPort*)[NSMachPort portWithMachPort:servicePortNum options:NSMachPortDeallocateReceiveRight];
        }

        if ((serverPort != MACH_PORT_NULL) && (localPort == MACH_PORT_NULL)) {
            // Retrieve raw mach port names.
            serverPortName = [serverPort machPort];

            if (gMach_bootstrap_unavailable_to_screensavers) {
               // Register server port with the current runloop.
                [serverPort setDelegate:self];
                [serverPort scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            } else {    // (! gMach_bootstrap_unavailable_to_screensavers)
                // Create our own local port.
                localPort = [[NSMachPort alloc] init];
                localPortName  = [localPort machPort];
                // Register our local port with the current runloop.
                [localPort setDelegate:self];
                [localPort scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

                // Check in with server.
                int kr;
                kr = _MGCCheckinClient(serverPortName, localPortName, &clientIndex);
                if(kr != 0) {
                    [NSApp terminate:nil];
                }

                runningSharedGraphics = true;

                if (childPid) {
                    gfxAppStartTime = 0.0;
                    childApp = [NSRunningApplication runningApplicationWithProcessIdentifier:childPid];
                }
            }   // (! gMach_bootstrap_unavailable_to_screensavers)
        }   // ((serverPort != MACH_PORT_NULL) && (localPort == MACH_PORT_NULL))
    }   // ((!gMach_bootstrap_unavailable_to_screensavers) || (serverPort == MACH_PORT_NULL))
}


- (void)cleanUpOpenGL
{
    if (gMach_bootstrap_unavailable_to_screensavers && (serverPort != MACH_PORT_NULL)) {
       [serverPort removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    }
    childPid=0;
    childApp = nil;

    if (gMach_bootstrap_unavailable_to_screensavers ||
            ((serverPort == MACH_PORT_NULL) && (localPort == MACH_PORT_NULL))
            ) {
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
            logoIsCovered = false;
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
        if (screensaverIsVisible) {
            if (gMach_bootstrap_unavailable_to_screensavers) {
            [serverPort scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            }
        }
    }
}

- (void)portDied:(NSNotification *)notification
{
    if (gMach_bootstrap_unavailable_to_screensavers) {
        return; // This should never be called if using MacOS 14 workaround
    }
	NSPort *port = [notification object];
	if(port == serverPort) {
        [ self closeServerPort ];
        [ self cleanUpOpenGL ];
    }
}

- (void)closeServerPort
{
    childApp = nil;
    gfxAppStartTime = 0.0;
    gfxAppPath[0] = '\0';
    gfxWuName[0] = '\0';

    if ([serverPort isValid]) {
        [serverPort invalidate];
//            [serverPort release];
    }
    serverPort = MACH_PORT_NULL;

    if (localPort != MACH_PORT_NULL) {
        [localPort removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

        if ([localPort isValid]) {
            [localPort invalidate];
        }
//          [localPort release];
        localPort = MACH_PORT_NULL;
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
        if(kr != 0) {
			[NSApp terminate:nil];
        }
	}
}

- (kern_return_t)displayFrame:(int32_t)frameIndex surfacemachport:(mach_port_t)iosurface_port
{
    if (!childPid) return 0;

	nextFrameIndex = frameIndex;

	if(!_ioSurfaceBuffers[frameIndex])
	{
		_ioSurfaceBuffers[frameIndex] = IOSurfaceLookupFromMachPort(iosurface_port);
        _ioSurfaceMachPorts[frameIndex] = iosurface_port;
	}

    IOSurfaceWidth = (int)IOSurfaceGetWidth((IOSurfaceRef)_ioSurfaceBuffers[frameIndex]);
    IOSurfaceHeight = (int)IOSurfaceGetHeight((IOSurfaceRef)_ioSurfaceBuffers[frameIndex]);

    if (openGLView == nil) {
        NSRect theframe;
        theframe.origin.x = theframe.origin.y = 0.0;
        theframe.size.width = IOSurfaceWidth;
        theframe.size.height = IOSurfaceHeight;
        openGLView = [[saverOpenGLView alloc] initWithFrame:theframe];
        [screenSaverView addSubview:openGLView];
        logoIsCovered = true;
    }

	if(!_textureNames[frameIndex])
    {
		_textureNames[frameIndex] = [openGLView setupIOSurfaceTexture:_ioSurfaceBuffers[frameIndex]];
    }

    if (openGLView) {
        okToDraw = true;    // Tell drawRect that we have real data to display

        [openGLView setNeedsDisplay:YES];
        [openGLView display];
    }

    if (gMach_bootstrap_unavailable_to_screensavers && (runningSharedGraphics == false)) {
        // We have now heard from gfx app so connection has been established
        runningSharedGraphics = true;
        if (childPid) {
            gfxAppStartTime = 0.0;
            childApp = [NSRunningApplication runningApplicationWithProcessIdentifier:childPid];
        }
    }
	return 0;
}

// For the MachO client, this is a no-op.
kern_return_t _MGSCheckinClient(mach_port_t server_port, mach_port_t client_port,
			       int32_t *client_index)
{
	return 0;
}

kern_return_t _MGSDisplayFrame(mach_port_t server_port, int32_t frame_index, mach_port_t iosurface_port)
{
	return [mySharedGraphicsController displayFrame:frame_index surfacemachport:iosurface_port];
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
       [ NSApp terminate:nil];
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

    glBindTexture(GL_TEXTURE_RECTANGLE, [mySharedGraphicsController currentTextureName]);
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


// On OS 10.13 or later, use MachO comunication and IOSurfaceBuffer to
// display the graphics output of our child graphics apps in our window.
static bool UseSharedOffscreenBuffer() {
    static bool alreadyTested = false;
    static bool needSharedGfxBuffer = false;

//return true;    // FOR TESTING ONLY
    if (alreadyTested) {
        return needSharedGfxBuffer;
    }
    alreadyTested = true;
    if (compareOSVersionTo(10, 13) >= 0) {
        needSharedGfxBuffer = true;
        return true;
    }
    return false;
}


