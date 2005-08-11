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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ValidateURL.h"
#endif

#include "stdwx.h"
#include "ValidateURL.h"


IMPLEMENT_DYNAMIC_CLASS(CValidateURL, wxValidator)


CValidateURL::CValidateURL(wxString *val) {
    m_stringValue = val ;
}


CValidateURL::CValidateURL(const CValidateURL& val) : wxValidator() {
    Copy(val);
}


CValidateURL::~CValidateURL() {}


bool CValidateURL::Copy(const CValidateURL& val) {
    wxValidator::Copy(val);

    m_stringValue = val.m_stringValue ;

    return TRUE;
}


bool CValidateURL::Validate(wxWindow *parent) {
    if(!CheckValidator())
        return FALSE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;

    if (!control->IsEnabled())
        return TRUE;

    bool ok = TRUE;
    wxURL val(control->GetValue());
    wxString str(control->GetValue());

    if (str.Length() == 0) {
        ok = FALSE;
        m_errormsg = _("Please specify a URL to continue.\nAn example would be:\nhttp://boinc.berkeley.edu/");
    } else if (wxURL_NOERR != val.GetError()) {
        ok = FALSE;

        if ((wxURL_NOPROTO == val.GetError()) || wxURL_SNTXERR == val.GetError()) {
            // Special case: we want to allow the user to specify the URL without
            //   specifing the protocol.
            ok = TRUE;
        }
        else if (wxURL_NOHOST == val.GetError())
            m_errormsg = _("'%s' does not contain a valid host name.");
        else if (wxURL_NOPATH == val.GetError())
            m_errormsg = _("'%s' does not contain a valid path.");
    }

    if (!ok) {
        wxASSERT_MSG(!m_errormsg.empty(), _T("you forgot to set errormsg"));

        m_validatorWindow->SetFocus();

        wxString buf;
        buf.Printf(m_errormsg, control->GetValue().c_str());

        wxMessageBox(buf, _("Validation conflict"),
            wxOK | wxICON_EXCLAMATION, parent
        );
    }

    return ok;
}


bool CValidateURL::TransferToWindow() {
    if(!CheckValidator())
        return FALSE;
    
    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    control->SetValue(* m_stringValue) ;

    return TRUE;
}


bool CValidateURL::TransferFromWindow() {
    if(!CheckValidator())
        return FALSE;

    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    *m_stringValue = control->GetValue() ;

    return TRUE;
}


bool CValidateURL::CheckValidator() const {
    wxCHECK_MSG(m_validatorWindow, FALSE,
        _T("No window associated with validator")
    );
    wxCHECK_MSG(m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), FALSE,
        _T("wxTextValidator is only for wxTextCtrl's")
    );
    wxCHECK_MSG(m_stringValue, FALSE,
        _T("No variable storage for validator")
    );
    return TRUE;
}


const char *BOINC_RCSID_1f1a9f5f09 = "$Id$";
