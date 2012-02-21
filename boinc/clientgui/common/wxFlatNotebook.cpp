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


#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "common/wxFlatNotebook.h"
#endif

#include "stdwx.h"
#include "common/wxFlatNotebook.h"
#include "common/wxFlatNotebookImages.h"


IMPLEMENT_DYNAMIC_CLASS(wxFlatNotebookEvent, wxNotifyEvent)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CHANGING)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CLOSING) 
DEFINE_EVENT_TYPE(wxEVT_COMMAND_FLATNOTEBOOK_CONTEXT_MENU)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CLOSED)

IMPLEMENT_DYNAMIC_CLASS(wxFlatNotebookBase, wxPanel)

BEGIN_EVENT_TABLE(wxFlatNotebookBase, wxPanel)
EVT_NAVIGATION_KEY(wxFlatNotebookBase::OnNavigationKey)
END_EVENT_TABLE()

wxFlatNotebookBase::wxFlatNotebookBase(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
	m_bForceSelection = false;
	m_nPadding = 6;
	m_nFrom = 0;	
	style |= wxTAB_TRAVERSAL;
	m_pages = NULL;
	wxPanel::Create(parent, id, pos, size, style, name);

	m_pages = new wxPageContainerBase(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
	
}

wxFlatNotebookBase::~wxFlatNotebookBase(void)
{
}

void wxFlatNotebookBase::Init()
{
	long style = GetWindowStyleFlag();
	m_pages->m_colorBorder = style & wxFNB_FANCY_TABS ? m_pages->m_colorTo : wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

	m_mainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_mainSizer);

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

	// Add the tab container to the sizer
	m_mainSizer->Insert(0, m_pages, 0, wxEXPAND);

	// Set default page height
	wxClientDC dc(this);
	int width, height;
	wxString stam = wxT("Tp");	// Temp data to get the text height;
	dc.GetTextExtent(stam, &width, &height);

	int tabHeight = height + 8; // We use 8 pixels as padding
	m_pages->SetSizeHints(wxSize(-1, tabHeight));

	m_pages->m_nFrom = m_nFrom;
	m_pDropTarget = new wxFNBDropTarget<wxFlatNotebookBase>(this, &wxFlatNotebookBase::OnDropTarget);
	SetDropTarget(m_pDropTarget);
}

wxPageContainerBase* wxFlatNotebookBase::CreatePageContainer()
{
	return new wxPageContainerBase(this, wxID_ANY);
}

void wxFlatNotebookBase::SetActiveTabTextColour(const wxColour& textColour)
{
	m_pages->m_activeTextColor = textColour;
}
void wxFlatNotebookBase::SetTabBorderWidth(int brWidth)
{
	m_pages->m_tabBorderWidth = brWidth;
}

wxDragResult wxFlatNotebookBase::OnDropTarget(wxCoord x, wxCoord y, int nTabPage, wxWindow * wnd_oldContainer)
{
	return m_pages->OnDropTarget(x, y, nTabPage, wnd_oldContainer);
}

void wxFlatNotebookBase::AddPage(wxWindow* window, const wxString& caption, const bool selected, const int imgindex)
{
	// sanity check
	if (!window)
		return;

	// reparent the window to us
	window->Reparent(this);

	// Add tab
	bool bSelected = selected || m_windows.empty();
	int curSel = m_pages->GetSelection();

	if( !m_pages->IsShown() )
		m_pages->Show();

	m_pages->AddPage(caption, bSelected, imgindex);
	m_windows.push_back(window);

	Freeze();

	// Check if a new selection was made
	if(bSelected)
	{
		if(curSel >= 0)
		{
			// Remove the window from the main sizer
			m_mainSizer->Detach(m_windows[curSel]);
			m_windows[curSel]->Hide();
		}
		if(m_windowStyle & wxFNB_BOTTOM)
		{
			m_mainSizer->Insert(0, window, 1, wxEXPAND);
		}
		else
		{
			// We leave a space of 1 pixel around the window
			m_mainSizer->Add(window, 1, wxEXPAND);
		}
	}
	else
	{
		// Hide the page
		window->Hide();
	}
	m_mainSizer->Layout();
	Thaw();
	Refresh();

}

void wxFlatNotebookBase::SetImageList(wxFlatNotebookImageList * imglist) 
{
	m_pages->SetImageList(imglist);
}

wxFlatNotebookImageList * wxFlatNotebookBase::GetImageList() 
{
	return m_pages->GetImageList();
}

bool wxFlatNotebookBase::InsertPage(size_t index, wxWindow* page, const wxString& text, bool select, const int imgindex)
{
	// sanity check
	if (!page)
		return false;

	// reparent the window to us
	page->Reparent(this);

	if(m_windows.empty())
	{
		AddPage(page, text, select, imgindex);
		return true;
	}
	index = std::min((unsigned int)index, (unsigned int)m_windows.size());
	// Insert tab
	bool bSelected = select || m_windows.empty();
	int curSel = m_pages->GetSelection();

	if(index <= m_windows.size())
	{
		std::vector<wxWindow*>::iterator iter = m_windows.begin() + index;
		m_windows.insert(iter, page);
		wxLogTrace(wxTraceMask(), _("New page inserted. Index = %i"), index);
	}
	else
	{		
		m_windows.push_back(page);
		wxLogTrace(wxTraceMask(), _("New page appended. Index = %i"), index);
	}
	m_pages->InsertPage(index, page, text, bSelected, imgindex);
	if((int)index <= curSel) curSel++;

	Freeze();

	// Check if a new selection was made
	if(bSelected)
	{
		if(curSel >= 0)
		{
			// Remove the window from the main sizer
			m_mainSizer->Detach(m_windows[curSel]);
			m_windows[curSel]->Hide();
		}
		m_pages->SetSelection(index);
	}
	else
	{
		// Hide the page
		page->Hide();
	}
	Thaw();
	m_mainSizer->Layout();
	Refresh();

	return true;
}

void wxFlatNotebookBase::SetSelection(size_t page)
{
	if(page >= m_windows.size())
		return;

	// Support for disabed tabs
	if(!m_pages->GetEnabled(page) && m_windows.size() > 1 && !m_bForceSelection)
		return;

	int curSel = m_pages->GetSelection();

	// program allows the page change
	Freeze();
	if(curSel >= 0)
	{
		// Remove the window from the main sizer
		m_mainSizer->Detach(m_windows[curSel]);
		m_windows[curSel]->Hide();
	}

	if(m_windowStyle & wxFNB_BOTTOM)
	{
		m_mainSizer->Insert(0, m_windows[page], 1, wxEXPAND);
	}
	else
	{
		// We leave a space of 1 pixel around the window
		m_mainSizer->Add(m_windows[page], 1, wxEXPAND);
	}

	m_windows[page]->Show();
	Thaw();

	m_mainSizer->Layout();
	m_pages->m_iActivePage = (int)page;
	m_pages->DoSetSelection(page);
}

void wxFlatNotebookBase::DeletePage(size_t page)
{
	if(page >= m_windows.size())
		return;

	// Fire a closing event
	wxFlatNotebookEvent event(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CLOSING, GetId());
	event.SetSelection((int)page);
	event.SetEventObject(this);
	GetEventHandler()->ProcessEvent(event);

	// The event handler allows it?
	if (!event.IsAllowed())
		return;

	Freeze();

	// Delete the requested page
	wxWindow *pageRemoved = m_windows[page];

	// If the page is the current window, remove it from the sizer
	// as well
	if((int)page == m_pages->GetSelection())
	{
		m_mainSizer->Detach(pageRemoved);
	}

	// Remove it from the array as well
	std::vector<wxWindow*>::iterator iter = std::find(m_windows.begin(), m_windows.end(), pageRemoved);
	if(iter != m_windows.end())
		m_windows.erase(iter);

	// Now we can destroy it; in wxWidgets use Destroy instead of delete
	pageRemoved->Destroy();

	Thaw();

	m_pages->DoDeletePage(page);
	Refresh();

	// Fire a closed event
	wxFlatNotebookEvent closedEvent(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CLOSED, GetId());
	closedEvent.SetSelection((int)page);
	closedEvent.SetEventObject(this);
	GetEventHandler()->ProcessEvent(closedEvent);
}

bool wxFlatNotebookBase::DeleteAllPages()
{
	if(m_windows.empty())
		return false;

	Freeze();
	std::vector<wxWindow*>::iterator iter = m_windows.begin();
	for(; iter != m_windows.end(); iter++)
	{
		delete (*iter);
	}

	m_windows.clear();

	Thaw();

	// Clear the container of the tabs as well
	m_pages->DeleteAllPages();
	return true;
}

wxWindow* wxFlatNotebookBase::GetCurrentPage() const
{
	int sel = m_pages->GetSelection();
	if(sel < 0)
		return NULL;

	return m_windows[sel];
}

wxWindow* wxFlatNotebookBase::GetPage(size_t page) const
{
	if(page >= m_windows.size())
		return NULL;

	return m_windows[page];
}

int wxFlatNotebookBase::GetPageIndex(wxWindow* win) const
{
	for (size_t i = 0; i < m_windows.size(); ++i)
	{
		if (m_windows[i] == win)
			return (int)i;
	}
	return -1;
}

int wxFlatNotebookBase::GetSelection() const 
{ 
	return m_pages->GetSelection(); 
}

void wxFlatNotebookBase::AdvanceSelection(bool bForward)
{
	m_pages->AdvanceSelection(bForward);
}

int wxFlatNotebookBase::GetPageCount() const
{
	return (int)m_pages->GetPageCount();
}

void wxFlatNotebookBase::OnNavigationKey(wxNavigationKeyEvent& event)
{
	if ( event.IsWindowChange() ) 
	{
		// change pages
		AdvanceSelection(event.GetDirection());
	}
	else 
	{
		// pass to the parent
		if ( GetParent() ) 
		{
			event.SetCurrentFocus(this);
			GetParent()->ProcessEvent(event);
		}
	}
}

bool wxFlatNotebookBase::GetPageShapeAngle(int page_index, unsigned int * result)
{
	if(page_index < 0 || page_index >= (int)m_pages->m_pagesInfoVec.size()) return false;
	*result = m_pages->m_pagesInfoVec[page_index].GetTabAngle();
	return true;
}

void wxFlatNotebookBase::SetPageShapeAngle(int page_index, unsigned int angle)
{
	if(page_index < 0 || page_index >= (int)m_pages->m_pagesInfoVec.size()) return;
	if(angle > 15) return;

	m_pages->m_pagesInfoVec[page_index].SetTabAngle(angle);
}

void wxFlatNotebookBase::SetAllPagesShapeAngle(unsigned int angle)
{
	if(angle > 15) return;
	for(unsigned int i = 0; i < m_pages->m_pagesInfoVec.size(); i++)
	{
		m_pages->m_pagesInfoVec[i].SetTabAngle(angle);
	}
	Refresh();
}

wxSize wxFlatNotebookBase::GetPageBestSize()
{
	return m_pages->GetClientSize();
}

bool wxFlatNotebookBase::SetPageText(size_t page, const wxString& text)
{
	bool bVal = m_pages->SetPageText(page, text);
	m_pages->Refresh();
	return bVal;
}

void wxFlatNotebookBase::SetPadding(const wxSize& padding)
{
	m_nPadding = padding.GetWidth();
}

void wxFlatNotebookBase::SetWindowStyleFlag(long style)
{
	wxPanel::SetWindowStyleFlag(style);

	if(m_pages) 
	{
		// For changing the tab position (i.e. placing them top/bottom)
		// refreshing the tab container is not enough
		m_pages->m_colorBorder = style & wxFNB_FANCY_TABS ? m_pages->m_colorTo : wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		SetSelection(m_pages->m_iActivePage);
	}
}

bool wxFlatNotebookBase::RemovePage(size_t page)
{
	if(page >= m_windows.size())
		return false;

	// Fire a closing event
	wxFlatNotebookEvent event(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CLOSING, GetId());
	event.SetSelection((int)page);
	event.SetEventObject(this);
	GetEventHandler()->ProcessEvent(event);

	// The event handler allows it?
	if (!event.IsAllowed())
		return false;

	Freeze();

	// Remove the requested page
	wxWindow *pageRemoved = m_windows[page];

	// If the page is the current window, remove it from the sizer
	// as well
	if((int)page == m_pages->GetSelection())
	{
		m_mainSizer->Detach(pageRemoved);
	}

	// Remove it from the array as well
	std::vector<wxWindow*>::iterator iter = std::find(m_windows.begin(), m_windows.end(), pageRemoved);
	if(iter != m_windows.end())
		m_windows.erase(iter);

	Thaw();

	m_pages->DoDeletePage(page);

	return true;
}

void wxFlatNotebookBase::SetRightClickMenu(wxMenu* menu)
{
	m_pages->m_pRightClickMenu = menu;
}

wxString wxFlatNotebookBase::GetPageText(size_t page)
{
	return m_pages->GetPageText(page);
}

void wxFlatNotebookBase::SetGradientColors(const wxColour& from, const wxColour& to, const wxColour& border)
{
	m_pages->m_colorFrom = from;
	m_pages->m_colorTo   = to;
	m_pages->m_colorBorder = border;
}

void wxFlatNotebookBase::SetGradientColorsInactive(const wxColour& from, const wxColour& to, const wxColour& border)
{
	m_pages->m_colorFromInactive = from;
	m_pages->m_colorToInactive   = to;
	m_pages->m_colorBorderInactive = border;
}

void wxFlatNotebookBase::SetGradientColorFrom(const wxColour& from)
{
	m_pages->m_colorFrom = from;
}

void wxFlatNotebookBase::SetGradientColorTo(const wxColour& to)
{
	m_pages->m_colorTo   = to;
}

void wxFlatNotebookBase::SetGradientColorBorder(const wxColour& border)
{
	m_pages->m_colorBorder = border;
}

void wxFlatNotebookBase::SetUseBackground(bool useBg)
{
	m_pages->SetUseBackground(useBg);
}

void wxFlatNotebookBase::SetTabAreaBackgroundImage(wxBitmap* pTabAreaBackgroundImage)
{
	m_pages->SetTabAreaBackgroundImage(pTabAreaBackgroundImage);
}

/// Gets first gradient colour
const wxColour& wxFlatNotebookBase::GetGradientColorFrom()
{
	return m_pages->m_colorFrom;
}

/// Gets second gradient colour
const wxColour& wxFlatNotebookBase::GetGradientColorTo()
{
	return m_pages->m_colorTo;
}

/// Gets the tab border colour
const wxColour& wxFlatNotebookBase::SetGradientColorBorder()
{
	return m_pages->m_colorBorder;
}

/// Get the active tab text
const wxColour& wxFlatNotebookBase::GetActiveTabTextColour()
{
	return m_pages->m_activeTextColor;
}

void wxFlatNotebookBase::SetPageImageIndex(size_t page, int imgindex)
{
	m_pages->SetPageImageIndex(page, imgindex);
}

int wxFlatNotebookBase::GetPageImageIndex(size_t page)
{
	return m_pages->GetPageImageIndex(page);
}

bool wxFlatNotebookBase::GetEnabled(size_t page)
{
	return m_pages->GetEnabled(page);
}

void wxFlatNotebookBase::Enable(size_t page, bool enabled)
{
	if(page >= m_windows.size())
		return;

	m_windows[page]->Enable(enabled);
	m_pages->Enable(page, enabled);
}

const wxColour& wxFlatNotebookBase::GetNonActiveTabTextColour()
{
	return m_pages->m_nonActiveTextColor;
}

void wxFlatNotebookBase::SetNonActiveTabTextColour(const wxColour& color)
{
	m_pages->m_nonActiveTextColor = color;
}

void wxFlatNotebookBase::SetTabAreaColour(const wxColour& color)
{
	m_pages->m_tabAreaColor = color;
}

const wxColour& wxFlatNotebookBase::GetTabAreaColour()
{
	return m_pages->m_tabAreaColor;
}

void wxFlatNotebookBase::SetActiveTabColour(const wxColour& color)
{
	m_pages->m_activeTabColor = color;
}

const wxColour& wxFlatNotebookBase::GetActiveTabColour()
{
	return m_pages->m_activeTabColor;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 
//	wxPageContainerBase
//
///////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(wxPageContainerBase, wxPanel)
EVT_PAINT(wxPageContainerBase::OnPaint)
EVT_SIZE(wxPageContainerBase::OnSize)
EVT_LEFT_DOWN(wxPageContainerBase::OnLeftDown)
EVT_LEFT_UP(wxPageContainerBase::OnLeftUp)
EVT_RIGHT_DOWN(wxPageContainerBase::OnRightDown)
EVT_MIDDLE_DOWN(wxPageContainerBase::OnMiddleDown)
EVT_MOTION(wxPageContainerBase::OnMouseMove)
EVT_ERASE_BACKGROUND(wxPageContainerBase::OnEraseBackground)
EVT_LEAVE_WINDOW(wxPageContainerBase::OnMouseLeave)
EVT_ENTER_WINDOW(wxPageContainerBase::OnMouseEnterWindow)
END_EVENT_TABLE()

wxPageContainerBase::wxPageContainerBase(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: m_ImageList(NULL) 
, m_iActivePage(-1)
, m_pDropTarget(NULL)
, m_nLeftClickZone(wxFNB_NOWHERE)
{
	m_pRightClickMenu = NULL;
	m_nXButtonStatus = wxFNB_BTN_NONE;
	m_pParent = parent;
	m_nRightButtonStatus = wxFNB_BTN_NONE;
	m_nLeftButtonStatus = wxFNB_BTN_NONE;
	m_nTabXButtonStatus = wxFNB_BTN_NONE;

	m_colorTo = wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	m_colorFrom   = wxColor(*wxWHITE);
	m_colorToInactive = wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	m_colorFromInactive   = wxColor(*wxWHITE);
	m_activeTabColor = wxColor(*wxWHITE);
	m_activeTextColor = wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	m_tabAreaColor = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);

	wxWindow::Create(parent, id, pos, size, style | wxNO_BORDER | wxNO_FULL_REPAINT_ON_RESIZE);

	m_nonActiveTextColor = wxT("GREY");
	m_pDropTarget = new wxFNBDropTarget<wxPageContainerBase>(this, &wxPageContainerBase::OnDropTarget);
	SetDropTarget(m_pDropTarget);

	//bg
	m_useBg = false;
	m_tabBorderWidth = 1;
}

int wxPageContainerBase::GetButtonAreaWidth(void)
{
	int btnareawidth;
	long style = GetParent()->GetWindowStyleFlag();
	btnareawidth = ((wxFlatNotebookBase *)m_pParent)->m_nPadding * 2;

	if ((style & wxFNB_NO_X_BUTTON) == 0)
		btnareawidth += BUTTON_SPACE;

	if ((style & wxFNB_NO_NAV_BUTTONS) == 0)
		btnareawidth += BUTTON_SPACE*2;

	return btnareawidth;
}

wxPageContainerBase::~wxPageContainerBase(void)
{
	if(m_pRightClickMenu)
	{
		delete m_pRightClickMenu;
		m_pRightClickMenu = NULL;
	}
}

void wxPageContainerBase::OnPaint(wxPaintEvent &event)
{
	wxBufferedPaintDC dc(this);
	if(m_pagesInfoVec.empty() || m_nFrom >= (int)m_pagesInfoVec.size())
	{
		Hide();
		event.Skip();
		return;
	}

	// Get the text hight
	int height, width, tabHeight;
	long style = GetParent()->GetWindowStyleFlag();
	wxString stam = wxT("Tp");	// Temp data to get the text height;
	dc.GetTextExtent(stam, &width, &height);

	tabHeight = height + 8; // We use 8 pixels as padding

	// Calculate the number of rows required for drawing the tabs
	wxRect rect = GetClientRect();
	int clientWidth = rect.width;

	// Set the maximum client size
	SetSizeHints(wxSize(GetButtonsAreaLength(), tabHeight));

	// Set brushes, pens and fonts
	wxFont normalFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	wxFont boldFont = normalFont;
	boldFont.SetWeight(wxFONTWEIGHT_BOLD);

	wxPen borderPen = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));

	wxBrush backBrush;
	if(style & wxFNB_VC71)
		backBrush = wxBrush(wxColour(247, 243, 233));
	else 
		backBrush = wxBrush(m_tabAreaColor);

	wxBrush noselBrush = wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	wxBrush selBrush = wxBrush(m_activeTabColor);

	wxSize size = GetSize();

	// Background
	dc.SetTextBackground(style & wxFNB_VC71 ? wxColour(247, 243, 233) : GetBackgroundColour());
	dc.SetTextForeground(m_activeTextColor);
	dc.SetBrush(backBrush);

	// If border style is set, set the pen to be border pen
	if(style & wxFNB_TABS_BORDER_SIMPLE)
		dc.SetPen(borderPen);
	else
		dc.SetPen(*wxTRANSPARENT_PEN);

    ///////////////////////////////////////////////////////////////////////////////

    if (!m_useBg) {
        dc.DrawRectangle(0, 0, size.x, size.y); // draws background around the tabs
	} else {
		if(m_TabAreaBackgroundImage.Ok()) { 
			dc.DrawBitmap(m_TabAreaBackgroundImage, 0, 0); 
		}
	}

    ///////////////////////////////////////////////////////////////////////////////

	// We always draw the bottom/upper line of the tabs
	// regradless the style
	dc.SetPen(borderPen);
	if(!m_useBg){
        DrawTabsLine(dc, GetClientRect()); // draws line around the tabs only if not using background
	}
	

	// Restore the pen
	dc.SetPen(borderPen);

	if(style & wxFNB_VC71 && !(style & wxFNB_BOTTOM))
	{
		wxPen pen = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		pen.SetWidth(4);
		dc.SetPen(pen);
		dc.DrawLine(0, size.y, size.x, size.y);

		// Restore the pen
		dc.SetPen(borderPen);
	}

	// Draw labels
	int pom;
	dc.SetFont(boldFont);
	int posx = ((wxFlatNotebookBase *)m_pParent)->m_nPadding;
	int i = 0;

	// Update all the tabs from 0 to 'm_nFrom' to be non visible
	for(i=0; i<m_nFrom; i++)
	{
		m_pagesInfoVec[i].SetPosition(wxPoint(-1, -1));
		m_pagesInfoVec[i].GetRegion().Clear();
	}

	int shapePoints(0);

	if(style & wxFNB_VC71)
		tabHeight = (style & wxFNB_BOTTOM) ? tabHeight - 4 :  tabHeight;
	else if(style & wxFNB_FANCY_TABS)
		tabHeight = (style & wxFNB_BOTTOM) ? tabHeight - 2 :  tabHeight;

	// Draw the visible tabs
	for(i=m_nFrom; i<(int)m_pagesInfoVec.size(); i++) 
	{
		if(style != wxFNB_VC71)
			shapePoints = (int)(tabHeight*tan((double)m_pagesInfoVec[i].GetTabAngle()/180.0*M_PI));
		else
			shapePoints = 0;

		dc.SetPen(borderPen);
		dc.SetBrush((i==GetSelection()) ? selBrush : noselBrush);

		// Calculate the text length using the bold font, so when selecting a tab
		// its width will not change
		dc.SetFont(boldFont);
		dc.GetTextExtent(GetPageText(i), &width, &pom);

		// Now set the font to the correct font
		dc.SetFont((i==GetSelection()) ? boldFont : normalFont);

		// Set a minimum size to a tab
		if(width < 20)
			width = 20;

		// Add the padding to the tab width
		// Tab width:
		// +-----------------------------------------------------------+
		// | PADDING | IMG | IMG_PADDING | TEXT | PADDING | x |PADDING |
		// +-----------------------------------------------------------+

		int tabWidth = ((wxFlatNotebookBase *)m_pParent)->m_nPadding * 2 + width;
		int imageYCoord = style & wxFNB_BOTTOM ? 3 : 6;
		
		/// Style to add a small 'x' button on the top right
		/// of the tab
		if(style & wxFNB_X_ON_TAB && i == GetSelection())
		{
			/// The xpm image that contains the 'x' button is 9 pixles
			tabWidth += ((wxFlatNotebookBase *)m_pParent)->m_nPadding + 9;
		}

		if(!(style & wxFNB_VC71) && !(style & wxFNB_FANCY_TABS))
			// Default style
			tabWidth += 2 * shapePoints;

		bool hasImage = (m_ImageList != NULL && m_pagesInfoVec[i].GetImageIndex() != -1);

		// For VC71 style, we only add the icon size (16 pixels)
		if(hasImage)
		{
			if( (style & wxFNB_VC71) || (style & wxFNB_FANCY_TABS))
				tabWidth += (16 + ((wxFlatNotebookBase*)m_pParent)->m_nPadding);
			else
				// Default style
				tabWidth += (16 + ((wxFlatNotebookBase*)m_pParent)->m_nPadding) + shapePoints / 2;
		}

		// Check if we can draw more
		if(posx + tabWidth + GetButtonsAreaLength() >= clientWidth)
			break;

		// By default we clean the tab region
		m_pagesInfoVec[i].GetRegion().Clear();
		
		// Clean the 'x' buttn on the tab 
		// 'Clean' rectanlge is a rectangle with width or height
		// with values lower than or equal to 0
		m_pagesInfoVec[i].GetXRect().SetSize(wxSize(-1, -1));

		// Draw the tab
		if(style & wxFNB_FANCY_TABS)
			DrawFancyTab(dc, posx, i, hasImage, tabWidth, tabHeight);
		else if(style & wxFNB_VC71)
			DrawVC71Tab(dc, posx, i, hasImage, tabWidth, tabHeight);
		else
			DrawStandardTab(dc, posx, i, hasImage, tabWidth, tabHeight);


		// Text drawing offset from the left border of the 
		// rectangle
		int textOffset;

		// The width of the images are 16 pixels
		if(hasImage)
			textOffset = ((wxFlatNotebookBase *)m_pParent)->m_nPadding * 2 + 16 + shapePoints / 2 ; 
		else
			textOffset = ((wxFlatNotebookBase *)m_pParent)->m_nPadding + shapePoints / 2 ;

		if(i != GetSelection())
		{
			// Set the text background to be like the vertical lines
			dc.SetTextForeground(m_nonActiveTextColor);
		}

		// Draw the image for the tab if any
#if (defined(__WXMSW__) || defined(__WXMAC__))
		imageYCoord = style & wxFNB_BOTTOM ? 3 : 6;
#else
		imageYCoord = style & wxFNB_BOTTOM ? 3 : 10;
#endif

		if(hasImage)
		{
			int imageXOffset = textOffset - 16 - ((wxFlatNotebookBase *)m_pParent)->m_nPadding;
			dc.DrawBitmap((*m_ImageList)[m_pagesInfoVec[i].GetImageIndex()], 
				posx + imageXOffset, imageYCoord, true);
		}

		dc.DrawText(GetPageText(i), posx + textOffset, imageYCoord);
		
		int textWidth, textHeight;
		dc.GetTextExtent(GetPageText(i), &textWidth, &textHeight);

		int tabCloseButtonXCoord = posx + textOffset + textWidth + ((wxFlatNotebookBase *)m_pParent)->m_nPadding;
		/// From version 1.2 - a style to add 'x' button
		/// on a tab
		if(style & wxFNB_X_ON_TAB)
		{
			int tabCloseButtonYCoord = imageYCoord + 3;
			wxRect x_rect(tabCloseButtonXCoord, tabCloseButtonYCoord, FNB::tab_x_size, FNB::tab_x_size);
			DrawTabX(dc, x_rect, i);	
		}

		// Restore the text forground
		dc.SetTextForeground(m_activeTextColor);

		// Update the tab position & size
		m_pagesInfoVec[i].SetPosition(wxPoint(posx, VERTICAL_BORDER_PADDING));
		m_pagesInfoVec[i].SetSize(wxSize(tabWidth, tabHeight));

		posx += tabWidth;
	}

	// Update all tabs that can not fit into the screen as non-visible
	for(; i<(int)m_pagesInfoVec.size(); i++)
	{
		m_pagesInfoVec[i].SetPosition(wxPoint(-1, -1));
		m_pagesInfoVec[i].GetRegion().Clear();
	}

	if(GetNumOfVisibleTabs() < (int)m_pagesInfoVec.size()){
		long style = GetParent()->GetWindowStyleFlag();
		if(style & wxFNB_NO_NAV_BUTTONS){
			style ^= wxFNB_NO_NAV_BUTTONS;
			GetParent()->SetWindowStyleFlag(style);
		}
	}
	
	// Draw the left/right/close buttons 
	// Left arrow
	DrawLeftArrow(dc);
	DrawRightArrow(dc);
	DrawX(dc);
}

// Tabs drawings
void wxPageContainerBase::DrawFancyTab(wxBufferedPaintDC& dc,
								   const int& posx, 
								   const int &tabIdx, 
								   const bool & /*hasImage*/, 
								   const int &tabWidth, 
								   const int &tabHeight)
{
	// Fancy tabs - like with VC71 but with the following differences:
	// - The Selected tab is colored with gradient color
	wxPen borderPen = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
	wxPen pen = (tabIdx==GetSelection()) ? wxPen(m_colorBorder) : wxPen(m_colorBorderInactive);
	long style = GetParent()->GetWindowStyleFlag();
	dc.SetPen(pen);
	dc.SetBrush((tabIdx==GetSelection()) ? wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)) : wxBrush(wxColour(247, 243, 233)));
	if(tabIdx == GetSelection())
	{
		int posy = (style & wxFNB_BOTTOM) ? 0 : VERTICAL_BORDER_PADDING;

		wxRect rect(posx, posy, tabWidth, tabHeight);
		FillGradientColor(dc, rect);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		pen.SetWidth(m_tabBorderWidth);
		dc.SetPen(pen);

		dc.DrawRectangle(rect);
		pen.SetWidth(1);
		dc.SetPen(pen);
	}
	else
	{
		// We dont draw a rectangle for non selected tabs, but only 
		// vertical line on the left
		//dc.SetPen(borderPen);
		//dc.DrawLine(posx + tabWidth, VERTICAL_BORDER_PADDING + 3, posx + tabWidth, tabHeight - 4);
		int posy = (style & wxFNB_BOTTOM) ? 0 : VERTICAL_BORDER_PADDING;

		wxRect rect(posx, posy, tabWidth, tabHeight);
		FillGradientColorInactive(dc, rect);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		pen.SetWidth(m_tabBorderWidth);
		dc.SetPen(pen);

		dc.DrawRectangle(rect);
		pen.SetWidth(1);
		dc.SetPen(pen);
	}
}

void wxPageContainerBase::DrawVC71Tab(wxBufferedPaintDC& dc, 
								  const int& posx, 
								  const int &tabIdx, 
								  const bool &/*hasImage*/, 
								  const int &tabWidth, 
								  const int &tabHeight)
{
	// Visual studio 7.1 style
	wxPen borderPen = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
	long style = GetParent()->GetWindowStyleFlag();

	dc.SetPen((tabIdx==GetSelection()) ? wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)) : borderPen);
	dc.SetBrush((tabIdx==GetSelection()) ? wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)) : wxBrush(wxColour(247, 243, 233)));

	if(tabIdx == GetSelection())
	{
		int posy = (style & wxFNB_BOTTOM) ? 0 : VERTICAL_BORDER_PADDING;
		dc.DrawRectangle(posx, posy, tabWidth, tabHeight);

		// Draw a black line on the left side of the 
		// rectangle
		wxPen pen = wxPen(*wxBLACK);
		dc.SetPen(pen);

		int blackLineHeight = (style & wxFNB_BOTTOM) ? VERTICAL_BORDER_PADDING + tabHeight - 3 : VERTICAL_BORDER_PADDING + tabHeight - 5;
		dc.DrawLine(posx + tabWidth, (style & wxFNB_BOTTOM) ? 0 : VERTICAL_BORDER_PADDING, 
			posx + tabWidth, blackLineHeight - 1);

		// To give the tab more 3D look we do the following
		// Incase the tab is on top, 
		// Draw a thik white line on topof the rectangle
		// Otherwise, draw a thin (1 pixel) black line at the bottom

		pen = wxPen((style & wxFNB_BOTTOM) ? *wxBLACK : *wxWHITE);
		pen.SetWidth((style & wxFNB_BOTTOM) ? 1 : 2);
		dc.SetPen(pen);
		int whiteLinePosY = (style & wxFNB_BOTTOM) ? blackLineHeight - 1: VERTICAL_BORDER_PADDING ;
		dc.DrawLine(posx , whiteLinePosY, posx + tabWidth, whiteLinePosY);
	}
	else
	{
		// We dont draw a rectangle for non selected tabs, but only 
		// vertical line on the left
		int blackLineHeight = (style & wxFNB_BOTTOM) ? tabHeight - 5 : VERTICAL_BORDER_PADDING + tabHeight - 8;
		dc.DrawLine(posx + tabWidth, (style & wxFNB_BOTTOM) ? 3 : VERTICAL_BORDER_PADDING + 1, posx + tabWidth, blackLineHeight + 1);
	}
}


void wxPageContainerBase::DrawStandardTab(wxBufferedPaintDC& dc, 
									  const int& posx, 
									  const int &tabIdx, 
									  const bool &/*hasImage*/, 
									  const int &tabWidth, 
									  const int &tabHeight)
{
	// Default style
	wxPen borderPen = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
	long style = GetParent()->GetWindowStyleFlag();

	wxPoint tabPoints[7];
	tabPoints[0].x = posx;
	tabPoints[0].y = (style & wxFNB_BOTTOM) ? 0 : tabHeight;

	tabPoints[1].x = (int)(posx+(tabHeight-2)*tan((double)m_pagesInfoVec[tabIdx].GetTabAngle()/180.0*M_PI));
	tabPoints[1].y = (style & wxFNB_BOTTOM) ? tabHeight - (VERTICAL_BORDER_PADDING+2) : (VERTICAL_BORDER_PADDING+2);

	tabPoints[2].x = tabPoints[1].x+2;
	tabPoints[2].y = (style & wxFNB_BOTTOM) ? tabHeight - VERTICAL_BORDER_PADDING : VERTICAL_BORDER_PADDING;

	tabPoints[3].x = (int)(posx+tabWidth-(tabHeight-2)*tan((double)m_pagesInfoVec[tabIdx].GetTabAngle()/180.0*M_PI))-2;
	tabPoints[3].y = (style & wxFNB_BOTTOM) ? tabHeight - VERTICAL_BORDER_PADDING : VERTICAL_BORDER_PADDING;

	tabPoints[4].x = tabPoints[3].x+2;
	tabPoints[4].y = (style & wxFNB_BOTTOM) ? tabHeight - (VERTICAL_BORDER_PADDING+2) : (VERTICAL_BORDER_PADDING+2);

	tabPoints[5].x = posx+tabWidth;
	tabPoints[5].y = (style & wxFNB_BOTTOM) ? 0 : tabHeight;

	tabPoints[6].x = tabPoints[0].x;
	tabPoints[6].y = tabPoints[0].y;

	if(tabIdx == GetSelection())
	{
		// Draw the tab as rounded rectangle
		dc.DrawPolygon(7, tabPoints);
	}
	else
	{
		if(tabIdx != GetSelection() - 1)
		{
			// Draw a vertical line to the right of the text
			int pt1x, pt1y, pt2x, pt2y;
			pt1x = tabPoints[5].x;
			pt1y = (style & wxFNB_BOTTOM) ? 4 : tabHeight - 4;
			pt2x = tabPoints[5].x;
			pt2y = (style & wxFNB_BOTTOM) ? tabHeight - 4 : 4 ;
			dc.DrawLine(pt1x, pt1y, pt2x, pt2y);
		}
	}

	if(style & wxFNB_BOTTOM && tabIdx == GetSelection())
	{
		wxPen savePen = dc.GetPen();
		wxPen whitePen = wxPen(*wxWHITE);
		whitePen.SetWidth(1);
		dc.SetPen(whitePen);

		dc.DrawLine(tabPoints[0], tabPoints[5]);

		// Restore the pen
		dc.SetPen(savePen);
	}
}

void wxPageContainerBase::AddPage(const wxString& caption, const bool selected, const int imgindex)
{
	if(selected)
	{
		m_iActivePage = (int)m_pagesInfoVec.size();
	}
	m_pagesInfoVec.push_back(wxPageInfo(caption, imgindex));
	Refresh();
}

bool wxPageContainerBase::InsertPage(size_t index, wxWindow* /*page*/, const wxString& text, bool select, const int imgindex)
{
	if(select)
	{
		m_iActivePage = (int)m_pagesInfoVec.size();
	}
	std::vector<wxPageInfo>::iterator iter = m_pagesInfoVec.begin() + index;
	m_pagesInfoVec.insert(iter, wxPageInfo(text, imgindex));
	Refresh();
	return true;
}

void wxPageContainerBase::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Refresh(); // Call on paint
}

void wxPageContainerBase::OnMiddleDown(wxMouseEvent& event)
{
	// Test if this style is enabled
	long style = GetParent()->GetWindowStyleFlag();
	if(!(style & wxFNB_MOUSE_MIDDLE_CLOSES_TABS))
		return;

	wxPageInfo pgInfo;
	int tabIdx;
	int where = HitTest(event.GetPosition(), pgInfo, tabIdx);
	switch(where)
	{
	case wxFNB_TAB:
		{
			DeletePage((size_t)tabIdx);
			break;
		}
	default:
		break;
	}
	event.Skip();
}

void wxPageContainerBase::OnRightDown(wxMouseEvent& event)
{
	if(m_pRightClickMenu)
	{
		wxPageInfo pgInfo;
		int tabIdx;
		int where = HitTest(event.GetPosition(), pgInfo, tabIdx);
		switch(where)
		{
		case wxFNB_TAB:
		case wxFNB_TAB_X:
			{
				if(!m_pagesInfoVec[tabIdx].GetEnabled())
					break;

				// Set the current tab to be active
				SetSelection((size_t)tabIdx);

				// If the owner has defined a context menu for the tabs,
				// popup the right click menu
				if (m_pRightClickMenu)
					PopupMenu(m_pRightClickMenu);
				else
				{
					// send a message to popup a custom menu
					wxFlatNotebookEvent event(wxEVT_COMMAND_FLATNOTEBOOK_CONTEXT_MENU, GetParent()->GetId());
					event.SetSelection((int)tabIdx);
					event.SetOldSelection((int)m_iActivePage);
					event.SetEventObject(GetParent());
					GetParent()->GetEventHandler()->ProcessEvent(event);
				}
			}
			break;
		default:
			break;
		}
	}
	event.Skip();
}

void wxPageContainerBase::OnLeftDown(wxMouseEvent& event)
{
	wxPageInfo pgInfo;
	int tabIdx;

	// Reset buttons status
	m_nXButtonStatus     = wxFNB_BTN_NONE;
	m_nLeftButtonStatus  = wxFNB_BTN_NONE;
	m_nRightButtonStatus = wxFNB_BTN_NONE;	
	m_nTabXButtonStatus  = wxFNB_BTN_NONE;

	m_nLeftClickZone = HitTest(event.GetPosition(), pgInfo, tabIdx);
	switch(m_nLeftClickZone)
	{
	case wxFNB_LEFT_ARROW:
		m_nLeftButtonStatus = wxFNB_BTN_PRESSED;
		Refresh();
		break;
	case wxFNB_RIGHT_ARROW:
		m_nRightButtonStatus = wxFNB_BTN_PRESSED;
		Refresh();
		break;
	case wxFNB_X:
		m_nXButtonStatus = wxFNB_BTN_PRESSED;
		Refresh();
		break;
	case wxFNB_TAB_X:
		m_nTabXButtonStatus = wxFNB_BTN_PRESSED;
		Refresh();
		break;
	case wxFNB_TAB:
		{
			if(m_iActivePage != tabIdx)
			{
				// Incase the tab is disabled, we dont allow to choose it
				if(!m_pagesInfoVec[tabIdx].GetEnabled())
					break;

				int oldSelection = m_iActivePage;

				wxFlatNotebookEvent event(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CHANGING, GetParent()->GetId());
				event.SetSelection((int)tabIdx);
				event.SetOldSelection((int)oldSelection);
				event.SetEventObject(GetParent());
				if(!GetParent()->GetEventHandler()->ProcessEvent(event) || event.IsAllowed())
				{
					SetSelection(tabIdx);

					// Fire a wxEVT_COMMAND_TABBEDCTRL_PAGE_CHANGED event
					event.SetEventType(wxEVT_COMMAND_FLATNOTEBOOK_PAGE_CHANGED);
					event.SetOldSelection((int)oldSelection);
					GetParent()->GetEventHandler()->ProcessEvent(event);
				}
			}
			break;
		}
	}
}

void wxPageContainerBase::OnLeftUp(wxMouseEvent& event)
{
	wxPageInfo pgInfo;
	int tabIdx;

	// forget the zone that was initially clicked
	m_nLeftClickZone = wxFNB_NOWHERE;

	int where = HitTest(event.GetPosition(), pgInfo, tabIdx);
	switch(where)
	{
	case wxFNB_LEFT_ARROW:
		{
			if(m_nFrom == 0)
				break;

			// Make sure that the button was pressed before 
			if(m_nLeftButtonStatus != wxFNB_BTN_PRESSED)
				break;

			m_nLeftButtonStatus = wxFNB_BTN_HOVER;

			// We scroll left with bulks of 5
			int scrollLeft = GetNumTabsCanScrollLeft();

			m_nFrom -= scrollLeft;
			if(m_nFrom < 0)
				m_nFrom = 0;

			Refresh();
			break;
		}
	case wxFNB_RIGHT_ARROW:
		{
			if(m_nFrom >= (int)m_pagesInfoVec.size() - 1)
				break;

			// Make sure that the button was pressed before 
			if(m_nRightButtonStatus != wxFNB_BTN_PRESSED)
				break;

			m_nRightButtonStatus = wxFNB_BTN_HOVER;

			// Check if the right most tab is visible, if it is
			// don't rotate right anymore
			if(m_pagesInfoVec[m_pagesInfoVec.size()-1].GetPosition() != wxPoint(-1, -1))
				break;

			int lastVisibleTab = GetLastVisibleTab();
			if(lastVisibleTab < 0)
			{
				// Probably the screen is too small for displaying even a single 
				// tab, in this case we do nothing
				break;
			}

			m_nFrom += GetNumOfVisibleTabs();
			Refresh();
			break;
		}
	case wxFNB_X:
		{
			// Make sure that the button was pressed before 
			if(m_nXButtonStatus != wxFNB_BTN_PRESSED)
				break;

			m_nXButtonStatus = wxFNB_BTN_HOVER;

			DeletePage((size_t)m_iActivePage);
			break;
		}
	case wxFNB_TAB_X:
		{
			// Make sure that the button was pressed before 
			if(m_nTabXButtonStatus != wxFNB_BTN_PRESSED)
				break;

			m_nTabXButtonStatus = wxFNB_BTN_HOVER;

			DeletePage((size_t)m_iActivePage);
			break;
		}

	}
}

int wxPageContainerBase::HitTest(const wxPoint& pt, wxPageInfo& pageInfo, int &tabIdx)
{
	wxRect rect = GetClientRect();
	int btnLeftPos = GetLeftButtonPos();
	int btnRightPos = GetRightButtonPos();
	int btnXPos = GetXPos();
	long style = GetParent()->GetWindowStyleFlag();
	tabIdx = -1;
	if(m_pagesInfoVec.empty())
	{
		return wxFNB_NOWHERE;
	}

	rect = wxRect(btnXPos, 5, 12, 12);
    if(rect.Contains(pt))
	{
		return (style & wxFNB_NO_X_BUTTON) ? wxFNB_NOWHERE : wxFNB_X;
	}

	rect = wxRect(btnRightPos, 5, 12, 12);
	if(rect.Contains(pt))
	{
		return (style & wxFNB_NO_NAV_BUTTONS) ? wxFNB_NOWHERE : wxFNB_RIGHT_ARROW;
	}
	rect = wxRect(btnLeftPos, 5, 12, 12);
	if(rect.Contains(pt))
	{
		return (style & wxFNB_NO_NAV_BUTTONS) ? wxFNB_NOWHERE : wxFNB_LEFT_ARROW;
	}

	// Test whether a left click was made on a tab
	bool bFoundMatch = false;
	for(size_t cur=m_nFrom; cur<m_pagesInfoVec.size(); cur++)
	{
		wxPageInfo pgInfo = m_pagesInfoVec[cur];
		if(pgInfo.GetPosition() == wxPoint(-1, -1))
			continue;
		if(style & wxFNB_X_ON_TAB && (int)cur == GetSelection())
		{
			// 'x' button exists on a tab
			if(m_pagesInfoVec[cur].GetXRect().Contains(pt))
			{
				pageInfo = pgInfo;
				tabIdx = (int)cur;
				return wxFNB_TAB_X;
			}
		}

		wxRect tabRect = wxRect(pgInfo.GetPosition().x, pgInfo.GetPosition().y, 
			pgInfo.GetSize().x, pgInfo.GetSize().y);
		if(tabRect.Contains(pt))
		{
			// We have a match
			// wxGetApp().SafeMessageBox(pgInfo.m_strCaption);
			pageInfo = pgInfo;
			tabIdx = (int)cur;
			return wxFNB_TAB;
		}
	}

	if(bFoundMatch)
		return wxFNB_TAB;

	// Default
	return wxFNB_NOWHERE;
}

void wxPageContainerBase::SetSelection(size_t page)
{
	wxFlatNotebookBase* book = (wxFlatNotebookBase*)GetParent();
	book->SetSelection(page);
	DoSetSelection(page);
}

void wxPageContainerBase::DoSetSelection(size_t page)
{
	// Make sure that the selection is visible
	long style = GetParent()->GetWindowStyleFlag();
	if(style & wxFNB_NO_NAV_BUTTONS)
	{
		// Incase that we dont have navigation buttons, 
		// there is no point of checking if the tab is visible
		// Just do the refresh
		Refresh();
		return;
	}

	if(page < m_pagesInfoVec.size())
	{
		//! fix for tabfocus
		wxWindow* da_page = ((wxFlatNotebookBase *)m_pParent)->GetPage(page);
		if ( da_page!=NULL )
			da_page->SetFocus();
	}

	if(!IsTabVisible(page))
	{
		if(page == m_pagesInfoVec.size() - 1)
		{
			// Incase the added tab is last, 
			// the function IsTabVisible() will always return false
			// and thus will cause an evil behaviour that the new
			// tab will hide all other tabs, we need to check if the 
			// new selected tab can fit to the current screen
			if(!CanFitToScreen(page))
			{
				m_nFrom = (int)page;
			}
			Refresh();
		}
		else
		{
			// Redraw the tabs starting from page
			m_nFrom = (int)page;
			Refresh();
		}
	}
}

void wxPageContainerBase::DeletePage(size_t page)
{
	wxFlatNotebookBase* book = (wxFlatNotebookBase*)GetParent();
	book->DeletePage(page);
	book->Refresh();
}

bool wxPageContainerBase::IsTabVisible(size_t page)
{
	int iPage = (int)page;
	int iLastVisiblePage = GetLastVisibleTab();

	return iPage <= iLastVisiblePage && iPage >= m_nFrom;
}

void wxPageContainerBase::DoDeletePage(size_t page)
{
	// Remove the page from the vector
	wxFlatNotebookBase* book = (wxFlatNotebookBase*)GetParent();
	std::vector<wxPageInfo>::iterator iter = m_pagesInfoVec.begin();
	std::vector<wxPageInfo>::iterator endIter = m_pagesInfoVec.end();

	m_pagesInfoVec.erase(iter + page);
	
	// Thanks to Yiaanis AKA Mandrav
	if (m_iActivePage >= (int)page)
		m_iActivePage--; 

	// The delete page was the last first on the array, 
	// but the book still has more pages, so we set the 
	// active page to be the first one (0)
	if(m_iActivePage < 0 && !m_pagesInfoVec.empty())
		m_iActivePage = 0;

	// Refresh the tabs
	if(m_iActivePage >= 0)
	{
		book->m_bForceSelection = true;
		book->SetSelection(m_iActivePage);
		book->m_bForceSelection = false;
	}

	if(m_pagesInfoVec.empty())
	{	
		// Erase the page container drawings
		wxClientDC dc(this);
		dc.Clear();
	}
}

void wxPageContainerBase::DeleteAllPages()
{
	m_iActivePage = -1;
	m_nFrom = 0;
	m_pagesInfoVec.clear();

	// Erase the page container drawings
	wxClientDC dc(this);
	dc.Clear();
}

void wxPageContainerBase::DrawTabX(wxDC& dc, const wxRect& rect, const int& tabIdx)
{
	long style = GetParent()->GetWindowStyleFlag();
	if(!(style & wxFNB_X_ON_TAB) || !CanDrawXOnTab())
		return;

	/// We draw the 'x' on the active tab only
	if(tabIdx != GetSelection() || tabIdx < 0)
		return;

	// Set the bitmap according to the button status
	wxBitmap xBmp;
	switch(m_nTabXButtonStatus)
	{
	case wxFNB_BTN_HOVER:
		xBmp = wxBitmap(FNB::tab_x_button_hilite_xpm);
		break;
	case wxFNB_BTN_PRESSED:
		xBmp = wxBitmap(FNB::tab_x_button_pressed_xpm);
		break;
	case wxFNB_BTN_NONE:
	default:
		xBmp = wxBitmap(FNB::tab_x_button_xpm);
		break;
	}

	/// Set the masking
	xBmp.SetMask(new wxMask(xBmp, MASK_COLOR));

	// Draw the new bitmap
	dc.DrawBitmap(xBmp, rect.x, rect.y, true);

	// Update the vectpr
	m_pagesInfoVec[tabIdx].SetXRect(rect);
}

void wxPageContainerBase::DrawLeftArrow(wxDC& dc)
{
	long style = GetParent()->GetWindowStyleFlag();
	if(style & wxFNB_NO_NAV_BUTTONS)
		return;

	// Make sure that there are pages in the container
	if(m_pagesInfoVec.empty())
		return;

	wxRect rect = GetClientRect();
	int btnLeftPos = GetLeftButtonPos();
	rect = wxRect(btnLeftPos, 5, 16, 16);

	// Set the bitmap according to the button status
	wxBitmap arrowBmp;
	switch(m_nLeftButtonStatus)
	{
	case wxFNB_BTN_HOVER:
		arrowBmp = wxBitmap(FNB::left_arrow_hilite_xpm);
		break;
	case wxFNB_BTN_PRESSED:
		arrowBmp = wxBitmap(FNB::left_arrow_pressed_xpm);
		break;
	case wxFNB_BTN_NONE:
	default:
		arrowBmp = wxBitmap(FNB::left_arrow_xpm);
		break;
	}

	if(m_nFrom == 0)
	{
		// Handle disabled arrow
		arrowBmp = wxBitmap(FNB::left_arrow_disabled_xpm);
	}

	arrowBmp.SetMask(new wxMask(arrowBmp, MASK_COLOR));

	// Erase old bitmap
	wxPen pen = wxPen(style & wxFNB_VC71 ? wxColour(247, 243, 233) : m_tabAreaColor);
	wxBrush brush = wxBrush(style & wxFNB_VC71 ? wxColour(247, 243, 233) :m_tabAreaColor);
	pen.SetWidth(1);
	dc.SetPen(pen);
	dc.SetBrush(brush);

	dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

	// Draw the new bitmap
	dc.DrawBitmap(arrowBmp, btnLeftPos, 5, true);
}

void wxPageContainerBase::DrawRightArrow(wxDC& dc)
{
	long style = GetParent()->GetWindowStyleFlag();
	if(style & wxFNB_NO_NAV_BUTTONS)
		return;

	// Make sure that there are pages in the container
	if(m_pagesInfoVec.empty())
		return;

	wxRect rect = GetClientRect();
	int btnLeftPos = GetRightButtonPos();
	rect = wxRect(btnLeftPos, 5, 16, 16);

	// Set the bitmap according to the button status
	wxBitmap arrowBmp;
	switch(m_nRightButtonStatus)
	{
	case wxFNB_BTN_HOVER:
		arrowBmp = wxBitmap(FNB::right_arrow_hilite_xpm);
		break;
	case wxFNB_BTN_PRESSED:
		arrowBmp = wxBitmap(FNB::right_arrow_pressed_xpm);
		break;
	case wxFNB_BTN_NONE:
	default:
		arrowBmp = wxBitmap(FNB::right_arrow_xpm);
		break;
	}


	// Check if the right most tab is visible, if it is
	// don't rotate right anymore
	if(m_pagesInfoVec[m_pagesInfoVec.size()-1].GetPosition() != wxPoint(-1, -1))
	{
		arrowBmp = wxBitmap(FNB::right_arrow_disabled_xpm);
	}

	arrowBmp.SetMask(new wxMask(arrowBmp, MASK_COLOR));

	// Erase old bitmap
	wxPen pen = wxPen(style & wxFNB_VC71 ? wxColour(247, 243, 233) : m_tabAreaColor);
	wxBrush brush = wxBrush(style & wxFNB_VC71 ? wxColour(247, 243, 233) :m_tabAreaColor);
	pen.SetWidth(1);
	dc.SetPen(pen);
	dc.SetBrush(brush);

	dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

	// Draw the new bitmap
	dc.DrawBitmap(arrowBmp, btnLeftPos, 5, true);
} 

void wxPageContainerBase::DrawX(wxDC& dc)
{
	// Check if this style is enabled
	long style = GetParent()->GetWindowStyleFlag();
	if(style & wxFNB_NO_X_BUTTON)
		return;

	// Make sure that there are pages in the container
	if(m_pagesInfoVec.empty())
		return;

	wxRect rect = GetClientRect();
	int btnLeftPos = GetXPos();
	rect = wxRect(btnLeftPos, 5, 16, 16);

	// Set the bitmap according to the button status
	wxBitmap xbmp;
	switch(m_nXButtonStatus)
	{
	case wxFNB_BTN_HOVER:
		xbmp = wxBitmap(FNB::x_button_hilite_xpm);
		break;
	case wxFNB_BTN_PRESSED:
		xbmp = wxBitmap(FNB::x_button_pressed_xpm);
		break;
	case wxFNB_BTN_NONE:
	default:
		xbmp = wxBitmap(FNB::x_button_xpm);
		break;
	}

	xbmp.SetMask(new wxMask(xbmp, MASK_COLOR));

	// Erase old bitmap
	wxPen pen = wxPen(style & wxFNB_VC71 ? wxColour(247, 243, 233) : m_tabAreaColor);
	wxBrush brush = wxBrush(style & wxFNB_VC71 ? wxColour(247, 243, 233) : m_tabAreaColor);
	pen.SetWidth(1);
	dc.SetPen(pen);
	dc.SetBrush(brush);
	dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

	// Draw the new bitmap
	dc.DrawBitmap(xbmp, btnLeftPos, 5, true);
}

void wxPageContainerBase::OnMouseMove(wxMouseEvent& event)
{
	if (!m_pagesInfoVec.empty() && IsShown())
	{
		const int xButtonStatus = m_nXButtonStatus;
		const int xTabButtonStatus = m_nTabXButtonStatus;
		const int rightButtonStatus = m_nRightButtonStatus;
		const int leftButtonStatus = m_nLeftButtonStatus;
		long style = GetParent()->GetWindowStyleFlag();

		m_nXButtonStatus = wxFNB_BTN_NONE;
		m_nRightButtonStatus = wxFNB_BTN_NONE;
		m_nLeftButtonStatus = wxFNB_BTN_NONE;
		m_nTabXButtonStatus = wxFNB_BTN_NONE;

		wxPageInfo pgInfo;
		int tabIdx;

		switch(HitTest(event.GetPosition(), pgInfo, tabIdx))
		{
		case wxFNB_X:
			if (event.LeftIsDown())
			{
				m_nXButtonStatus = (m_nLeftClickZone==wxFNB_X) ? wxFNB_BTN_PRESSED : wxFNB_BTN_NONE;
			}
			else
			{
				m_nXButtonStatus = wxFNB_BTN_HOVER;
			}
			break;
		case wxFNB_TAB_X:
			if (event.LeftIsDown())
			{
				m_nTabXButtonStatus = (m_nLeftClickZone==wxFNB_TAB_X) ? wxFNB_BTN_PRESSED : wxFNB_BTN_NONE;
			}
			else
			{
				m_nTabXButtonStatus = wxFNB_BTN_HOVER;
			}
			break;
		case wxFNB_RIGHT_ARROW:
			if (event.LeftIsDown())
			{
				m_nRightButtonStatus = (m_nLeftClickZone==wxFNB_RIGHT_ARROW) ? wxFNB_BTN_PRESSED : wxFNB_BTN_NONE;
			}
			else
			{
				m_nRightButtonStatus = wxFNB_BTN_HOVER;
			}
			break;

		case wxFNB_LEFT_ARROW:
			if (event.LeftIsDown())
			{
				m_nLeftButtonStatus = (m_nLeftClickZone==wxFNB_LEFT_ARROW) ? wxFNB_BTN_PRESSED : wxFNB_BTN_NONE;
			}
			else
			{
				m_nLeftButtonStatus = wxFNB_BTN_HOVER;
			}
			break;

		case wxFNB_TAB:
			// Call virtual method for showing tooltip
			ShowTabTooltip(tabIdx);
			if(!GetEnabled((size_t)tabIdx))
			{
				// Set the cursor to be 'No-entry'
				::wxSetCursor(wxCURSOR_NO_ENTRY);
			}
			if(event.LeftIsDown() && !(style & wxFNB_NODRAG))
			{
				wxFNBDragInfo draginfo(this, tabIdx);
				wxCustomDataObject dataobject(wxDataFormat(wxT("wxFNB")));
				dataobject.SetData(sizeof(wxFNBDragInfo), &draginfo);
				wxDropSource dragSource(this);
				dragSource.SetData(dataobject);
				dragSource.DoDragDrop(wxDrag_DefaultMove);
			}
			break;
		}

		const bool bRedrawX = m_nXButtonStatus != xButtonStatus;
		const bool bRedrawRight = m_nRightButtonStatus != rightButtonStatus;
		const bool bRedrawLeft = m_nLeftButtonStatus != leftButtonStatus;
		const bool bRedrawTabX = m_nTabXButtonStatus != xTabButtonStatus;

		if (bRedrawX || bRedrawRight || bRedrawLeft || bRedrawTabX)
		{
			wxClientDC dc(this);
			if (bRedrawX)
			{
				DrawX(dc);
			}
			if (bRedrawLeft)
			{
				DrawLeftArrow(dc);
			}
			if (bRedrawRight)
			{
				DrawRightArrow(dc);
			}
			if(bRedrawTabX)
			{
				DrawTabX(dc, pgInfo.GetXRect(), tabIdx);
			}
		}
	}
	event.Skip();
}

int wxPageContainerBase::GetLastVisibleTab()
{
	int i;
	for(i=m_nFrom; i<(int)m_pagesInfoVec.size(); i++)
	{
		if(m_pagesInfoVec[i].GetPosition() == wxPoint(-1, -1))
			break;
	}
	return (i-1);
}

int wxPageContainerBase::GetNumTabsCanScrollLeft()
{
	int i;

	// Reserved area for the buttons (<>x)
	wxRect rect = GetClientRect();
	int clientWidth = rect.width;
	int posx = ((wxFlatNotebookBase *)m_pParent)->m_nPadding, numTabs = 0, pom = 0, width, shapePoints,
		height, tabHeight, tabWidth;

	wxClientDC dc(this);

	// Incase we have error prevent crash
	if(m_nFrom < 0)
		return 0;

	long style = GetParent()->GetWindowStyleFlag();
	for(i=m_nFrom; i>=0; i--)
	{
		wxFont boldFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
		boldFont.SetWeight(wxFONTWEIGHT_BOLD);
		dc.SetFont(boldFont);

		wxString stam = wxT("Tp");	// Temp data to get the text height;
		dc.GetTextExtent(stam, &width, &height);

		tabHeight = height + 8; // We use 6 pixels as padding
		if(style & wxFNB_VC71)
			tabHeight = (style & wxFNB_BOTTOM) ? tabHeight - 4 :  tabHeight;
		else if(style & wxFNB_FANCY_TABS)
			tabHeight = (style & wxFNB_BOTTOM) ? tabHeight - 3 :  tabHeight;

		dc.GetTextExtent(GetPageText(i), &width, &pom);
		if(style != wxFNB_VC71)
			shapePoints = (int)(tabHeight*tan((double)m_pagesInfoVec[i].GetTabAngle()/180.0*M_PI));
		else
			shapePoints = 0;

		tabWidth = ((wxFlatNotebookBase *)m_pParent)->m_nPadding * 2 + width;
		if(!(style & wxFNB_VC71))
			// Default style
			tabWidth += 2 * shapePoints;

		bool hasImage = (m_ImageList != NULL && m_pagesInfoVec[i].GetImageIndex() != -1);

		// For VC71 style, we only add the icon size (16 pixels)
		if(hasImage)
		{
			if( !IsDefaultTabs() )
				tabWidth += (16 + ((wxFlatNotebookBase*)m_pParent)->m_nPadding);
			else
				// Default style
				tabWidth += (16 + ((wxFlatNotebookBase*)m_pParent)->m_nPadding) + shapePoints / 2;
		}

		if(posx + tabWidth + GetButtonsAreaLength() >= clientWidth)
			break;

		numTabs++;
		posx += tabWidth;
	}
	return numTabs;
}

bool wxPageContainerBase::IsDefaultTabs()
{
	long style = GetParent()->GetWindowStyleFlag();
	bool res = (style & wxFNB_VC71) || (style & wxFNB_FANCY_TABS);
	return !res;
}

void wxPageContainerBase::AdvanceSelection(bool bForward)
{
	int nSel = GetSelection();

	if(nSel < 0)
		return;

	int nMax = (int)GetPageCount() - 1;
	if ( bForward )
		SetSelection(nSel == nMax ? 0 : nSel + 1);
	else
		SetSelection(nSel == 0 ? nMax : nSel - 1);
}


void wxPageContainerBase::OnMouseLeave(wxMouseEvent& event)
{
	m_nLeftButtonStatus = wxFNB_BTN_NONE;
	m_nXButtonStatus = wxFNB_BTN_NONE;
	m_nRightButtonStatus = wxFNB_BTN_NONE;
	m_nTabXButtonStatus = wxFNB_BTN_NONE;

	wxClientDC dc(this);

	DrawX(dc);
	DrawLeftArrow(dc);
	DrawRightArrow(dc);
	DrawTabX(dc, m_pagesInfoVec[GetSelection()].GetXRect(), GetSelection());

    event.Skip();
}

void wxPageContainerBase::OnMouseEnterWindow(wxMouseEvent& event)
{
	m_nLeftButtonStatus = wxFNB_BTN_NONE;
	m_nXButtonStatus = wxFNB_BTN_NONE;
	m_nRightButtonStatus = wxFNB_BTN_NONE;
	m_nLeftClickZone = wxFNB_BTN_NONE;

	event.Skip();
}

void wxPageContainerBase::ShowTabTooltip(int tabIdx)
{
	wxWindow *pWindow = ((wxFlatNotebookBase *)m_pParent)->GetPage(tabIdx);
	wxToolTip *pToolTip = pWindow->GetToolTip();
	if(pToolTip && pToolTip->GetWindow() == pWindow)
		SetToolTip(pToolTip->GetTip());
}

void wxPageContainerBase::FillGradientColor(wxBufferedDC& dc, const wxRect& rect)
{    
	// gradient fill from colour 1 to colour 2 with top to bottom

	if(rect.height < 1 || rect.width < 1)
		return;

	int size = rect.height;

	// calculate gradient coefficients
	long style = GetParent()->GetWindowStyleFlag();
	wxColour col2 = (style & wxFNB_BOTTOM) ? m_colorTo : m_colorFrom;
	wxColour col1 = (style & wxFNB_BOTTOM) ? m_colorFrom : m_colorTo;

	double rstep = double((col2.Red() -   col1.Red())) / double(size), rf = 0,
		gstep = double((col2.Green() - col1.Green())) / double(size), gf = 0,
		bstep = double((col2.Blue() -  col1.Blue())) / double(size), bf = 0;

	wxColour currCol;
	for(int y = rect.y; y < rect.y + size; y++)
	{
		currCol.Set(
			(unsigned char)(col1.Red() + rf),
			(unsigned char)(col1.Green() + gf),
			(unsigned char)(col1.Blue() + bf)
			);
		dc.SetBrush( wxBrush( currCol, wxSOLID ) );
		dc.SetPen(wxPen(currCol));
		dc.DrawLine(rect.x, y, rect.x + rect.width, y);
		rf += rstep; gf += gstep; bf += bstep;
	}
}

void wxPageContainerBase::FillGradientColorInactive(wxBufferedDC& dc, const wxRect& rect)
{    
	// gradient fill from colour 1 to colour 2 with top to bottom

	if(rect.height < 1 || rect.width < 1)
		return;

	int size = rect.height;

	// calculate gradient coefficients
	long style = GetParent()->GetWindowStyleFlag();
	wxColour col2 = (style & wxFNB_BOTTOM) ? m_colorToInactive : m_colorFromInactive;
	wxColour col1 = (style & wxFNB_BOTTOM) ? m_colorFromInactive : m_colorToInactive;

	double rstep = double((col2.Red() -   col1.Red())) / double(size), rf = 0,
		gstep = double((col2.Green() - col1.Green())) / double(size), gf = 0,
		bstep = double((col2.Blue() -  col1.Blue())) / double(size), bf = 0;

	wxColour currCol;
	for(int y = rect.y; y < rect.y + size; y++)
	{
		currCol.Set(
			(unsigned char)(col1.Red() + rf),
			(unsigned char)(col1.Green() + gf),
			(unsigned char)(col1.Blue() + bf)
			);
		dc.SetBrush( wxBrush( currCol, wxSOLID ) );
		dc.SetPen(wxPen(currCol));
		dc.DrawLine(rect.x, y, rect.x + rect.width, y);
		rf += rstep; gf += gstep; bf += bstep;
	}
}

void wxPageContainerBase::SetPageImageIndex(size_t page, int imgindex)
{
	if(page < m_pagesInfoVec.size())
	{
		m_pagesInfoVec[page].SetImageIndex(imgindex);
		Refresh();
	}
}
void wxPageContainerBase::SetUseBackground(bool useBg)
{
	m_useBg = useBg;
}

void wxPageContainerBase::SetTabAreaBackgroundImage(wxBitmap* pTabAreaBackgroundImage)
{
	m_TabAreaBackgroundImage = wxBitmap(*pTabAreaBackgroundImage);
}

int wxPageContainerBase::GetPageImageIndex(size_t page)
{
	if(page < m_pagesInfoVec.size())
	{
		return m_pagesInfoVec[page].GetImageIndex();
	}
	return -1;
}

wxDragResult wxPageContainerBase::OnDropTarget(wxCoord x, wxCoord y, int nTabPage, wxWindow * wnd_oldContainer)
{
	// Disable drag'n'drop for disabled tab
	if(!((wxPageContainerBase *)wnd_oldContainer)->m_pagesInfoVec[nTabPage].GetEnabled())
		return wxDragCancel;

	wxLogTrace(wxTraceMask(), _("Old Page Index = %i"), nTabPage);
	wxPageContainerBase * oldContainer = (wxPageContainerBase *)wnd_oldContainer;
	int nIndex = -1;
	wxPageInfo pgInfo;
	int where = HitTest(wxPoint(x, y), pgInfo, nIndex);
	wxLogTrace(wxTraceMask(), _("OnDropTarget: index by HitTest = %i"), nIndex);
	wxFlatNotebookBase * oldNotebook = (wxFlatNotebookBase *)oldContainer->GetParent();
	wxFlatNotebookBase * newNotebook = (wxFlatNotebookBase *)GetParent();	

	if(oldNotebook == newNotebook)
	{
		if(nTabPage >= 0)
		{
			switch(where) 
			{
			case wxFNB_TAB:
				MoveTabPage(nTabPage, nIndex);
				break;
			case wxFNB_NOWHERE:
				MoveTabPage(nTabPage, GetLastVisibleTab()+1);
				break;
			default:
				break;
			}
		}
	}
	else
	{
#if defined(__WXMSW__) || defined(__WXGTK__)
		if(nTabPage >= 0)
		{				
			wxWindow * window = oldNotebook->GetPage(nTabPage);
			if(window)
			{
				wxString caption = oldContainer->GetPageText(nTabPage);
				int imageindex = oldContainer->GetPageImageIndex(nTabPage);
				oldNotebook->RemovePage(nTabPage);
				window->Reparent(newNotebook);

				newNotebook->InsertPage(nIndex, window, caption, true, imageindex);
			}
		}
#endif
	}
	return wxDragMove;
}

void wxPageContainerBase::MoveTabPage(int nMove, int nMoveTo)
{
	if(nMove == nMoveTo)
		return;
	else if(nMoveTo < nMove)
		nMoveTo++;

	// Remove the window from the main sizer
	int nCurSel = ((wxFlatNotebookBase *)m_pParent)->m_pages->GetSelection();
	((wxFlatNotebookBase *)m_pParent)->m_mainSizer->Detach(((wxFlatNotebookBase *)m_pParent)->m_windows[nCurSel]);
	((wxFlatNotebookBase *)m_pParent)->m_windows[nCurSel]->Hide();

	wxWindow *pWindow = ((wxFlatNotebookBase *)m_pParent)->m_windows[nMove];
	((wxFlatNotebookBase *)m_pParent)->m_windows.erase(((wxFlatNotebookBase *)m_pParent)->m_windows.begin() + nMove);
	((wxFlatNotebookBase *)m_pParent)->m_windows.insert(((wxFlatNotebookBase *)m_pParent)->m_windows.begin() + nMoveTo - 1, pWindow);

	wxPageInfo pgInfo = m_pagesInfoVec[nMove];
	m_pagesInfoVec.erase(m_pagesInfoVec.begin() + nMove);
	m_pagesInfoVec.insert(m_pagesInfoVec.begin() + nMoveTo - 1, pgInfo);

	// Add the page according to the style
	wxBoxSizer* pSizer = ((wxFlatNotebookBase *)m_pParent)->m_mainSizer;
	long style = GetParent()->GetWindowStyleFlag();


	if(style & wxFNB_BOTTOM)
	{
		pSizer->Insert(0, pWindow, 1, wxEXPAND);
	}
	else
	{
		// We leave a space of 1 pixel around the window
		pSizer->Add(pWindow, 1, wxEXPAND);
	}
	pWindow->Show();

	pSizer->Layout();
	m_iActivePage = nMoveTo-1;
	DoSetSelection(m_iActivePage);
	Refresh();
}

bool wxPageContainerBase::CanFitToScreen(size_t page)
{
	// Incase the from is greater than page,
	// we need to reset the m_nFrom, so in order
	// to force the caller to do so, we return false
	if(m_nFrom > (int)page)
		return false;

	// Calculate the tab width including borders and image if any
	wxClientDC dc(this);

	int width, pom, shapePoints, height, tabHeight;
	long style = GetParent()->GetWindowStyleFlag();

	wxString stam = wxT("Tp");	// Temp data to get the text height;
	dc.GetTextExtent(stam, &width, &height);
	dc.GetTextExtent(GetPageText(page), &width, &pom);

	tabHeight = height + 8; // We use 6 pixels as padding

	if(style & wxFNB_VC71)
		tabHeight = (style & wxFNB_BOTTOM) ? tabHeight - 4 :  tabHeight;
	else if(style & wxFNB_FANCY_TABS)
		tabHeight = (style & wxFNB_BOTTOM) ? tabHeight - 2 :  tabHeight;

	int tabWidth = ((wxFlatNotebookBase *)m_pParent)->m_nPadding * 2 + width;
	if(!(style & wxFNB_VC71))
		shapePoints = (int)(tabHeight*tan((double)m_pagesInfoVec[page].GetTabAngle()/180.0*M_PI));
	else
		shapePoints = 0;

	if(!(style & wxFNB_VC71))
		// Default style
		tabWidth += 2 * shapePoints;

	bool hasImage = (m_ImageList != NULL);
	if(hasImage) hasImage &= m_pagesInfoVec[page].GetImageIndex() != -1;

	// For VC71 style, we only add the icon size (16 pixels)
	if(hasImage && ( (style & wxFNB_VC71) || (style & wxFNB_FANCY_TABS)) )
		tabWidth += 16;
	else
		// Default style
		tabWidth += 16 + shapePoints / 2;

	// Check if we can draw more
	int posx = ((wxFlatNotebookBase *)m_pParent)->m_nPadding;

	if(m_nFrom >= 0)
	{
		for(int i=m_nFrom; i<(int)m_pagesInfoVec.size(); i++)
		{
			if(m_pagesInfoVec[i].GetPosition() == wxPoint(-1, -1))
				break;
			posx += m_pagesInfoVec[i].GetSize().x;
		}
	}

	wxRect rect = GetClientRect();
	int clientWidth = rect.width;

	if(posx + tabWidth + GetButtonsAreaLength() >= clientWidth)
		return false;
	return true;
}

int wxPageContainerBase::GetNumOfVisibleTabs()
{
	int i=m_nFrom;
	int counter = 0;
	for(; i<(int)m_pagesInfoVec.size(); i++, ++counter)
	{
		if(m_pagesInfoVec[i].GetPosition() == wxPoint(-1, -1))
			break;
	}
	return counter;
}

bool wxPageContainerBase::GetEnabled(size_t page)
{
	if(page >= m_pagesInfoVec.size())
		return true;	// Seems strange, but this is the default
	return m_pagesInfoVec[page].GetEnabled();
}

void wxPageContainerBase::Enable(size_t page, bool enabled)
{
	if(page >= m_pagesInfoVec.size())
		return ;
	return m_pagesInfoVec[page].Enable(enabled);
}

int wxPageContainerBase::GetLeftButtonPos()
{
	long style = GetParent()->GetWindowStyleFlag();
	wxRect rect = GetClientRect();
	int clientWidth = rect.width;
	if(style & wxFNB_NO_X_BUTTON)
		return clientWidth - 38;
	else
		return clientWidth - 54;
}

int wxPageContainerBase::GetRightButtonPos()
{
	long style = GetParent()->GetWindowStyleFlag();
	wxRect rect = GetClientRect();
	int clientWidth = rect.width;
	if(style & wxFNB_NO_X_BUTTON)
		return clientWidth - 22;
	else
		return clientWidth - 38;
}

int wxPageContainerBase::GetXPos()
{
	long style = GetParent()->GetWindowStyleFlag();
	wxRect rect = GetClientRect();
	int clientWidth = rect.width;
	if(style & wxFNB_NO_X_BUTTON)
		return clientWidth;
	else
		return clientWidth - 22;
}

int wxPageContainerBase::GetButtonsAreaLength()
{
	long style = GetParent()->GetWindowStyleFlag();
	if(style & wxFNB_NO_NAV_BUTTONS && style & wxFNB_NO_X_BUTTON)
		return 0;	
	else if((style & wxFNB_NO_NAV_BUTTONS) && !(style & wxFNB_NO_X_BUTTON))
		return 53 - 16;
	else if(!(style & wxFNB_NO_NAV_BUTTONS) && (style & wxFNB_NO_X_BUTTON))
		return 53 - 16;
	else
		// All buttons
		return 53;
}

void wxPageContainerBase::DrawTabsLine(wxDC& dc, const wxRect& rect)
{
	wxRect clientRect = rect;
	long style = GetParent()->GetWindowStyleFlag();

	//dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW)));
	dc.SetPen(wxPen(m_tabAreaColor)); // eliminates line
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(clientRect);

	if(!(style & wxFNB_TABS_BORDER_SIMPLE))
	{
		dc.SetPen(wxPen(m_tabAreaColor));
		dc.DrawLine(0, 0, 0, clientRect.height);
		if(style & wxFNB_BOTTOM)
			dc.DrawLine(0, clientRect.height - 1, clientRect.width, clientRect.height - 1);
		else
			dc.DrawLine(0, 0, clientRect.width, 0);
		dc.DrawLine(clientRect.width - 1, 0, clientRect.width - 1, clientRect.height);
	}
}
