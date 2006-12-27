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

#pragma warning( disable : 4100)
bool CBOINCGridCtrl::isReadOnly(int row,int col) const {
	return true;
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

/* ############# */
CBOINCGridCellProgressRenderer::CBOINCGridCellProgressRenderer(int col) : wxGridCellStringRenderer()
{
	column = col;
}

void CBOINCGridCellProgressRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	if(col==column) {
		DoDrawing(grid,attr,dc,rect,row,col,isSelected);
	}
	else {
		wxGridCellStringRenderer::Draw(grid,attr,dc,rect,row,col,isSelected);
	}
}

void CBOINCGridCellProgressRenderer::DoDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected) {
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    int hAlign, vAlign;
    attr.GetAlignment(&hAlign, &vAlign);

    // now we only have to draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);

	wxString value = grid.GetCellValue(row,col);
	wxString strValue = value + " %"; 
	double dv = atof(value);
	wxRect p1(rect);
	wxRect p2(rect);
	int r = (int)((rect.GetRight()-rect.GetLeft())*dv / 100.0);
	p1.SetRight(rect.GetLeft()+r);
	p2.SetLeft(rect.GetLeft()+r+1);
	dc.SetClippingRegion(rect);
	wxBrush old = dc.GetBrush();
	dc.SetBrush(*wxBLUE_BRUSH);
	dc.DrawRectangle(p1);
	dc.SetBrush(old);
	dc.DrawRectangle(p2);	
	dc.DestroyClippingRegion();
	grid.DrawTextRectangle(dc, strValue, rect, hAlign, vAlign);
}
