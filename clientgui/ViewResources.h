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

#ifndef _VIEWRESOURCES_H_
#define _VIEWRESOURCES_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewResources.cpp"
#endif


#include "BOINCBaseView.h"


class CResource : public wxObject
{
public:
	CResource();
	~CResource();

	wxInt32  GetProjectName( wxString& strProjectName );
	wxInt32  GetDiskSpace( wxString& strDiskSpace );

	wxInt32  SetProjectName( wxString& strProjectName );
	wxInt32  SetDiskSpace( wxString& strDiskSpace );

protected:
	wxString m_strProjectName;
    wxString m_strDiskSpace;
};


class CViewResources : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewResources )

public:
    CViewResources();
    CViewResources(wxNotebook* pNotebook);

    ~CViewResources();

    virtual wxString        GetViewName();
    virtual char**          GetViewIcon();

protected:

    bool                    m_bTaskHeaderHidden;

    bool                    m_bTipsHeaderHidden;

    bool                    m_bItemSelected;

    std::vector<CResource*> m_ResourceCache;

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;

    virtual wxString        OnDocGetItemText( long item, long column ) const;

    virtual void            OnTaskLinkClicked( const wxHtmlLinkInfo& link );
    virtual void            OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y );

    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual wxInt32         UpdateCache( long item, long column, wxString& strNewData );

    virtual void            UpdateSelection();
    virtual void            UpdateTaskPane();

    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatDiskSpace( wxInt32 item, wxString& strBuffer ) const;


    //
    // Globalization/Localization
    //
    wxString                VIEW_HEADER;

    wxString                SECTION_TASK;
    wxString                SECTION_TIPS;

    wxString                BITMAP_RESOURCES;
    wxString                BITMAP_TASKHEADER;
    wxString                BITMAP_TIPSHEADER;

    wxString                LINKDESC_DEFAULT;

};


#endif

