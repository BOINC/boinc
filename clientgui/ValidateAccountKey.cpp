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
// Revision 1.7  2005/01/02 18:29:20  ballen
// Modified CVS id strings.  After some fussing with different versions
// of gcc to try and force them to not complain with -Wall but to always
// include this, I decided to take a simpler approach.  All these strings
// now have global linkage.  To prevent namespace conflicts they all
// have different names.  For the record, the variable extension is a hash made of the first ten characters of the md5sum of the file path, eg:
//     md5hash=`boinc/api/x_opengl.C | md5sum | cut -c 1-10`
//
// Revision 1.6  2004/12/08 00:39:12  ballen
// Moved RCSID strings to the end of all .c, .C and .cpp files as per
// David's request.
//
// Revision 1.5  2004/12/02 20:17:34  rwalton
// *** empty log message ***
//
// Revision 1.4  2004/11/22 19:17:06  davea
// *** empty log message ***
//
// Revision 1.3  2004/10/26 16:58:34  rwalton
// *** empty log message ***
//
// Revision 1.2  2004/10/22 16:06:10  rwalton
// *** empty log message ***
//
// Revision 1.1  2004/09/22 21:53:04  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ValidateAccountKey.h"
#endif

#include "stdwx.h"
#include "ValidateAccountKey.h"


IMPLEMENT_DYNAMIC_CLASS(CValidateAccountKey, wxValidator)


CValidateAccountKey::CValidateAccountKey(wxString *val)
{
    m_stringValue = val ;
}


CValidateAccountKey::CValidateAccountKey(const CValidateAccountKey& val)
    : wxValidator()
{
    Copy(val);
}


CValidateAccountKey::~CValidateAccountKey()
{
}


bool CValidateAccountKey::Copy(const CValidateAccountKey& val)
{
    wxValidator::Copy(val);

    m_stringValue = val.m_stringValue ;

    return TRUE;
}


bool CValidateAccountKey::Validate(wxWindow *parent)
{
    if( !CheckValidator() )
        return FALSE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;

    if ( !control->IsEnabled() )
        return TRUE;

    wxString val(control->GetValue());

    bool ok = TRUE;

    if ( (!wxIsAlphaNumeric(val)) )
    {
        ok = FALSE;

        m_errormsg = _("Invalid Account Key; please enter a valid Account Key");
    }

    if ( !ok )
    {
        wxASSERT_MSG( !m_errormsg.empty(), _T("you forgot to set errormsg") );

        m_validatorWindow->SetFocus();

        wxString buf;
        buf.Printf(m_errormsg, control->GetValue().c_str());

        wxMessageBox(buf, _("Validation conflict"),
                     wxOK | wxICON_EXCLAMATION, parent);
    }

    return ok;
}


bool CValidateAccountKey::TransferToWindow(void)
{
    if( !CheckValidator() )
        return FALSE;
    
    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    control->SetValue(* m_stringValue) ;

    return TRUE;
}


bool CValidateAccountKey::TransferFromWindow(void)
{
    if( !CheckValidator() )
        return FALSE;

    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    * m_stringValue = control->GetValue() ;

    return TRUE;
}


bool CValidateAccountKey::wxIsAlphaNumeric(const wxString& val)
{
    int i;
    for ( i = 0; i < (int)val.Length(); i++)
    {
        if (!wxIsalnum(val[i]))
            return FALSE;
    }
    return TRUE;
}


bool CValidateAccountKey::CheckValidator() const
{
    wxCHECK_MSG( m_validatorWindow, FALSE,
                    _T("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), FALSE,
                    _T("wxTextValidator is only for wxTextCtrl's") );
    wxCHECK_MSG( m_stringValue, FALSE,
                    _T("No variable storage for validator") );

    return TRUE;
}


const char *BOINC_RCSID_0c2c4c6b07 = "$Id$";
