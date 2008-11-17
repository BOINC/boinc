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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCGridCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCGridCtrl.h"
#include "BOINCBaseView.h"
#include "res/sortascending.xpm"
#include "res/sortdescending.xpm"

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
	sortColumn=-1;
	sortAscending=true;
	sortNeededByLabelClick=false;
	m_pkColumnIndex1=-1;
	m_pkColumnIndex2=-1;
	m_cursorcol=-1;
	m_cursorrow=-1;
	m_arrSelectedKeys1.Clear();
	m_arrSelectedKeys2.Clear();
	
	//load sorting bitmaps
	ascBitmap = wxBitmap(sortascending_xpm);
	descBitmap = wxBitmap(sortdescending_xpm);
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

/* sets the column with unique values */
void CBOINCGridCtrl::SetPrimaryKeyColumns(int col1,int col2) {
	m_pkColumnIndex1 = col1;
	m_pkColumnIndex2 = col2;
}

/* If the user clicked inside the grid, the former selected cells must be deselected.
   Because the code in OnListRender() of the GridViews raises SelectionEvents too,
   call to GetBatchCount() here is necessary to decide what to do
 */
void CBOINCGridCtrl::ClearSavedSelection() {
	if(this->GetBatchCount()<=0) {
		m_arrSelectedKeys1.Empty();
		m_arrSelectedKeys2.Empty();
	}
}

/* save the key values of the currently selected rows for later restore */
void CBOINCGridCtrl::SaveSelection() {
	if(m_pkColumnIndex1>=0) {
		wxArrayInt arrSelRows = GetSelectedRows2();	
		m_arrSelectedKeys1.Empty();
		m_arrSelectedKeys2.Empty();
		for(unsigned int i=0; i< arrSelRows.GetCount();i++) {
			m_arrSelectedKeys1.Add(GetCellValue(arrSelRows[i],m_pkColumnIndex1));
			if (m_pkColumnIndex2 >= 0) {
                            m_arrSelectedKeys2.Add(GetCellValue(arrSelRows[i],m_pkColumnIndex2));
                        } else {
                           m_arrSelectedKeys2.Add(wxEmptyString);
                        }
		}
	}
}

/* select all rows, that were formerly selected
   this raises selection events without user interaction */
void CBOINCGridCtrl::RestoreSelection() {
    BeginBatch();       // To prevent flicker
	ClearSelection();
	for(unsigned int i=0;i < m_arrSelectedKeys1.size();i++) {
		int index = GetTable()->FindRowIndexByColValue(
                    m_pkColumnIndex1,m_arrSelectedKeys1[i],m_pkColumnIndex2,m_arrSelectedKeys2[i]
                );
		if(index >=0) {
			SelectRow(index,true);
		}
	}
    EndBatch();
}

void CBOINCGridCtrl::SaveGridCursorPosition() {
	m_cursorcol = GetGridCursorCol();
	m_cursorrow = GetGridCursorRow();	
	if(m_cursorrow>=0 && m_cursorcol >=0) {
		m_szCursorKey1 = GetCellValue(m_cursorrow,m_pkColumnIndex1);
                if (m_pkColumnIndex2 >= 0) {
                    m_szCursorKey2 = GetCellValue(m_cursorrow,m_pkColumnIndex2);
                }
	}
}

void CBOINCGridCtrl::RestoreGridCursorPosition() {
	int index = GetTable()->FindRowIndexByColValue(m_pkColumnIndex1,m_szCursorKey1,m_pkColumnIndex2,m_szCursorKey2);
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
	pConfig->Write(wxT("SortColumn"),this->sortColumn);
	pConfig->Write(wxT("SortAscending"),this->sortAscending);
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
		sortColumn = iTempValue;
	}
	pConfig->Read(wxT("SortAscending"),&iTempValue,-1);
	if(-1 != iTempValue) {
		sortAscending = iTempValue != 0 ? true : false;
	}

    return true;
}

/* for convienience purpose only */
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

/* draws text with ellipses instead cutting it */
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
				//changes applies here
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

        // now draw the text
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
	if(col == sortColumn) {
		int x = rect.GetRight() - ascBitmap.GetWidth() - 2;
		int y = rect.GetY();
		dc.DrawBitmap(this->sortAscending ? descBitmap : ascBitmap,x,y,true);
	}
}

/* handles left mouse click on column header */
void CBOINCGridCtrl::OnLabelLClick(wxGridEvent& ev) {
	if(ev.GetCol() != -1) {
        //same column as last time, then change only sort direction
        if(sortColumn == ev.GetCol()) {
			sortAscending = ! sortAscending;
		} else {
            int tmpOldColumn = sortColumn;

            sortColumn = ev.GetCol();
			sortAscending = true;

            // Force a repaint of the label
			if ( -1 != tmpOldColumn ) {
				SetColLabelValue(tmpOldColumn, GetColLabelValue(tmpOldColumn));
			}
		}

        // Force a repaint of the label
        SetColLabelValue(ev.GetCol(), GetColLabelValue(ev.GetCol()));
		//
		sortNeededByLabelClick=true;
		// Update and sort data
		wxTimerEvent tEvent;
		wxDynamicCast(GetParent(),CBOINCBaseView)->FireOnListRender(tEvent);
	}
        // The base class calls ClearSelection(), so do NOT call ev.Skip();
}

void CBOINCGridCtrl::SortData() {
	GetTable()->SortData(sortColumn,sortAscending);
	sortNeededByLabelClick=false;
}

void CBOINCGridCtrl::SetColumnSortType(int col,int sortType/*=CST_STRING*/) {
	GetTable()->SetColumnSortType(col,sortType);
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
			//alternate background colours
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

void CBOINCGridCellRenderer::DoNormalTextDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected) {
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
	this->DrawBackground(grid, dc, rectCell, row, isSelected);

    int hAlign, vAlign;
    attr.GetAlignment(&hAlign, &vAlign);

    int overflowCols = 0;

    if (attr.GetOverflow())
    {
        int cols = grid.GetNumberCols();
        int best_width = GetBestSize(grid,attr,dc,row,col).GetWidth();
        int cell_rows, cell_cols;
        attr.GetSize( &cell_rows, &cell_cols ); // shouldn't get here if <=0
        if ((best_width > rectCell.width) && (col < cols) && grid.GetTable())
        {
            int i, c_cols, c_rows;
            for (i = col+cell_cols; i < cols; i++)
            {
                bool is_empty = true;
                for (int j=row; j<row+cell_rows; j++)
                {
                    // check w/ anchor cell for multicell block
                    grid.GetCellSize(j, i, &c_rows, &c_cols);
                    if (c_rows > 0) c_rows = 0;
                    if (!grid.GetTable()->IsEmptyCell(j+c_rows, i))
                    {
                        is_empty = false;
                        break;
                    }
                }
                if (is_empty)
                    rect.width += grid.GetColSize(i);
                else
                {
                    i--;
                    break;
                }
                if (rect.width >= best_width) break;
            }
            overflowCols = i - col - cell_cols + 1;
            if (overflowCols >= cols) overflowCols = cols - 1;
        }

        if (overflowCols > 0) // redraw overflow cells w/ proper hilight
        {
            hAlign = wxALIGN_LEFT; // if oveflowed then it's left aligned
            wxRect clip = rect;
            clip.x += rectCell.width;
            // draw each overflow cell individually
            int col_end = col+cell_cols+overflowCols;
            if (col_end >= grid.GetNumberCols())
                col_end = grid.GetNumberCols() - 1;
            for (int i = col+cell_cols; i <= col_end; i++)
            {
                clip.width = grid.GetColSize(i) - 1;
                dc.DestroyClippingRegion();
                dc.SetClippingRegion(clip);

                SetTextColoursAndFont(grid, attr, dc,
                        grid.IsInSelection(row,i));

                grid.DrawTextRectangle(dc, grid.GetCellValue(row, col),
                        rect, hAlign, vAlign);
                clip.x += grid.GetColSize(i) - 1;
            }

            rect = rectCell;
            rect.Inflate(-1);
            rect.width++;
            dc.DestroyClippingRegion();
        }
    }

    // now we only have to draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);

	//get a real grid class pointer
    CBOINCGridCtrl* bgrid = wxDynamicCast(&grid, CBOINCGridCtrl);

	//use the overloaded method here
	bgrid->DrawTextRectangle(dc, grid.GetCellValue(row, col),
                           rect, hAlign, vAlign);

}

/* ############# message cell renderer ######################## */

/* Constructor
   prio argument - the column index, that holds prio values
*/
CBOINCGridCellMessageRenderer::CBOINCGridCellMessageRenderer(int priocol){
	column = priocol;
}

void CBOINCGridCellMessageRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	wxString szError(wxT("Error"));
	if(grid.GetCellValue(row,column).Trim(false).IsSameAs(szError)) {
		attr.SetTextColour(*wxRED);
	}
	else {
		attr.SetTextColour(*wxBLACK);
	}
	CBOINCGridCellRenderer::Draw(grid,attr,dc,rect,row,col,isSelected);
}

/* ############# progress cell renderer ########################## */
CBOINCGridCellProgressRenderer::CBOINCGridCellProgressRenderer(int col,bool percentAppending/*=true*/) : CBOINCGridCellRenderer()
{
	column = col;
	m_bDoPercentAppending = percentAppending;
}

void CBOINCGridCellProgressRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	if(col==column) {
		DoProgressDrawing(grid,attr,dc,rect,row,col,isSelected);
	}
	else {
		DoNormalTextDrawing(grid,attr,dc,rect,row,col,isSelected);
	}
}

/* paints the progress bar */
void CBOINCGridCellProgressRenderer::DoProgressDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected) {
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
	this->DrawBackground(grid, dc, rectCell, row, isSelected);
	// set text attributes
    int hAlign, vAlign;
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

/* enables or disbale the appendign of the percent sign to the progress text */
void CBOINCGridCellProgressRenderer::SetPercentAppending(bool enable/*=true*/) {
	m_bDoPercentAppending = enable;
}


/* ############ GridTable class ############ */
CBOINCGridTable::CBOINCGridTable(int rows, int cols) : wxGridStringTable(rows,cols) {
	//init column sort types with CST_STRING
	for(int i=0; i< cols; i++) {
		arrColumnSortTypes.Add(CST_STRING);
	}
}

CBOINCGridTable::~CBOINCGridTable() {
}


void CBOINCGridTable::SetColumnSortType(int col,int sortType/*=CST_STRING*/) {
	if(col>=0 && col < ((int)arrColumnSortTypes.GetCount())) {
		arrColumnSortTypes[col] = sortType;
	}
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
	switch(arrColumnSortTypes[col]) {
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
		int indexold = FindRowIndexByColValue(col,arColValues[i],-1,arColValues[i]);
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

/* finds the first row index for the cell with value value1 in column col1 and value2 in col2
   ignores col2 and value2 if col2 < 0
   !! only use this (outside sorting method) with a pair of columns, that have unique value pairs !!
*/
int CBOINCGridTable::FindRowIndexByColValue(int col1,wxString& value1,int col2,wxString& value2) {

	for(int i=0; i < this->GetNumberRows(); i++) {
            if(! this->GetValue(i,col1).IsSameAs(value1)) continue;
            if (col2 < 0) return i;
            if(this->GetValue(i,col2).IsSameAs(value2)) return i;
	}
	return -1;
}

/* for convinience purposes only */
CBOINCGridTable* CBOINCGridCtrl::GetTable() {
	return wxDynamicCast(wxGrid::GetTable(), CBOINCGridTable);
}
