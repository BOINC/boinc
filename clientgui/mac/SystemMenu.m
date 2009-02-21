/* SystemMenu.m */

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

/* Cocoa routines for BOINC Manager OS X System Icon Menu.
    Written by Charlie Fenton for UC Berkeley Space Sciences Laboratory 2005
    
    Adapted from Apple Developer Tech Support sample code, including:
      SpellingChecker-CarbonCocoa
      MenuMadness
*/

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

@interface SystemMenu : NSObject {
}
- (void)BuildSysMenu:(MenuRef)menuToCopy;
- (void)postEvent:(id)sender;
@end




SystemMenu *gSystemMenu = NULL;
NSStatusItem *gStatusItem = NULL;

void SetSystemMenuIcon(CGImageRef theIcon);
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

void	InitializeCocoa() {
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
void	SetUpSystemMenu(MenuRef menuToCopy, CGImageRef theIcon) {
    NSAutoreleasePool* pool;
    
    if (gSystemMenu == NULL)
        InitializeCocoa();
        
    pool	= [[NSAutoreleasePool alloc] init];
    
    if (gSystemMenu)
        [gSystemMenu release];
        
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
    UInt32 tag;
    OSErr err;

    // Add the submenu
    newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:@"BOINC!" action:NULL keyEquivalent:@""];
    sysMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"BOINC!"];
    [newItem setSubmenu:sysMenu];

    if (gStatusItem == NULL) {
        bar = [NSStatusBar systemStatusBar];

        gStatusItem = [bar statusItemWithLength:NSSquareStatusItemLength];
        [gStatusItem retain];
    }
    
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
        err = GetMenuItemCommandID(menuToCopy, i, &tag);
        
       if ((PLstrcmp(s, "\p-") == 0) || (tag == 0))
        {
            [sysMenu addItem:[NSMenuItem separatorItem]];
            continue;
        }
        
        CFText = CFStringCreateWithPascalString(kCFAllocatorDefault, s, kCFStringEncodingMacRoman);
        if (CFText != NULL)
        {
            newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:(NSString*)CFText action:NULL keyEquivalent:@""];
           if (err == noErr) {
                [newItem setTarget:self];
                [sysMenu addItem:newItem];
                if( IsMenuItemEnabled(menuToCopy, i) )
                    [newItem setEnabled:YES];
                else
                    [newItem setEnabled:NO];
                
                // setTag and setAction are needed only in OS 10.5
                [newItem setTag:tag];
                [newItem setAction:@selector(postEvent:)];

                [newItem release];
            }
            CFRelease(CFText);
        }
    }
    
    [sysMenu setAutoenablesItems:NO];
    return;
}


// postEvent is needed only in OS 10.5
- (void)postEvent:(id)sender {
    HICommand commandStruct;
    EventRef theEvent;
    OSStatus err;

    // Build a kEventClassCommand CarbonEvent and set the CommandId 
    //  to the value of the menu item's tag
    err = CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, 
                                0, kEventAttributeUserEvent, &theEvent);
    commandStruct.commandID = [sender tag];
    commandStruct.attributes = kHICommandFromMenu;
    commandStruct.menu.menuRef = (MenuRef)'BNC!';
        
    if (err == noErr)
        err = SetEventParameter(theEvent, kEventParamDirectObject, 
                                typeHICommand, sizeof(HICommand), &commandStruct);
    if (err == noErr)
        err = SendEventToEventTarget(theEvent, GetApplicationEventTarget());
//    SysBeep(4);
}


void SetSystemMenuIcon(CGImageRef theIcon)
{
    if (theIcon == NULL) 
    {
        // A NULL icon handle is a request to remove the status item from the menu bar
        [[gStatusItem statusBar] removeStatusItem:gStatusItem];
        [gStatusItem release];
        [gSystemMenu release];
        gStatusItem = NULL;
        gSystemMenu = NULL;
        
        return;
    }
        
    NSRect imageRect = NSMakeRect(0.0, 0.0, 0.0, 0.0);
    CGContextRef imageContext = nil;
    NSImage* theImage = nil;
 
    // Get the image dimensions.
    imageRect.size.height = CGImageGetHeight(theIcon);
    imageRect.size.width = CGImageGetWidth(theIcon);
 
    // Create a new image to receive the Quartz image data.
    theImage = [[NSImage alloc] initWithSize:imageRect.size];
    [theImage lockFocus];
 
    // Get the Quartz context and draw.
    imageContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    CGContextDrawImage(imageContext, *(CGRect*)&imageRect, theIcon);
    [theImage unlockFocus];

    [gStatusItem setImage:theImage];
    
    [theImage release];
    
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
