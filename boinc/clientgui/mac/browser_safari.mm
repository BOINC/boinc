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

#include "str_util.h"
#include "browser.h"

#include <Cocoa/Cocoa.h>

bool detect_cookie_safari(std::string& project_url, std::string& name, std::string& value)
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
        if (!starts_with([ theNameString cStringUsingEncoding:NSMacOSRomanStringEncoding ], name.c_str()))
            continue;
        theValueString = [ aCookie value ];
        value = [ theValueString cStringUsingEncoding:NSMacOSRomanStringEncoding ];
#else
        if (!starts_with([ theNameString cString ], name.c_str()))
            continue;
        theValueString = [ aCookie value ];
        value = [ theValueString cString ];
#endif
        retval = true;
    }

bail:
    [pool release];

    return retval;
}