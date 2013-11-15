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
#pragma implementation "ValidateURL.h"
#endif

#include "stdwx.h"
#include "ValidateURL.h"
#include "BOINCGUIApp.h"
#include "url.h"


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
    std::string canonicalize_url;

    canonicalize_url = control->GetValue().Trim().Trim(false).mb_str();  // trim spaces before and after

    if (canonicalize_url.size() == 0) {
        ok = FALSE;
        m_errortitle = _("Missing URL");
        m_errormsg = _("Please specify a URL.\nFor example:\nhttp://www.example.com/");
    } else {
        canonicalize_master_url(canonicalize_url);
        wxURI uri(wxString(canonicalize_url.c_str(), wxConvUTF8));
        wxURL url(wxString(canonicalize_url.c_str(), wxConvUTF8));
        wxString strURL(canonicalize_url.c_str(), wxConvUTF8);
        wxString strServer(uri.GetServer());
        int iServerDotLocation = strServer.Find(wxT("."));
        int iFirstPart = (int)strServer.Mid(0, iServerDotLocation).Length();
        int iSecondPart = (int)strServer.Mid(iServerDotLocation + 1).Length();

        if (-1 == iServerDotLocation) {
            ok = FALSE;
            m_errortitle = _("Invalid URL");
            m_errormsg = _("Please specify a valid URL.\nFor example:\nhttp://boincproject.example.com");
        } else if (0 == iFirstPart) {
            ok = FALSE;
            m_errortitle = _("Invalid URL");
            m_errormsg = _("Please specify a valid URL.\nFor example:\nhttp://boincproject.example.com");
        } else if (0 == iSecondPart) {
            ok = FALSE;
            m_errortitle = _("Invalid URL");
            m_errormsg = _("Please specify a valid URL.\nFor example:\nhttp://boincproject.example.com");
        } else if (wxURL_NOERR != url.GetError()) {
            ok = FALSE;

            if ((wxURL_NOPROTO == url.GetError()) || wxURL_SNTXERR == url.GetError()) {
                // Special case: we want to allow the user to specify the URL without
                //   specifing the protocol.
                wxURL urlNoProtoSpecified(wxT("http://") + strURL);
                if (wxURL_NOERR == urlNoProtoSpecified.GetError()) {
                    ok = TRUE;
                } else {
                    m_errortitle = _("Invalid URL");
                    m_errormsg = _("'%s' does not contain a valid host name.");
                }
            } else if (wxURL_NOHOST == url.GetError()) {
                m_errortitle = _("Invalid URL");
                m_errormsg = _("'%s' does not contain a valid host name.");
            } else if (wxURL_NOPATH == url.GetError()) {
                m_errortitle = _("Invalid URL");
                m_errormsg = _("'%s' does not contain a valid path.");
            }
        }
    }

    if (!ok) {
        wxASSERT_MSG(!m_errortitle.empty(), wxT("you forgot to set errortitle"));
        wxASSERT_MSG(!m_errormsg.empty(), wxT("you forgot to set errormsg"));

        m_validatorWindow->SetFocus();

        wxString buf;
        buf.Printf(m_errormsg, control->GetValue().c_str());

        wxGetApp().SafeMessageBox(buf, m_errortitle,
            wxOK | wxICON_EXCLAMATION, parent
        );
    }

    if (ok) {
        control->SetValue(wxString(canonicalize_url.c_str(), wxConvUTF8));
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

