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

#ifndef _LOGBOINC_H_
#define _LOGBOINC_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "LogBOINC.cpp"
#endif


class wxLogBOINC : public wxLogStderr
{
    DECLARE_NO_COPY_CLASS(wxLogBOINC)

public:
    wxLogBOINC();

protected:
    virtual void DoLogString(const wxChar *szString, time_t t);
};


#endif

