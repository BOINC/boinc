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

#ifndef BOINC_VALIDATEURL_H
#define BOINC_VALIDATEURL_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ValidateURL.cpp"
#endif


class CValidateURL : public wxValidator {
    DECLARE_DYNAMIC_CLASS( CValidateURL )

public:

    CValidateURL( wxString *val = 0 );
    CValidateURL( const CValidateURL& val );

    ~CValidateURL();

    virtual wxObject* Clone() const { return new CValidateURL(*this); }
    virtual bool      Copy( const CValidateURL& val );

    virtual bool      Validate(wxWindow *parent);
    virtual bool      TransferToWindow();
    virtual bool      TransferFromWindow();

protected:
    wxString*         m_stringValue;
    wxString          m_errortitle;
    wxString          m_errormsg;

    virtual bool      CheckValidator() const;

};

#endif

