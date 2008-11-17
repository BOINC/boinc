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

#ifndef _BOINCGRIDCTRL_H_
#define _BOINCGRIDCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCGridCtrl.cpp"
#endif

#define CST_STRING		0
#define CST_TIME	    1
#define CST_LONG		2
#define CST_FLOAT		3
#define CST_DATETIME	4

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
	int column;
	bool m_bDoPercentAppending;
public:
	CBOINCGridCellProgressRenderer(int col,bool percentAppending=true);
	void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);	
	void SetPercentAppending(bool enable=true);
protected:
	void DoProgressDrawing(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected);
};


/* message renderer */
class CBOINCGridCellMessageRenderer : public CBOINCGridCellRenderer {
	int column;
public:
	CBOINCGridCellMessageRenderer(int priocol);
	void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);	
};

/* grid table */
class CBOINCGridTable : public wxGridStringTable {
public:
	CBOINCGridTable(int rows,int cols);
	virtual ~CBOINCGridTable();	
	void SortData(int col,bool ascending);
	int FindRowIndexByColValue(int col1,wxString& value1,int col2,wxString& value2);
	void SetColumnSortType(int col,int sortType=CST_STRING);
private:
	wxArrayInt arrColumnSortTypes;
};

/* grid ctrl */
class CBOINCGridCtrl :	public wxGrid
{
public:
	CBOINCGridCtrl(wxWindow* parent, wxWindowID iGridWindowID);
	~CBOINCGridCtrl();
	int GetFirstSelectedRow();
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
	void SetColumnSortType(int col,int sortType=CST_STRING);
	wxArrayInt GetSelectedRows2();
	CBOINCGridTable* GetTable();
	//methods to handle selection and grid cursor positions correct with sorting 
	void SetPrimaryKeyColumns(int col1,int col2);
	void SaveSelection();
	void RestoreSelection();
	void SaveGridCursorPosition();
	void RestoreGridCursorPosition();
	void ClearSavedSelection();
	void Setup();
	int sortColumn;
	bool sortAscending;
	bool sortNeededByLabelClick;
protected:
	DECLARE_EVENT_TABLE()
private:
	wxString FormatTextWithEllipses(wxDC& dc,const wxString &text,int width);
	
	
	int ccollast,crowlast;
	wxBitmap ascBitmap;
	wxBitmap descBitmap;
	int m_pkColumnIndex1; //col index to use as a primary key
        int m_pkColumnIndex2; //col index to use as secondary key
	wxArrayString m_arrSelectedKeys1;//array for remembering the current selected rows by primary key column value
	wxArrayString m_arrSelectedKeys2;//array for remembering the current selected rows by secondary key column value
	int m_cursorcol; //saved grid cursor column index
	int m_cursorrow; //saved grid cursor row index
	wxString m_szCursorKey1;//primary key value for grid cursor cell	
	wxString m_szCursorKey2;//secondary key value for grid cursor cell	
};

#endif //_BOINCGRIDCTRL_H_
