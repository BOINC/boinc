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

#ifndef _VALIDATEACCOUNTKEY_H_
#define _VALIDATEACCOUNTKEY_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ValidateAccountKey.cpp"
#endif


class CValidateAccountKey : public wxValidator
{
    DECLARE_DYNAMIC_CLASS( CValidateAccountKey )

public:

    CValidateAccountKey( wxString *val = 0 );
    CValidateAccountKey( const CValidateAccountKey& val );

    ~CValidateAccountKey();

    virtual wxObject* Clone() const { return new CValidateAccountKey(*this); }
    virtual bool      Copy( const CValidateAccountKey& val );

    virtual bool      Validate(wxWindow *parent);
    virtual bool      TransferToWindow();
    virtual bool      TransferFromWindow();

protected:
    wxString*         m_stringValue;
    wxString          m_errormsg;

    bool              wxIsAlphaNumeric(const wxString& val);
    virtual bool      CheckValidator() const;

};


#endif

