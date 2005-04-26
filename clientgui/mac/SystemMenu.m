/* SystemMenu.m */

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

/* Cocoa routines for BOINC Manager OS X System Icon Menu.
    Written by Charlie Fenton for UC Berkeley Space Sciences Laboratory 2005
    
    Adapted from Apple Developer Tech Support sample code, including:
      SpellingChecker-CarbonCocoa
      MenuMadness
*/

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#define	_DEBUG	0

@interface SystemMenu : NSObject {
}
- (void)BuildSysMenu:(MenuRef)menuToCopy;
@end




SystemMenu *gSystemMenu;
NSStatusItem *gStatusItem;

void SetSystemMenuIcon(PicHandle theIcon);
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr);

/*
	Needed to make sure the Cocoa framework has a chance to initialize things in a Carbon app.
	
	NSApplicationLoad() is an API introduced in 10.2 which is a startup function to call when
	running Cocoa code from a Carbon application.

	Because NSApplicationLoad() is not available pre 10.2, we load the function pointer through
	CFBundle.  If we encounter an error during this process, (i.e. it's not there), we fall back
	to calling NSApplication *NSApp=[NSApplication sharedApplication];
	The fallback method does have bugs with regard to window activation seen when hiding and showing
	the main application.
*/
typedef BOOL (*NSApplicationLoadFuncPtr)( void );

void	InitializeCocoa()
{
	CFBundleRef 				appKitBundleRef;
	NSApplicationLoadFuncPtr                myNSApplicationLoad;
	OSStatus				err;
	
	//	Load the "AppKit.framework" bundl to locate NSApplicationLoad
	err = LoadFrameworkBundle( CFSTR("AppKit.framework"), &appKitBundleRef );
	if (err != noErr) goto FallbackMethod;
	
	//	Manually load the Mach-O function pointers for the routines we will be using.
	myNSApplicationLoad	= (NSApplicationLoadFuncPtr) CFBundleGetFunctionPointerForName( appKitBundleRef, CFSTR("NSApplicationLoad") );
	if ( myNSApplicationLoad == NULL ) goto FallbackMethod;

	(void) myNSApplicationLoad();
	return;

FallbackMethod:
	{   /* NSApplication *NSApp = */ [NSApplication sharedApplication]; }
}



/*
*/
void	SetUpSystemMenu(MenuRef menuToCopy, PicHandle theIcon)
{
    NSAutoreleasePool* pool;
    
    InitializeCocoa();
        
    pool	= [[NSAutoreleasePool alloc] init];
    
    gSystemMenu = [[SystemMenu alloc] init];
    [gSystemMenu retain];
    
    [gSystemMenu BuildSysMenu:menuToCopy];
//    [gStatusItem setImage: [NSImage imageNamed:@"Icon0"]];
    SetSystemMenuIcon(theIcon);

    [pool release];
}


@implementation SystemMenu

- (void)BuildSysMenu:(MenuRef)menuToCopy {
    NSStatusBar *bar;
    NSMenuItem *newItem;
    NSMenu *sysMenu;
    int i, n;
    Str255 s;
    CFStringRef CFText;

    // Add the submenu
    newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:@"BOINC!" action:NULL keyEquivalent:@""];
    sysMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"BOINC!"];
    [newItem setSubmenu:sysMenu];

    bar = [NSStatusBar systemStatusBar];

    gStatusItem = [bar statusItemWithLength:NSSquareStatusItemLength];
    [gStatusItem retain];

    [gStatusItem setTitle: NSLocalizedString(@"",@"")];
    [gStatusItem setHighlightMode:YES];
    [gStatusItem setMenu:sysMenu];

    [sysMenu release];
    [newItem release];

    n = CountMenuItems(menuToCopy);
    // Add the items
    for (i=1; i<=n; i++)
    {
        GetMenuItemText(menuToCopy, i, s); 
        if (PLstrcmp(s, "\p-") == 0)
        {
            [sysMenu addItem:[NSMenuItem separatorItem]];
            continue;
        }
        
        CFText = CFStringCreateWithPascalString(kCFAllocatorDefault, s, kCFStringEncodingMacRoman);
        if (CFText != NULL)
        {
            newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:(NSString*)CFText action:NULL keyEquivalent:@""];
            [newItem setTarget:self];
            [sysMenu addItem:newItem];
            [newItem release];
            CFRelease(CFText);
        }
    }
    
    return;
}


void SetSystemMenuIcon(PicHandle theIcon)
{
    unsigned theLength  = GetHandleSize((Handle)theIcon);
    NSData* theData = [[NSData alloc] initWithBytes:*theIcon length:theLength];
    NSImage* theImage = [[NSImage alloc] initWithData:theData];

    [gStatusItem setImage:theImage];
    
    [theImage release];
    [theData release];
    
    return;
}


static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	if ( bundlePtr == nil )	return( -1 );
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
	    if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	if (err != noErr && *bundlePtr != nil) {
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	}	
	if (baseURL != nil) {
		CFRelease(baseURL);
	}	
	
	return err;
}

@end
