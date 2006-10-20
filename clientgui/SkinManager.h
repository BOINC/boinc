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
    bool Ok();
    int  Parse(MIOFILE& in);

    wxBitmap* GetBitmap() { return &m_bmpBitmap; };
    wxColour* GetBackgroundColor() { return &m_colBackgroundColor; };
    wxColour* GetTransparencyMask() { return &m_colTransparencyMask; };

    bool      SetDefaults(const char** default_image);
    bool      SetDefaults(const char** default_image, wxString strBackgroundColor, wxString strTransparenyMask);

private:
    wxBitmap  m_bmpBitmap;
    wxColour  m_colBackgroundColor;
    wxColor   m_colTransparencyMask;
};


class CSkinSimpleButton : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinSimpleButton )

public:
    CSkinSimpleButton();
    ~CSkinSimpleButton();

    void Clear();
    bool Ok();
    int  Parse(MIOFILE& in);

    wxBitmap* GetBitmap() { return &m_bmpBitmap; };
    wxBitmap* GetBitmapClicked() { return &m_bmpBitmapClicked; };

    bool      SetDefaults(const char** default_image, const char** default_clicked_image);

private:
    wxBitmap  m_bmpBitmap;
    wxBitmap  m_bmpBitmapClicked;
};


class CSkinSimpleTab : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinSimpleTab )

public:
    CSkinSimpleTab();
    ~CSkinSimpleTab();

    void Clear();
    bool Ok();
    int  Parse(MIOFILE& in);

    wxBitmap* GetBitmap() { return &m_bmpBitmap; };
    wxColour* GetBorderColor() { return &m_colBorderColor; };
    wxColour* GetGradientFromColor() { return &m_colGradientFromColor; };
    wxColour* GetGradientToColor() { return &m_colGradientToColor; };

    bool SetDefaults(
        const char** default_image,
        wxString strBorderColor,
        wxString strGradientFromColor,
        wxString strGradientToColor
    );

private:
    wxBitmap  m_bmpBitmap;
    wxColour  m_colBorderColor;
    wxColour  m_colGradientFromColor;
    wxColour  m_colGradientToColor;
};


class CSkinSimple : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinSimple )

public:
    CSkinSimple();
    ~CSkinSimple();

    void Clear();
    int  Parse(MIOFILE& in);

    bool Ok();

    bool ValidateSkin();

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
    CSkinSimpleButton*  GetRightArrowButton() { return &m_RightArrowButton; }
    CSkinSimpleButton*  GetLeftArrowButton() { return &m_LeftArrowButton; }
    CSkinSimpleButton*  GetSaveButton() { return &m_SaveButton; }
    CSkinSimpleButton*  GetCancelButton() { return &m_CancelButton; }
    CSkinSimpleButton*  GetChangeButton() { return &m_ChangeButton; }
    CSkinSimpleButton*  GetCloseButton() { return &m_CloseButton; }
    CSkinSimpleButton*  GetClearButton() { return &m_ClearButton; }

    CSkinImage*         GetMessagesLink() { return &m_MessagesLink; }
    CSkinImage*         GetMessagesAlertLink() { return &m_MessagesAlertLink; }
    CSkinImage*         GetSuspendLink() { return &m_SuspendLink; }
    CSkinImage*         GetResumeLink() { return &m_ResumeLink; }
    CSkinImage*         GetPreferencesLink() { return &m_PreferencesLink; }
    CSkinImage*         GetAdvancedLink() { return &m_AdvancedLink; }

    CSkinImage*         GetPreferencesDialogBackgroundImage() { return &m_PreferencesDialogBackgroundImage; }
    CSkinImage*         GetMessagesDialogBackgroundImage() { return &m_MessagesDialogBackgroundImage; }

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
    CSkinSimpleButton   m_RightArrowButton;
    CSkinSimpleButton   m_LeftArrowButton;
    CSkinSimpleButton   m_SaveButton;
    CSkinSimpleButton   m_CancelButton;
    CSkinSimpleButton   m_ChangeButton;
    CSkinSimpleButton   m_CloseButton;
    CSkinSimpleButton   m_ClearButton;

    CSkinImage          m_MessagesLink;
    CSkinImage          m_MessagesAlertLink;
    CSkinImage          m_SuspendLink;
    CSkinImage          m_ResumeLink;
    CSkinImage          m_PreferencesLink;
    CSkinImage          m_AdvancedLink;

    CSkinImage          m_PreferencesDialogBackgroundImage;
    CSkinImage          m_MessagesDialogBackgroundImage;
};


class CSkinAdvanced : public CSkinItem
{
    DECLARE_DYNAMIC_CLASS( CSkinAdvanced )

public:
    CSkinAdvanced();
    ~CSkinAdvanced();

    void Clear();
    bool Ok();
    int  Parse(MIOFILE& in);

    bool ValidateSkin();

    wxString    GetApplicationName() { return m_strApplicationName; }
    wxIcon*     GetApplicationIcon() { return &m_iconApplicationIcon; }
    wxIcon*     GetApplicationDisconnectedIcon() { return &m_iconApplicationDisconnectedIcon; }
    wxIcon*     GetApplicationSnoozeIcon() { return &m_iconApplicationSnoozeIcon; }
    wxBitmap*   GetApplicationLogo() { return &m_bitmapApplicationLogo; }
    wxString    GetCompanyName() { return m_strCompanyName; }
    wxString    GetCompanyWebsite() { return m_strCompanyWebsite; }
    wxString    GetProjectName() { return m_strProjectName; }
    int         GetDefaultTab() { return m_iDefaultTab; }
    wxString    GetExitMessage() { return m_strExitMessage; }
    bool        IsBranded() { return m_bIsBranded; }
    bool        IsDefaultTabSpecified() { return m_bDefaultTabSpecified; }

private:
    bool        m_bIsBranded;
    wxString    m_strApplicationName;
    wxIcon      m_iconApplicationIcon;
    wxIcon      m_iconApplicationDisconnectedIcon;
    wxIcon      m_iconApplicationSnoozeIcon;
    wxBitmap    m_bitmapApplicationLogo;
    wxString    m_strCompanyName;
    wxString    m_strCompanyWebsite;
    wxString    m_strProjectName;
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
    bool Ok();
    int  Parse(MIOFILE& in);

    bool ValidateSkin();

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
    bool Ok();
    int  Parse(MIOFILE& in);

    bool ValidateSkin();

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
    bool Ok();
    int  Parse(MIOFILE& in);

    bool ValidateSkin();

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
    ~CSkinManager();

    bool ReloadSkin(wxLocale* pLocale, wxString strSkin);

    CSkinSimple*        GetSimple() { return &m_SimpleSkin; }
    CSkinAdvanced*      GetAdvanced() { return &m_AdvancedSkin; }
    CSkinWizards*       GetWizards() { return &m_WizardsSkin; }

private:
    CSkinSimple         m_SimpleSkin;
    CSkinAdvanced       m_AdvancedSkin;
    CSkinWizards        m_WizardsSkin;

    void Clear();
    bool Ok();
    int  Parse(MIOFILE& in, wxString strDesiredLocale);

    bool ValidateSkin();
};


#endif
