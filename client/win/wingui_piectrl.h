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

#ifndef __WIN_PIECTRL_H_
#define __WIN_PIECTRL_H_

// includes

#include "wingui.h"

// constants

#define PIE_MAJOR_MAX		0.25		// max size of the screen of the pie's major axis
#define PIE_MINOR_MAX		0.25		// max size of the screen of the pie's minor axis
#define PIE_BUFFER			20			// buffer pixels around edge of pie chart
#define PIE_DEPTH			0.25		// depth of pie chart
#define PI					3.14159		// pi

//////////
// class:		CPieChartCtrl
// parent:		CWnd
// description:	contains the functionality of a pie chart
class CPieChartCtrl : public CWnd
{
public:
							CPieChartCtrl();
	void					AddPiece(LPTSTR, COLORREF, double);
	void					RemovePiece(int);
	void					SetPiece(int, double);
	void					SetPieceLabel(int, LPTSTR); 
	BOOL					Create(DWORD, const RECT&, CWnd*, UINT);
	void					SetFont(CFont*);
	void					SetTotal(double);
	int						GetItemCount();

protected:
	double					m_xTotal;				// total amount of pie
	CArray<double,double>	m_xValues;				// specific values of pieces
	CArray<COLORREF,COLORREF>	m_colors;			// colors of pieces
	CArray<CString,CString>		m_strLabels;		// labels of pieces
	CArray<DWORD,DWORD>		m_dwData;				// data for pieces
	CFont*					m_pFont;				// font for control


	void					DrawPiePiece(CDC*, double, double);
	void					CirclePoint(CPoint*, int, double, CPoint*);
	void					EllipsePoint(CRect*, double, CPoint*);

	afx_msg void			OnPaint();
    DECLARE_MESSAGE_MAP()
};

#endif
