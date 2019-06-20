// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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

#define _VIRTUALBOX60_
#define _VIRTUALBOX_IMPORT_FUNCTIONS_

#include "boinc_win.h"
#include "atlcomcli.h"
#include "atlsafe.h"
#include "atlcoll.h"
#include "atlstr.h"
#include "win_util.h"
#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "network.h"
#include "boinc_api.h"
#include "floppyio.h"
#include "vboxlogging.h"
#include "vboxwrapper.h"
#include "vbox_mscom60.h"


#import "file:vbox60.tlb" rename_namespace("vbox60"), named_guids, raw_interfaces_only

using std::string;
using namespace vbox60;

namespace vbox60 {

    class VBOX_PRIV {
    public:
        VBOX_PRIV() {};
        ~VBOX_PRIV() {};

        IVirtualBoxPtr m_pVirtualBox;
        ISessionPtr m_pSession;
        IMachinePtr m_pMachine;
    };

#include "vbox_mscom_impl.cpp"

}


