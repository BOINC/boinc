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


class CSkinSimpleButton : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinSimpleButton )

public:
    CSkinSimpleButton();
    ~CSkinSimpleButton();

    void Clear();
    int  Parse(MIOFILE& in);

    wxBitmap* GetBitmap();
    wxBitmap* GetBitmapClicked();

    bool SetDefaults(
        wxString strComponentName,
        const char** ppDefaultImage, 
        const char** ppDefaultClickedImage
    );

    bool Validate();

private:
    wxString     m_strComponentName;
    wxString     m_strDesiredBitmap;
    wxString     m_strDesiredBitmapClicked;
    const char** m_ppDefaultBitmap;
    const char** m_ppDefaultBitmapClicked;
    wxBitmap     m_bmpBitmap;
    wxBitmap     m_bmpBitmapClicked;
};


class CSkinSimpleTab : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinSimpleTab )

public:
    CSkinSimpleTab();
    ~CSkinSimpleTab();

    void Clear();
    int  Parse(MIOFILE& in);

    wxBitmap* GetBitmap();
    wxColour* GetBorderColor();
    wxColour* GetGradientFromColor();
    wxColour* GetGradientToColor();

    bool SetDefaults(
        wxString strComponentName,
        const char** ppDefaultImage,
        wxString strBorderColor,
        wxString strGradientFromColor,
        wxString strGradientToColor
    );

    bool Validate();

private:
    wxString     m_strComponentName;
    wxString     m_strDesiredBitmap;
    wxString     m_strDesiredBorderColor;
    wxString     m_strDesiredGradientFromColor;
    wxString     m_strDesiredGradientToColor;
    const char** m_ppDefaultBitmap;
    wxString     m_strDefaultBorderColor;
    wxString     m_strDefaultGradientFromColor;
    wxString     m_strDefaultGradientToColor;
    wxBitmap     m_bmpBitmap;
    wxColour     m_colBorderColor;
    wxColour     m_colGradientFromColor;
    wxColour     m_colGradientToColor;
};


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
    CSkinImage*         GetSpacerImage() { return &m_SpacerImage; }
    wxColour            GetStaticLineColor() { return m_StaticLineColor; }

    CSkinImage*         GetStateIndicatorBackgroundImage() { return &m_StateIndicatorBackgroundImage; }
    CSkinImage*         GetConnectingIndicatorImage() { return &m_ConnectingIndicatorImage; }
    CSkinImage*         GetErrorIndicatorImage() { return &m_ErrorIndicatorImage; }

    CSkinSimpleTab*     GetWorkunitActiveTab() { return &m_WorkunitActiveTab; }
    CSkinSimpleTab*     GetWorkunitSuspendedTab() { return &m_WorkunitSuspendedTab; }
    CSkinImage*         GetWorkunitTabAreaBackgroundImage() { return &m_WorkunitTabAreaBackgroundImage; }
    CSkinImage*         GetWorkunitAreaBackgroundImage() { return &m_WorkunitAreaBackgroundImage; }
    CSkinImage*         GetWorkunitAnimationBackgroundImage() { return &m_WorkunitAnimationBackgroundImage; }
    CSkinImage*         GetWorkunitAnimationImage() { return &m_WorkunitAnimationImage; }
    CSkinImage*         GetWorkunitGaugeBackgroundImage() { return &m_WorkunitGaugeBackgroundImage; }
    CSkinImage*         GetWorkunitGaugeProgressIndicatorImage() { return &m_WorkunitGaugeProgressIndicatorImage; }

    CSkinImage*         GetProjectAreaBackgroundImage() { return &m_ProjectAreaBackgroundImage; }
    CSkinImage*         GetProjectImage() { return &m_ProjectImage; }

    CSkinSimpleButton*  GetAttachProjectButton() { return &m_AttachProjectButton; }
    CSkinSimpleButton*  GetHelpButton() { return &m_HelpButton; }
    CSkinSimpleButton*  GetRightArrowButton() { return &m_RightArrowButton; }
    CSkinSimpleButton*  GetLeftArrowButton() { return &m_LeftArrowButton; }
    CSkinSimpleButton*  GetSaveButton() { return &m_SaveButton; }
    CSkinSimpleButton*  GetSynchronizeButton() { return &m_SynchronizeButton; }
    CSkinSimpleButton*  GetCancelButton() { return &m_CancelButton; }
    CSkinSimpleButton*  GetCloseButton() { return &m_CloseButton; }
    CSkinSimpleButton*  GetCopyAllButton() { return &m_CopyAllButton; }
    CSkinSimpleButton*  GetCopyButton() { return &m_CopyButton; }

    CSkinImage*         GetMessagesLink() { return &m_MessagesLink; }
    CSkinImage*         GetMessagesAlertLink() { return &m_MessagesAlertLink; }
    CSkinImage*         GetSuspendLink() { return &m_SuspendLink; }
    CSkinImage*         GetResumeLink() { return &m_ResumeLink; }
    CSkinImage*         GetPreferencesLink() { return &m_PreferencesLink; }
    CSkinImage*         GetAdvancedLink() { return &m_AdvancedLink; }

    CSkinImage*         GetDialogBackgroundImage() { return &m_DialogBackgroundImage; }

private:
	CSkinImage          m_BackgroundImage;
    CSkinImage          m_SpacerImage;
	wxColour            m_StaticLineColor;

    CSkinImage          m_StateIndicatorBackgroundImage;
    CSkinImage          m_ConnectingIndicatorImage;
    CSkinImage          m_ErrorIndicatorImage;

    CSkinSimpleTab      m_WorkunitActiveTab;
    CSkinSimpleTab      m_WorkunitSuspendedTab;
    CSkinImage          m_WorkunitTabAreaBackgroundImage;
    CSkinImage          m_WorkunitAreaBackgroundImage;
    CSkinImage          m_WorkunitAnimationBackgroundImage;
    CSkinImage          m_WorkunitAnimationImage;
    CSkinImage          m_WorkunitGaugeBackgroundImage;
    CSkinImage          m_WorkunitGaugeProgressIndicatorImage;

    CSkinImage          m_ProjectAreaBackgroundImage;
    CSkinImage          m_ProjectImage;

    CSkinSimpleButton   m_AttachProjectButton;
    CSkinSimpleButton   m_HelpButton;
    CSkinSimpleButton   m_RightArrowButton;
    CSkinSimpleButton   m_LeftArrowButton;
    CSkinSimpleButton   m_SaveButton;
    CSkinSimpleButton   m_SynchronizeButton;
    CSkinSimpleButton   m_CancelButton;
    CSkinSimpleButton   m_CloseButton;
    CSkinSimpleButton   m_CopyAllButton;
    CSkinSimpleButton   m_CopyButton;

    CSkinImage          m_MessagesLink;
    CSkinImage          m_MessagesAlertLink;
    CSkinImage          m_SuspendLink;
    CSkinImage          m_ResumeLink;
    CSkinImage          m_PreferencesLink;
    CSkinImage          m_AdvancedLink;

    CSkinImage          m_DialogBackgroundImage;
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
    bool        IsDefaultTabSpecified();

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

    wxBitmap*   GetWizardBitmap() { return &m_bitmapWizardBitmap; }
    wxString    GetWizardTitle() { return m_strTitle; }

private:
    wxBitmap    m_bitmapWizardBitmap;
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

    wxBitmap*   GetWizardBitmap() { return &m_bitmapWizardBitmap; }
    wxString    GetWizardTitle() { return m_strTitle; }
    wxString    GetAccountInfoMessage() { return m_strAccountInfoMessage; }

private:
    wxBitmap    m_bitmapWizardBitmap;
    wxString    m_strTitle;
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

    bool                ReloadSkin(wxLocale* pLocale, wxString strSkin);

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
