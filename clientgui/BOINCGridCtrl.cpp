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


CBOINCGridCtrl::CBOINCGridCtrl(
    wxWindow* parent, wxWindowID iGridWindowID
) : wxGrid(
    parent, iGridWindowID) {
}


CBOINCGridCtrl::~CBOINCGridCtrl() {
}

int CBOINCGridCtrl::GetFirstSelectedRow() {
	int ret= -1;
	//Row mode ?
	wxArrayInt selRows = this->GetSelectedRows();
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
        pConfig->SetPath(strBaseConfigLocation + label);
		pConfig->Write(wxT("Width"), this->GetColumnWidth(iIndex));
    }
    return true;
}


bool CBOINCGridCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxInt32     iIndex = 0;
    wxInt32     iTempValue = 0;
	wxInt32		iColumnCount = this->GetCols();

    wxASSERT(pConfig);

    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex < iColumnCount; iIndex++) {
		wxString label = this->GetColLabelValue(iIndex);
        pConfig->SetPath(strBaseConfigLocation + label);

        pConfig->Read(wxT("Width"), &iTempValue, -1);
        if (-1 != iTempValue) {
			this->SetColumnWidth(iIndex,iTempValue);
        }
    }

    return true;
}

void CBOINCGridCtrl::SetColAlignment(int col,int hAlign,int vAlign) {
    wxGridCellAttr *attr = m_table->GetAttr(-1, col, wxGridCellAttr::Col );
	if(!attr) {
		attr = new wxGridCellAttr;
	}
	attr->SetAlignment(hAlign,vAlign);
    SetColAttr(col, attr);
}

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

void CBOINCGridCtrl::DrawTextRectangle( wxDC& dc,
                               const wxArrayString& lines,
                               const wxRect& rect,
                               int horizAlign,
                               int vertAlign,
                               int textOrientation )
{
    long textWidth, textHeight;
    long lineWidth, lineHeight;
    int nLines;

    dc.SetClippingRegion( rect );

    nLines = lines.GetCount();
    if( nLines > 0 )
    {
        int l;
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
}

/* ############# */
CBOINCGridCellProgressRenderer::CBOINCGridCellProgressRenderer(int col) : wxGridCellStringRenderer()
{
	column = col;
}

void CBOINCGridCellProgressRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	if(col==column) {
		DoProgressDrawing(grid,attr,dc,rect,row,col,isSelected);
	}
	else {
		DoNormalTextDrawing(grid,attr,dc,rect,row,col,isSelected);
	}
}

void CBOINCGridCellProgressRenderer::DrawBackground(wxGrid& grid,
                              wxGridCellAttr& attr,
                              wxDC& dc,
                              const wxRect& rect,
                              int row, int col,
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

void CBOINCGridCellProgressRenderer::DoNormalTextDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected) {
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
	this->DrawBackground(grid, attr, dc, rectCell, row, col, isSelected);

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
    CBOINCGridCtrl* bgrid = dynamic_cast<CBOINCGridCtrl*> (&grid);
	//use the overloaded method here
	bgrid->DrawTextRectangle(dc, grid.GetCellValue(row, col),
                           rect, hAlign, vAlign);

}


void CBOINCGridCellProgressRenderer::DoProgressDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected) {
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
	this->DrawBackground(grid, attr, dc, rectCell, row, col, isSelected);
	// set text attributes
    int hAlign, vAlign;
	attr.GetAlignment(&hAlign, &vAlign);
    SetTextColoursAndFont(grid, attr, dc, isSelected);

	//calculate the two parts of the progress rect
	wxString value = grid.GetCellValue(row,col);
	wxString strValue = value + wxString (" %", wxConvUTF8 );
	double dv;
	value.ToDouble ( &dv );	 // NOTE: we should do error-checking/reporting here!!
	wxRect p1(rect);
	wxRect p2(rect);
	int r = (int)((rect.GetRight()-rect.GetLeft())*dv / 100.0);
	p1.SetRight(rect.GetLeft()+r);
	p2.SetLeft(rect.GetLeft()+r+1);
	p2.SetRight(rect.GetRight()-1);
	//start drawing
	dc.SetClippingRegion(rect);
	wxBrush old = dc.GetBrush();
	wxColour progressColour = wxTheColourDatabase->Find(wxString("LIGHT BLUE",wxConvUTF8));
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

