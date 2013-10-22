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

#ifndef _SKINMANAGER_H_
#define _SKINMANAGER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "SkinManager.cpp"
#endif


class CSkinItem : public wxObject
{
    DECLARE_DYNAMIC_CLASS( CSkinItem )

public:
    CSkinItem();
    ~CSkinItem();

    static wxColour ParseColor(wxString strColor);
};


class CSkinImage : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinImage )

public:
    CSkinImage();
    ~CSkinImage();

    void Clear();
    int  Parse(MIOFILE& in);

    wxBitmap* GetBitmap();
    wxColour* GetBackgroundColor();

    bool SetDefaults(
        wxString strComponentName, 
        const char** ppDefaultImage
    );

    bool SetDefaults(
        wxString strComponentName,
        const char** ppDefaultImage,
        wxString strBackgroundColor
    );

    bool Validate();

private:
    wxString     m_strComponentName;
    wxString     m_strDesiredBitmap;
    wxString     m_strDesiredBackgroundColor;
    const char** m_ppDefaultBitmap;
    wxString     m_strDefaultBackgroundColor;
    wxBitmap     m_bmpBitmap;
    wxColour     m_colBackgroundColor;
};


class CSkinIcon : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinIcon )

public:
    CSkinIcon();
    ~CSkinIcon();

    void Clear();
    int  Parse(MIOFILE& in);

    wxIcon* GetIcon();

    bool SetDefaults(
        wxString strComponentName,
        const char** ppDefaultIcon
    );

    bool Validate();

private:
    wxString     m_strComponentName;
    wxString     m_strDesiredIcon;
    wxString     m_strDesiredTransparencyMask;
    const char** m_ppDefaultIcon;
    wxIcon       m_icoIcon;
};


// Default opacity for Simple View white panels 
// is 60% (153 on a scale of 0 - 255).
#define MAX_OPACITY 255
#define DEFAULT_OPACITY 153

class CSkinSimple : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinSimple )

public:
    CSkinSimple();
    ~CSkinSimple();

    void Clear();
    int  Parse(MIOFILE& in);

    bool InitializeDelayedValidation();

    CSkinImage*         GetBackgroundImage() { return &m_BackgroundImage; }
    CSkinImage*         GetDialogBackgroundImage() { return &m_DialogBackgroundImage; }
    CSkinImage*         GetProjectImage() { return &m_ProjectImage; }
    wxColour            GetStaticLineColor() { return m_StaticLineColor; }
    wxColour            GetNoticeAlertColor() { return m_NoticeAlertColor; }
    CSkinImage*         GetWorkunitAnimationImage() { return &m_WorkunitAnimationImage; }
    CSkinImage*         GetWorkunitRunningImage() { return &m_WorkunitRunningImage; }
    CSkinImage*         GetWorkunitSuspendedImage() { return &m_WorkunitSuspendedImage; }
    CSkinImage*         GetWorkunitWaitingImage() { return &m_WorkunitWaitingImage; }
    int                 GetPanelOpacity() { return m_iPanelOpacity; }

private:
	CSkinImage          m_BackgroundImage;
    CSkinImage          m_DialogBackgroundImage;
    CSkinImage          m_ProjectImage;
	wxColour            m_StaticLineColor;
	wxColour            m_NoticeAlertColor;
    CSkinImage          m_WorkunitAnimationImage;
    CSkinImage          m_WorkunitRunningImage;
    CSkinImage          m_WorkunitSuspendedImage;
    CSkinImage          m_WorkunitWaitingImage;
    int                 m_iPanelOpacity;
};


class CSkinAdvanced : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinAdvanced )

public:
    CSkinAdvanced();
    ~CSkinAdvanced();

    void Clear();
    int  Parse(MIOFILE& in);

    bool InitializeDelayedValidation();

    wxString    GetApplicationName();
    wxString    GetApplicationShortName();
    wxIcon*     GetApplicationIcon();
    wxIcon*     GetApplicationIcon32();
    wxIcon*     GetApplicationDisconnectedIcon();
    wxIcon*     GetApplicationSnoozeIcon();
    wxBitmap*   GetApplicationLogo();
    wxString    GetOrganizationName();
    wxString    GetOrganizationWebsite();
    wxString    GetOrganizationHelpUrl();
    int         GetDefaultTab();
    wxString    GetExitMessage();
    bool        IsBranded();

private:
    bool        m_bIsBranded;
    wxString    m_strApplicationName;
    wxString    m_strApplicationShortName;
    CSkinIcon   m_iconApplicationIcon;
    CSkinIcon   m_iconApplicationIcon32;
    CSkinIcon   m_iconApplicationDisconnectedIcon;
    CSkinIcon   m_iconApplicationSnoozeIcon;
    wxBitmap    m_bitmapApplicationLogo;
    wxString    m_strOrganizationName;
    wxString    m_strOrganizationWebsite;
    wxString    m_strOrganizationHelpUrl;
    bool        m_bDefaultTabSpecified;
    int         m_iDefaultTab;
    wxString    m_strExitMessage;
};


class CSkinWizardATP : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinWizardATP )

public:
    CSkinWizardATP();
    ~CSkinWizardATP();

    void Clear();
    int  Parse(MIOFILE& in);

    bool InitializeDelayedValidation();

    wxString    GetWizardTitle() { return m_strTitle; }

private:
    wxString    m_strTitle;
};


class CSkinWizardATAM : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinWizardATAM )

public:
    CSkinWizardATAM();
    ~CSkinWizardATAM();

    void Clear();
    int  Parse(MIOFILE& in);

    bool InitializeDelayedValidation();

    wxString    GetAccountInfoMessage() { return m_strAccountInfoMessage; }

private:
    wxString    m_strAccountInfoMessage;
};


class CSkinWizards : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinWizards )

public:
    CSkinWizards();
    ~CSkinWizards();

    void Clear();
    int  Parse(MIOFILE& in);

    bool InitializeDelayedValidation();

    CSkinWizardATP*     GetWizardATP() { return &m_AttachToProjectWizard; }
    CSkinWizardATAM*    GetWizardATAM() { return &m_AttachToAccountManagerWizard; }

private:
    CSkinWizardATP      m_AttachToProjectWizard;
    CSkinWizardATAM     m_AttachToAccountManagerWizard;
};


class CSkinManager : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinManager )

public:
    CSkinManager();
    CSkinManager(bool debugSkins);
    ~CSkinManager();

    bool                ReloadSkin(wxString strSkin);

    wxArrayString&      GetCurrentSkins();
    wxString            GetDefaultSkinName();
    wxString            GetSelectedSkin() { return m_strSelectedSkin; }

    wxString            ConstructSkinFileName();
    wxString            ConstructSkinPath();
    wxString            GetSkinFileName();
    wxString            GetSkinsLocation();

    CSkinSimple*        GetSimple() { return &m_SimpleSkin; }
    CSkinAdvanced*      GetAdvanced() { return &m_AdvancedSkin; }
    CSkinWizards*       GetWizards() { return &m_WizardsSkin; }

private:
    CSkinSimple         m_SimpleSkin;
    CSkinAdvanced       m_AdvancedSkin;
    CSkinWizards        m_WizardsSkin;

    wxArrayString       m_astrSkins;
    wxString            m_strSelectedSkin;

    void Clear();
    int  Parse(MIOFILE& in, wxString strDesiredLocale);

    bool InitializeDelayedValidation();
};


#endif
