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

#define SUBSCIPTION_NAME_ONACPOWER \
    OLESTR("BOINC Subscription to SENS OnACPower Event")

#define SUBSCIPTION_NAME_ONBATTERYPOWER \
    OLESTR("BOINC Subscription to SENS OnBatteryPower Event")

#define SUBSCIPTION_NAME_BATTERYLOW \
    OLESTR("BOINC Subscription to SENS BatteryLow Event")

//
// Subscription Guids
//

// {07753E05-C62E-49b7-8043-602022A20D45}
EXTERN_C const GUID GUID_SUBSCRIPTION_ONACPOWER =
{ 0x7753e05, 0xc62e, 0x49b7, { 0x80, 0x43, 0x60, 0x20, 0x22, 0xa2, 0xd, 0x45 } };

// {716F2168-9E3D-4623-97EA-6ED717D5D692}
EXTERN_C const GUID GUID_SUBSCRIPTION_ONBATTERYPOWER = 
{ 0x716f2168, 0x9e3d, 0x4623, { 0x97, 0xea, 0x6e, 0xd7, 0x17, 0xd5, 0xd6, 0x92 } };

// {2307AB30-A367-4e02-940B-AE9ABC1B0587}
EXTERN_C const GUID GUID_SUBSCRIPTION_BATTERYLOW =
{ 0x2307ab30, 0xa367, 0x4e02, { 0x94, 0xb, 0xae, 0x9a, 0xbc, 0x1b, 0x5, 0x87 } };


const SENS_SUBSCRIPTION gSENSOnNowSubscriptions[] =
{
    {
    &GUID_SUBSCRIPTION_ONACPOWER,
    SUBSCIPTION_NAME_ONACPOWER,
    OLESTR("OnACPower"),
    FALSE,
    NULL,
    NULL
    },

    {
	&GUID_SUBSCRIPTION_ONBATTERYPOWER,
    SUBSCIPTION_NAME_ONBATTERYPOWER,
    OLESTR("OnBatteryPower"),
    FALSE,
    NULL,
    NULL
    },

    {
    &GUID_SUBSCRIPTION_BATTERYLOW,
    SUBSCIPTION_NAME_BATTERYLOW,
    OLESTR("BatteryLow"),
    FALSE,
    NULL,
    NULL
    },
};

#define SENS_ONNOW_SUBSCRIPTIONS_COUNT    (sizeof(gSENSOnNowSubscriptions)/sizeof(SENS_SUBSCRIPTION))
