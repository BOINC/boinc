// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

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
