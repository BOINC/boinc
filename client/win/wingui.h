// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#ifndef __WINGUI_H_
#define __WINGUI_H_

// includes

#include <afxwin.h>
#include <afxcmn.h>
#include <afxtempl.h>
#include <afxcoll.h>
#include <afxext.h>
#include <math.h>
#include "file_names.h"
#include "filesys.h"
#include "log_flags.h"
#include "client_state.h"
#include "account.h"
#include "error_numbers.h"
#include "resource.h"
#include "util.h"
#include "win_net.h"
#include "win_util.h"

// functions

int add_new_project();
void GetByteString(double nbytes, CString* str);
BOOL RequestNetConnect();
BOOL CALLBACK			EnumWindowsProc(HWND, LPARAM);
CWnd*					GetWndFromProcId(DWORD);

#endif
