///////////////////////////////////////////////////////////////////////////////
// Name:        wx/htmllbox.cpp
// Purpose:     wxHtmlListBox is a listbox whose items are wxHtmlCells
// Author:      Vadim Zeitlin
// Modified by: Charlie Fenton
// Created:     31.05.03
// RCS-ID:      $Id: htmllbox.h 49804 2007-11-10 01:09:42Z VZ $
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// Modified for BOINC from wx/htmllbox.cpp


// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "stdwx.h"
#include "BOINCHtmlLBox.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

const wxChar BOINCHtmlListBoxNameStr[] = wxT("BOINCHtmlListBox");

// ============================================================================
// private classes
// ============================================================================

// ----------------------------------------------------------------------------
// CBOINCHtmlListBoxCache
// ----------------------------------------------------------------------------

// this class is used by CBOINCHtmlListBox to cache the parsed representation of
// the items to avoid doing it anew each time an item must be drawn
class CBOINCHtmlListBoxCache
{
private:
    // invalidate a single item, used by Clear() and InvalidateRange()
    void InvalidateItem(size_t n)
    {
        m_items[n] = (size_t)-1;
        delete m_cells[n];
        m_cells[n] = NULL;
    }

public:
    CBOINCHtmlListBoxCache()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            m_items[n] = (size_t)-1;
            m_cells[n] = NULL;
        }

        m_next = 0;
    }

    ~CBOINCHtmlListBoxCache()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            delete m_cells[n];
        }
    }

    // completely invalidate the cache
    void Clear()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            InvalidateItem(n);
        }
    }

    // return the cached cell for this index or NULL if none
    wxHtmlCell *Get(size_t item) const
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( m_items[n] == item )
                return m_cells[n];
        }

        return NULL;
    }

    // returns true if we already have this item cached
    bool Has(size_t item) const { return Get(item) != NULL; }

    // ensure that the item is cached
    void Store(size_t item, wxHtmlCell *cell)
    {
        delete m_cells[m_next];
        m_cells[m_next] = cell;
        m_items[m_next] = item;

        // advance to the next item wrapping around if there are no more
        if ( ++m_next == SIZE )
            m_next = 0;
    }

    // forget the cached value of the item(s) between the given ones (inclusive)
    void InvalidateRange(size_t from, size_t to)
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( m_items[n] >= from && m_items[n] <= to )
            {
                InvalidateItem(n);
            }
        }
    }

private:
    // the max number of the items we cache
    enum { SIZE = 50 };

    // the index of the LRU (oldest) cell
    size_t m_next;

    // the parsed representation of the cached item or NULL
    wxHtmlCell *m_cells[SIZE];

    // the index of the currently cached item (only valid if m_cells != NULL)
    size_t m_items[SIZE];
};

// ----------------------------------------------------------------------------
// CBOINCHtmlListBoxStyle
// ----------------------------------------------------------------------------

// just forward wxDefaultHtmlRenderingStyle callbacks to the main class so that
// they could be overridden by the user code
class CBOINCHtmlListBoxStyle : public wxDefaultHtmlRenderingStyle
{
public:
    CBOINCHtmlListBoxStyle(const CBOINCHtmlListBox& hlbox) : m_hlbox(hlbox) { }

    virtual wxColour GetSelectedTextColour(const wxColour& colFg)
    {
        return m_hlbox.GetSelectedTextColour(colFg);
    }

    virtual wxColour GetSelectedTextBgColour(const wxColour& colBg)
    {
        return m_hlbox.GetSelectedTextBgColour(colBg);
    }

private:
    const CBOINCHtmlListBox& m_hlbox;

    DECLARE_NO_COPY_CLASS(CBOINCHtmlListBoxStyle)
};

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CBOINCHtmlListBox, CBOINCVListBox)
    EVT_SIZE(CBOINCHtmlListBox::OnSize)
    EVT_MOTION(CBOINCHtmlListBox::OnMouseMove)
    EVT_LEFT_DOWN(CBOINCHtmlListBox::OnLeftDown)
END_EVENT_TABLE()

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_ABSTRACT_CLASS(CBOINCHtmlListBox, CBOINCVListBox)


// ----------------------------------------------------------------------------
// CBOINCHtmlListBox creation
// ----------------------------------------------------------------------------

CBOINCHtmlListBox::CBOINCHtmlListBox()
    : wxHtmlWindowMouseHelper(this)
{
    Init();
}

// normal constructor which calls Create() internally
CBOINCHtmlListBox::CBOINCHtmlListBox(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
    : wxHtmlWindowMouseHelper(this)
{
    Init();

    (void)Create(parent, id, pos, size, style, name);
}

void CBOINCHtmlListBox::Init()
{
    m_htmlParser = NULL;
    m_htmlRendStyle = new CBOINCHtmlListBoxStyle(*this);
    m_cache = new CBOINCHtmlListBoxCache;
}

bool CBOINCHtmlListBox::Create(wxWindow *parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxString& name)
{
    return CBOINCVListBox::Create(parent, id, pos, size, style, name);
}

CBOINCHtmlListBox::~CBOINCHtmlListBox()
{
    delete m_cache;

    if ( m_htmlParser )
    {
        delete m_htmlParser->GetDC();
        delete m_htmlParser;
    }

    delete m_htmlRendStyle;
}

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox appearance
// ----------------------------------------------------------------------------

wxColour CBOINCHtmlListBox::GetSelectedTextColour(const wxColour& colFg) const
{
    return m_htmlRendStyle->
                wxDefaultHtmlRenderingStyle::GetSelectedTextColour(colFg);
}

wxColour
CBOINCHtmlListBox::GetSelectedTextBgColour(const wxColour& WXUNUSED(colBg)) const
{
    return GetSelectionBackground();
}

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox items markup
// ----------------------------------------------------------------------------

wxString CBOINCHtmlListBox::OnGetItemMarkup(size_t n) const
{
    // we don't even need to wrap the value returned by OnGetItem() inside
    // "<html><body>" and "</body></html>" because wxHTML can parse it even
    // without these tags
    return OnGetItem(n);
}

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox cache handling
// ----------------------------------------------------------------------------

void CBOINCHtmlListBox::CacheItem(size_t n) const
{
    if ( !m_cache->Has(n) )
    {
        if ( !m_htmlParser )
        {
            CBOINCHtmlListBox *self = wxConstCast(this, CBOINCHtmlListBox);

            self->m_htmlParser = new wxHtmlWinParser(self);
            m_htmlParser->SetDC(new wxClientDC(self));
            m_htmlParser->SetFS(&self->m_filesystem);
#if !wxUSE_UNICODE
            if (GetFont().Ok())
                m_htmlParser->SetInputEncoding(GetFont().GetEncoding());
#endif
            // use system's default GUI font by default:
            m_htmlParser->SetStandardFonts();
        }

        wxHtmlContainerCell *cell = (wxHtmlContainerCell *)m_htmlParser->
                Parse(OnGetItemMarkup(n));
        wxCHECK_RET( cell, _T("wxHtmlParser::Parse() returned NULL?") );

        // set the cell's ID to item's index so that CellCoordsToPhysical()
        // can quickly find the item:
        cell->SetId(wxString::Format(_T("%lu"), (unsigned long)n));

        cell->Layout(GetClientSize().x - (2 * GetMargins().x));

        m_cache->Store(n, cell);
    }
}

void CBOINCHtmlListBox::OnSize(wxSizeEvent& event)
{
    // we need to relayout all the cached cells
    m_cache->Clear();

    event.Skip();
}

void CBOINCHtmlListBox::RefreshItem(size_t item)
{
    wxRect rect;

    m_cache->InvalidateRange(item, item);

    GetItemRect(item, rect);
    RefreshRect(rect);
}

void CBOINCHtmlListBox::RefreshItems(size_t from, size_t to)
{
    wxRect rect;

    m_cache->InvalidateRange(from, to);

    for (size_t i = from; i <= to; ++i) {
        GetItemRect(i, rect);
        RefreshRect(rect);
    }
}

void CBOINCHtmlListBox::SetItemCount(size_t count)
{
    // the items are going to change, forget the old ones
    m_cache->Clear();

    CBOINCVListBox::SetItemCount(count);
}

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox implementation of CBOINCVListBox pure virtuals
// ----------------------------------------------------------------------------

void CBOINCHtmlListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    CacheItem(n);

    wxHtmlCell *cell = m_cache->Get(n);
    wxCHECK_RET( cell, _T("this cell should be cached!") );

    wxHtmlRenderingInfo htmlRendInfo;

    // draw the selected cell in selected state
    if ( IsSelected(n) )
    {
        wxHtmlSelection htmlSel;
        htmlSel.Set(wxPoint(0,0), cell, wxPoint(INT_MAX, INT_MAX), cell);
        htmlRendInfo.SetSelection(&htmlSel);
        if ( m_htmlRendStyle )
            htmlRendInfo.SetStyle(m_htmlRendStyle);
        htmlRendInfo.GetState().SetSelectionState(wxHTML_SEL_IN);
    }

    // note that we can't stop drawing exactly at the window boundary as then
    // even the visible cells part could be not drawn, so always draw the
    // entire cell
    cell->Draw(dc,
               rect.x + CELL_BORDER, rect.y + CELL_BORDER,
               0, INT_MAX, htmlRendInfo);
}

wxCoord CBOINCHtmlListBox::OnMeasureItem(size_t n) const
{
    CacheItem(n);

    wxHtmlCell *cell = m_cache->Get(n);
    wxCHECK_MSG( cell, 0, _T("this cell should be cached!") );

    return cell->GetHeight() + cell->GetDescent() + (2 * CELL_BORDER);
}

wxCoord CBOINCHtmlListBox::GetMaxItemWidth() const
{
    wxCoord w, maxWidth = 0;
    size_t n = GetItemCount();
    for (size_t i = 0; i < n; ++i) {
        CacheItem(i);
        wxHtmlCell *cell = m_cache->Get(i);
        w = cell->GetWidth();
        if (w > maxWidth) maxWidth = w;
    }
    return maxWidth;
}

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox implementation of wxHtmlListBoxWinInterface
// ----------------------------------------------------------------------------

void CBOINCHtmlListBox::SetHTMLWindowTitle(const wxString& WXUNUSED(title))
{
    // nothing to do
}

void CBOINCHtmlListBox::OnHTMLLinkClicked(const wxHtmlLinkInfo& link)
{
    OnLinkClicked(GetItemForCell(link.GetHtmlCell()), link);
}

void CBOINCHtmlListBox::OnLinkClicked(size_t WXUNUSED(n),
                                  const wxHtmlLinkInfo& link)
{
    wxHtmlLinkEvent event(GetId(), link);
    GetEventHandler()->ProcessEvent(event);
}

wxHtmlOpeningStatus
CBOINCHtmlListBox::OnHTMLOpeningURL(wxHtmlURLType WXUNUSED(type),
                                const wxString& WXUNUSED(url),
                                wxString *WXUNUSED(redirect)) const
{
    return wxHTML_OPEN;
}

wxPoint CBOINCHtmlListBox::HTMLCoordsToWindow(wxHtmlCell *cell,
                                          const wxPoint& pos) const
{
    return CellCoordsToPhysical(pos, cell);
}

wxWindow* CBOINCHtmlListBox::GetHTMLWindow() { return this; }

wxColour CBOINCHtmlListBox::GetHTMLBackgroundColour() const
{
    return GetBackgroundColour();
}

void CBOINCHtmlListBox::SetHTMLBackgroundColour(const wxColour& WXUNUSED(clr))
{
    // nothing to do
}

void CBOINCHtmlListBox::SetHTMLBackgroundImage(const wxBitmap& WXUNUSED(bmpBg))
{
    // nothing to do
}

void CBOINCHtmlListBox::SetHTMLStatusText(const wxString& WXUNUSED(text))
{
    // nothing to do
}

wxCursor CBOINCHtmlListBox::GetHTMLCursor(HTMLCursor type) const
{
    // we don't want to show text selection cursor in listboxes
    if (type == HTMLCursor_Text)
        return wxHtmlWindow::GetDefaultHTMLCursor(HTMLCursor_Default);

    // in all other cases, use the same cursor as wxHtmlWindow:
    return wxHtmlWindow::GetDefaultHTMLCursor(type);
}

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox utilities
// ----------------------------------------------------------------------------

int CBOINCHtmlListBox::HitTest(const wxPoint& pos)
{
    if ( CBOINCVListBox::HitTest(pos) == wxNOT_FOUND )
        return wxNOT_FOUND;

    int x, y;
    size_t n = GetItemCount();
    wxRect r;

    GetViewStart(&x, &y);
    wxPoint p(-x*PIXELS_PER_HORIZONTAL_SCROLL_UNIT, -y*PIXELS_PER_VERTICAL_SCROLL_UNIT);
    p.y -= GetMargins().y + CELL_BORDER;
    p.x += GetMargins().x - CELL_BORDER;

    r.SetTopLeft(p);
    r.SetHeight(0);
    for (size_t i = 0; i < n; ++i) {
        CacheItem(i);
        wxHtmlCell *aCell = m_cache->Get(i);
        r.height = OnGetItemHeight(i);
        r.width = aCell->GetWidth();
        if (r.Contains(pos)) {
            // convert mouse coordinates to coords relative to item's wxHtmlCell:
            return i;
        }
        r.y += r.height; // + (2 * GetMargins().y);
    }
    return wxNOT_FOUND;
}

void CBOINCHtmlListBox::GetItemRect(size_t item, wxRect& rect)
{
    wxPoint pos = GetRootCellCoords(item);
    pos.x += GetMargins().x - CELL_BORDER;
    pos.y -= GetMargins().y + CELL_BORDER;
    rect.SetTopLeft(pos);
    CacheItem(item);
    wxHtmlCell *cell = m_cache->Get(item);
    rect.height = OnGetItemHeight(item);
    rect.width = cell->GetWidth();
}

bool CBOINCHtmlListBox::IsVisible(size_t item)
{
    wxRect rect;
    
    GetItemRect(item, rect);
    
    return (rect.Intersects(GetRect()));
}

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox handling of HTML links
// ----------------------------------------------------------------------------

wxPoint CBOINCHtmlListBox::GetRootCellCoords(size_t n) const
{
    int x, y;
//    wxPoint pos(GetMargins().x - CELL_BORDER, -GetMargins().y - CELL_BORDER);
    wxPoint pos(0, 0);
    
    for (size_t i = 0; i < n; ++i) {
        pos.y += OnGetItemHeight(i);
    }
    GetViewStart(&x, &y);
    pos.x -= x*PIXELS_PER_HORIZONTAL_SCROLL_UNIT;
    pos.y -= y*PIXELS_PER_VERTICAL_SCROLL_UNIT;
    return pos;
}

bool CBOINCHtmlListBox::PhysicalCoordsToCell(wxPoint& pos, wxHtmlCell*& cell) const
{
    if ( CBOINCVListBox::HitTest(pos) == wxNOT_FOUND )
        return false;

    int x, y;
    size_t n = GetItemCount();
    wxRect r;
    wxPoint p(CELL_BORDER, CELL_BORDER);
    
    p += GetMargins();

    GetViewStart(&x, &y);
    p.x -= x*PIXELS_PER_HORIZONTAL_SCROLL_UNIT;
    p.y -= y*PIXELS_PER_VERTICAL_SCROLL_UNIT;
    r.SetTopLeft(p);
    r.SetHeight(0);
    for (size_t i = 0; i < n; ++i) {
        CacheItem(i);
        wxHtmlCell *aCell = m_cache->Get(i);
        r.height = aCell->GetHeight() + aCell->GetDescent() + (2 * CELL_BORDER);
        r.width = aCell->GetWidth();
        if (r.Contains(pos)) {
            cell = aCell;
            // convert mouse coordinates to coords relative to item's wxHtmlCell:
            pos -= r.GetTopLeft();
            return true;
        }
        r.y += r.height + (2 * GetMargins().y);
    }
    
    return false;
}

size_t CBOINCHtmlListBox::GetItemForCell(const wxHtmlCell *cell) const
{
    wxCHECK_MSG( cell, 0, _T("no cell") );

    cell = cell->GetRootCell();

    wxCHECK_MSG( cell, 0, _T("no root cell") );

    // the cell's ID contains item index, see CacheItem():
    unsigned long n;
    if ( !cell->GetId().ToULong(&n) )
    {
        wxFAIL_MSG( _T("unexpected root cell's ID") );
        return 0;
    }

    return n;
}

wxPoint
CBOINCHtmlListBox::CellCoordsToPhysical(const wxPoint& pos, wxHtmlCell *cell) const
{
    return pos + GetRootCellCoords(GetItemForCell(cell));
}

void CBOINCHtmlListBox::OnInternalIdle()
{
    CBOINCVListBox::OnInternalIdle();

    if ( wxHtmlWindowMouseHelper::DidMouseMove() )
    {
        wxPoint pos = ScreenToClient(wxGetMousePosition());
        wxHtmlCell *cell;

        if ( !PhysicalCoordsToCell(pos, cell) )
            return;

        wxHtmlWindowMouseHelper::HandleIdle(cell, pos);
    }
}

void CBOINCHtmlListBox::OnMouseMove(wxMouseEvent& event)
{
    wxHtmlWindowMouseHelper::HandleMouseMoved();
    event.Skip();
}

void CBOINCHtmlListBox::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    wxHtmlCell *cell;

    if ( !PhysicalCoordsToCell(pos, cell) )
    {
        event.Skip();
        return;
    }

    if ( !wxHtmlWindowMouseHelper::HandleMouseClick(cell, pos, event) )
    {
        // no link was clicked, so let the listbox code handle the click (e.g.
        // by selecting another item in the list):
        event.Skip();
    }
}
