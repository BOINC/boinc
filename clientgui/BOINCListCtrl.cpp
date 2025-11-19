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

    wxColour progressColor = isDarkMode ? wxColour(0, 64, 128) : wxColour(192, 217, 217);
    wxBrush progressBrush(progressColor);

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
            dc.SetPen(isDarkMode ? *wxBLACK_PEN : *wxWHITE_PEN);
            dc.SetBrush(isDarkMode ? *wxBLACK_BRUSH : *wxWHITE_BRUSH);
#endif
            dc.DrawRectangle( rr );

            dc.SetPen(*wxBLACK_PEN);
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
    if (m_listCtrl) {
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
