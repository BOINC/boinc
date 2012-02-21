///////////////////////////////////////////////////////////////////////////////
// Name:		wxFlatNotebook.cpp
// Purpose:     generic implementation of flat style notebook class.
// Author:      Eran Ifrah <eranif@bezeqint.net>
// Modified by: Priyank Bolia <soft@priyank.in>
// Created:     30/12/2005
// Modified:    01/01/2006
// Copyright:   Eran Ifrah (c)
// Licence:     wxWindows license <http://www.wxwidgets.org/licence3.txt>
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FNB_DROP_TARGET_H
#define _WX_FNB_DROP_TARGET_H

/**
\brief Contains the information about dragged page (page index and container).
*/
class wxFNBDragInfo
{
	wxWindow * m_Container;
	int m_PageIndex;	
public:		
	/**
	Constructor
	\param container - pointer to wxPageContainer object which contains dragged page
	\param pageindex - index of dragged page
	*/
	wxFNBDragInfo(wxWindow * container, int pageindex) : m_Container(container), m_PageIndex(pageindex){}	
	/**
	Returns wxPageContainer object which contains dragged page
	*/
	wxWindow * GetContainer() {return m_Container;}
	/**
	Returns the index of dragged page
	*/
	int GetPageIndex() {return m_PageIndex;}
};

/**
\brief Used for processing drag-n-drop opeartions
*/
template <class T>
class wxFNBDropTarget : public wxDropTarget
{
private:
	typedef wxDragResult (T::*pt2Func)(wxCoord, wxCoord, int, wxWindow *);
	T* m_pParent;
	pt2Func m_pt2CallbackFunc;
	wxCustomDataObject * m_DataObject;
public:
	/**
	\brief Constructor
	\param pParent - Object that will handle drag-n-drop operation
	\param pt2CallbackFunc - Pointer to callback method which should be called after dragging the notebook page
	*/
    wxFNBDropTarget(T* pParent, pt2Func pt2CallbackFunc)
		: m_pParent(pParent)
		, m_pt2CallbackFunc(pt2CallbackFunc)
		, m_DataObject(NULL)
	{
		m_DataObject = new wxCustomDataObject(wxDataFormat(wxT("wxFNB")));
		SetDataObject(m_DataObject);
	}
	/**
	\brief Virtual Destructor
	*/
	virtual ~wxFNBDropTarget(void) {}
	/**
	\brief Used for processing drop operation
	\param x - X-coordinate
	\param y - Y-coordinate
	\param def - Result of drag-n-drop operation
	*/
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult /*def*/)
	{		
		GetData();
		wxFNBDragInfo * draginfo = (wxFNBDragInfo *)m_DataObject->GetData();
		if(!draginfo) 
		{
			return wxDragNone;
		}
		return (m_pParent->*m_pt2CallbackFunc)(x, y, draginfo->GetPageIndex(), (T *)draginfo->GetContainer()); 	
	}
};

#endif
