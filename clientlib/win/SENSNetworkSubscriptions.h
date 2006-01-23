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

#define SUBSCIPTION_NAME_CONNECTIONMADE \
    OLESTR("BOINC Subscription to SENS ConnectionMade Event")

#define SUBSCIPTION_NAME_CONNECTIONMADE_NOQOC \
    OLESTR("BOINC Subscription to SENS ConnectionMadeNoQOCInfo Event")

#define SUBSCIPTION_NAME_CONNECTIONLOST \
    OLESTR("BOINC Subscription to SENS ConnectionLost Event")

#define SUBSCIPTION_NAME_DESTINATIONREACHABLE \
    OLESTR("BOINC Subscription to SENS DestinationReachable Event")

#define SUBSCIPTION_NAME_DESTINATIONREACHABLE_NOQOC \
    OLESTR("BOINC Subscription to SENS DestinationReachableNoQOCInfo Event")

//
// Subscription Guids
//

// {F6F32236-73E5-4914-BF55-8485623657FF}
EXTERN_C const GUID GUID_SUBSCRIPTION_CONNECTIONMADE =
{ 0xf6f32236, 0x73e5, 0x4914, { 0xbf, 0x55, 0x84, 0x85, 0x62, 0x36, 0x57, 0xff } };

// {6051A461-00B0-4185-9FC3-20FDE3C333FE}
EXTERN_C const GUID GUID_SUBSCRIPTION_CONNECTIONMADE_NOQOC = 
{ 0x6051a461, 0xb0, 0x4185, { 0x9f, 0xc3, 0x20, 0xfd, 0xe3, 0xc3, 0x33, 0xfe } };

// {A8EDB33C-55FF-4d5d-965A-27769CC279AD}
EXTERN_C const GUID GUID_SUBSCRIPTION_CONNECTIONLOST =
{ 0xa8edb33c, 0x55ff, 0x4d5d, { 0x96, 0x5a, 0x27, 0x76, 0x9c, 0xc2, 0x79, 0xad } };

// {6EE0AF08-A3BE-4be3-B048-8C752D585852}
EXTERN_C const GUID GUID_SUBSCRIPTION_DESTINATIONREACHABLE =
{ 0x6ee0af08, 0xa3be, 0x4be3, { 0xb0, 0x48, 0x8c, 0x75, 0x2d, 0x58, 0x58, 0x52 } };

// {C19482AE-4FD5-4e52-BE81-F25DD6E26A0A}
EXTERN_C const GUID GUID_SUBSCRIPTION_DESTINATIONREACHABLE_NOQOC =
{ 0xc19482ae, 0x4fd5, 0x4e52, { 0xbe, 0x81, 0xf2, 0x5d, 0xd6, 0xe2, 0x6a, 0xa } };


const SENS_SUBSCRIPTION gSENSNetworkSubscriptions[] =
{
    {
    &GUID_SUBSCRIPTION_CONNECTIONMADE,
    SUBSCIPTION_NAME_CONNECTIONMADE,
    OLESTR("ConnectionMade"),
    FALSE,
    NULL,
    NULL
    },

    {
	&GUID_SUBSCRIPTION_CONNECTIONMADE_NOQOC,
    SUBSCIPTION_NAME_CONNECTIONMADE_NOQOC,
    OLESTR("ConnectionMadeNoQOCInfo"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_CONNECTIONLOST,
    SUBSCIPTION_NAME_CONNECTIONLOST,
    OLESTR("ConnectionLost"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_DESTINATIONREACHABLE,
    SUBSCIPTION_NAME_DESTINATIONREACHABLE,
    OLESTR("DestinationReachable"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_DESTINATIONREACHABLE_NOQOC,
    SUBSCIPTION_NAME_DESTINATIONREACHABLE_NOQOC,
    OLESTR("DestinationReachableNoQOCInfo"),
    FALSE,
    NULL,
    NULL
    },
};

#define SENS_NETWORK_SUBSCRIPTIONS_COUNT    (sizeof(gSENSNetworkSubscriptions)/sizeof(SENS_SUBSCRIPTION))
