// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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
// Revision History:
//
// $Log$
// Revision 1.1  2004/09/22 21:53:05  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ValidateAccountKey.h"
#endif

#include "stdwx.h"
#include "ValidateURL.h"


IMPLEMENT_DYNAMIC_CLASS(CValidateURL, wxValidator)


CValidateURL::CValidateURL(wxString *val)
{
    m_stringValue = val ;
}


CValidateURL::CValidateURL(const CValidateURL& val)
    : wxValidator()
{
    Copy(val);
}


CValidateURL::~CValidateURL()
{
}


bool CValidateURL::Copy(const CValidateURL& val)
{
    wxValidator::Copy(val);

    m_stringValue = val.m_stringValue ;

    return TRUE;
}


bool CValidateURL::Validate(wxWindow *parent)
{
    if( !CheckValidator() )
        return FALSE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;

    if ( !control->IsEnabled() )
        return TRUE;

    wxURL val(control->GetValue());

    bool ok = TRUE;

    if ( wxURL_NOERR != val.GetError() )
    {
        ok = FALSE;

        if      ( wxURL_SNTXERR == val.GetError() )
            m_errormsg = _("'%s' contains a syntax error.");
        else if ( wxURL_NOPROTO == val.GetError() )
            m_errormsg = _("'%s' does not contain a protocol which can get this URL.");
        else if ( wxURL_NOHOST == val.GetError() )
            m_errormsg = _("'%s' does not contain a host name.");
        else if ( wxURL_NOPATH == val.GetError() )
            m_errormsg = _("'%s' does not contain a valid path.");
    }

    if ( !ok )
    {
        wxASSERT_MSG( !m_errormsg.empty(), _T("you forgot to set errormsg") );

        m_validatorWindow->SetFocus();

        wxString buf;
        buf.Printf(m_errormsg, control->GetValue());

        wxMessageBox(buf, _("Validation conflict"),
                     wxOK | wxICON_EXCLAMATION, parent);
    }

    return ok;
}


bool CValidateURL::TransferToWindow(void)
{
    if( !CheckValidator() )
        return FALSE;
    
    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    control->SetValue(* m_stringValue) ;

    return TRUE;
}


bool CValidateURL::TransferFromWindow(void)
{
    if( !CheckValidator() )
        return FALSE;

    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    * m_stringValue = control->GetValue() ;

    return TRUE;
}


bool CValidateURL::CheckValidator() const
{
    wxCHECK_MSG( m_validatorWindow, FALSE,
                    _T("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), FALSE,
                    _T("wxTextValidator is only for wxTextCtrl's") );
    wxCHECK_MSG( m_stringValue, FALSE,
                    _T("No variable storage for validator") );

    return TRUE;
}

