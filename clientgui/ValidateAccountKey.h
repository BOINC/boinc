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

