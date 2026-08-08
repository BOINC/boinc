// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

//  MacNotification.mm

#include "MacGUI.pch"
#include "BOINCGUIApp.h"    // For wxGetApp()
#include "BOINCTaskBar.h"
#include <dlfcn.h>
#import <Cocoa/Cocoa.h>

// Weak linking of objective-C classes is not supported before
// OS 10.6.8 so to be compatible with OS 10.5 we must use the
// objective-C equivalent of dlopen() and dlsym().
static Class NSUserNotificationClass = nil;
static Class NSUserNotificationCenterClass = nil;
static NSString **NSUserNotificationDefaultSoundNamePtr = nil;


@interface MacNotification : NSObject <NSUserNotificationCenterDelegate>
{
    NSUserNotification *theNotification;
}
- (void) showNewNoticesNotification:(NSString *)theTitle withMessage:(NSString *)theMessage;

@end

@implementation MacNotification

- (void) showNewNoticesNotification:(NSString *)theTitle withMessage:(NSString *)theMessage
{
    [[NSUserNotificationCenterClass defaultUserNotificationCenter] setDelegate:self];

    theNotification = [[NSUserNotificationClass alloc] init];
    [theNotification setTitle:theTitle];
    [theNotification setInformativeText:theMessage];
    [theNotification setDeliveryDate:[NSDate dateWithTimeInterval:0 sinceDate:[NSDate date]]];
    if (NSUserNotificationDefaultSoundNamePtr == nil) {
        NSUserNotificationDefaultSoundNamePtr = (NSString **)dlsym(RTLD_DEFAULT, "NSUserNotificationDefaultSoundName");
    }
    if (NSUserNotificationDefaultSoundNamePtr != nil) {
        [theNotification setSoundName:*NSUserNotificationDefaultSoundNamePtr];
    }
    NSUserNotificationCenter *center = [NSUserNotificationCenterClass defaultUserNotificationCenter];
    [center deliverNotification:theNotification];
}

- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    if ([[notification title] isEqualToString:[theNotification title]] ) {
        wxGetApp().ShowNotifications();
    }
}

@end

bool CTaskBarIcon::IsBalloonsSupported() {
    if ((NSUserNotificationClass != nil) && (NSUserNotificationCenterClass != nil)) return true;

    NSBundle *bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/Foundation.framework"];
    NSError *err = nil;
    bool loaded = [bundle loadAndReturnError:&err];
    if (!loaded) return false;


    NSUserNotificationClass = NSClassFromString(@"NSUserNotification");
    if (NSUserNotificationClass == nil) return false;

    NSUserNotificationCenterClass = NSClassFromString(@"NSUserNotificationCenter");
    return (NSUserNotificationCenterClass != nil);
}

static MacNotification* MacNotificationClass = nil;

bool CTaskBarIcon::QueueBalloon(
        const wxIcon&,
        const wxString title,
        const wxString message,
        unsigned int
    ) {

    if (!IsBalloonsSupported())return false;

    if (MacNotificationClass == nil) {
        MacNotificationClass = [MacNotification alloc];
    }

    if (MacNotificationClass) {
        char *utf8Title = (char *)(title.utf8_str().data());
        NSString *theTitle = [[NSString alloc] initWithUTF8String:utf8Title];

        char *utf8Message = (char *)(message.utf8_str().data());
        NSString *theMessage = [[NSString alloc] initWithUTF8String:utf8Message];

        [MacNotificationClass showNewNoticesNotification:theTitle withMessage:theMessage];
    }

    return false;
}



// wxTopLevel::RequestUserAttention() doesn't have an API to cancel
// after a timeout, so we must call Notification Manager directly on Mac
void CTaskBarIcon::MacRequestUserAttention() {
    m_pNotificationRequest = [NSApp requestUserAttention:NSInformationalRequest];
}

void CTaskBarIcon::MacCancelUserAttentionRequest() {
    if (m_pNotificationRequest) {
        [NSApp cancelUserAttentionRequest:m_pNotificationRequest];
        m_pNotificationRequest = 0;
    }
}


int CTaskBarIcon::SetDockBadge(wxBitmap* bmp) {
    // Reset to standard Dock tile (no badge)
    [NSApp setApplicationIconImage:nil];

    if (bmp == NULL) {
        return 0;
    }

    NSImage *appIcon = [NSApp applicationIconImage];
    NSImage *buf = [appIcon copy];
    NSImage *badge = bmp->GetNSImage();

    [buf lockFocus];
    [badge drawAtPoint:NSMakePoint(0, 0)
            fromRect:NSZeroRect
            operation:NSCompositeSourceOver
            fraction:1.0f
    ];

    [buf unlockFocus];
    [NSApp setApplicationIconImage:buf];
    [buf release];

    return 0;
}
