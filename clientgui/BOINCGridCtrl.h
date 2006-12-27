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

#ifndef _BOINCGRIDCTRL_H_
#define _BOINCGRIDCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCGridCtrl.cpp"
#endif

/* default progress renderer */
class CBOINCGridCellProgressRenderer : public wxGridCellStringRenderer {
	int column;
public:
	CBOINCGridCellProgressRenderer(int col);
	void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);	
protected:
	void DoDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected);
};

/* grid ctrl */
class CBOINCGridCtrl :	public wxGrid
{
public:
	CBOINCGridCtrl(wxWindow* parent, wxWindowID iGridWindowID);
public:
	~CBOINCGridCtrl();
	bool isReadOnly(int row, int col) const;
	int GetFirstSelectedRow();
};



#endif //_BOINCGRIDCTRL_H_
