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

#ifndef __WINGUI_H_
#define __WINGUI_H_

#include "file_names.h"
#include "filesys.h"
#include "log_flags.h"
#include "client_state.h"
#include "error_numbers.h"
#include "boinc_gui.h"
#include "util.h"
#include "win_net.h"
#include "win_util.h"

// functions

int add_new_project();
BOOL RequestNetConnect();


void GetLanguageFilename(CString& path);
void UpdateLanguageStrings(CWnd* wnd, char const * windowname, int const* pnIDs, CString * const * pStrs = NULL);

#endif
