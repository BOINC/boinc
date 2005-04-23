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
//  Mac_Saver_ModuleView.m
//  BOINC_Saver_Module
//

#import "Mac_Saver_ModuleView.h"

int gGoToBlank;      // True if we are to blank the screen
int gBlankingTime;   // Delay in minutes before blanking the screen

@implementation BOINC_Saver_ModuleView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview {
    NSQuickDrawView * myQDView;

    self = [ super initWithFrame:frame isPreview:isPreview ];
    if (self) {
        // grab the screensaver defaults
        mBundleID = [[ NSBundle bundleForClass:[self class]] bundleIdentifier ];
        ScreenSaverDefaults *defaults = [ ScreenSaverDefaults defaultsForModuleWithName:mBundleID ];
        
        // try to load the version key, used to see if we have any saved settings
        mVersion = [defaults floatForKey:@"version"];
        if (!mVersion) {
            // no previous settings so define our defaults
            mVersion = 1;
            gGoToBlank = NO;
            gBlankingTime = 1;
            
            // write out the defaults
            [ defaults setInteger:mVersion forKey:@"version" ];
            [ defaults setInteger:gGoToBlank forKey:@"GoToBlank" ];
            [ defaults setInteger:gBlankingTime forKey:@"BlankingTime" ];
            
            // synchronize
            [defaults synchronize];
        }

        // set defaults...
        gGoToBlank = [ defaults integerForKey:@"GoToBlank" ];
        gBlankingTime = [ defaults integerForKey:@"BlankingTime" ];

        myQDView = [[ NSQuickDrawView alloc ] initWithFrame:frame ];
        if (!myQDView) {
            [ self autorelease ];
            return nil;
        }
        
        [ self setAutoresizesSubviews:YES ];	// make sure the subview resizes.
        [ self addSubview:myQDView ];
        [ myQDView release ];
        if (isPreview)
            previewQDView = myQDView;
        else
            mainQDView = myQDView;
            
        [ self setAnimationTimeInterval:1/8.0 ];
    }

    return self;
}

- (void)startAnimation {
    int newFrequency;

    [ super startAnimation ];

    if ( [ self isPreview ] )
        return;
        
    newFrequency = initBOINCSaver([ self isPreview ]);        
    if (newFrequency)
        [ self setAnimationTimeInterval:1.0/newFrequency ];
}

- (void)stopAnimation {
    [ super stopAnimation ];

    if ( [ self isPreview ] )
        return;

    closeBOINCSaver();
}

- (void)drawRect:(NSRect)rect {
    NSView* myView = nil;
    NSWindow* myWindow = nil;
    
    [ super drawRect:rect ];

    // Set our blanking window slightly below screen saver window level 
    // so that our application's graphics can draw over it.
    if (! [ self isPreview ] ) {
        myView = [ NSView focusView ];
        if (myView)
            myWindow = [ myView window ];
        if (myWindow)
            [ myWindow setLevel:kCGStatusWindowLevel ];
    }
//  optionally draw here
}

- (void)animateOneFrame {
    int newFrequency;
    NSRect theFrame = [ self frame ];
    Point p = { 0, 0 };

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
        [ previewQDView lockFocus ];
        drawPreview([ previewQDView qdPort ]);        
        [ previewQDView unlockFocus ];
        QDFlushPortBuffer([ previewQDView qdPort ], nil);
        [ self setAnimationTimeInterval:1/30.0 ];
#endif
        return;
    }
        
    [ mainQDView lockFocus ];
    LocalToGlobal(&p);
    if ((p.h != 0) || (p.v != 0)) {
        [ mainQDView unlockFocus ];
        
        // Hide window on second display to aid in debugging
//      [[[ NSView focusView] window ] setLevel:kCGMinimumWindowLevel ];
        return;         // We draw only to main screen
    }
    
    newFrequency = drawGraphics([ mainQDView qdPort ]);
    [ mainQDView unlockFocus ];
    QDFlushPortBuffer([ mainQDView qdPort ], nil);
    if (newFrequency)
        [ self setAnimationTimeInterval:(1.0/newFrequency) ];
}

- (BOOL)hasConfigureSheet {
    return YES;
}

// Display the configuration sheet for the user to choose their settings
- (NSWindow*)configureSheet
{
	// if we haven't loaded our configure sheet, load the nib named MyScreenSaver.nib
	if (!mConfigureSheet)
            [ NSBundle loadNibNamed:@"BOINCSaver" owner:self ];
	// set the UI state
	[ mGoToBlankCheckbox setState:gGoToBlank ];
        mBlankingTimeString = [[ NSString alloc ] initWithFormat:@"%d", gBlankingTime ];
	[ mBlankingTimeTextField setStringValue:mBlankingTimeString ];
    
	return mConfigureSheet;
}

// Called when the user clicked the SAVE button
- (IBAction) closeSheetSave:(id) sender
{
    // get the defaults
	ScreenSaverDefaults *defaults = [ ScreenSaverDefaults defaultsForModuleWithName:mBundleID ];

	// save the UI state
	gGoToBlank = [ mGoToBlankCheckbox state ];
	mBlankingTimeString = [ mBlankingTimeTextField stringValue ];
        gBlankingTime = [ mBlankingTimeString intValue ];
	
	// write the defaults
	[ defaults setInteger:gGoToBlank forKey:@"GoToBlank" ];
	[ defaults setInteger:gBlankingTime forKey:@"BlankingTime" ];
	
	// synchronize
    [ defaults synchronize ];

	// end the sheet
    [ NSApp endSheet:mConfigureSheet ];
}

// Called when the user clicked the CANCEL button
- (IBAction) closeSheetCancel:(id) sender
{
	// nothing to configure
    [ NSApp endSheet:mConfigureSheet ];
}


@end
