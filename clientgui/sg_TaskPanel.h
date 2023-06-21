// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#ifndef BOINC_SG_TASKPANEL_H
#define BOINC_SG_TASKPANEL_H

// Comment???
//
#define SELECTBYRESULTNAME 0

#include "sg_PanelBase.h"


typedef struct {
    RESULT * result;
    char result_name[256];
    char project_url[256];
    int dotColor;
    wxArrayString slideShowFileNames;
    int lastSlideShown;
    double project_files_downloaded_time;
} TaskSelectionData;



///////////////////////////////////////////////////////////////////////////
/// Class CScrolledTextBox
///////////////////////////////////////////////////////////////////////////////
class CScrolledTextBox : public wxScrolledWindow
{
    DECLARE_DYNAMIC_CLASS( CScrolledTextBox )
    DECLARE_EVENT_TABLE()
	public:
        CScrolledTextBox();
		CScrolledTextBox( wxWindow* parent);
        ~CScrolledTextBox();

        void SetValue(const wxString& s);
        virtual void OnEraseBackground(wxEraseEvent& event);

    private:
        int Wrap(const wxString& text, int widthMax, int *lineHeight);
        bool IsStartOfNewLine();
        void OnOutputLine(const wxString& line);

        wxBoxSizer*                 m_TextSizer;
        bool                        m_eol;
        wxString                    m_text;
        int                         m_hLine;
};



///////////////////////////////////////////////////////////////////////////
/// Class CSlideShowPanel
///////////////////////////////////////////////////////////////////////////////

class CSlideShowPanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CSlideShowPanel )
    DECLARE_EVENT_TABLE()

	public:
        CSlideShowPanel();
		CSlideShowPanel( wxWindow* parent);
		~CSlideShowPanel();

        void OnSlideShowTimer(wxTimerEvent& WXUNUSED(event));
        void SetDescriptionText(void);
        void AdvanceSlideShow(bool changeSlide, bool reload);
        void OnPaint(wxPaintEvent& WXUNUSED(event));
        void OnEraseBackground(wxEraseEvent& event);

    private:
        CScrolledTextBox*           m_description;
        wxTimer*                    m_ChangeSlideTimer;
        wxBitmap                    m_SlideBitmap;
        bool                        m_bCurrentSlideIsDefault;
        bool                        m_bGotAllProjectsList;
        bool                        m_bHasBeenDrawn;
        ALL_PROJECTS_LIST           m_AllProjectsList;
};


///////////////////////////////////////////////////////////////////////////////
/// Class CSimpleTaskPanel
///////////////////////////////////////////////////////////////////////////////

#ifdef __WXMAC__
#include "MacBitmapComboBox.h"
#else
#define CBOINCBitmapComboBox wxBitmapComboBox
#endif

class CSimpleTaskPanel : public CSimplePanelBase
{
    DECLARE_DYNAMIC_CLASS( CSimpleTaskPanel )
    DECLARE_EVENT_TABLE()

    public:
        CSimpleTaskPanel();
		CSimpleTaskPanel( wxWindow* parent);
		~CSimpleTaskPanel();

        TaskSelectionData* GetTaskSelectionData();
        wxString GetSelectedTaskString() { return m_TaskSelectionCtrl->GetValue(); }
        CBOINCBitmapComboBox* GetTaskSelectionCtrl() { return m_TaskSelectionCtrl; }
        void UpdatePanel(bool delayShow=false);
        void OnTaskSelection(wxCommandEvent &event);
        wxRect* GetProgressRect();
        void ReskinInterface();

	private:
        void GetApplicationAndProjectNames(RESULT* result, wxString* appName, wxString* projName);
        wxString GetElapsedTimeString(double f);
        wxString GetTimeRemainingString(double f);
        wxString GetStatusString(RESULT* result);
        void FindSlideShowFiles(TaskSelectionData *selData);
        void UpdateTaskSelectionList(bool reskin);
        bool isRunning(RESULT* result);
		bool DownloadingResults();
		bool Suspended();
		bool ProjectUpdateScheduled();
		void DisplayIdleState();

	protected:
#ifdef __WXMAC__
        void OnEraseBackground(wxEraseEvent& event);
#endif
        wxRect*                     m_progressBarRect;
		CTransparentStaticText*     m_myTasksLabel;
		CBOINCBitmapComboBox*       m_TaskSelectionCtrl;
		CTransparentStaticText*     m_TaskProjectLabel;
		CTransparentStaticText*     m_TaskProjectName;
#if SELECTBYRESULTNAME
		CTransparentStaticText*     m_TaskApplicationName;
#endif
        CSlideShowPanel*            m_SlideShowArea;
		CTransparentStaticText*     m_ElapsedTimeValue;
		CTransparentStaticText*     m_TimeRemainingValue;
		wxGauge*                    m_ProgressBar;
		CTransparentStaticText*     m_ProgressValueText;
		CTransparentStaticText*     m_StatusValueText;
		wxButton*                   m_TaskCommandsButton;
        wxRect                      m_ProgressRect;
        int                         m_oldWorkCount;
        int                         m_ipctDoneX1000;
		time_t                      error_time;
        bool                        m_bStableTaskInfoChanged;
        int                         m_CurrentTaskSelection;
        wxString                    m_sNotAvailableString;
        wxString                    m_sNoProjectsString;
};

#endif
