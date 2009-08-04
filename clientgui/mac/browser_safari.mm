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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "config.h"
#include "str_util.h"
#include "browser.h"

#include <Cocoa/Cocoa.h>

bool detect_setup_authenticator_safari(std::string& project_url, std::string& authenticator)
{    
    NSHTTPCookieStorage *cookieStorage;
    NSArray *theCookies;
    NSHTTPCookie *aCookie;
    NSURL *theURL;
    NSString *theURLString, *theValueString, *theNameString;
    NSDate *expirationDate;
    unsigned int i, n;
    bool retval = false;

    NSAutoreleasePool* pool;
    
    pool = [[NSAutoreleasePool alloc] init];
    
    
    theURLString = [ NSString stringWithCString:project_url.c_str() ];
    
    theURL = [ NSURL URLWithString:theURLString ];

    cookieStorage = [ NSHTTPCookieStorage sharedHTTPCookieStorage ];
    
    if (cookieStorage == NULL)
        goto bail;
    
    theCookies = [ cookieStorage cookiesForURL:theURL ];

    if (theCookies == NULL)
        goto bail;

    n = [ theCookies count ];
    for (i=0; i<n; i++) {
        aCookie = (NSHTTPCookie*)[ theCookies objectAtIndex:i ];

        // has the cookie expired?
        expirationDate = [ aCookie expiresDate ];
        if ([ expirationDate compare:[ NSDate date ]] == NSOrderedAscending)
            continue;
            
        theNameString = [ aCookie name ];
        // is this the right cookie?
#ifdef cStringUsingEncoding     // Available only is OS 10.4 and later
        if (!starts_with([ theNameString cStringUsingEncoding:NSMacOSRomanStringEncoding ], "Setup"))
            continue;
        theValueString = [ aCookie value ];
        authenticator = [ theValueString cStringUsingEncoding:NSMacOSRomanStringEncoding ];
#else
        if (!starts_with([ theNameString cString ], "Setup"))
            continue;
        theValueString = [ aCookie value ];
        authenticator = [ theValueString cString ];
#endif
        // If validation failed, null out the authenticator just in case
        //   somebody tries to use it, otherwise copy in the real deal.
        if (is_authenticator_valid(authenticator)) {
            retval = true;
            break;
        } else {
            authenticator = "";
        }
    }

bail:
    [pool release];

    return retval;
}