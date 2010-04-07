// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#import "Mac_Saver_ModuleView.h"
#include <Carbon/Carbon.h>
#include <AppKit/AppKit.h>
#include <QTKit/QTKitDefines.h> // For NSInteger

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

void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

int gGoToBlank;      // True if we are to blank the screen
int gBlankingTime;   // Delay in minutes before blanking the screen
NSString *gPathToBundleResources = NULL;
NSString *mBundleID = NULL; // our bundle ID
NSImage *gBOINC_Logo = NULL;

int gTopWindowListIndex = -1;

NSRect gMovingRect;
float gImageXIndent;
float gTextBoxHeight;
NSPoint gCurrentPosition;
NSPoint gCurrentDelta;

CGContextRef myContext;
bool isErased;

#define TEXTBOXMINWIDTH 400.0
#define MINTEXTBOXHEIGHT 40.0
#define MAXTEXTBOXHEIGHT 300.0
#define TEXTBOXTOPBORDER 15
#define SAFETYBORDER 20.0
#define MINDELTA 8
#define MAXDELTA 16

int signof(float x) {
    return (x > 0.0 ? 1 : -1);
}

@implementation BOINC_Saver_ModuleView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview {
    self = [ super initWithFrame:frame isPreview:isPreview ];
    return self;
}

// If there are multiple displays, this may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
- (void)startAnimation {
    NSBundle * myBundle;
    int newFrequency;
    int period;
    
    initBOINCSaver();  

    if (gBOINC_Logo == NULL) {
        if (self) {
            myBundle = [ NSBundle bundleForClass:[self class]];
            // grab the screensaver defaults
            if (mBundleID == NULL) {
                mBundleID = [ myBundle bundleIdentifier ];
            }

            // Path to our copy of switcher utility application in this screensaver bundle
            if (gPathToBundleResources == NULL) {
                gPathToBundleResources = [ myBundle resourcePath ];
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
                [defaults synchronize];
            }

            // get defaults...
            gGoToBlank = [ defaults integerForKey:@"GoToBlank" ];
            gBlankingTime = [ defaults integerForKey:@"BlankingTime" ];
            period = [ defaults integerForKey:@"DefaultPeriod" ];
            setGFXDefaultPeriod((double)(period * 60));
            period = [ defaults integerForKey:@"SciencePeriod" ];
            setGFXSciencePeriod((double)(period * 60));
            period = [ defaults integerForKey:@"ChangePeriod" ];
            setGGFXChangePeriod((double)(period * 60));

           [ self setAutoresizesSubviews:YES ];	// make sure the subview resizes.

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
            
            [ self setAnimationTimeInterval:1/8.0 ];
        }
    }
    
    [ super startAnimation ];

    if ( [ self isPreview ] ) {
        [ self setAnimationTimeInterval:1.0/8.0 ];
        return;
    }
    
    newFrequency = startBOINCSaver();  
    if (newFrequency)
        [ self setAnimationTimeInterval:1.0/newFrequency ];
}

// If there are multiple displays, this may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
- (void)stopAnimation {
    [ super stopAnimation ];

    if ( ! [ self isPreview ] ) {
        closeBOINCSaver();
    }
 
    gTopWindowListIndex = -1;
    
    if (gBOINC_Logo) {
        [ gBOINC_Logo release ];
    }
    gBOINC_Logo = NULL;
    
}

// If there are multiple displays, this may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
- (void)drawRect:(NSRect)rect {
    [ super drawRect:rect ];

//  optionally draw here
}

// If there are multiple displays, this may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
- (void)animateOneFrame {
    int newFrequency = 0;
    int coveredFreq = 0;
    NSRect theFrame = [ self frame ];
    NSInteger myWindowNumber;
    NSInteger windowList[20];
    NSInteger i, n;
    NSRect currentDrawingRect, eraseRect;
    NSPoint imagePosition;
    char *msg;
    CFStringRef cf_msg;
    AbsoluteTime timeToUnblock, frameStartTime = UpTime();

   if ([ self isPreview ]) {
#if 1   // Currently drawRect just draws our logo in the preview window
        NSString *fileName = [[ NSBundle bundleForClass:[ self class ]] pathForImageResource:@"boinc" ];
        if (fileName) {
            NSImage *myImage = [[ NSImage alloc ] initWithContentsOfFile:fileName ];
            [ myImage setScalesWhenResized:YES ];
            [ myImage setSize:theFrame.size ];
            [ myImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver ];
            [ myImage release ];
        }
        [ self setAnimationTimeInterval:1/1.0 ];
#else   // Code for possible future use if we want to draw more in preview
        myContext = [[NSGraphicsContext currentContext] graphicsPort];
        drawPreview(myContext);        
        [ self setAnimationTimeInterval:1/30.0 ];
#endif
        return;
    }

   myContext = [[NSGraphicsContext currentContext] graphicsPort];
//    [myContext retain];
    
    NSWindow *myWindow = [ self window ];
    NSRect windowFrame = [ myWindow frame ];
    if ( (windowFrame.origin.x != 0) || (windowFrame.origin.y != 0) ) {
        // Hide window on second display to aid in debugging
#ifdef _DEBUG
        [ myWindow setLevel:kCGMinimumWindowLevel ];
        NSInteger alpha = 0;
        [ myWindow setAlphaValue:alpha ];   // For OS 10.6
#endif
        return;         // We draw only to main screen
    }

    NSRect viewBounds = [self bounds];

    newFrequency = getSSMessage(&msg, &coveredFreq);

    // NOTE: My tests seem to confirm that the top window is always the first 
    // window returned by NSWindowList under OS 10.5 and the second window 
    // returned by NSWindowList under OS 10.3.9 and OS 10.4.  However, Apple's 
    // documentation is unclear whether we can depend on this.  So I have 
    // added some safety by doing two things:
    // [1] Only use the NSWindowList test when we have started project graphics.
    // [2] Assume that our window is covered 45 seconds after starting project 
    //     graphics even if the NSWindowList test did not indicate that is so.
    //
    // getSSMessage() returns a non-zero value for coveredFreq only if we have started 
    // project graphics.
    //
    // If we should use a different frequency when our window is covered by another 
    // window, then check whether there is a window at a higher z-level than ours.

    // Assuming our window(s) are initially the top window(s), 
    // determine our position in the window list when no graphics 
    // applications have covered us.
    if (gTopWindowListIndex < 0) {
        myWindowNumber = [ myWindow windowNumber ];
        NSWindowList(20, windowList);
        NSCountWindows(&n);
        if (n > 20) n = 20; 
        for (i=0; i<n; i++) {
            if (windowList[i] == myWindowNumber) {
                gTopWindowListIndex = i;
                break;
            }
        }
    }

    if (coveredFreq) {
        if ( (msg != NULL) && (msg[0] != '\0') ) {
            myWindowNumber = [ myWindow windowNumber ];

            windowList[0] = 0;
            NSWindowList(20, windowList);
            NSCountWindows(&n);
            if (gTopWindowListIndex < n) { 
                if (windowList[gTopWindowListIndex] != myWindowNumber) {
                    // Project graphics application has a window open above ours
                    // Don't waste CPU cycles since our window is obscured by application graphics
                    newFrequency = coveredFreq;
                    msg = NULL;
                    windowIsCovered();
                }
            }
        } else {
            newFrequency = coveredFreq;
        }
    }
    
    // Clear the previous drawing area
    currentDrawingRect = gMovingRect;
    currentDrawingRect.origin.x = (float) ((int)gCurrentPosition.x);
    currentDrawingRect.origin.y += (float) ((int)gCurrentPosition.y - gTextBoxHeight);

    if ( (msg != NULL) && (msg[0] != '\0') ) {

        // Set direction of motion to "bounce" off edges of screen
       if (currentDrawingRect.origin.x <= SAFETYBORDER) {
            gCurrentDelta.x = (float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.y = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.y)) / 16.;
        }
        if ( (currentDrawingRect.origin.x + currentDrawingRect.size.width) >= 
                    (viewBounds.origin.x + viewBounds.size.width - SAFETYBORDER) ) {
            gCurrentDelta.x = -(float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.y = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.y)) / 16.;
        }
        if (currentDrawingRect.origin.y <= SAFETYBORDER) {
            gCurrentDelta.y = (float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.x = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.x)) / 16.;
        }
        if ( (currentDrawingRect.origin.y + currentDrawingRect.size.height) >= 
                   (viewBounds.origin.y + viewBounds.size.height - SAFETYBORDER) ) {
            gCurrentDelta.y = -(float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.x = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.x)) / 16.;
        }
#if 0
        // For testing
        gCurrentDelta.x = 0;
        gCurrentDelta.y = 0;
#endif

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

            isErased  = true;
        }

        // Get the new drawing area
        gCurrentPosition.x += gCurrentDelta.x;
        gCurrentPosition.y += gCurrentDelta.y;
        
        imagePosition.x = (float) ((int)gCurrentPosition.x + gImageXIndent);
        imagePosition.y = (float) (int)gCurrentPosition.y;

        [ gBOINC_Logo compositeToPoint:imagePosition operation:NSCompositeCopy ];

        if ( (msg != NULL) && (msg[0] != '\0') ) {
            cf_msg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingMacRoman);

            CGRect bounds = CGRectMake((float) ((int)gCurrentPosition.x), 
                                 viewBounds.size.height - imagePosition.y + TEXTBOXTOPBORDER,
                                 bounds.origin.x + gMovingRect.size.width,
                                 bounds.origin.y + (int)MAXTEXTBOXHEIGHT
                            );

            CGContextSaveGState (myContext);
            CGContextTranslateCTM (myContext, 0, viewBounds.origin.y + viewBounds.size.height);
            CGContextScaleCTM (myContext, 1.0f, -1.0f);


#ifdef __x86_64__
            CTFontRef myFont = CTFontCreateWithName(CFSTR("Helvetica"), 20, NULL);

            HIThemeTextInfo textInfo = {kHIThemeTextInfoVersionOne, kThemeStateActive, kThemeSpecifiedFont, 
                                        kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, 
                                        kHIThemeTextBoxOptionNone, kHIThemeTextTruncationNone, 0, false,
                                        0, myFont
                                        };

#else
            GrafPtr port;
            GetPort(&port);
            SetPortTextFont(port, kFontIDHelvetica);
            SetPortTextSize(port, 20);
            
            HIThemeTextInfo textInfo = {0, kThemeStateActive, kThemeCurrentPortFont, //kThemeMenuItemCmdKeyFont, //kThemePushButtonFont, 
                                        kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, 
                                        kHIThemeTextBoxOptionNone, kHIThemeTextTruncationNone, 0, false 
                                        };
#endif

            // Use only APIs available in Mac OS 10.3.9
//            HIThemeSetTextFill(kThemeTextColorWhite, NULL, myContext, kHIThemeOrientationNormal);
//            SetThemeTextColor(kThemeTextColorWhite, 32, true);

            CGFloat myWhiteComponents[] = {1.0, 1.0, 1.0, 1.0};
            CGColorSpaceRef myColorSpace = CGColorSpaceCreateDeviceRGB ();
            CGColorRef myTextColor = CGColorCreate(myColorSpace, myWhiteComponents);

            CGContextSetFillColorWithColor(myContext, myTextColor);

            HIThemeDrawTextBox(cf_msg, &bounds, &textInfo, myContext, kHIThemeOrientationNormal);

            CGColorRelease(myTextColor);
            CGColorSpaceRelease(myColorSpace);
            CGContextRestoreGState (myContext);
            CFRelease(cf_msg);
        }
        
        gTextBoxHeight = MAXTEXTBOXHEIGHT + TEXTBOXTOPBORDER;
        gMovingRect.size.height = [gBOINC_Logo size].height + gTextBoxHeight;
        
        isErased  = false;
        
    } else {        // Empty or NULL message
        if (!isErased) {
            eraseRect = NSInsetRect(currentDrawingRect, -1, -1);
            [[NSColor blackColor] set];
            isErased  = true;
            NSRectFill(eraseRect);
            gTextBoxHeight = MAXTEXTBOXHEIGHT;
            gMovingRect.size.height = [gBOINC_Logo size].height + gTextBoxHeight;
        }
    }
    
    if (newFrequency)
        [ self setAnimationTimeInterval:(1.0/newFrequency) ];
    // setAnimationTimeInterval does not seem to be working, so we 
    // throttle the screensaver directly here.
    timeToUnblock = AddDurationToAbsolute(durationSecond/newFrequency, frameStartTime);
    MPDelayUntil(&timeToUnblock);
}

- (BOOL)hasConfigureSheet {
    return YES;
}

// Display the configuration sheet for the user to choose their settings
- (NSWindow*)configureSheet
{
    int period;

	// if we haven't loaded our configure sheet, load the nib named MyScreenSaver.nib
	if (!mConfigureSheet)
        [ NSBundle loadNibNamed:@"BOINCSaver" owner:self ];
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
    int period;

    // get the defaults
	ScreenSaverDefaults *defaults = [ ScreenSaverDefaults defaultsForModuleWithName:mBundleID ];

	// save the UI state
	gGoToBlank = [ mGoToBlankCheckbox state ];
	mBlankingTimeString = [ mBlankingTimeTextField stringValue ];
    gBlankingTime = [ mBlankingTimeString intValue ];
    if ((gBlankingTime < 0) || (gBlankingTime > 999)) goto Bad;

	mDefaultPeriodString = [ mDefaultPeriodTextField stringValue ];
    period = [ mDefaultPeriodString intValue ];
    if (!validateNumericString((CFStringRef)mDefaultPeriodString)) goto Bad;
    if ((period < 0) || (period > 999)) goto Bad;
    setGFXDefaultPeriod((double)(period * 60));

	mSciencePeriodString = [ mSciencePeriodTextField stringValue ];
    period = [ mSciencePeriodString intValue ];
    if (!validateNumericString((CFStringRef)mSciencePeriodString)) goto Bad;
    if ((period < 0) || (period > 999)) goto Bad;
    setGFXSciencePeriod((double)(period * 60));

	mChangePeriodString = [ mChangePeriodTextField stringValue ];
    period = [ mChangePeriodString intValue ];
     if (!validateNumericString((CFStringRef)mChangePeriodString)) goto Bad;
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
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please enter a number between 0 and 999."];
    [alert setAlertStyle:NSCriticalAlertStyle];
    [alert beginSheetModalForWindow:mConfigureSheet modalDelegate:self didEndSelector:nil contextInfo:nil];
}

// Called when the user clicked the CANCEL button
- (IBAction) closeSheetCancel:(id) sender
{
	// nothing to configure
    [ NSApp endSheet:mConfigureSheet ];
}

@end
