// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

