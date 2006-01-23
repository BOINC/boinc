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
#include "resource.h"       // main symbols

#ifndef MIDL_DEFINE_GUID
#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#endif

#ifndef IID_IBOINCSENSSink
MIDL_DEFINE_GUID(IID, IID_IBOINCSENSSink,0x30DFEB87,0xFDFB,0x48BB,0xA9,0x02,0xDB,0x4F,0x4C,0x13,0xE0,0x87);
#endif

#ifndef CLSID_CBOINCSENSSink
MIDL_DEFINE_GUID(CLSID, CLSID_CBOINCSENSSink,0x33955752,0x24F7,0x4DA4,0x81,0xC9,0xE5,0xEA,0x66,0x94,0xA5,0x74);
#endif

// IBOINCSENSSink
[
	object,
	uuid("30DFEB87-FDFB-48BB-A902-DB4F4C13E087"),
	dual,	
    helpstring("IBOINCSENSSink Interface"),
	pointer_default(unique)
]
__interface IBOINCSENSSink : IDispatch
{
};



// CBOINCSENSSink

[
    coclass,
    threading("free"),
    vi_progid("BOINCSENS.BOINCSENSSink"),
    progid("BOINCSENS.BOINCSENSSink.1"),
    version(1.0),
    uuid("33955752-24F7-4DA4-81C9-E5EA6694A574"),
    helpstring("BOINCSENSSink Class")
]
class ATL_NO_VTABLE CBOINCSENSSink : 
    public IBOINCSENSSink,
    public IDispatchImpl<ISensNetwork, &__uuidof(ISensNetwork), &LIBID_SensEvents, /* wMajor = */ 2, /* wMinor = */ 0>,
    public IDispatchImpl<ISensLogon, &__uuidof(ISensLogon), &LIBID_SensEvents, /* wMajor = */ 2, /* wMinor = */ 0>,
    public IDispatchImpl<ISensOnNow, &__uuidof(ISensOnNow), &LIBID_SensEvents, /* wMajor = */ 2, /* wMinor = */ 0>
{
public:
    CBOINCSENSSink();
    HRESULT FinalConstruct();
    void FinalRelease();

    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:

    // ISensNetwork Methods
public:
    STDMETHOD(ConnectionMade)(BSTR bstrConnection, unsigned long ulType, SENS_QOCINFO * lpQOCInfo);
    STDMETHOD(ConnectionMadeNoQOCInfo)(BSTR bstrConnection, unsigned long ulType);
    STDMETHOD(ConnectionLost)(BSTR bstrConnection, unsigned long ulType);
    STDMETHOD(DestinationReachable)(BSTR bstrDestination, BSTR bstrConnection, unsigned long ulType, SENS_QOCINFO * lpQOCInfo);
    STDMETHOD(DestinationReachableNoQOCInfo)(BSTR bstrDestination, BSTR bstrConnection, unsigned long ulType);

    // ISensOnNow Methods
public:
    STDMETHOD(OnACPower)();
    STDMETHOD(OnBatteryPower)(unsigned long dwBatteryLifePercent);
    STDMETHOD(BatteryLow)(unsigned long dwBatteryLifePercent);

    // ISensLogon Methods
public:
    STDMETHOD(Logon)(BSTR bstrUserName);
    STDMETHOD(Logoff)(BSTR bstrUserName);
    STDMETHOD(StartShell)(BSTR bstrUserName);
    STDMETHOD(DisplayLock)(BSTR bstrUserName);
    STDMETHOD(DisplayUnlock)(BSTR bstrUserName);
    STDMETHOD(StartScreenSaver)(BSTR bstrUserName);
    STDMETHOD(StopScreenSaver)(BSTR bstrUserName);
};

