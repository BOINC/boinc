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
#pragma implementation "ValidateAccountKey.h"
#endif

#include "stdwx.h"
#include "ValidateAccountKey.h"
#include "BOINCGUIApp.h"


IMPLEMENT_DYNAMIC_CLASS(CValidateAccountKey, wxValidator)


CValidateAccountKey::CValidateAccountKey(wxString *val) {
    m_stringValue = val ;
}


CValidateAccountKey::CValidateAccountKey(const CValidateAccountKey& val)
    : wxValidator()
{
    Copy(val);
}


CValidateAccountKey::~CValidateAccountKey() {}


bool CValidateAccountKey::Copy(const CValidateAccountKey& val) {
    wxValidator::Copy(val);

    m_stringValue = val.m_stringValue ;

    return TRUE;
}


bool CValidateAccountKey::Validate(wxWindow *parent) {
    if(!CheckValidator())
        return FALSE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;

    if (!control->IsEnabled())
        return TRUE;

    bool ok = TRUE;
    wxString val(control->GetValue().Trim().Trim(false));  // trim spaces before and after

    if (val.Length() == 0) {
        ok = FALSE;
        m_errormsg = _("Please specify an account key to continue.");
    } else if ((!wxIsAlphaNumeric(val))) {
        ok = FALSE;
        m_errormsg = _("Invalid Account Key; please enter a valid Account Key");
    }

    if (!ok) {
        wxASSERT_MSG(!m_errormsg.empty(), _T("you forgot to set errormsg"));

        m_validatorWindow->SetFocus();

        wxString buf;
        buf.Printf(m_errormsg, control->GetValue().c_str());

        wxGetApp().SafeMessageBox(buf, _("Validation conflict"),
            wxOK | wxICON_EXCLAMATION, parent
        );
    }

    return ok;
}


bool CValidateAccountKey::TransferToWindow(void) {
    if(!CheckValidator())
        return FALSE;

    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    control->SetValue(* m_stringValue) ;

    return TRUE;
}


bool CValidateAccountKey::TransferFromWindow(void) {
    if(!CheckValidator())
        return FALSE;

    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    * m_stringValue = control->GetValue() ;

    return TRUE;
}


bool CValidateAccountKey::wxIsAlphaNumeric(const wxString& val) {
    int i;
    for (i = 0; i < (int)val.Length(); i++) {
        if (!wxIsalnum(val[i]))
            return FALSE;
    }
    return TRUE;
}


bool CValidateAccountKey::CheckValidator() const {
    wxCHECK_MSG(m_validatorWindow, FALSE,
                    _T("No window associated with validator"));
    wxCHECK_MSG(m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), FALSE,
                    _T("wxTextValidator is only for wxTextCtrl's"));
    wxCHECK_MSG(m_stringValue, FALSE,
                    _T("No variable storage for validator"));

    return TRUE;
}

