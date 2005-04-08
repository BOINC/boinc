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

#ifndef _BOINCTASKCTRL_H_
#define _BOINCTASKCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCTaskCtrl.cpp"
#endif


class CBOINCBaseView;

class CBOINCTaskCtrl : public wxHtmlWindow
{
    DECLARE_DYNAMIC_CLASS( CBOINCTaskCtrl )

public:
    CBOINCTaskCtrl();
    CBOINCTaskCtrl( CBOINCBaseView* pView, wxWindowID iHtmlWindowID, wxInt32 iHtmlWindowFlags );

    ~CBOINCTaskCtrl();

    virtual void                BeginTaskPage();
    virtual void                BeginTaskSection(  const wxString& strTaskHeaderFilename, 
                                                   bool  bHidden );
#if 0
    virtual void                BeginTaskSection(  const wxString& strLink,
                                                   const wxString& strTaskHeaderFilename, 
                                                   bool  bHidden );
#endif
    virtual void                CreateTask(        const wxString& strLink,
                                                   const wxString& strTaskName,
                                                   bool  bHidden );
    virtual void                CreateTask(        const wxString& strLink,
                                                   const wxString& strTaskIconFilename, 
                                                   const wxString& strTaskName,
                                                   bool  bHidden );
    virtual void                CreateTaskSeparator( bool  bHidden );
    virtual void                EndTaskSection(    bool  bHidden );
    virtual void                UpdateQuickTip(    const wxString& strIconFilename,
                                                   const wxString& strTip,
                                                   bool  bHidden );
#if 0
    virtual void                UpdateQuickTip(    const wxString& strLink,
                                                   const wxString& strIconFilename,
                                                   const wxString& strTip,
                                                   bool  bHidden );
#endif
    virtual void                EndTaskPage();


    virtual void                CreateTaskHeader(  const wxString& strFilename, 
                                                   const wxBitmap& itemTaskBitmap, 
                                                   const wxString& strTaskName ); 


    virtual void                AddVirtualFile(    const wxString& strFilename, 
                                                   wxImage& itemImage, 
                                                   long lType );
    virtual void                AddVirtualFile(    const wxString& strFilename, 
                                                   const wxBitmap& itemBitmap, 
                                                   long lType );
    virtual void                RemoveVirtualFile( const wxString& strFilename );


    virtual bool                OnSaveState( wxConfigBase* pConfig );
    virtual bool                OnRestoreState( wxConfigBase* pConfig );

    virtual void                OnLinkClicked( const wxHtmlLinkInfo& link );
    virtual void                OnCellClicked( wxHtmlCell* cell, wxCoord x, wxCoord y, const wxMouseEvent& event );
    virtual void                OnCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y );
    virtual wxHtmlOpeningStatus OnOpeningURL( wxHtmlURLType type, const wxString& url, wxString *redirect );

private:
    
    CBOINCBaseView*         m_pParentView;

    wxString                m_strTaskPage;

};


#endif

