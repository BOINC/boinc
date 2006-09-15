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


#ifndef _DLG_MESSAGES_H_ 
#define _DLG_MESSAGES_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_DlgMessages.cpp"
#endif

#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL

class SkinClass;
class CSGUIListCtrl;

class CDlgMessages:public wxDialog
{
public:
	CDlgMessages(wxWindow* parent, wxString dirPref,wxWindowID id = -1, const wxString& title = wxT(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("dialogBox"));
	//Skin Class
	SkinClass *appSkin;
	//btns
	wxBitmap* btmpClose; 
	wxBitmap* btmpCloseClick; 
	wxBitmap* btmpWindowBg;
	wxBitmapButton *btnClose;

	// Pointer control
	virtual ~CDlgMessages();
	void initBefore();
	void CreateDialog();
	void LoadSkinImages();
	void initAfter();
	//
	virtual wxString        OnListGetItemText( long item, long column ) const;
	virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

	DECLARE_EVENT_TABLE()

protected:
	wxInt32                 m_iPreviousDocCount;

    wxListItemAttr*         m_pMessageInfoAttr;
    wxListItemAttr*         m_pMessageErrorAttr;

	bool                    m_bProcessingTaskRenderEvent;
    bool                    m_bProcessingListRenderEvent;
    bool                    m_bForceUpdateSelection;

	wxTimer*                m_pRefreshMessagesTimer;

	bool                    _EnsureLastItemVisible();
    virtual bool            EnsureLastItemVisible();
	wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatPriority( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatMessage( wxInt32 item, wxString& strBuffer ) const;

	wxString m_SkinDirPrefix;
	void OnEraseBackground(wxEraseEvent& event);
	void OnBtnClick(wxCommandEvent& event);
	void VwXEvOnEraseBackground(wxEraseEvent& event);
	void VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap* bitMap,int opz);
    //
	virtual void            OnListRender( wxTimerEvent& event );
	virtual wxInt32         GetDocCount();
	//
	CSGUIListCtrl *m_pList;
};


#endif  // end CDlgMessages
