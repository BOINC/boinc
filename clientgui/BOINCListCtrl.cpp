// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseView.h"
#include "BOINCListCtrl.h"
#include "Events.h"


#ifndef wxHAS_LISTCTRL_COLUMN_ORDER
#define GetColumnOrder(x) x
#define GetColumnIndexFromOrder(x) x
#endif

BEGIN_EVENT_TABLE(MyEvtHandler, wxEvtHandler)
    EVT_PAINT(MyEvtHandler::OnPaint)
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(MyEvtHandler, wxEvtHandler)

MyEvtHandler::MyEvtHandler() {}

MyEvtHandler::MyEvtHandler(CBOINCListCtrl *theListControl) {
    m_listCtrl = theListControl;
#ifdef __WXGTK__
    m_view_startX = 0;
#endif
}


DEFINE_EVENT_TYPE(wxEVT_CHECK_SELECTION_CHANGED)

#if USE_NATIVE_LISTCONTROL
DEFINE_EVENT_TYPE(wxEVT_DRAW_PROGRESSBAR)
#endif

BEGIN_EVENT_TABLE(CBOINCListCtrl, LISTCTRL_BASE)

#if USE_NATIVE_LISTCONTROL
    EVT_DRAW_PROGRESSBAR(CBOINCListCtrl::OnDrawProgressBar)
#else
#ifdef __WXMAC__
    EVT_SIZE(CBOINCListCtrl::OnSize)    // In MacAccessibility.mm
#endif
#endif

#if ! USE_LIST_CACHE_HINT
    EVT_LEFT_DOWN(CBOINCListCtrl::OnMouseDown)
#endif
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(CBOINCListCtrl, LISTCTRL_BASE)


CBOINCListCtrl::CBOINCListCtrl() {}


CBOINCListCtrl::CBOINCListCtrl(
    CBOINCBaseView* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags
) : LISTCTRL_BASE(
    pView, iListWindowID, wxDefaultPosition, wxSize(-1, -1), iListWindowFlags
) {
    m_pParentView = pView;

    // Enable Zebra Striping
    EnableAlternateRowColours(true);

#if USE_NATIVE_LISTCONTROL
    m_bProgressBarEventPending = false;
    PushEventHandler(new MyEvtHandler(this));
#else
    savedHandler = GetMainWin()->GetEventHandler();
    GetMainWin()->PushEventHandler(new MyEvtHandler(this));
#ifdef __WXMAC__
    SetupMacAccessibilitySupport();
#endif
#endif
}


CBOINCListCtrl::~CBOINCListCtrl()
{
#if USE_NATIVE_LISTCONTROL
    PopEventHandler(true);
#else
    GetMainWin()->PopEventHandler(true);
#ifdef __WXMAC__
    RemoveMacAccessibilitySupport();
#endif
#endif

    m_iRowsNeedingProgressBars.Clear();
}


bool CBOINCListCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxInt32     iIndex = 0;
    wxInt32     iStdColumnCount = 0;
    wxInt32     iActualColumnCount = GetColumnCount();
    int         i, j;

    wxASSERT(pConfig);

    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    iStdColumnCount = m_pParentView->m_iStdColWidthOrder.size();

    // Cycle through the columns recording their widths
    for (iIndex = 0; iIndex < iActualColumnCount; iIndex++) {
        m_pParentView->m_iStdColWidthOrder[m_pParentView->m_iColumnIndexToColumnID[iIndex]] = GetColumnWidth(iIndex);
    }

    for (iIndex = 0; iIndex < iStdColumnCount; iIndex++) {
        pConfig->SetPath(strBaseConfigLocation + m_pParentView->m_aStdColNameOrder->Item(iIndex));
        pConfig->Write(wxT("Width"), m_pParentView->m_iStdColWidthOrder[iIndex]);
    }

    // Save sorting column and direction
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("SortColumn"), m_pParentView->m_iSortColumnID);
    pConfig->Write(wxT("ReverseSortOrder"), m_pParentView->m_bReverseSort);

    // Save Column Order
    wxString strColumnOrder;
    wxString strBuffer;
    wxString strHiddenColumns;
    wxArrayInt aOrder(iActualColumnCount);
    CBOINCBaseView* pView = (CBOINCBaseView*)GetParent();
    wxASSERT(wxDynamicCast(pView, CBOINCBaseView));

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
    aOrder = GetColumnsOrder();
#else
    for (i = 0; i < iActualColumnCount; ++i) {
        aOrder[i] = i;
    }
#endif

    strColumnOrder.Printf(wxT("%s"), pView->m_aStdColNameOrder->Item(pView->m_iColumnIndexToColumnID[aOrder[0]]));

    for (i = 1; i < iActualColumnCount; ++i)
    {
        strBuffer.Printf(wxT(";%s"), pView->m_aStdColNameOrder->Item(pView->m_iColumnIndexToColumnID[aOrder[i]]));
        strColumnOrder += strBuffer;
    }

    pConfig->Write(wxT("ColumnOrder"), strColumnOrder);

    strHiddenColumns = wxEmptyString;
    for (i = 0; i < iStdColumnCount; ++i) {
        bool found = false;
        for (j = 0; j < iActualColumnCount; ++j) {
            if (pView->m_iColumnIndexToColumnID[aOrder[j]] == i) {
                found = true;
                break;
            }
        }
        if (found) continue;
        if (!strHiddenColumns.IsEmpty()) {
            strHiddenColumns += wxT(";");
        }
        strHiddenColumns += pView->m_aStdColNameOrder->Item(i);
    }
    pConfig->Write(wxT("HiddenColumns"), strHiddenColumns);

    return true;
}


bool CBOINCListCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxInt32     iIndex = 0;
    wxInt32     iStdColumnCount = 0;
    wxInt32     iTempValue = 0;

    wxASSERT(pConfig);

    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    iStdColumnCount = m_pParentView->m_iStdColWidthOrder.size();

    // Cycle through the possible columns updating column widths
    for (iIndex = 0; iIndex < iStdColumnCount; iIndex++) {
        pConfig->SetPath(strBaseConfigLocation + m_pParentView->m_aStdColNameOrder->Item(iIndex));
        pConfig->Read(wxT("Width"), &iTempValue, -1);
        if (-1 != iTempValue) {
            m_pParentView->m_iStdColWidthOrder[iIndex] = iTempValue;
        }
    }

    // Restore sorting column and direction
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("ReverseSortOrder"), &iTempValue,-1);
    if (-1 != iTempValue) {
            m_pParentView->m_bReverseSort = iTempValue != 0 ? true : false;
    }
    pConfig->Read(wxT("SortColumn"), &iTempValue,-1);
    if (-1 != iTempValue) {
        m_pParentView->m_iSortColumnID = iTempValue;
    }

    // Restore Column Order
    wxString strColumnOrder;
    wxString strHiddenColumns;
    CBOINCBaseView* pView = (CBOINCBaseView*)GetParent();

    if (pConfig->Read(wxT("ColumnOrder"), &strColumnOrder)) {
        wxArrayString orderArray;
        TokenizedStringToArray(strColumnOrder, ";", &orderArray);
        SetListColumnOrder(orderArray);

        // If the user installed a new version of BOINC, new columns may have
        // been added that didn't exist in the older version. Check for this.
        //
        // This will also be triggered if the locale is changed, which will cause
        // SetListColumnOrder() to be called again so the wxListCtrl will be set
        // up with the correctly labeled columns.
        //
        bool foundNewColumns = false;
        bool foundNewDefaultColumns = false;

        if (pConfig->Read(wxT("HiddenColumns"), &strHiddenColumns)) {
            wxArrayString hiddenArray;
            wxArrayString defaultArray;
            TokenizedStringToArray(strHiddenColumns, ";", &hiddenArray);
            int shownCount = orderArray.size();
            int hiddenCount = hiddenArray.size();
            int totalCount = pView->m_aStdColNameOrder->size();
            for (int i = 0; i < totalCount; ++i) {  // cycles through updated array of columns.
                wxString columnNameToFind = pView->m_aStdColNameOrder->Item(i);
                bool found = false;
                for (int j = 0; j < shownCount; ++j) {  // cycles through list of visible columns.
                    if (orderArray[j].IsSameAs(columnNameToFind)) {
                        found = true;
                        break;
                    }
                }
                if (found) continue;

                for (int j = 0; j < hiddenCount; ++j) {  // cycles through the hidden columns.
                    if (hiddenArray[j].IsSameAs(columnNameToFind)) {
                        found = true;
                        break;
                    }
                }
                if (found) continue;

                foundNewColumns = true;
                // If we got this far, then we know this column is new.
                // Now it needs to be determined if the new column should be shown by default or not.
                // Create array of default columns.
                //
                defaultArray.Clear();
                for (int k = 0; k < pView->m_iNumDefaultShownColumns; ++k) {
                    defaultArray.Add(pView->m_aStdColNameOrder->Item(pView->m_iDefaultShownColumns[k]));
                }
                for (size_t k = 0; k < defaultArray.GetCount(); ++k) {
                    if (defaultArray[k].IsSameAs(columnNameToFind)) {
                        orderArray.Add(columnNameToFind);
                        foundNewDefaultColumns = true;
                        break;
                    }
                }
                if (!foundNewDefaultColumns) {
                    hiddenArray.Add(columnNameToFind);  // No need to order new hidden columns since they are hidden.
                }
            }
        }
        if (foundNewColumns) {
            if (foundNewDefaultColumns) {
                bool wasInStandardOrder = IsColumnOrderStandard();
                SetListColumnOrder(orderArray);
                if (wasInStandardOrder) SetStandardColumnOrder();
            }
        }
    } else {
        // No "ColumnOrder" tag in pConfig
        // Show all columns in default column order
        wxASSERT(wxDynamicCast(pView, CBOINCBaseView));

        SetDefaultColumnDisplay();
    }

    if (m_pParentView->m_iSortColumnID != -1) {
        m_pParentView->InitSort();
    }

    return true;
}


void CBOINCListCtrl::TokenizedStringToArray(wxString tokenized, const char * delimiters, wxArrayString* array) {
    wxString name;

    array->Clear();
    wxStringTokenizer tok(tokenized, delimiters);
    while (tok.HasMoreTokens())
    {
        name = tok.GetNextToken();
        if (name.IsEmpty()) continue;
        array->Add(name);
    }
}


// SetListColumnOrder() is called mostly from OnRestoreState(), so we don't
// call OnSaveState() from here.  CDlgHiddenColumns calls OnSaveState()
// when we really need to do that.
//
// Unfortunately, we have no way of immediately calling OnSaveState() when
// the user manually reorders columns because that does not generate a
// notification from MS Windows so wxWidgets can't generate an event.
//
void CBOINCListCtrl::SetListColumnOrder(wxArrayString& orderArray) {
    int stdCount, columnPosition;
    int shownColCount = orderArray.GetCount();
    int columnIndex = 0;    // Column number among shown columns before re-ordering
    int columnID = 0;       // ID of column, e.g. COLUMN_PROJECT, COLUMN_STATUS, etc.
    int sortColumnIndex = -1;
    wxArrayInt aOrder(shownColCount);

    CBOINCBaseView* pView = (CBOINCBaseView*)GetParent();
    wxASSERT(wxDynamicCast(pView, CBOINCBaseView));

    // Manager will crash if the scroll bar is not at the left-most position on the
    // current view if columns are modified.
    //
    pView->Freeze();
    pView->m_iColumnIndexToColumnID.Clear();
    DeleteAllColumns();
    stdCount = pView->m_aStdColNameOrder->GetCount();

    pView->m_iColumnIDToColumnIndex.Clear();
    for (columnID=0; columnID<stdCount; ++columnID) {
        pView->m_iColumnIDToColumnIndex.Add(-1);
    }

    for (columnID=0; columnID<stdCount; ++columnID) {
        for (columnPosition=0; columnPosition<shownColCount; ++columnPosition) {
            if (orderArray[columnPosition].IsSameAs(pView->m_aStdColNameOrder->Item(columnID))) {
                aOrder[columnPosition] = columnIndex;
                pView->AppendColumn(columnID);
                pView->m_iColumnIndexToColumnID.Add(columnID);
                pView->m_iColumnIDToColumnIndex[columnID] = columnIndex;

                ++columnIndex;
                break;
            }
        }
    }

    // Prevent a crash bug if we just changed to a new locale.
    //
    // If a column has the same name in both the old and new locale, we guard against
    // changing the sort column to that column.
    //
    // CBOINCListCtrl::OnRestoreState() may have incorrectly added the column names in
    // the new locale as "new" columns, so check against both shownColCount and stdCount.
    int limit = wxMin(shownColCount, stdCount);
    if (columnIndex < limit) {
        SetStandardColumnOrder();
        for (columnID=0; columnID<limit; ++columnID) {
            aOrder[columnID] = columnID;
            pView->AppendColumn(columnID);
            pView->m_iColumnIndexToColumnID.Add(columnID);
            pView->m_iColumnIDToColumnIndex[columnID] = columnID;
        }
    }

    // If sort column is now hidden, set the new first column as sort column
    if (pView->m_iSortColumnID >= 0) {
        sortColumnIndex = pView->m_iColumnIDToColumnIndex[pView->m_iSortColumnID];
        if (sortColumnIndex < 0) {
            pView->m_iSortColumnID = pView->m_iColumnIndexToColumnID[0];
            pView->m_bReverseSort = false;
            pView->SetSortColumn(0);
        } else {
            // Redraw the sort arrow, etc.
            pView->SetSortColumn(sortColumnIndex);
        }
    }

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
    int colCount = GetColumnCount();
    if ((shownColCount > 0) && (shownColCount <= stdCount) && (colCount == shownColCount)) {
        SetColumnsOrder(aOrder);
    }
#endif
    pView->Thaw();
}


bool CBOINCListCtrl::IsColumnOrderStandard() {
#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
    int i;
    wxArrayInt aOrder = GetColumnsOrder();
    int orderCount = aOrder.GetCount();
    for (i=1; i<orderCount; ++i) {
        if(aOrder[i] < aOrder[i-1]) return false;
    }
#endif
    return true;
}


void CBOINCListCtrl::SetStandardColumnOrder() {
    int i;
    int colCount = GetColumnCount();
    wxArrayInt aOrder(colCount);

    for (i=0; i<colCount; ++i) {
        aOrder[i] = i;
    }
#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
    if (colCount) {
        SetColumnsOrder(aOrder);
    }
#endif
}


void CBOINCListCtrl::SetDefaultColumnDisplay() {
    int i;
    wxArrayString orderArray;
    CBOINCBaseView* pView = (CBOINCBaseView*)GetParent();

    wxASSERT(wxDynamicCast(pView, CBOINCBaseView));

    orderArray.Clear();
    for (i=0; i<pView->m_iNumDefaultShownColumns; ++i) {
        orderArray.Add(pView->m_aStdColNameOrder->Item(pView->m_iDefaultShownColumns[i]));
    }

    SetListColumnOrder(orderArray);
    SetStandardColumnOrder();
}


void CBOINCListCtrl::SelectRow(int row, bool setSelected) {
    SetItemState(row,  setSelected ? wxLIST_STATE_SELECTED : 0, wxLIST_STATE_SELECTED);
}


void CBOINCListCtrl::AddPendingProgressBar(int row) {
    bool duplicate = false;
    int n = (int)m_iRowsNeedingProgressBars.GetCount();
    for (int i=0; i<n; ++i) {
        if (m_iRowsNeedingProgressBars[i] == row) {
            duplicate = true;
        }
    }
    if (!duplicate) {
        m_iRowsNeedingProgressBars.Add(row);
    }
}


wxString CBOINCListCtrl::OnGetItemText(long item, long column) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemText(item, column);
}


int CBOINCListCtrl::OnGetItemImage(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemImage(item);
}


void CBOINCListCtrl::DrawProgressBars()
{
    long topItem, numItems, numVisibleItems, row;
    wxRect r, rr;
    int w = 0, x = 0, xx, yy, ww;
    int progressColumn = -1;
    bool isDarkMode = wxGetApp().GetIsDarkMode();

    if (m_pParentView->GetProgressColumn() >= 0) {
        progressColumn = m_pParentView->m_iColumnIDToColumnIndex[m_pParentView->GetProgressColumn()];
    }

#if USE_NATIVE_LISTCONTROL
    wxClientDC dc(this);
    m_bProgressBarEventPending = false;
#else
    wxClientDC dc(GetMainWin());   // Available only in wxGenericListCtrl
#endif

    if (progressColumn < 0) {
        m_iRowsNeedingProgressBars.Clear();
        return;
    }

    int n = (int)m_iRowsNeedingProgressBars.GetCount();
    if (n <= 0) return;

    wxColour progressColor;
    wxColour remainderColor;
    wxColour textColor;

    if (isDarkMode) {
        progressColor = wxColour(96, 96, 96);
        remainderColor = wxColour(24, 24, 24);
        textColor = wxColour(230, 230, 230);
    } else {
        progressColor = wxColour(192, 217, 217);
        remainderColor = *wxWHITE;
        textColor = *wxBLACK;
    }

    wxBrush progressBrush(progressColor);
    wxBrush remainderBrush(remainderColor);
    wxPen remainderPen(remainderColor);

    numItems = GetItemCount();
    if (numItems) {
        topItem = GetTopItem();     // Doesn't work properly for Mac Native control in wxMac-2.8.7

        numVisibleItems = GetCountPerPage();
        ++numVisibleItems;

        if (numItems <= (topItem + numVisibleItems)) numVisibleItems = numItems - topItem;

        x = 0;
        int progressColumnPosition = GetColumnOrder(progressColumn);
        for (int i=0; i<progressColumnPosition; i++) {
            x += GetColumnWidth(GetColumnIndexFromOrder(i));
        }
        w = GetColumnWidth(progressColumn);

#if USE_NATIVE_LISTCONTROL
        x -= GetScrollPos(wxHORIZONTAL);
#else
        CalcScrolledPosition(x, 0, &x, &yy);
#endif
        wxFont theFont = GetFont();
        dc.SetFont(theFont);

        for (int i=0; i<n; ++i) {
            row = m_iRowsNeedingProgressBars[i];
            if (row < topItem) continue;
            if (row > (topItem + numVisibleItems -1)) continue;


            GetItemRect(row, r);
#if ! USE_NATIVE_LISTCONTROL
            r.y = r.y - GetHeaderHeight() - 1;
#endif
            r.x = x;
            r.width = w;
            r.Inflate(-1, -2);
            rr = r;

            wxString progressString = m_pParentView->GetProgressText(row);
            dc.GetTextExtent(progressString, &xx, &yy);

            r.y += (r.height - yy - 1) / 2;

            // Adapted from ellipis code in wxRendererGeneric::DrawHeaderButtonContents()
            if (xx > r.width) {
                int ellipsisWidth;
                dc.GetTextExtent( wxT("..."), &ellipsisWidth, NULL);
                if (ellipsisWidth > r.width) {
                    progressString.Clear();
                    xx = 0;
                } else {
                    do {
                        progressString.Truncate( progressString.length() - 1 );
                        dc.GetTextExtent( progressString, &xx, &yy);
                    } while (xx + ellipsisWidth > r.width && progressString.length() );
                    progressString.append( wxT("...") );
                    xx += ellipsisWidth;
                }
            }

            dc.SetLogicalFunction(wxCOPY);
            dc.SetBackgroundMode(wxBRUSHSTYLE_SOLID);
            dc.SetPen(progressColor);
            dc.SetBrush(progressBrush);
            dc.DrawRectangle( rr );

            rr.Inflate(-2, -1);
            ww = rr.width * m_pParentView->GetProgressValue(row);
            rr.x += ww;
            rr.width -= ww;

#if 0
            // Show background stripes behind progress bars
            wxListItemAttr* attr = m_pParentView->FireOnListGetItemAttr(row);
            wxColour bkgd = attr->GetBackgroundColour();
            dc.SetPen(bkgd);
            dc.SetBrush(bkgd);
#else
            dc.SetPen(remainderPen);
            dc.SetBrush(remainderBrush);
#endif
            dc.DrawRectangle( rr );

            dc.SetPen(*wxBLACK_PEN);
            // dc.SetTextForeground(textColor);
            dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
            if (xx > (r.width - 7)) {
                dc.DrawText(progressString, r.x, r.y);
            } else {
                dc.DrawText(progressString, r.x + (w - 8 - xx), r.y);
            }
        }
    }
    m_iRowsNeedingProgressBars.Clear();
}

#if USE_NATIVE_LISTCONTROL

void MyEvtHandler::OnPaint(wxPaintEvent & event)
{
    event.Skip();
    // In dark mode, progress bars are drawn via NM_CUSTOMDRAW instead,
    // so skip posting the deferred-paint event entirely.
    if (m_listCtrl && !wxGetApp().GetIsDarkMode()) {
        m_listCtrl->PostDrawProgressBarEvent();
    }
}

void CBOINCListCtrl::PostDrawProgressBarEvent() {
    if (m_bProgressBarEventPending) return;

    CDrawProgressBarEvent newEvent(wxEVT_DRAW_PROGRESSBAR, this);
    AddPendingEvent(newEvent);
    m_bProgressBarEventPending = true;
}

void CBOINCListCtrl::OnDrawProgressBar(CDrawProgressBarEvent& event) {
    DrawProgressBars();
    event.Skip();
}

// -----------------------------------------------------------------------
// Dark mode progress bar rendering via NM_CUSTOMDRAW
// -----------------------------------------------------------------------
//
// WHY THIS IS NEEDED:
//
// BOINC's list views (Tasks, Transfers, Projects) show progress bars
// inside a wxListCtrl column. These aren't native progress bar controls --
// they are custom-drawn rectangles with text on top, painted by
// DrawProgressBars() above.
//
// In light mode, the rendering flow works like this:
//
//   1. Windows sends WM_PAINT to the ListView.
//   2. wxWidgets' MyEvtHandler::OnPaint() fires (we pushed it onto the
//      event handler chain in the constructor). It calls event.Skip()
//      to let the native ListView paint itself, then posts a custom
//      wxEVT_DRAW_PROGRESSBAR event via PostDrawProgressBarEvent().
//   3. That posted event is processed after WM_PAINT completes, calling
//      DrawProgressBars(), which uses wxClientDC to draw directly onto
//      the screen surface -- on top of whatever the ListView just painted.
//
// This works fine in light mode because the native Win32 ListView paints
// directly to the screen, so our wxClientDC drawing persists until the
// next WM_PAINT.
//
// In dark mode, wxWidgets enables full owner-draw rendering for the
// ListView via MSWEnableDarkMode(). This changes the painting pipeline:
//
//   1. wxWidgets intercepts NM_CUSTOMDRAW notifications from the ListView.
//   2. For each item, wxWidgets' HandleItemPaint() calls FillRect() to
//      paint the entire row background, draws the item text, and returns
//      CDRF_SKIPDEFAULT to suppress the native theme rendering.
//   3. Critically, all of this drawing happens on a BACK BUFFER HDC --
//      the ListView uses double-buffering (LVS_EX_DOUBLEBUFFER) to
//      eliminate flicker. The back buffer is blitted to the screen only
//      after the entire NM_CUSTOMDRAW cycle completes.
//
// This breaks the old approach: our wxClientDC draws onto the screen
// surface AFTER WM_PAINT, but the back buffer doesn't contain our
// progress bars. The next time the ListView needs to repaint (e.g. on
// hover, which triggers hot-tracking animation via the Explorer theme),
// it blits its back buffer to screen -- erasing our progress bars.
// On hover this happens rapidly, making progress bars flicker or vanish.
//
// THE FIX:
//
// We override MSWOnNotify() which delegates to HandleDarkModeCustomDraw()
// in dark mode to intercept NM_CUSTOMDRAW at two stages:
//
//   CDDS_ITEMPREPAINT: We let wxWidgets do its normal dark-mode row
//   painting via the base class, but we OR in CDRF_NOTIFYPOSTPAINT
//   into the return value. This tells the ListView to send us another
//   notification after it's done painting the item.
//
//   CDDS_ITEMPOSTPAINT: We draw the progress bar directly onto the
//   back buffer's HDC (provided in NMLVCUSTOMDRAW::nmcd.hdc). Since
//   we're drawing on the same HDC that gets blitted to screen, the
//   progress bars survive the double-buffer blit and persist across
//   hover/repaint cycles.
//
// In light mode, MSWOnNotify() delegates to the base class for all
// notifications, and the PostDrawProgressBarEvent() path handles
// progress bars as before.
//
// This entire block is compiled only on Windows (#if USE_NATIVE_LISTCONTROL).
// -----------------------------------------------------------------------
bool CBOINCListCtrl::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) {
    if (wxGetApp().GetIsDarkMode()) {
        return HandleDarkModeCustomDraw(idCtrl, lParam, result);
    }
    // In light mode, delegate to wxListCtrl's default processing.
    return LISTCTRL_BASE::MSWOnNotify(idCtrl, lParam, result);
}

bool CBOINCListCtrl::HandleDarkModeCustomDraw(int idCtrl, WXLPARAM lParam, WXLPARAM *result) {
    // lParam points to an NMHDR; for NM_CUSTOMDRAW it's actually the
    // larger NMLVCUSTOMDRAW struct (NMHDR is the first member).
    NMHDR* nmhdr = (NMHDR*)lParam;
    if (nmhdr->code == NM_CUSTOMDRAW) {
        NMLVCUSTOMDRAW* pcd = (NMLVCUSTOMDRAW*)lParam;
        DWORD dwDrawStage = pcd->nmcd.dwDrawStage;

        if (dwDrawStage == CDDS_ITEMPREPAINT) {
            // The base class (wxListCtrl) handles dark-mode row painting
            // here -- background fill, text, selection highlight, etc.
            // We delegate to it, then OR in CDRF_NOTIFYPOSTPAINT so the
            // ListView will send us a CDDS_ITEMPOSTPAINT notification
            // after the row is fully painted. wxWidgets doesn't request
            // post-paint by default, so we must add it ourselves.
            bool handled = LISTCTRL_BASE::MSWOnNotify(idCtrl, lParam, result);
            if (handled) {
                *result |= CDRF_NOTIFYPOSTPAINT;
            }
            return handled;
        }

        if (dwDrawStage == CDDS_ITEMPOSTPAINT) {
            // The row has been fully painted (background, text, selection).
            // Now draw the progress bar on top, using the back buffer's
            // HDC so it survives the double-buffer blit to screen.
            int progressColumnID = m_pParentView->GetProgressColumn();
            if (progressColumnID >= 0) {
                // Map the column ID (e.g. COLUMN_PROGRESS) to the actual
                // display index, which may differ if columns are reordered.
                int progressColumn = m_pParentView->m_iColumnIDToColumnIndex[progressColumnID];
                if (progressColumn >= 0) {
                    int item = (int)pcd->nmcd.dwItemSpec;
                    DrawItemProgressBar(pcd->nmcd.hdc, item, progressColumn);
                }
            }

            // Restore the 1px bottom border of the row. In dark mode,
            // wxWidgets' hot-tracking background fill paints over the
            // bottom pixel that visually separates adjacent rows.
            // Skip for focused items (preserves the dotted focus rect)
            // and selected items (preserves consistent selection padding
            // on all four sides).
            if (!(pcd->nmcd.uItemState & (CDIS_FOCUS | CDIS_SELECTED))) {
                wxRect itemRect;
                if (GetItemRect((int)pcd->nmcd.dwItemSpec, itemRect)) {
                    RECT rcBorder = {
                        itemRect.x,
                        itemRect.y + itemRect.height - 1,
                        itemRect.x + itemRect.width,
                        itemRect.y + itemRect.height
                    };
                    HBRUSH hBorderBrush = CreateSolidBrush(RGB(96, 96, 96));
                    FillRect(pcd->nmcd.hdc, &rcBorder, hBorderBrush);
                    DeleteObject(hBorderBrush);
                }
            }

            *result = CDRF_DODEFAULT;
            return true;
        }
    }
    // For any notification we don't handle, delegate to wxListCtrl's default processing.
    return LISTCTRL_BASE::MSWOnNotify(idCtrl, lParam, result);
}

// Draw a single progress bar for one item using Win32 GDI on the provided HDC.
// This is a port of the relevant logic from DrawProgressBars() above, but uses
// raw Win32 GDI calls instead of wxDC, because we're drawing directly onto the
// ListView's NM_CUSTOMDRAW back buffer HDC (not a wxClientDC screen surface).
//
// wxWidgets does have an internal wxDCTemp class that wraps an existing HDC,
// but it is a private implementation detail (declared in wx/msw/dc.h, not part
// of the public API) and relying on it would be fragile across wxWidgets
// versions. Since we already depend on Win32 NM_CUSTOMDRAW for the drawing
// hook itself, using native GDI here is consistent and avoids coupling to
// wxWidgets internals.
//
// The progress bar layout is a two-layer rectangle:
//
//   +------ outer rect (progress color) -------+
//   | +--- inner left (filled portion) ------+ |
//   | |                  | remainder color   | |
//   | +--------------------------------------- |
//   +------------------------------------------+
//
// With a percentage text label drawn on top (e.g. "42.50%").
//
void CBOINCListCtrl::DrawItemProgressBar(HDC hdc, int item, int progressColumn) {
    // GetSubItemRect returns the cell bounds in client coordinates,
    // which are already correct for the NM_CUSTOMDRAW HDC.
    wxRect r;
    if (!GetSubItemRect(item, progressColumn, r)) return;

    // Get the progress percentage (0.0-1.0) and display text from the
    // parent view (e.g. CViewWork, CViewTransfers, CViewProjects).
    wxString progressString = m_pParentView->GetProgressText(item);
    double progressValue = m_pParentView->GetProgressValue(item);

    bool isDarkMode = wxGetApp().GetIsDarkMode();

    // Colors match those used in DrawProgressBars() for visual consistency.
    COLORREF progressColor, remainderColor, textColor;
    if (isDarkMode) {
        progressColor = RGB(96, 96, 96);
        remainderColor = RGB(24, 24, 24);
        textColor = RGB(230, 230, 230);
    } else {
        progressColor = RGB(192, 217, 217);
        remainderColor = RGB(255, 255, 255);
        textColor = RGB(0, 0, 0);
    }

    // Shrink the cell rect inward to add padding around the progress bar.
    // (-1, -2) matches the Inflate() call in DrawProgressBars().
    r.Inflate(-1, -2);

    // Fill the entire outer rect with the progress color. This forms both
    // the filled portion and a 2px/1px border around the inner area.
    RECT rcOuter = { r.x, r.y, r.x + r.width, r.y + r.height };
    HBRUSH hProgressBrush = CreateSolidBrush(progressColor);
    FillRect(hdc, &rcOuter, hProgressBrush);
    DeleteObject(hProgressBrush);

    // The inner rect is inset by (2, 1) from the outer rect. We fill only
    // the unfilled portion (right side) with the remainder color, leaving
    // the filled portion showing through as the progress color from above.
    RECT rcInner = { r.x + 2, r.y + 1, r.x + r.width - 2, r.y + r.height - 1 };
    int innerWidth = rcInner.right - rcInner.left;
    int filledWidth = (int)(innerWidth * progressValue);
    RECT rcRemainder = { rcInner.left + filledWidth, rcInner.top, rcInner.right, rcInner.bottom };
    HBRUSH hRemainderBrush = CreateSolidBrush(remainderColor);
    FillRect(hdc, &rcRemainder, hRemainderBrush);
    DeleteObject(hRemainderBrush);

    // Draw the percentage text (e.g. "42.50%") on top of the bar.
    RECT rcText = { r.x, r.y, r.x + r.width, r.y + r.height };
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor);

    // Use the wxListCtrl's font for consistent appearance.
    // GetFont().GetHFONT() returns the native Win32 HFONT handle.
    HFONT hFont = (HFONT)GetFont().GetHFONT();
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // Measure text width to decide alignment: if the text fits with at
    // least 7px to spare, right-align it; otherwise left-align so it
    // doesn't get clipped. DT_WORD_ELLIPSIS handles truncation if the
    // column is too narrow even for left-aligned text.
    SIZE textSize;
    GetTextExtentPoint32W(hdc, progressString.wc_str(), progressString.length(), &textSize);

    UINT dtFlags = DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX;
    if (textSize.cx > (r.width - 7)) {
        dtFlags |= DT_LEFT;
    } else {
        dtFlags |= DT_RIGHT;
        rcText.right -= 4;  // Small right margin for visual balance
    }

    DrawTextW(hdc, progressString.wc_str(), progressString.length(), &rcText, dtFlags);

    // Restore the original font to avoid leaking GDI state.
    SelectObject(hdc, hOldFont);
}

#else

void MyEvtHandler::OnPaint(wxPaintEvent & event)
{
    if (m_listCtrl) {
        m_listCtrl->savedHandler->ProcessEvent(event);
        m_listCtrl->DrawProgressBars();
#ifdef __WXGTK__
        // Work around a wxWidgets 3.0 bug in wxGenericListCtrl (Linux
        // only) which causes headers to be misaligned after horizontal
        // scrolling due to wxListHeaderWindow::OnPaint() calling
        // parent->GetViewStart() before the parent window has been
        // scrolled to the new position.
        int view_startX;
        m_listCtrl->GetViewStart( &view_startX, NULL );
        if (view_startX != m_view_startX) {
            m_view_startX = view_startX;
            ((wxWindow *)m_listCtrl->m_headerWin)->Refresh();
            ((wxWindow *)m_listCtrl->m_headerWin)->Update();
        }
#endif
    } else {
        event.Skip();
    }
}

#endif


#if ! USE_LIST_CACHE_HINT

// Work around features in multiple selection virtual wxListCtrl:
//  * It does not send deselection events (except ctrl-click).
//  * It does not send selection events if you add to selection
//    using Shift_Click.
//
// Post a special event.  This will allow this mouse event to
// propogate through the chain to complete any selection or
// deselection operation, then the special event will trigger
// CBOINCBaseView::OnCheckSelectionChanged() to respond to the
// selection change, if any.
//
void CBOINCListCtrl::OnMouseDown(wxMouseEvent& event) {
    CCheckSelectionChangedEvent newEvent(wxEVT_CHECK_SELECTION_CHANGED, this);
    m_pParentView->GetEventHandler()->AddPendingEvent(newEvent);
    event.Skip();
}

#endif


// To reduce flicker, refresh only changed columns (except
// on Mac, which is double-buffered to eliminate flicker.)
void CBOINCListCtrl::RefreshCell(int row, int col) {
    wxRect r;

    GetSubItemRect(row, col, r);
    RefreshRect(r);
}
