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

// define column values data types
#define CST_STRING		0
#define CST_TIME	    1
#define CST_LONG		2
#define CST_FLOAT		3
#define CST_DATETIME	4
// new (currently unused)
#define CST_DOUBLE		5
#define CST_INT			6

// this class represent a pair of String/Int 
class pair : public wxObject {
public:
	int docIndex;//the document array index
	wxString value;//the string value
};

// variant pair
class pairVariant : public wxObject {
public:
	int docIndex;
	wxVariant varValue;
};

WX_DECLARE_OBJARRAY(pair, ArrayPair);
WX_DECLARE_OBJARRAY(pairVariant, ArrayPairVariant);

/* generic boinc grid cell renderer */
class CBOINCGridCellRenderer : public wxGridCellStringRenderer {	
public:
	CBOINCGridCellRenderer();
	void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);
protected:
	void DoNormalTextDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected);
	void DrawBackground(wxGrid& grid,wxDC& dc,const wxRect& rect,int row,bool isSelected);
};

/* progress renderer */
class CBOINCGridCellProgressRenderer : public CBOINCGridCellRenderer {
	int m_progressColumn;
	bool m_bDoPercentAppending;
public:
	CBOINCGridCellProgressRenderer(int progressColumn,bool percentAppending=true);
	void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);	
	void SetPercentAppending(bool enable=true);
protected:
	void DoProgressDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected);
};


/* message renderer */
class CBOINCGridCellMessageRenderer : public CBOINCGridCellRenderer {
	int m_prioColumn;
public:
	CBOINCGridCellMessageRenderer(int priocol);
	void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);	
};

/* grid table */
class CBOINCGridTable : public wxGridStringTable {
public:
	CBOINCGridTable(int rows,int cols);
	virtual ~CBOINCGridTable();	
	//sorting methods
	void SortData(int col,bool ascending);//old style
	void SortData2(int col,bool ascending);//new style for virtual grids
	void SetColumnSortType(int col,int sortType=CST_STRING);
	//
	int FindRowIndexByColValue(int col,wxString& value);
	
	//overloaded to synchronize m_arDocRows
	bool AppendRows(size_t numRows=1);
	bool DeleteRows( size_t pos = 0, size_t numRows = 1 );
	//helper methods for docRow/GridRow associations
	int GetDocRowIndex(int gridRow);
	void ResetDocRows();

protected:
	wxArrayInt m_arrColumnSortTypes;
	wxArrayInt m_arDocRows;
};

/* grid ctrl */
class CBOINCGridCtrl :	public wxGrid
{
public:
	CBOINCGridCtrl(wxWindow* parent, wxWindowID iGridWindowID);
	~CBOINCGridCtrl();

	int GetFirstSelectedRow();
	wxArrayInt GetSelectedRows2();

	bool OnSaveState(wxConfigBase* pConfig);
	bool OnRestoreState(wxConfigBase* pConfig);

	void SetColAlignment(int col,int hAlign,int vAlign);
    void DrawTextRectangle( wxDC& dc, const wxArrayString& lines, const wxRect&,
                            int horizontalAlignment = wxALIGN_LEFT,
                            int verticalAlignment = wxALIGN_TOP,
                            int textOrientation = wxHORIZONTAL );
    void DrawTextRectangle( wxDC& dc, const wxString&, const wxRect&,
                            int horizontalAlignment = wxALIGN_LEFT,
                            int verticalAlignment = wxALIGN_TOP,
	                        int textOrientation = wxHORIZONTAL );
	virtual void DrawColLabel( wxDC& dc, int col );
	void OnLabelLClick(wxGridEvent& ev);

	void SortData();
	void SortData2();
	void SetColumnSortType(int col,int sortType=CST_STRING);

	
	CBOINCGridTable* GetTable();
	//methods to handle selection and grid cursor positions correct with sorting 
	void SetPrimaryKeyColumn(int col);
	void SaveSelection();
	void RestoreSelection();
	void SaveGridCursorPosition();
	void RestoreGridCursorPosition();
	void ClearSavedSelection();
	
	void Setup();

	void RefreshColumn(int col);	

	bool m_sortNeededByLabelClick;
protected:
	DECLARE_EVENT_TABLE()
	wxString FormatTextWithEllipses(wxDC& dc,const wxString &text,int width);
private:	
	int m_sortColumn;
	bool m_sortAscending;	
	wxBitmap m_ascBitmap;
	wxBitmap m_descBitmap;
	int m_pkColumnIndex;//col index act as a primary key
	wxArrayString m_arrSelectedKeys;//array for remembering the current selected rows by primary key column value
	int m_cursorcol; //saved grid cursor column index
	int m_cursorrow; //saved grid cursor row index
	wxString m_szCursorKey;//key value for grid cursor cell	
	
};

#endif //_BOINCGRIDCTRL_H_
