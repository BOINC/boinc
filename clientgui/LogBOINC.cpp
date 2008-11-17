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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "LogBOINC.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "diagnostics.h"
#include "LogBOINC.h"


wxLogBOINC::wxLogBOINC() {
    m_fp = stdout;
}

void wxLogBOINC::DoLogString(const wxChar *szString, time_t t) {
    diagnostics_cycle_logs();
#ifdef __WXMSW__
    wxString strDebug = szString;
    strDebug += wxT("\r\n");
    ::OutputDebugString(strDebug.c_str());
#endif
    wxLogStderr::DoLogString(szString, t);
}

const char *BOINC_RCSID_4f7bf42814="$Id: LogBOINC.cpp 13804 2007-10-09 11:35:47Z fthomas $";
