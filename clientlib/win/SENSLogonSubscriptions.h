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

#pragma once

#define SUBSCIPTION_NAME_DISPLAYLOCK \
    OLESTR("BOINC Subscription to SENS DisplayLock Event")

#define SUBSCIPTION_NAME_DISPLAYUNLOCK \
    OLESTR("BOINC Subscription to SENS DisplayUnLock Event")

#define SUBSCIPTION_NAME_LOGOFF \
    OLESTR("BOINC Subscription to SENS Logoff Event")

#define SUBSCIPTION_NAME_LOGON \
    OLESTR("BOINC Subscription to SENS Logon Event")

#define SUBSCIPTION_NAME_STARTSCREENSAVER \
    OLESTR("BOINC Subscription to SENS StartScreenSaver Event")

#define SUBSCIPTION_NAME_STARTSHELL \
    OLESTR("BOINC Subscription to SENS StartShell Event")

#define SUBSCIPTION_NAME_STOPSCREENSAVER \
    OLESTR("BOINC Subscription to SENS StopScreenSaver Event")

//
// Subscription Guids
//

// {B6EEAA4F-D713-4cf2-B798-4D03E8B8FE59}
EXTERN_C const GUID GUID_SUBSCRIPTION_DISPLAYLOCK =
{ 0xb6eeaa4f, 0xd713, 0x4cf2, { 0xb7, 0x98, 0x4d, 0x3, 0xe8, 0xb8, 0xfe, 0x59 } };

// {C51E3293-2E53-4a97-9D83-0DE49FE33141}
EXTERN_C const GUID GUID_SUBSCRIPTION_DISPLAYUNLOCK = 
{ 0xc51e3293, 0x2e53, 0x4a97, { 0x9d, 0x83, 0xd, 0xe4, 0x9f, 0xe3, 0x31, 0x41 } };

// {C5EEBA2E-0C7E-41ab-AB58-BA0F049FE99C}
EXTERN_C const GUID GUID_SUBSCRIPTION_LOGOFF =
{ 0xc5eeba2e, 0xc7e, 0x41ab, { 0xab, 0x58, 0xba, 0xf, 0x4, 0x9f, 0xe9, 0x9c } };

// {909C5924-A5A3-4506-838D-F8633754C2BC}
EXTERN_C const GUID GUID_SUBSCRIPTION_LOGON =
{ 0x909c5924, 0xa5a3, 0x4506, { 0x83, 0x8d, 0xf8, 0x63, 0x37, 0x54, 0xc2, 0xbc } };

// {63616A1E-8811-4b29-B131-99B52002A330}
EXTERN_C const GUID GUID_SUBSCRIPTION_STARTSCREENSAVER =
{ 0x63616a1e, 0x8811, 0x4b29, { 0xb1, 0x31, 0x99, 0xb5, 0x20, 0x2, 0xa3, 0x30 } };

// {4E5C1C2E-3C5D-4b8a-9D10-80A7C844E738}
EXTERN_C const GUID GUID_SUBSCRIPTION_STARTSHELL =
{ 0x4e5c1c2e, 0x3c5d, 0x4b8a, { 0x9d, 0x10, 0x80, 0xa7, 0xc8, 0x44, 0xe7, 0x38 } };

// {BBCF2081-8E09-4708-B4FF-7DE11C16E85E}
EXTERN_C const GUID GUID_SUBSCRIPTION_STOPSCREENSAVER =
{ 0xbbcf2081, 0x8e09, 0x4708, { 0xb4, 0xff, 0x7d, 0xe1, 0x1c, 0x16, 0xe8, 0x5e } };


const SENS_SUBSCRIPTION gSENSLogonSubscriptions[] =
{
    {
    &GUID_SUBSCRIPTION_DISPLAYLOCK,
    SUBSCIPTION_NAME_DISPLAYLOCK,
    OLESTR("DisplayLock"),
    FALSE,
    NULL,
    NULL
    },

    {
	&GUID_SUBSCRIPTION_DISPLAYUNLOCK,
    SUBSCIPTION_NAME_DISPLAYUNLOCK,
    OLESTR("DisplayUnLock"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_LOGOFF,
    SUBSCIPTION_NAME_LOGOFF,
    OLESTR("Logoff"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_LOGON,
    SUBSCIPTION_NAME_LOGON,
    OLESTR("Logon"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_STARTSCREENSAVER,
    SUBSCIPTION_NAME_STARTSCREENSAVER,
    OLESTR("StartScreenSaver"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_STARTSHELL,
    SUBSCIPTION_NAME_STARTSHELL,
    OLESTR("StartShell"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_STOPSCREENSAVER,
    SUBSCIPTION_NAME_STOPSCREENSAVER,
    OLESTR("StopScreenSaver"),
    FALSE,
    NULL,
    NULL
    }
};

#define SENS_LOGON_SUBSCRIPTIONS_COUNT    (sizeof(gSENSLogonSubscriptions)/sizeof(SENS_SUBSCRIPTION))
