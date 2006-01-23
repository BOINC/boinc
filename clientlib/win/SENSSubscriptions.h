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

typedef struct _SENS_SUBSCRIPTION
{
    const GUID  *pSubscriptionID;
    LPOLESTR    strSubscriptionName;
    LPOLESTR    strMethodName;
    BOOL        bPublisherPropertyPresent;
    LPOLESTR    strPropertyMethodName;
    LPOLESTR    strPropertyMethodNameValue;

} SENS_SUBSCRIPTION, *PSENS_SUBSCRIPTION;


typedef struct _SENS_SUBSCRIPTION_GROUP
{
    CComBSTR           bstrPublisherID;
    CComBSTR           bstrInterfaceID;
    CComBSTR           bstrEventClassID;
    unsigned int       uiSubscriptionCount;
    PSENS_SUBSCRIPTION pSubscriptions;

} SENS_SUBSCRIPTION_GROUP, *PSENS_SUBSCRIPTION_GROUP;


typedef struct _NETWORK_CONNECTION
{
    CComBSTR            bstrConnection;
    unsigned long       ulType;
    SENS_QOCINFO        QOCInfo;

} NETWORK_CONNECTION, *PNETWORK_CONNECTION;

extern std::vector<PNETWORK_CONNECTION> gpNetworkConnections;

EXTERN_C __declspec(dllexport) BOOL BOINCRegisterSubscriptions(void);
EXTERN_C __declspec(dllexport) BOOL BOINCUnregisterSubscriptions(void);

EXTERN_C __declspec(dllexport) BOOL BOINCIsNetworkActive();
