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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCGridCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCGridCtrl.h"
#include "BOINCBaseView.h"
#include "res/sortascending.xpm"
#include "res/sortdescending.xpm"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayPair);
WX_DEFINE_OBJARRAY(ArrayPairVariant);

/* static compare functions and helper variable */
static bool reverseCompareOrder=false;
/* compare Strings that contains floats */
static int CompareFloatString(const wxString& first,const wxString& second) {
	double dFirst;
	double dSecond;
	first.ToDouble(&dFirst);
	second.ToDouble(&dSecond);
	if(dFirst < dSecond) {
		return reverseCompareOrder ? 1 : -1 ;
	}
	if(dSecond < dFirst) {
		return reverseCompareOrder ? -1 : 1 ;
	}
	return 0;
}

/* compare Strings case insensitive */
static int CompareStringStringNoCase(const wxString& first,const wxString& second) {
	int ret = first.CmpNoCase(second);
	return reverseCompareOrder ? ret * (-1) : ret;
}

/* compare Strings that contains int/long numbers */
static int CompareLongString(const wxString& first,const wxString& second) {
	long lFirst;
	long lSecond;
	first.ToLong(&lFirst);
	second.ToLong(&lSecond);
	if(lFirst < lSecond) {
		return reverseCompareOrder ? 1 : -1 ;
	}
	if(lSecond < lFirst) {
		return reverseCompareOrder ? -1 : 1 ;
	}
	return 0;
}

/* compare strings that contains time periods in format 00:00:00 */
static int CompareTimeString(const wxString& first,const wxString& second) {
	long dtFirst,dtSecond;

	wxString hours,minutes,seconds;
	long lHours,lMinutes,lSeconds;

	//converting the first string to long value
	hours = first.BeforeFirst(':');
	seconds = first.AfterLast(':');
	minutes = first.AfterFirst(':').BeforeFirst(':');
	hours.ToLong(&lHours);
	minutes.ToLong(&lMinutes);
	seconds.ToLong(&lSeconds);
	dtFirst = lSeconds + lMinutes * 60 + lHours * 3600;
	//converting the second string
	hours = second.BeforeFirst(':');
	seconds = second.AfterLast(':');
	minutes = second.AfterFirst(':').BeforeFirst(':');
	hours.ToLong(&lHours);
	minutes.ToLong(&lMinutes);
	seconds.ToLong(&lSeconds);
	dtSecond = lSeconds + lMinutes * 60 + lHours * 3600;

	if(dtFirst < dtSecond) {
		return reverseCompareOrder ? 1 : -1 ;
	}
	if(dtSecond < dtFirst) {
		return reverseCompareOrder ? -1 : 1 ;
	}
	return 0;
}

/* compare Strings that contains dateTime*/
static int CompareDateString(const wxString& first,const wxString& second) {
	wxDateTime dtFirst,dtSecond;

	//dtFirst.ParseDateTime(first);
	//dtSecond.ParseDateTime(second);
	dtFirst.ParseFormat(first,wxT(" %x %X"));
	dtSecond.ParseFormat(second,wxT(" %x %X"));

	if(dtFirst < dtSecond) {
		return reverseCompareOrder ? 1 : -1 ;
	}
	if(dtSecond < dtFirst) {
		return reverseCompareOrder ? -1 : 1 ;
	}
	return 0;
}

/* compare Strings that contains int/long numbers */
static int CompareLongString2(pair** first,pair** second) {
	return CompareLongString((*first)->value,(*second)->value);
}

/* compare strings that contains time periods in format 00:00:00 */
static int CompareTimeString2(pair** first,pair** second) {
	return CompareTimeString((*first)->value,(*second)->value);
}

/* compare Strings that contains dateTime*/
static int CompareDateString2(pair** first,pair** second) {
	return CompareDateString((*first)->value,(*second)->value);
}

/* compare Strings that contains floats */
static int CompareFloatString2(pair** first,pair** second) {
	return CompareFloatString((*first)->value,(*second)->value);
}

/* compare Strings case insensitive */
static int CompareStringStringNoCase2(pair** first,pair** second) {	
	return CompareStringStringNoCase((*first)->value,(*second)->value);
}

/* ########## grid ctrl implementation ############### */
BEGIN_EVENT_TABLE (CBOINCGridCtrl, wxGrid)
	EVT_GRID_LABEL_LEFT_CLICK(CBOINCGridCtrl::OnLabelLClick)
END_EVENT_TABLE ()

/* Constructor, don't call any grid methods here, because they could raise events, 
   which couldn't be handled correctly while the grid isn't constructed completly.
   Instead call Setup() and place all further initialization there
   */
CBOINCGridCtrl::CBOINCGridCtrl(wxWindow* parent, wxWindowID iGridWindowID) : wxGrid(parent, iGridWindowID) {
	//init members
	m_sortColumn=-1;
	m_sortAscending=true;
	m_sortNeededByLabelClick=false;
	m_pkColumnIndex=-1;
	m_cursorcol=-1;
	m_cursorrow=-1;		
	//load sorting bitmaps
	m_ascBitmap = wxBitmap(sortascending_xpm);
	m_descBitmap = wxBitmap(sortdescending_xpm);
}

/* make settings for the grid here instead in the constructor */ 
void CBOINCGridCtrl::Setup() {
	//make grid cursor invisible
	SetCellHighlightPenWidth(0);
	SetCellHighlightROPenWidth(0);
	//change default selection colours
	SetSelectionBackground(*wxLIGHT_GREY);
	SetSelectionForeground(*wxBLACK);
	//
	SetRowLabelSize(1);//hide row labels
	SetColLabelSize(20); //make header row smaller as default
	SetColLabelAlignment(wxALIGN_LEFT,wxALIGN_CENTER);
	EnableGridLines(false);
	EnableDragRowSize(false);//prevent the user from changing the row height with the mouse
	EnableDragCell(false);
	EnableEditing(false);//make the whole grid read-only
	SetDefaultCellBackgroundColour(*wxWHITE);
#ifdef __WXMAC__
	SetLabelFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	SetDefaultCellFont(wxFont(12, wxSWISS, wxNORMAL, wxNORMAL, FALSE));
#else
	SetLabelFont(*wxNORMAL_FONT);
#endif
	this->SetScrollLineX(5);
	this->SetScrollLineY(5);
}

CBOINCGridCtrl::~CBOINCGridCtrl() {
}

/* use this method instead of wxGrid::GetSelectedRows() 
   because a bug in wxGrid::GetSelectedRows() doesn't return anything although some rows are selected */
wxArrayInt CBOINCGridCtrl::GetSelectedRows2() {
	wxArrayInt ret;
	for(int i= 0; i< this->GetRows();i++) {
		for(int j=0; j< this->GetCols();j++) {
			if(this->IsInSelection(i,j)) {
				ret.Add(i);
				//break the inner loop here to prevent adding the same row twice
				break;
			}
		}
	}
	return ret;
}

/* sets the column with unique values (not needed anymore with virtual grid)*/
void CBOINCGridCtrl::SetPrimaryKeyColumn(int col) {
	m_pkColumnIndex = col;
}

/* If the user clicked inside the grid, the former selected cells must be deselected.
   Because the code in OnListRender() of the GridViews raises SelectionEvents too,
   call to GetBatchCount() here is necessary to decide what to do
 */
void CBOINCGridCtrl::ClearSavedSelection() {
	if(this->GetBatchCount()<=0) {
		m_arrSelectedKeys.Empty();
	}
}

/* save the key values of the currently selected rows for later restore */
void CBOINCGridCtrl::SaveSelection() {
	if(m_pkColumnIndex>=0) {
		wxArrayInt arrSelRows = GetSelectedRows2();	
		for(unsigned int i=0; i< arrSelRows.GetCount();i++) {
			m_arrSelectedKeys.Add(GetCellValue(arrSelRows[i],m_pkColumnIndex));
		}
	}
}

/* select all rows, that were formerly selected
   this raises selection events without user interaction */
void CBOINCGridCtrl::RestoreSelection() {
	ClearSelection();
	for(unsigned int i=0;i < m_arrSelectedKeys.size();i++) {
		int index = GetTable()->FindRowIndexByColValue(m_pkColumnIndex,m_arrSelectedKeys[i]);
		if(index >=0) {
			SelectRow(index,true);
		}
	}
}

void CBOINCGridCtrl::SaveGridCursorPosition() {
	m_cursorcol = GetGridCursorCol();
	m_cursorrow = GetGridCursorRow();	
	if(m_cursorrow>=0 && m_cursorcol >=0) {
		m_szCursorKey = GetCellValue(m_cursorrow,m_pkColumnIndex);
	}
}

void CBOINCGridCtrl::RestoreGridCursorPosition() {
	int index = GetTable()->FindRowIndexByColValue(m_pkColumnIndex,m_szCursorKey);
	if(index >=0) {		
		SetGridCursor(index,m_cursorcol);		
	}
}

int CBOINCGridCtrl::GetFirstSelectedRow() {
	int ret= -1;
	//Row mode ?
	wxArrayInt selRows = this->GetSelectedRows2();
	if(selRows.size() >0) {
		return selRows[0];
	}
	//block mode ?
	wxGridCellCoordsArray  selBlocks =  this->GetSelectionBlockTopLeft();
	if(selBlocks.size()>0) {
		return selBlocks[0].GetRow();
	}
	//cell mode
	wxGridCellCoordsArray  selCells =  this->GetSelectionBlockTopLeft();
	if(selCells.size()>0) {
		return selCells[0].GetRow();
	}

	return ret;
}

/* saves column widths and sorting attributes */
bool CBOINCGridCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = this->GetCols();

    wxASSERT(pConfig);
    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex < iColumnCount; iIndex++) {
        wxString label = this->GetColLabelValue(iIndex);
        // Don't save width for hidden / invisible columns
        if (label.IsEmpty()) {
            continue;
        }
        pConfig->SetPath(strBaseConfigLocation + label);
		pConfig->Write(wxT("Width"), this->GetColumnWidth(iIndex));
    }
	//save sorting column
	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Write(wxT("SortColumn"),this->m_sortColumn);
	pConfig->Write(wxT("SortAscending"),this->m_sortAscending);
    return true;
}

/* restores column widths and sorting attributes */
bool CBOINCGridCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxInt32     iIndex = 0;
    wxInt32     iTempValue = 0;
	wxInt32		iColumnCount = GetCols();

    wxASSERT(pConfig);

    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex < iColumnCount; iIndex++) {
        wxString label = GetColLabelValue(iIndex);
        // Don't restore width for hidden / invisible columns
        if (label.IsEmpty()) {
            continue;
        }
        pConfig->SetPath(strBaseConfigLocation + label);

        pConfig->Read(wxT("Width"), &iTempValue, -1);
        if (-1 != iTempValue) {
			SetColSize(iIndex, iTempValue);
        }
    }
	//read sorting
	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Read(wxT("SortColumn"),&iTempValue,-1);
	if(-1 != iTempValue) {
		m_sortColumn = iTempValue;
	}
	pConfig->Read(wxT("SortAscending"),&iTempValue,-1);
	if(-1 != iTempValue) {
		m_sortAscending = iTempValue != 0 ? true : false;
	}

    return true;
}

/* setting the horizontal and vertical aligment of the column (for convienience purpose only) */
void CBOINCGridCtrl::SetColAlignment(int col,int hAlign,int vAlign) {
    wxGridCellAttr *attr = m_table->GetAttr(-1, col, wxGridCellAttr::Col );
	if(!attr) {
		attr = new wxGridCellAttr;
	}
	attr->SetAlignment(hAlign,vAlign);
    SetColAttr(col, attr);
}

/* copied source code from wxGrid to make overloading possible */
void CBOINCGridCtrl::DrawTextRectangle( wxDC& dc,
                                const wxString& value,
                                const wxRect& rect,
                                int horizAlign,
                                int vertAlign,
                                int textOrientation )
{
    wxArrayString lines;

    StringToLines( value, lines );
    //Forward to new API.
    DrawTextRectangle(  dc,
        lines,
        rect,
        horizAlign,
        vertAlign,
        textOrientation );

}

/* draws text with ellipses (...) instead cutting it at the end of the column */
void CBOINCGridCtrl::DrawTextRectangle( wxDC& dc,
                               const wxArrayString& lines,
                               const wxRect& rect,
                               int horizAlign,
                               int vertAlign,
                               int textOrientation )
{
    long textWidth, textHeight;
    long lineWidth, lineHeight;
    unsigned int nLines;

    dc.SetClippingRegion( rect );

    nLines = (unsigned int)lines.GetCount();
    if( nLines > 0 )
    {
        unsigned int l;
        float x = 0.0, y = 0.0;

        if( textOrientation == wxHORIZONTAL )
            GetTextBoxSize(dc, lines, &textWidth, &textHeight);
        else
            GetTextBoxSize( dc, lines, &textHeight, &textWidth );

        switch( vertAlign )
        {
        case wxALIGN_BOTTOM:
            if( textOrientation == wxHORIZONTAL )
                y = rect.y + (rect.height - textHeight - 1);
            else
                x = rect.x + rect.width - textWidth;
            break;

        case wxALIGN_CENTRE:
            if( textOrientation == wxHORIZONTAL )
                y = rect.y + ((rect.height - textHeight)/2);
            else
                x = rect.x + ((rect.width - textWidth)/2);
            break;

        case wxALIGN_TOP:
        default:
            if( textOrientation == wxHORIZONTAL )
                y = rect.y + 1;
            else
                x = rect.x + 1;
            break;
        }

        // Align each line of a multi-line label
        for( l = 0; l < nLines; l++ )
        {
            dc.GetTextExtent(lines[l], &lineWidth, &lineHeight);

            switch( horizAlign )
            {
            case wxALIGN_RIGHT:
                if( textOrientation == wxHORIZONTAL )
                    x = rect.x + (rect.width - lineWidth - 1);
                else
                    y = rect.y + lineWidth + 1;
                break;

            case wxALIGN_CENTRE:
                if( textOrientation == wxHORIZONTAL )
                    x = rect.x + ((rect.width - lineWidth)/2);
                else
                    y = rect.y + rect.height - ((rect.height - lineWidth)/2);
                break;

            case wxALIGN_LEFT:
            default:
                if( textOrientation == wxHORIZONTAL )
                    x = rect.x + 1;
                else
                    y = rect.y + rect.height - 1;
                break;
            }

            if( textOrientation == wxHORIZONTAL )
            {
				//drawing the text happens here
				wxString formattedText = FormatTextWithEllipses(dc,lines[l],rect.width);
                dc.DrawText( formattedText, (int)x, (int)y );
                y += lineHeight;
            }
            else
            {
                dc.DrawRotatedText( lines[l], (int)x, (int)y, 90.0 );
                x += lineHeight;
            }
        }
    }
    dc.DestroyClippingRegion();
}

wxString CBOINCGridCtrl::FormatTextWithEllipses(wxDC& dc,const wxString &text,int width)
{
	wxString retText;
    wxString drawntext, ellipsis;
    wxCoord w, h, base_w;
    wxListItem item;

    // determine if the string can fit inside the current width
    dc.GetTextExtent(text, &w, &h);
    if (w <= width)
    {
		retText = text;
    }
    else // otherwise, truncate and add an ellipsis if possible
    {
        // determine the base width
        ellipsis = wxString(wxT("..."));
        dc.GetTextExtent(ellipsis, &base_w, &h);

        // continue until we have enough space or only one character left
        wxCoord w_c, h_c;
        size_t len = text.Length();
        drawntext = text.Left(len);
        while (len > 1)
        {
            dc.GetTextExtent(drawntext.Last(), &w_c, &h_c);
            drawntext.RemoveLast();
            len--;
            w -= w_c;
            if (w + base_w <= width)
                break;
        }

        // if still not enough space, remove ellipsis characters
        while (ellipsis.Length() > 0 && w + base_w > width)
        {
            ellipsis = ellipsis.Left(ellipsis.Length() - 1);
            dc.GetTextExtent(ellipsis, &base_w, &h);
        }

        // now the text is ready for drawing
		retText = drawntext + ellipsis;
    }

	return retText;
}

/* not virtual in wxGrid, so code copied and modified her for painting sorting icons */
void CBOINCGridCtrl::DrawColLabel( wxDC& dc, int col )
{
    if ( GetColWidth(col) <= 0 || m_colLabelHeight <= 0 )
        return;

    int colLeft = GetColLeft(col);

    wxRect rect;
    int colRight = GetColRight(col) - 1;

    dc.SetPen( wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW),1, wxSOLID) );
    dc.DrawLine( colRight, 0,
                 colRight, m_colLabelHeight-1 );

    dc.DrawLine( colLeft, 0, colRight, 0 );

    dc.DrawLine( colLeft, m_colLabelHeight-1,
                 colRight+1, m_colLabelHeight-1 );

    dc.SetPen( *wxWHITE_PEN );
    dc.DrawLine( colLeft, 1, colLeft, m_colLabelHeight-1 );
    dc.DrawLine( colLeft, 1, colRight, 1 );
    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.SetTextForeground( GetLabelTextColour() );
    dc.SetFont( GetLabelFont() );

    int hAlign, vAlign, orient;
    GetColLabelAlignment( &hAlign, &vAlign );
    orient = GetColLabelTextOrientation();

    rect.SetX( colLeft + 2 );
    rect.SetY( 2 );
    rect.SetWidth( GetColWidth(col) - 4 );
    rect.SetHeight( m_colLabelHeight - 4 );
    DrawTextRectangle( dc, GetColLabelValue( col ), rect, hAlign, vAlign, orient );
	//paint sorting indicators, if needed
	if(col == m_sortColumn) {
		int x = rect.GetRight() - m_ascBitmap.GetWidth() - 2;
		int y = rect.GetY();
		dc.DrawBitmap(this->m_sortAscending ? m_descBitmap : m_ascBitmap,x,y,true);
	}
}

/* handles left mouse click on column header */
void CBOINCGridCtrl::OnLabelLClick(wxGridEvent& ev) {
	if(ev.GetCol() != -1) {
        //same column as last time, then change only sort direction
        if(m_sortColumn == ev.GetCol()) {
			m_sortAscending = ! m_sortAscending;
		} else {
            int tmpOldColumn = m_sortColumn;

            m_sortColumn = ev.GetCol();
			m_sortAscending = true;

            // Force a repaint of the label
			if ( -1 != tmpOldColumn ) {
				SetColLabelValue(tmpOldColumn, GetColLabelValue(tmpOldColumn));
			}
		}

        // Force a repaint of the label
        SetColLabelValue(ev.GetCol(), GetColLabelValue(ev.GetCol()));
		//
		m_sortNeededByLabelClick=true;
		// Update and sort data
		wxTimerEvent tEvent;
		wxDynamicCast(GetParent(),CBOINCBaseView)->FireOnListRender(tEvent);
	}

	ev.Skip();
}

void CBOINCGridCtrl::SortData() {
	GetTable()->SortData(m_sortColumn,m_sortAscending);
	m_sortNeededByLabelClick=false;
}

void CBOINCGridCtrl::SortData2() {
	GetTable()->SortData2(m_sortColumn,m_sortAscending);
	m_sortNeededByLabelClick=false;
}

void CBOINCGridCtrl::SetColumnSortType(int col,int sortType/*=CST_STRING*/) {
	GetTable()->SetColumnSortType(col,sortType);
}

/* forces an update paint of the given column */
void CBOINCGridCtrl::RefreshColumn(int col) {
	wxRect r;
	r.SetTop(0);
	r.SetLeft(GetColLeft(col));
	r.SetRight(GetColRight(col));
	r.SetBottom(GetSize().GetHeight());
	Refresh(true,&r);
}

/* for convinience purposes only */
CBOINCGridTable* CBOINCGridCtrl::GetTable() {
	return wxDynamicCast(wxGrid::GetTable(), CBOINCGridTable);
}

/* ################### generic grid cell renderer #################### */
CBOINCGridCellRenderer::CBOINCGridCellRenderer() : wxGridCellStringRenderer() {
}

void CBOINCGridCellRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	DoNormalTextDrawing(grid,attr,dc,rect,row,col,isSelected);
}

/* draws alternating background colours */
void CBOINCGridCellRenderer::DrawBackground(wxGrid& grid,
                              wxDC& dc,
                              const wxRect& rect,
                              int row,
                              bool isSelected)
{
    dc.SetBackgroundMode( wxSOLID );

    // grey out fields if the grid is disabled
    if( grid.IsEnabled() )
    {
        if ( isSelected )
        {
            dc.SetBrush( wxBrush(grid.GetSelectionBackground(), wxSOLID) );
        }
        else
        {
			//alternating background colours
			if(row % 2 == 0) {
				dc.SetBrush(wxBrush(wxColour(240,240,240)));
			}
			else {
				dc.SetBrush(*wxWHITE_BRUSH);
			}			
        }
    }
    else
    {
        dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE), wxSOLID));
    }

    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.DrawRectangle(rect);
}

/* do the text drawing */
void CBOINCGridCellRenderer::DoNormalTextDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected) {
	int hAlign, vAlign;
    wxRect rect = rectCell;
    rect.Inflate(-1);
    // erase only this cells background, overflow cells should have been erased
	this->DrawBackground(grid, dc, rectCell, row, isSelected);    
    // now we only have to draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);
	//get a real grid class pointer
    CBOINCGridCtrl* bgrid = wxDynamicCast(&grid, CBOINCGridCtrl);
	attr.GetAlignment(&hAlign, &vAlign);
	//use the overloaded method here
	bgrid->DrawTextRectangle(dc, grid.GetCellValue(row, col),rect, hAlign, vAlign);
}

/* ############# message cell renderer ######################## */
/* Constructor
   prio argument - the column index, that holds prio values
*/
CBOINCGridCellMessageRenderer::CBOINCGridCellMessageRenderer(int priocol){
	m_prioColumn = priocol;
}

void CBOINCGridCellMessageRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	wxString szError(wxT("Error"));
	if(grid.GetCellValue(row,m_prioColumn).Trim(false).IsSameAs(szError)) {
		attr.SetTextColour(*wxRED);
	}
	else {
		attr.SetTextColour(*wxBLACK);
	}
	CBOINCGridCellRenderer::Draw(grid,attr,dc,rect,row,col,isSelected);
}

/* ############# progress cell renderer ########################## */
CBOINCGridCellProgressRenderer::CBOINCGridCellProgressRenderer(int progressColumn,bool percentAppending/*=true*/) : CBOINCGridCellRenderer()
{
	m_progressColumn = progressColumn;
	m_bDoPercentAppending = percentAppending;
}

void CBOINCGridCellProgressRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	if(m_progressColumn == col) {
		DoProgressDrawing(grid,attr,dc,rect,row,col,isSelected);
	}
	else {
		DoNormalTextDrawing(grid,attr,dc,rect,row,col,isSelected);
	}
}

/* paints the progress bar */
void CBOINCGridCellProgressRenderer::DoProgressDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected) {
	int hAlign, vAlign;
    wxRect rect = rectCell;
    rect.Inflate(-1);
    // erase only this cells background, overflow cells should have been erased
	this->DrawBackground(grid, dc, rectCell, row, isSelected);
	// set text attributes    
	attr.GetAlignment(&hAlign, &vAlign);
    SetTextColoursAndFont(grid, attr, dc, isSelected);
	//calculate the two parts of the progress rect
    //
	double dv = 0.0;
	wxString strValue = grid.GetCellValue(row,col);
	if(m_bDoPercentAppending) {
		strValue = strValue + wxT(" %");
	}
    // Project view uses the format:  %0.0f (%0.2f%%)
    // Everyone else uses: %.3f%%
    if (strValue.Find(wxT("(")) != wxNOT_FOUND) {
        strValue.SubString(strValue.Find(wxT("(")) + 1, strValue.Find(wxT(")")) - 1).ToDouble( &dv );
    } else {
    	strValue.ToDouble ( &dv );	 // NOTE: we should do error-checking/reporting here!!
    }

	wxRect p1(rect);
	wxRect p2(rect);
	int r = (int)((rect.GetRight()-rect.GetLeft())*dv / 100.0);
	p1.SetRight(rect.GetLeft()+r);
	p2.SetLeft(rect.GetLeft()+r+1);
	p2.SetRight(rect.GetRight()-1);
	//start drawing
	dc.SetClippingRegion(rect);
	wxBrush old = dc.GetBrush();
	wxColour progressColour = wxTheColourDatabase->Find(wxT("LIGHT BLUE"));
	wxBrush* progressBrush = wxTheBrushList->FindOrCreateBrush(progressColour);
	wxPen* progressPen = wxThePenList->FindOrCreatePen(progressColour,1,wxSOLID);
	//draw the outline rectangle
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(*progressPen);
	dc.DrawRectangle(rect);
	// Draw the left part
	dc.SetBrush(*progressBrush);
	dc.DrawRectangle(p1);
	//draw the right part
	dc.SetBrush(old);
	dc.DrawRectangle(p2);
	//
	dc.DestroyClippingRegion();
	// draw the text
	grid.DrawTextRectangle(dc, strValue, rect, hAlign, vAlign);
}

/* enables or disable the appending of the percent sign to the progress text */
void CBOINCGridCellProgressRenderer::SetPercentAppending(bool enable/*=true*/) {
	m_bDoPercentAppending = enable;
}

/* ############ GridTable class ############ */
CBOINCGridTable::CBOINCGridTable(int rows, int cols) : wxGridStringTable(rows,cols) {
	//init column sort types with CST_STRING
	for(int i=0; i< cols; i++) {
		m_arrColumnSortTypes.Add(CST_STRING);
	}
	//add a single pseudo column to the docRow array to avoid assertions in initialization phase
	m_arDocRows.Add(-1);
}

CBOINCGridTable::~CBOINCGridTable() {	
}

void CBOINCGridTable::SetColumnSortType(int col,int sortType/*=CST_STRING*/) {
	if(col>=0 && col < ((int)m_arrColumnSortTypes.GetCount())) {
		m_arrColumnSortTypes[col] = sortType;
	}
}

void CBOINCGridTable::SortData2(int col,bool ascending) {
	//valid column index ?
	if(col<0) {
		return;
	}
	//build up an help array with docIndex/value pairs
	ArrayPair arPairs;
	//get the docIndexes and values of the sorting column
	for(int i=0; i < this->GetNumberRows(); i++) {
		pair* p = new pair();
		p->docIndex = m_arDocRows[i];
		p->value = this->GetValue(i,col);
		arPairs.Add(p);
	}
	//sort the sorting column values
	reverseCompareOrder = !ascending;
	//decide, which sort function to call
	switch(m_arrColumnSortTypes[col]) {
		case CST_FLOAT:
			arPairs.Sort(CompareFloatString2);
			break;
		case CST_LONG:
			arPairs.Sort(CompareLongString2);
			break;
		case CST_TIME:
			arPairs.Sort(CompareTimeString2);
			break;
		case CST_DATETIME:
			arPairs.Sort(CompareDateString2);
			break;
		default:
			arPairs.Sort(CompareStringStringNoCase2);
			break;
	}
	//sorting the pairs is done, write back the doc rows index array
	for(unsigned int i=0; i< arPairs.GetCount();i++) {
		m_arDocRows[i] = arPairs[i].docIndex;
	}	
	//free the help array
	arPairs.Clear();
}

void CBOINCGridTable::SortData(int col,bool ascending) {
	//valid column index ?
	if(col<0) {
		return;
	}
	wxArrayString arColValues;	
	//get the values of the sorting column
	for(int i=0; i < this->GetNumberRows(); i++) {
		arColValues.Add(this->GetValue(i,col));
	}
	//sort the sorting column values
	reverseCompareOrder = !ascending;
	//decide, which sort function to call
	switch(m_arrColumnSortTypes[col]) {
		case CST_FLOAT:
			arColValues.Sort(CompareFloatString);
			break;
		case CST_LONG:
			arColValues.Sort(CompareLongString);
			break;
		case CST_TIME:
			arColValues.Sort(CompareTimeString);
			break;
		case CST_DATETIME:
			arColValues.Sort(CompareDateString);
			break;
		default:
			arColValues.Sort(CompareStringStringNoCase);
			break;
	}
	//rebuild the underlaying 2-dimensional string array
	wxGridStringArray newArray;
	for(unsigned int i=0; i< arColValues.GetCount();i++) {
		//find the original row index
		int indexold = FindRowIndexByColValue(col,arColValues[i]);
		wxArrayString rowArray;
		for(int j=0; j < this->GetNumberCols(); j++) {
			rowArray.Add(this->GetValue(indexold,j));
			//delete the value in the original string array
			//to prevent finding the wrong index in case of identical values
			//f.e. the project name is the same for multiple work units or transfers
			this->SetValue(indexold,j,wxEmptyString);
		}
		newArray.Add(rowArray);
	}
	//fill the table data array with the sorted values
	for(unsigned int i=0; i< newArray.GetCount(); i++) {
		for(unsigned int j=0; j < newArray[i].GetCount();j++) {
			this->SetValue(i,j,newArray[i][j]);
		}
	}
}

/* finds the first row index for the cell value value in column col
   !! only use this (outside sorting method) with a column, that has unique values !!
*/
int CBOINCGridTable::FindRowIndexByColValue(int col,wxString& value) {

	for(int i=0; i < this->GetNumberRows(); i++) {
		wxString curr = GetValue(i,col);
		if(this->GetValue(i,col).IsSameAs(value)) {
			return i;
		}
	}
	return -1;
}


bool CBOINCGridTable::AppendRows(size_t numRows/*=1*/) {
	for(unsigned int i=0; i< numRows;i++) {
		m_arDocRows.Add(-1);
	}
	return wxGridStringTable::AppendRows(numRows);	
}

bool CBOINCGridTable::DeleteRows( size_t pos/* = 0*/, size_t numRows/* = 1*/ ) {	
	m_arDocRows.RemoveAt(pos,numRows);	
	return wxGridStringTable::DeleteRows(pos,numRows);
}

int CBOINCGridTable::GetDocRowIndex(int gridRow) {	
	return m_arDocRows[gridRow];
}

void CBOINCGridTable::ResetDocRows() {
	for(unsigned int i=0; i< m_arDocRows.GetCount();i++) {
		m_arDocRows[i]=i;
	}
}
