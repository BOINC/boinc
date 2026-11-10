// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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
//

#ifndef BOINC_PROXYPAGE_H
#define BOINC_PROXYPAGE_H

class CErrProxyPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CErrProxyPage)

public:
    CErrProxyPage() = default;
    CErrProxyPage(CWizardAttach* parent);
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnPageChanged(wxWizardEvent& event);
    void OnPageChanging(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

    wxString GetProxyHTTPServer() const { return m_strProxyHTTPServer ; }
    void SetProxyHTTPServer(wxString value) { m_strProxyHTTPServer = value ; }

    wxString GetProxyHTTPPort() const { return m_strProxyHTTPPort ; }
    void SetProxyHTTPPort(wxString value) { m_strProxyHTTPPort = value ; }

    wxString GetProxyHTTPUsername() const { return m_strProxyHTTPUsername ; }
    void SetProxyHTTPUsername(wxString value) { m_strProxyHTTPUsername = value ; }

    wxString GetProxyHTTPPassword() const { return m_strProxyHTTPPassword ; }
    void SetProxyHTTPPassword(wxString value) { m_strProxyHTTPPassword = value ; }

    wxString GetProxySOCKSServer() const { return m_strProxySOCKSServer ; }
    void SetProxySOCKSServer(wxString value) { m_strProxySOCKSServer = value ; }

    wxString GetProxySOCKSPort() const { return m_strProxySOCKSPort ; }
    void SetProxySOCKSPort(wxString value) { m_strProxySOCKSPort = value ; }

    wxString GetProxySOCKSUsername() const { return m_strProxySOCKSUsername ; }
    void SetProxySOCKSUsername(wxString value) { m_strProxySOCKSUsername = value ; }

    wxString GetProxySOCKSPassword() const { return m_strProxySOCKSPassword ; }
    void SetProxySOCKSPassword(wxString value) { m_strProxySOCKSPassword = value ; }

private:
    wxStaticText* m_pTitleStaticCtrl = nullptr;
    wxStaticBox* m_pProxyHTTPDescriptionCtrl = nullptr;
    wxStaticText* m_pProxyHTTPServerStaticCtrl = nullptr;
    wxTextCtrl* m_pProxyHTTPServerCtrl = nullptr;
    wxStaticText* m_pProxyHTTPPortStaticCtrl = nullptr;
    wxTextCtrl* m_pProxyHTTPPortCtrl = nullptr;
    wxStaticText* m_pProxyHTTPUsernameStaticCtrl = nullptr;
    wxTextCtrl* m_pProxyHTTPUsernameCtrl = nullptr;
    wxStaticText* m_pProxyHTTPPasswordStaticCtrl = nullptr;
    wxTextCtrl* m_pProxyHTTPPasswordCtrl = nullptr;
    wxStaticBox* m_pProxySOCKSDescriptionCtrl = nullptr;
    wxStaticText* m_pProxySOCKSServerStaticCtrl = nullptr;
    wxTextCtrl* m_pProxySOCKSServerCtrl = nullptr;
    wxStaticText* m_pProxySOCKSPortStaticCtrl = nullptr;
    wxTextCtrl* m_pProxySOCKSPortCtrl = nullptr;
    wxStaticText* m_pProxySOCKSUsernameStaticCtrl = nullptr;
    wxTextCtrl* m_pProxySOCKSUsernameCtrl = nullptr;
    wxStaticText* m_pProxySOCKSPasswordStaticCtrl = nullptr;
    wxTextCtrl* m_pProxySOCKSPasswordCtrl = nullptr;
    wxString m_strProxyHTTPServer;
    wxString m_strProxyHTTPPort;
    wxString m_strProxyHTTPUsername;
    wxString m_strProxyHTTPPassword;
    wxString m_strProxySOCKSServer;
    wxString m_strProxySOCKSPort;
    wxString m_strProxySOCKSUsername;
    wxString m_strProxySOCKSPassword;

    CWizardAttach *m_pParent = nullptr;
    CBOINCWizardPage *m_pPrev = nullptr;
};

#endif
