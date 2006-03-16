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


#include "stdafx.h"
#include "BOINCSENSSink.h"
#include "SENSSubscriptions.h"
#include "SENSNetworkSubscriptions.h"
#include "SENSOnNowSubscriptions.h"
#include "SENSLogonSubscriptions.h"


IEventSystem*                       gpIEventSystem = NULL;

SENS_SUBSCRIPTION_GROUP             gSubscriptionGroups[3];
unsigned int                        giSubscriptionGroupCount = 0;

std::vector<PNETWORK_CONNECTION>    gpNetworkConnections;


EXTERN_C __declspec(dllexport) BOOL BOINCIsNetworkAlive( LPDWORD lpdwFlags )
{
    unsigned int i = 0;
    BOOL bReturnValue = FALSE;
    BOOL bCachePopulated = FALSE;
    PNETWORK_CONNECTION pNetworkConnection = NULL;

    // Check cached connection state
    if (!gpNetworkConnections.empty()) {
        bCachePopulated = TRUE;
        for (i=0; i<gpNetworkConnections.size(); i++) {
            pNetworkConnection = gpNetworkConnections[i];
            if (pNetworkConnection->ulType & *lpdwFlags) {
                bReturnValue = TRUE;
            }
        }
    }

    // If we do not have any cached information, then fall back to other
    //   methods
    if (!bCachePopulated) {
        bReturnValue = IsNetworkAlive( lpdwFlags );
        if (!bReturnValue) {
            DWORD current_flags = NULL;
            DWORD desired_flags = NULL;

            if (NETWORK_ALIVE_LAN & *lpdwFlags)
                desired_flags |= INTERNET_CONNECTION_LAN;

            if (NETWORK_ALIVE_WAN & *lpdwFlags)
                desired_flags |= INTERNET_CONNECTION_MODEM;

            // TODO: Find out if AOL is registered as a LAN or WAN connection.
            //   Until then, assume both are okay.
            if (NETWORK_ALIVE_AOL & *lpdwFlags)
                desired_flags |= INTERNET_CONNECTION_LAN | INTERNET_CONNECTION_MODEM;

            BOOL retval = InternetGetConnectedState(&current_flags, 0);
            if (retval && (current_flags & desired_flags)) {
                bReturnValue = TRUE;
            } else {
                bReturnValue = FALSE;
            }
        }
    }

    return bReturnValue;
}


EXTERN_C __declspec(dllexport) BOOL BOINCIsNetworkAlwaysOnline()
{
    DWORD dwFlags = NETWORK_ALIVE_LAN;
    return BOINCIsNetworkAlive(&dwFlags);
}


BOOL NetworkTrackerStartup()
{
    USES_CONVERSION;
    unsigned int                i = 0, j = 0;
	HRESULT                     hr = 0;
    IBOINCSENSSink*             pIBOINCSENSSink = NULL;

    CComBSTR                    bstrPROGID_EventSubscription;
	IEventSubscription*         pIEventSubscription = NULL;

    PSENS_SUBSCRIPTION_GROUP    pSubscriptionGroup = NULL;
    PSENS_SUBSCRIPTION          pSubscription = NULL;

    CComBSTR                    bstrSubscriberCLSID;
	CComBSTR                    bstrSubscriptionID;
	CComBSTR                    bstrSubscriptionName;
	CComBSTR                    bstrMethodName;


    // Assign the correct program is value to be used by the event
    //   registration store.
	bstrPROGID_EventSubscription = PROGID_EventSubscription;


    // Clear the cache
    gpNetworkConnections.clear();


    // Try and create some important references to COM objects before
    //   doing anything else.
    //

    // IEventSystem
	hr = CoCreateInstance(
                            CLSID_CEventSystem,
							NULL,
							CLSCTX_SERVER,
							IID_IEventSystem,
							(LPVOID*)&gpIEventSystem);
	if (FAILED(hr))
    {
		ATLTRACE(TEXT("Error creating event system! (0x%x)"), hr);
		return FALSE;
    }
    
    // IBOINCSENSSink
    hr = CoCreateInstance(
							CLSID_CBOINCSENSSink,
							NULL,
							CLSCTX_SERVER,
							IID_IBOINCSENSSink,
							(LPVOID*)&pIBOINCSENSSink);
	if (FAILED(hr))
	{
		ATLTRACE(TEXT("Failed to create pIBOINCSENSSink (0x%x)"), hr);
		return FALSE;
	}


    // Prepare for registration by pre-populating the registration
    //   multidimensional array with the known good stuff.

    // Prepare the SENS Network registration group.
    gSubscriptionGroups[0].bstrPublisherID = SENSGUID_PUBLISHER;
    gSubscriptionGroups[0].bstrEventClassID = SENSGUID_EVENTCLASS_NETWORK;
    gSubscriptionGroups[0].bstrInterfaceID = IID_ISensNetwork;
    gSubscriptionGroups[0].uiSubscriptionCount = SENS_NETWORK_SUBSCRIPTIONS_COUNT;
    gSubscriptionGroups[0].pSubscriptions = (PSENS_SUBSCRIPTION)&gSENSNetworkSubscriptions;

    // Prepare the SENS OnNow registration group.
    gSubscriptionGroups[1].bstrPublisherID = SENSGUID_PUBLISHER;
    gSubscriptionGroups[1].bstrEventClassID = SENSGUID_EVENTCLASS_ONNOW;
    gSubscriptionGroups[1].bstrInterfaceID = IID_ISensOnNow;
    gSubscriptionGroups[1].uiSubscriptionCount = SENS_ONNOW_SUBSCRIPTIONS_COUNT;
    gSubscriptionGroups[1].pSubscriptions = (PSENS_SUBSCRIPTION)&gSENSOnNowSubscriptions;

    // Prepare the SENS Logon registration group.
    gSubscriptionGroups[2].bstrPublisherID = SENSGUID_PUBLISHER;
    gSubscriptionGroups[2].bstrEventClassID = SENSGUID_EVENTCLASS_LOGON;
    gSubscriptionGroups[2].bstrInterfaceID = IID_ISensLogon;
    gSubscriptionGroups[2].uiSubscriptionCount = SENS_LOGON_SUBSCRIPTIONS_COUNT;
    gSubscriptionGroups[2].pSubscriptions = (PSENS_SUBSCRIPTION)&gSENSLogonSubscriptions;


    // Start to register the events by walking through the registration groups
    //
    giSubscriptionGroupCount = 
        sizeof(gSubscriptionGroups) / sizeof(SENS_SUBSCRIPTION_GROUP);

    for (i = 0; i < giSubscriptionGroupCount; i++)
    {
        pSubscriptionGroup = &gSubscriptionGroups[i];
        for (j = 0; j < pSubscriptionGroup->uiSubscriptionCount; j++)
        {
            pSubscription = &pSubscriptionGroup->pSubscriptions[j];

		    // Get a new IEventSubscription object.
		    hr = CoCreateInstance(
								    CLSID_CEventSubscription,
								    NULL,
								    CLSCTX_SERVER,
								    IID_IEventSubscription,
								    (LPVOID*) &pIEventSubscription);

		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("Error getting IEventSubscription object (0x%x)"), hr);
			    return FALSE;
		    }

		    bstrSubscriptionID = *pSubscription->pSubscriptionID;
		    hr = pIEventSubscription->put_SubscriptionID(bstrSubscriptionID);
		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("pIEventSubscription->put_SubscriptionID (0x%x)"), hr);
			    return FALSE;
		    }

		    bstrSubscriptionName = pSubscription->strSubscriptionName;
		    hr = pIEventSubscription->put_SubscriptionName(bstrSubscriptionName);
		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("pIEventSubscription->put_SubscriptionName (0x%x)"), hr);
			    return FALSE;
		    }

		    bstrMethodName = pSubscription->strMethodName;
		    hr = pIEventSubscription->put_MethodName(bstrMethodName);
		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("pIEventSubscription->put_MethodName (0x%x)"), hr);
			    return FALSE;
		    }

		    hr = pIEventSubscription->put_EventClassID(pSubscriptionGroup->bstrEventClassID);
		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("pIEventSubscription->put_EventClassID (0x%x)"), hr);
			    return FALSE;
		    }

		    hr = pIEventSubscription->put_SubscriberInterface(pIBOINCSENSSink);
		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("pIEventSubscription->put_SubscriberInterface (0x%x)"), hr);
			    return FALSE;
		    }

		    // Initialize it.
		    hr = gpIEventSystem->Store(
                                         bstrPROGID_EventSubscription, 
                                         pIEventSubscription);
		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("gpIEventSystem->Store (0x%x)"), hr);
			    return FALSE;
		    }
            else
            {
                ATLTRACE(TEXT("Subscription Sucess: %s\n"), COLE2T(pSubscription->strMethodName));
            }

		    pIEventSubscription->Release();
		    pIEventSubscription = NULL;
        }
    }

	if (pIEventSubscription != NULL)
		pIEventSubscription->Release();

	if (pIBOINCSENSSink != NULL)
		pIBOINCSENSSink->Release();

    return TRUE;
}


void NetworkTrackerShutdown()
{
    USES_CONVERSION;
    unsigned int                i = 0, j = 0;
	HRESULT                     hr = 0;
	int                         errorIndex = 0;
	CComBSTR                    bstrQuery;
    PSENS_SUBSCRIPTION_GROUP    pSubscriptionGroup = NULL;
    PSENS_SUBSCRIPTION          pSubscription = NULL;
    PNETWORK_CONNECTION         pNetworkConnection = NULL;

	for (i = 0; i < giSubscriptionGroupCount; i++)
	{
        pSubscriptionGroup = &gSubscriptionGroups[i];
        for (j = 0; j < pSubscriptionGroup->uiSubscriptionCount; j++)
        {
            pSubscription = &pSubscriptionGroup->pSubscriptions[j];

            bstrQuery.Empty();
		    bstrQuery = TEXT("SubscriptionID=");
		    bstrQuery += *pSubscription->pSubscriptionID;

		    hr = gpIEventSystem->Remove(
									    PROGID_EventSubscription,
									    bstrQuery,
									    &errorIndex);
		    if (FAILED(hr))
		    {
			    ATLTRACE(TEXT("gpIEventSystem->Remove (0x%x)"), hr);
		    }
        }
	}


    // Clear the cache
    for (i=0; i<gpNetworkConnections.size(); i++) {
        delete gpNetworkConnections[i];
    }
    gpNetworkConnections.clear();
}
